#ifndef WINDOW_H
#define WINDOW_H

#include <qwt/qwt_thermo.h>
#include <qwt/qwt_knob.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

#include <QBoxLayout>
#include <QTimerEvent>
#include <QtGui/QPushButton>

#include "ad7705.h"

#define BUFFER_LENGTH 32

// class definition 'Window'
class Window : public QWidget
{
	// must include the Q_OBJECT macro for for the Qt signals/slots framework to work with this class
	Q_OBJECT
		
	AD7705 instAD7705;

public:
	double buffer[BUFFER_LENGTH];
	int buf_w_idx;
	int activeChannel;
	Window(); // default constructor - called when a Window is declared without arguments

protected:
	void timerEvent( QTimerEvent * event );

public slots:
	void setGain(double gain);
	void handleButton();

// internal variables for the window class
private:
  // graphical elements from the Qwt library - http://qwt.sourceforge.net/annotated.html
	QwtKnob      knob;
	QwtPlot      plot;
	QwtPlotCurve curve;
	QPushButton  button;

	// layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
	QVBoxLayout  vLayout;  // vertical layout
	QHBoxLayout  hLayout;  // horizontal layout

	static const int plotDataSize = 100;

	// data arrays for the plot
	double xData[plotDataSize];
	double yData[plotDataSize];

	double gain;
	int count;
	
};

#endif // WINDOW_H
