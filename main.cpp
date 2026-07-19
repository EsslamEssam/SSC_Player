#include "Core/Application/ApplicationManager.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    ApplicationManager appManager;

    appManager.start();


    return a.exec();
}