#include "meshframe.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	GLMainWindow w;
	w.setWindowTitle (QString("Q Face"));
	
	w.setMinimumSize(1800, 700);

	w.resize (780,460);
	w.show();

	return a.exec();
}