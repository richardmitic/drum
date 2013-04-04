#include <window.h>

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// create the window
	Window window;
	window.showMaximized();

	// Get data every WELL FAST
	window.startTimer(5);
	// Draw window every 100ms
	window.startTimer(50);

	// execute the application
	return app.exec();
}
