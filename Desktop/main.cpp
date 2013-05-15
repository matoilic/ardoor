#include <QApplication>
#include "MainWindow.h"
#include "test.cpp"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("BFH");
    QCoreApplication::setOrganizationDomain("bfh.ch");
    QCoreApplication::setApplicationName("ARDoor");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
