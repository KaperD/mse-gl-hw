#include "FractalWindow.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QScreen>
#include <QtMath>

#include <array>

void FractalWindow::init()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/diffuse.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
									  ":/Shaders/diffuse.fs");
	program_->link();

	// Bind attributes
	program_->bind();

	std::string path = "../../../stylized_modular_fireplace/scene.gltf";
	model_ = std::unique_ptr<Model>(new Model(path, *program_, this));

	// Release all
	program_->release();

	vao_.release();

	ibo_.release();
	vbo_.release();

	// Uncomment to enable depth test and face cullin
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE);

	// Clear all FBO buffers
	float col = 185.0f / 255.0f;
	glClearColor(col, col, col, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FractalWindow::render()
{
	// Configure viewport
	const auto retinaScale = devicePixelRatio();
	glViewport(0, 0, static_cast<GLint>(width() * retinaScale),
			   static_cast<GLint>(height() * retinaScale));

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	doMovement();

	// Calculate MVP matrix
	QMatrix4x4 model;

	QMatrix4x4 view;

	view.lookAt(
		cameraPos_,
		cameraPos_ + cameraDirection_,
		{0, 1, 0});

	QMatrix4x4 projection;
	projection.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	// Bind VAO and shader program
	program_->bind();

	// Update uniform value

	// Draw
	program_->setUniformValue(program_->uniformLocation("lightPos"), lightPos_);
	program_->setUniformValue(program_->uniformLocation("viewPos"), cameraPos_);
	program_->setUniformValue(program_->uniformLocation("blinn"), true);
	program_->setUniformValue(program_->uniformLocation("lightType"), static_cast<int>(light_type_));
	program_->setUniformValue(program_->uniformLocation("time"), static_cast<float>(animation_time_) / 3000.0f);

	model_->Draw(model, view, projection);

	// Release VAO and shader program
	program_->release();

	// Increment frame counter
	++frame_;
	long long fps = 1000 / time_.elapsed();
	fpsLabel_->setText("FPS: " + QString::number(fps));
	time_.start();
}

void FractalWindow::doMovement() {
	float cameraSpeed = 0.005f * static_cast<float>(time_.elapsed());
	if(keys[Qt::Key_W])
	{
		cameraPos_ += cameraSpeed * cameraDirection_;
	}
	if(keys[Qt::Key_S])
	{
		cameraPos_ -= cameraSpeed * cameraDirection_;
	}
	if(keys[Qt::Key_A])
	{
		cameraPos_ -= (QVector3D::crossProduct(cameraDirection_, cameraUp_)).normalized() * cameraSpeed;
	}
	if(keys[Qt::Key_D])
	{
		cameraPos_ += (QVector3D::crossProduct(cameraDirection_, cameraUp_)).normalized() * cameraSpeed;
	}
	if(keys[Qt::Key_Space]) {
		cameraPos_ += cameraUp_ * cameraSpeed;
	}
	if(keys[Qt::Key_Shift])
	{
		cameraPos_ -= cameraUp_ * cameraSpeed;
	}
}

void FractalWindow::mousePressEvent(QMouseEvent * e)
{
	lastX = e->x();
	lastY = e->y();
	isPressed_ = true;
}

void FractalWindow::mouseReleaseEvent(QMouseEvent * e)
{
	static_cast<void>(e);
	isPressed_ = false;
}
void FractalWindow::keyPressEvent(QKeyEvent * event)
{
	keys[event->key()] = true;
}
void FractalWindow::mouseMoveEvent(QMouseEvent * event)
{
	if (!isPressed_) {
		return;
	}

	auto xoffset = static_cast<float >(event->x() - lastX);
	auto yoffset = static_cast<float>(lastY - event->y());
	lastX = event->x();
	lastY = event->y();

	GLfloat sensitivity = 0.1;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;

	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	QVector3D front{
		cos(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)),
		sin(qDegreesToRadians(pitch)),
		sin(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch))
	};
	cameraDirection_ = front.normalized();
}
void FractalWindow::keyReleaseEvent(QKeyEvent * event)
{
	keys[event->key()] = false;
}
void FractalWindow::setAnimationTime(int time)
{
	animation_time_ = time;
}
void FractalWindow::setLightType(LightType type)
{
	light_type_ = type;
}
void FractalWindow::moveLightToCurrentPosition()
{
	lightPos_ = cameraPos_;
}
