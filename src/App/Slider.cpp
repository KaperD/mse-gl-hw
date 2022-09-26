#include "Slider.h"

#include <QHBoxLayout>

Slider::Slider(const QString& label, int minValue, int maxValue, int initialValue, QWidget *parent)
	: QWidget(parent) {

	auto *hbox = new QHBoxLayout(this);

	label_ = new QLabel(label, this);
	hbox->addWidget(label_);

	slider_ = new QSlider(Qt::Horizontal, this);
	slider_->setMinimum(minValue);
	slider_->setMaximum(maxValue);
	slider_->setValue(initialValue);
	hbox->addWidget(slider_);

	valueLabel_ = new QLabel(QString::number(initialValue), this);
	hbox->addWidget(valueLabel_);

	connect(slider_, &QSlider::valueChanged, valueLabel_,
			qOverload<int>(&QLabel::setNum));
}

Slider::Slider(const QString & label, float minValue, float maxValue, float initialValue, QWidget * parent)
	: QWidget(parent) {

	auto *hbox = new QHBoxLayout(this);

	label_ = new QLabel(label, this);
	hbox->addWidget(label_);

	slider_ = new QSlider(Qt::Horizontal, this);
	slider_->setMinimum(int(minValue * 10));
	slider_->setMaximum(int(maxValue * 10));
	slider_->setValue(int(initialValue * 10));
	hbox->addWidget(slider_);

	valueLabel_ = new QLabel(QString::number(initialValue), this);
	hbox->addWidget(valueLabel_);

	connect(slider_, &QSlider::valueChanged, valueLabel_,
			[=](int x){ valueLabel_->setText(QString::number(0.1 * x, 'f', 1)); });
}
