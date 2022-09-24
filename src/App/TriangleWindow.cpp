#include "TriangleWindow.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QScreen>

#include <array>

namespace
{

constexpr std::array<GLfloat, 15u> vertices = {
	0.0f, 0.707f, 1.f, 0.f, 0.f,
	-0.5f, -0.5f, 0.f, 1.f, 0.f,
	0.5f, -0.5f, 0.f, 0.f, 1.f,
};
constexpr std::array<GLuint, 3u> indices = {0, 1, 2};

}// namespace

void TriangleWindow::init()
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
	program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, static_cast<int>(5 * sizeof(GLfloat)));

	program_->enableAttributeArray(1);
	program_->setAttributeBuffer(1, GL_FLOAT, static_cast<int>(2 * sizeof(GLfloat)), 3,
								 static_cast<int>(5 * sizeof(GLfloat)));

	matrixUniform_ = program_->uniformLocation("matrix");

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

void TriangleWindow::render()
{
	// Configure viewport
	const auto retinaScale = devicePixelRatio();
	glViewport(0, 0, static_cast<GLint>(width() * retinaScale),
			   static_cast<GLint>(height() * retinaScale));

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate MVP matrix
	QMatrix4x4 matrix;
	matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	matrix.translate(0, 0, -2);
	const auto angle = 100.0 * static_cast<double>(frame_) / screen()->refreshRate();
	matrix.rotate(static_cast<float>(angle), rotationAxis_);

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	// Update uniform value
	program_->setUniformValue(matrixUniform_, matrix);

	// Draw
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

	// Release VAO and shader program
	vao_.release();
	program_->release();

	// Increment frame counter
	++frame_;
}

void TriangleWindow::mousePressEvent(QMouseEvent * e)
{
	mousePressPosition_ = QVector2D(e->localPos());
}

void TriangleWindow::mouseReleaseEvent(QMouseEvent * e)
{
	const auto diff = QVector2D(e->localPos()) - mousePressPosition_;
	rotationAxis_ = QVector3D(diff.y(), diff.x(), 0.0).normalized();
}