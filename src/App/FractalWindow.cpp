#include "FractalWindow.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QScreen>
#include <QtMath>

#include <random>
#include <algorithm>

void FractalWindow::init()
{
	const auto retinaScale = devicePixelRatio();
	const auto screen_width = static_cast<int>(width() * retinaScale);
	const auto screen_height = static_cast<int>(height() * retinaScale);

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

	glGenFramebuffers(1, &program_fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, program_fbo_);

	glGenTextures(1, &depth_tex_);
	glBindTexture(GL_TEXTURE_2D, depth_tex_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex_, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> dist(0.0,1.0);
	for (std::size_t i = 0; i < ssao_kernel_.size(); ++i) {
		float scale = (float)i / (float)(ssao_kernel_.size());
		auto& vec = ssao_kernel_[i];
		vec.setX(dist(rng) * 2.0f - 1.0f);
		vec.setY(dist(rng) * 2.0f - 1.0f);
		vec.setZ(dist(rng));
		vec.normalize();
		vec *= (0.1f + 0.9f * scale * scale);
		vec *= dist(rng);
		vec.setZ(vec.z() + 0.03f);
	}
	std::shuffle(ssao_kernel_.begin(), ssao_kernel_.end(), rng);

	click_program_ = std::make_unique<QOpenGLShaderProgram>(this);
	click_program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/click.vs");
	click_program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
									  ":/Shaders/click.fs");
	click_program_->link();

	click_fbo_ = std::make_unique<QOpenGLFramebufferObject>(static_cast<int>(1 * retinaScale), static_cast<int>(1 * retinaScale), QOpenGLFramebufferObject::Attachment::Depth);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void FractalWindow::render()
{
	float col = 185.0f / 255.0f;
	glClearColor(col, col, col, 1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	doMovement();
	// Configure viewport
	const auto retinaScale = devicePixelRatio();
	const auto screen_width = static_cast<GLint>(width() * retinaScale);
	const auto screen_height = static_cast<GLint>(height() * retinaScale);
	glViewport(0, 0, screen_width, screen_height);
	glBindFramebuffer(GL_FRAMEBUFFER, program_fbo_);
	drawModel(*click_program_);
	QOpenGLFramebufferObject::bindDefault();
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
	glClearColor(0.0, 0.0, 0.0, 0.0);
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
	projection.perspective(60.0f, 4.0f / 3.0f, 0.1f, 10.0f);

	projection = zoom_ * projection;

	// Bind VAO and shader program
	program.bind();

	// Update uniform value

	// Draw
	const auto retinaScale = devicePixelRatio();
	const auto screen_width = static_cast<float>(width() * retinaScale);
	const auto screen_height = static_cast<float>(height() * retinaScale);
	program.setUniformValue(program.uniformLocation("screenSize"), QVector2D{screen_width, screen_height});
	program.setUniformValue(program.uniformLocation("lightPos"), lightPos_);
	program.setUniformValue(program.uniformLocation("viewPos"), cameraPos_);
	program.setUniformValue(program.uniformLocation("blinn"), true);
	program.setUniformValue(program.uniformLocation("useAO"), use_ao_);
	program.setUniformValue(program.uniformLocation("useOnlyAO"), use_only_ao_);
	program.setUniformValue(program.uniformLocation("time"), static_cast<float>(animation_time_) / 3000.0f);
	program.setUniformValue(program.uniformLocation("gAspectRatio"), 4.0f / 3.0f);
	program.setUniformValue(program.uniformLocation("gTanHalfFOV"), static_cast<float>(qTan(qDegreesToRadians(60.0f / 2.0f))));
	program.setUniformValue(program.uniformLocation("gSampleRad"), 0.1f);
	program.setUniformValue(program.uniformLocation("gProj"), projection);
	program.setUniformValue(program.uniformLocation("gView"), view);
	program.setUniformValue(program.uniformLocation("gKernelSize"), kernel_size_);
	program.setUniformValueArray(program.uniformLocation("gKernel"), ssao_kernel_.data(), 64);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, depth_tex_);
	program.setUniformValue(program.uniformLocation("gDepthMap"), 6);

	model_->Draw(model, view, projection, program);

	// Release VAO and shader program
	program.release();
}
void FractalWindow::setUseAO(bool use)
{
	use_ao_ = use;
}
void FractalWindow::setUseOnlyAO(bool use)
{
	use_only_ao_ = use;
}
void FractalWindow::setKernelSize(int size)
{
	kernel_size_ = size;
}
