#include "mainwindow.h"

#include "Core/Application/ApplicationManager.h"
#include "Core/License/UI/activationwindow.h"
#include "Core/Utils/Logger/Logger.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    ApplicationManager appManager;

    appManager.start();


    ActivationWindow activation;

    activation.show();


    // MainWindow w;
    // w.show();


    return a.exec();
}