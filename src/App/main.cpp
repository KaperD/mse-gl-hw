#include <QApplication>
#include <QSurfaceFormat>
#include <QVBoxLayout>
#include <QWidget>

#include "FractalWindow.h"
#include "Slider.h"


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

	auto fpsLabel = new QLabel("FPS: ");

	FractalWindow glWindow(fpsLabel);
	glWindow.setFormat(format);

	QWidget *container = QWidget::createWindowContainer(&glWindow);
	container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto window = new QWidget;

	auto l = new QVBoxLayout(nullptr);

	l->addWidget(fpsLabel, 0, Qt::Alignment(Qt::AlignHCenter));

	l->addWidget(container);

	auto iterationsSlider = new Slider("Iterations", 1, 1000, 100);
	QObject::connect(iterationsSlider->slider_, &QSlider::valueChanged, &glWindow,
			qOverload<int>(&FractalWindow::setIterations));
	l->addWidget(iterationsSlider, 0, Qt::Alignment(Qt::AlignBottom));

	auto radiusSlider = new Slider("Radius", 0.1f, 40.0f, 2.0f);
	QObject::connect(radiusSlider->slider_, &QSlider::valueChanged, &glWindow,
					 [&](int x){ glWindow.setRadius(0.1f * x); });
	l->addWidget(radiusSlider, 0, Qt::Alignment(Qt::AlignBottom));

	window->setLayout(l);
	window->resize(640, 480);
	window->show();

	glWindow.setAnimated(true);

	return app.exec();
}
