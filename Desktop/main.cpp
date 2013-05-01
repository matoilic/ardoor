#include <QApplication>
#include "MainWindow.h"
#include "test.cpp"

int main(int argc, char *argv[])
{
    test();
    return 0;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
