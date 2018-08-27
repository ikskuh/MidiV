#include "mvisualizationcontainer.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MVisualizationContainer container;
	container.show();

	return a.exec();
}
