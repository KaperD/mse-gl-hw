#pragma once

#include <Base/GLWindow.hpp>

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>

#include <memory>

class TriangleWindow final : public fgl::GLWindow
{

public:
	void init() override;
	void render() override;

protected:
	void mousePressEvent(QMouseEvent * e) override;
	void mouseReleaseEvent(QMouseEvent * e) override;

private:
	GLint matrixUniform_ = -1;

	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;

	std::unique_ptr<QOpenGLShaderProgram> program_ = nullptr;

	size_t frame_ = 0;

	QVector2D mousePressPosition_{0., 0.};
	QVector3D rotationAxis_{0., 1., 0.};
};
