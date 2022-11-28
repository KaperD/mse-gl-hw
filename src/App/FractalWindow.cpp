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

	click_program_ = std::make_unique<QOpenGLShaderProgram>(this);
	click_program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/click.vs");
	click_program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
									  ":/Shaders/click.fs");
	click_program_->link();

	const auto retinaScale = devicePixelRatio();
	click_fbo_ = std::make_unique<QOpenGLFramebufferObject>(static_cast<int>(1 * retinaScale), static_cast<int>(1 * retinaScale), QOpenGLFramebufferObject::Attachment::Depth);

	// Clear all FBO buffers
	float col = 185.0f / 255.0f;
	glClearColor(col, col, col, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void FractalWindow::render()
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	doMovement();
	QOpenGLFramebufferObject::bindDefault();
	// Configure viewport
	const auto retinaScale = devicePixelRatio();
	const auto screen_width = static_cast<GLint>(width() * retinaScale);
	const auto screen_height = static_cast<GLint>(height() * retinaScale);
	glViewport(0, 0, screen_width, screen_height);
	drawModel(*program_);

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
void FractalWindow::moveLightToCurrentPosition()
{
	lightPos_ = cameraPos_;
}
void FractalWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	const auto retinaScale = devicePixelRatio();
	const auto screen_width = static_cast<int>(width() * retinaScale);
	const auto screen_height = static_cast<int>(height() * retinaScale);

	const auto width = static_cast<float>(1 * retinaScale);
	const auto height = static_cast<float>(1 * retinaScale);
	const auto x = static_cast<int>(event->x() * retinaScale);
	const auto y = screen_height - static_cast<int>(event->y() * retinaScale);
	float sx, sy;
	float tx, ty;
	sx = static_cast<float>(screen_width) / width;
	sy = static_cast<float>(screen_height) / height;
	tx = static_cast<float>(screen_width - 2 * x) / width;
	ty = static_cast<float>(screen_height - 2 * y) / height;
	auto old_zoom = zoom_;
	zoom_ =
		{
			sx, 0, 0, tx,
			0, sy, 0, ty,
			0, 0,  1, 0,
			0, 0,  0, 1
		};

	click_fbo_->bind();
	glViewport(0, 0, static_cast<int>(1 * retinaScale), static_cast<int>(1 * retinaScale));
	drawModel(*click_program_);

	zoom_ = old_zoom;

	int id_value;
	glReadPixels(0, 0, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &id_value);
	if (id_value != 0) {
		model_->ClickOnMesh(id_value - 1);
	}
	click_fbo_->release();
}
void FractalWindow::drawModel(QOpenGLShaderProgram& program)
{
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Calculate MVP matrix
	QMatrix4x4 model;

	QMatrix4x4 view;

	view.lookAt(
		cameraPos_,
		cameraPos_ + cameraDirection_,
		{0, 1, 0});

	QMatrix4x4 projection;
	projection.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	projection = zoom_ * projection;

	// Bind VAO and shader program
	program.bind();

	// Update uniform value

	// Draw
	program.setUniformValue(program.uniformLocation("lightPos"), lightPos_);
	program.setUniformValue(program.uniformLocation("viewPos"), cameraPos_);
	program.setUniformValue(program.uniformLocation("blinn"), true);
	program.setUniformValue(program.uniformLocation("time"), static_cast<float>(animation_time_) / 3000.0f);
	model_->Draw(model, view, projection, program);

	// Release VAO and shader program
	program.release();
}
