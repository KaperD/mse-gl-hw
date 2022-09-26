#include "FractalWindow.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QScreen>

#include <array>

namespace
{

constexpr std::array<GLfloat, 8u> vertices = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f,
};
constexpr std::array<GLuint, 6u> indices = {0, 1, 2, 1, 2, 3};

}// namespace

void FractalWindow::init()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/diffuse.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
									  ":/Shaders/diffuse.fs");
	program_->link();

	// Create VAO object
	vao_.create();
	vao_.bind();

	// Create VBO
	vbo_.create();
	vbo_.bind();
	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	// Create IBO
	ibo_.create();
	ibo_.bind();
	ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));

	// Bind attributes
	program_->bind();

	program_->enableAttributeArray(0);
	program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, static_cast<int>(2 * sizeof(GLfloat)));

	shiftUniform_ = program_->uniformLocation("shift");
	zoomScaleUniform_ = program_->uniformLocation("zoom_scale");
	iterationsUniform_ = program_->uniformLocation("iterations");
	radiusUniform_ = program_->uniformLocation("radius");

	// Release all
	program_->release();

	vao_.release();

	ibo_.release();
	vbo_.release();

	// Uncomment to enable depth test and face culling
	// glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);

	// Clear all FBO buffers
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

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform value
	program_->setUniformValue(shiftUniform_, globalShift_ + currentShift_);
	program_->setUniformValue(zoomScaleUniform_, zoomScale_);
	program_->setUniformValue(iterationsUniform_, iterations_);
	program_->setUniformValue(radiusUniform_, radius_ * radius_);

	// Draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Release VAO and shader program
	vao_.release();
	program_->release();

	// Increment frame counter
	++frame_;
	int fps = int((double)frame_ / ((double)time_.elapsed() / 1000.0));
	fpsLabel_->setText("FPS: " + QString::number(fps));
	if (frame_ % 100 != 0) {
		time_.start();
		frame_ = 0;
	}
}

void FractalWindow::mousePressEvent(QMouseEvent * e)
{
	mousePressPosition_ = QVector2D(e->localPos());
	isPressed_ = true;
}

void FractalWindow::mouseReleaseEvent(QMouseEvent * e)
{
	const auto diff = QVector2D(e->localPos()) - mousePressPosition_;
	globalShift_ += QVector2D(2 * -diff.x() / (float)width(), 2 * diff.y() / (float)height());
	currentShift_.setX(0);
	currentShift_.setY(0);
	isPressed_ = false;
}

void FractalWindow::mouseMoveEvent(QMouseEvent * e)
{
	if (isPressed_) {
		const auto diff = QVector2D(e->localPos()) - mousePressPosition_;
		currentShift_.setX(2 * -diff.x() / (float)width());
		currentShift_.setY(2 * diff.y() / (float)height());
	}
}

void FractalWindow::wheelEvent(QWheelEvent * e)
{
	float oldZoomScale = zoomScale_;
	float x = e->position().x() / width();
	float y = 1.0f - (e->position().y() / height());
	zoomScale_ = std::max(0.1, zoomScale_ + e->pixelDelta().y() / 200. * zoomScale_);
	globalShift_ = zoomScale_ / oldZoomScale * (QVector2D(-1, -1) + globalShift_ + 2 * QVector2D(x, y)) - QVector2D(-1, -1) - 2 * QVector2D(x, y);
}

void FractalWindow::setIterations(int iterations)
{
	iterations_ = iterations;
}

void FractalWindow::setRadius(float radius)
{
	radius_ = radius;
}
