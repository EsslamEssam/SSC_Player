#include "ApplicationManager.h"

#include "../../mainwindow.h"
#include "../License/UI/activationwindow.h"


ApplicationManager::ApplicationManager(QObject *parent)
    : QObject(parent)
    , m_mainWindow(nullptr)
    , m_activationWindow(nullptr)
{

}



void ApplicationManager::start()
{
    m_logger.info("Application Manager Started");


    m_logger.info(
        "Application: " + m_config.appName()
        );


    m_logger.info(
        "Version: " + m_config.appVersion()
        );


    if(m_license.validateLicense())
    {
        m_logger.info("License Valid");


        m_mainWindow = new MainWindow();

        m_mainWindow->show();

    }
    else
    {
        m_logger.error("License Invalid");


        m_activationWindow = new ActivationWindow();


        connect(
            m_activationWindow,
            &ActivationWindow::licenseActivated,
            this,
            [this]()
            {
                m_logger.info("License Activated");


                m_mainWindow = new MainWindow();

                m_mainWindow->show();


                m_activationWindow->close();
            }
            );


        m_activationWindow->show();

    }
}