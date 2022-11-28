#include <QApplication>
#include <QRadioButton>
#include <QSurfaceFormat>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>

#include "FractalWindow.h"
#include "Slider.h"

#define STB_IMAGE_IMPLEMENTATION
#include "assimp/contrib/stb/stb_image.h"

namespace
{
constexpr auto g_sampels = 16;
constexpr auto g_gl_major_version = 3;
constexpr auto g_gl_minor_version = 3;
}// namespace

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);

	QSurfaceFormat format;
	format.setSamples(g_sampels);
	format.setVersion(g_gl_major_version, g_gl_minor_version);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setDepthBufferSize(16);
	format.setStencilBufferSize(8);

	auto fpsLabel = new QLabel("FPS: ");

	FractalWindow glWindow(fpsLabel);
	glWindow.setFormat(format);

	QWidget * container = QWidget::createWindowContainer(&glWindow);
	container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto window = new QWidget;

	auto l = new QVBoxLayout(nullptr);

	l->addWidget(fpsLabel, 0, Qt::Alignment(Qt::AlignHCenter));

	l->addWidget(container);

	auto buttons = new QWidget;
	auto * hbox = new QHBoxLayout(buttons);

	auto moveLight = new QPushButton("Move light to current position", buttons);
	QObject::connect(moveLight, &QPushButton::clicked, &glWindow,
					 [&]() { glWindow.moveLightToCurrentPosition(); });

	hbox->addWidget(moveLight);
	hbox->setAlignment(Qt::AlignLeft);
	l->addWidget(buttons, 0, Qt::Alignment(Qt::AlignBottom));

	auto timeSlider = new Slider("Animation", 0, 3000, 0);
	QObject::connect(timeSlider->slider_, &QSlider::valueChanged, &glWindow,
					 [&](int x) { glWindow.setAnimationTime(x); });
	l->addWidget(timeSlider, 0, Qt::Alignment(Qt::AlignBottom));

	window->setLayout(l);
	window->showFullScreen();
	window->show();

	glWindow.setAnimated(true);

	return app.exec();
}
