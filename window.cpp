#include "window.h"

#include <iostream>  // for sine stuff
using namespace std;


Window::Window() : plot( QString("Sensor Channel 1") ), gain(1), count(0), instAD7705(), buf_w_idx(0), activeChannel(CHAN1)
{
	
	// set up the gain knob
	knob.setValue(gain);
	button.setText("Switch channel");
	button.resize(100,100);

	// use the Qt signals/slots framework to update the gain -
	// every time the knob is moved, the setGain function will be called
	connect( &knob, SIGNAL(valueChanged(double)), SLOT(setGain(double)) );
	connect( &button, SIGNAL(released()), SLOT(handleButton()) );

	// set up the initial plot data
	for( int index=0; index<plotDataSize; ++index )
	{
		xData[index] = index;
		yData[index] = gain * instAD7705.read(activeChannel);
	}

	// make a plot curve from the data and attach it to the plot
	curve.setSamples(xData, yData, plotDataSize);
	curve.attach(&plot);
	
	plot.replot();
	plot.show();
	
	vLayout.addWidget(&knob);
	vLayout.addWidget(&button);
	
	hLayout.addLayout(&vLayout);
	hLayout.addWidget(&plot);
	
	setLayout(&hLayout);
}



void Window::timerEvent( QTimerEvent * event )
{
	switch( event->timerId() )
	{
		// Fast timer. Get data and store in buffer.
		case 0x1:
			buffer[buf_w_idx] = gain * instAD7705.read(activeChannel);
			buf_w_idx += 1;
			break;
		
		// Slow timer. Display buffer on screen.
		case 0x1000002:
			// add the new input to the plot
			memmove( yData, yData+buf_w_idx, (plotDataSize-1) * sizeof(double) );
			memcpy( &(yData[plotDataSize-buf_w_idx]), buffer, buf_w_idx * sizeof(double));
			curve.setSamples(xData, yData, plotDataSize);
			plot.replot();
	
			// reset buffer index
			buf_w_idx = 0;
			
			break;
		
		default:
			cout << "strange timer" << endl;
			
	}
}


// Change display gain
void Window::setGain(double gain)
{
	// for example purposes just change the amplitude of the generated input
	this->gain = gain;
}

// Switch between ADC channels 1 and 2
void Window::handleButton()
{	
	char title[100];
	
	cout << "Channel changed" << endl;
	activeChannel = 1-activeChannel;
	sprintf(title, "Sensor Channel %d", activeChannel+1);
	plot.setTitle( QString(title) );
	
	instAD7705.initChannel(activeChannel);
}


