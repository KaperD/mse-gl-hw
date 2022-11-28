#pragma once

#include <Base/GLWindow.hpp>

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>

#include <QElapsedTimer>
#include <QLabel>
#include <memory>
#include <unordered_map>

#include "Model.h"

class FractalWindow final : public fgl::GLWindow
{

public:
	explicit FractalWindow(QLabel* fpsLabel) : fpsLabel_(fpsLabel) {};
	void init() override;
	void render() override;
	void setAnimationTime(/*from 0 to 1000*/int time);
	void moveLightToCurrentPosition();

protected:
	void mousePressEvent(QMouseEvent * e) override;
	void mouseReleaseEvent(QMouseEvent * e) override;
	void keyPressEvent(QKeyEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;
	void keyReleaseEvent(QKeyEvent * event) override;
	void mouseDoubleClickEvent(QMouseEvent * event) override;
	void drawModel(QOpenGLShaderProgram& program);

private:
	int animation_time_ = 0;

	std::unique_ptr<QOpenGLShaderProgram> program_ = nullptr;

	std::unique_ptr<QOpenGLShaderProgram> click_program_ = nullptr;
	std::unique_ptr<QOpenGLFramebufferObject> click_fbo_ = nullptr;

	size_t frame_ = 0;

	bool isPressed_ = false;

	QVector3D cameraPos_{2, 1, -4};
	QVector3D lightPos_{2, 4, -4};
	QVector3D cameraDirection_{0, 0, 1};
	QVector3D cameraUp_{0, 1, 0};

	QMatrix4x4 zoom_;

	int lastX = 0;
	int lastY = 0;

	float yaw = 90.0f;
	float pitch = 0.0f;

	std::unordered_map<int, bool> keys {{}};
	void doMovement();

	QElapsedTimer time_;
	QLabel* fpsLabel_;

	std::unique_ptr<Model> model_;
};
