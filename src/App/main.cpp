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

	auto without_ao = new QRadioButton("Without AO", buttons);
	without_ao->setChecked(true);
	QObject::connect(without_ao, &QRadioButton::toggled, &glWindow,
					 [&](bool checked) {
		 if (checked)
		 {
			 glWindow.setUseAO(false);
			 glWindow.setUseOnlyAO(false);
		 }
 	});

	auto with_ao = new QRadioButton("With AO", buttons);
	QObject::connect(with_ao, &QRadioButton::toggled, &glWindow,
					 [&](bool checked) { if (checked)
		{
			glWindow.setUseAO(true);
			glWindow.setUseOnlyAO(false);
		}
	});

	auto only_ao = new QRadioButton("Only AO", buttons);
	QObject::connect(only_ao, &QRadioButton::toggled, &glWindow,
					 [&](bool checked) { if (checked)
		{
			glWindow.setUseAO(true);
			glWindow.setUseOnlyAO(true);
		}
	});


	hbox->addWidget(moveLight);
	hbox->addWidget(without_ao);
	hbox->addWidget(with_ao);
	hbox->addWidget(only_ao);
	hbox->setAlignment(Qt::AlignLeft);
	l->addWidget(buttons, 0, Qt::Alignment(Qt::AlignBottom));

	auto timeSlider = new Slider("Animation", 0, 3000, 0);
	QObject::connect(timeSlider->slider_, &QSlider::valueChanged, &glWindow,
					 [&](int x) { glWindow.setAnimationTime(x); });
	l->addWidget(timeSlider, 0, Qt::Alignment(Qt::AlignBottom));

	auto kernelSizeSlider = new Slider("Kernel Size", 4, 128, 64);
	QObject::connect(kernelSizeSlider->slider_, &QSlider::valueChanged, &glWindow,
					 [&](int x) { glWindow.setKernelSize(x); });
	l->addWidget(kernelSizeSlider, 0, Qt::Alignment(Qt::AlignBottom));

	window->setLayout(l);
	window->showFullScreen();
	window->show();

	glWindow.setAnimated(true);

	return app.exec();
}
