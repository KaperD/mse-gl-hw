#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>

class Slider : public QWidget {

	Q_OBJECT

public:
	Slider(const QString& label, int minValue, int maxValue, int initialValue, QWidget *parent = nullptr);
	Slider(const QString& label, float minValue, float maxValue, float initialValue, QWidget *parent = nullptr);

	QSlider *slider_;
	QLabel *label_;
	QLabel *valueLabel_;
};
