#include "ApplicationManager.h"

#include "../../mainwindow.h"
#include "../License/UI/ActivationWindow.h"
#include "../Security/ScreenCaptureManager.h"
#include "../Server/ServerClient.h"

#include <QMessageBox>


ApplicationManager::ApplicationManager(QObject *parent)
    : QObject(parent)
    , m_mainWindow(nullptr)
    , m_activationWindow(nullptr)
    , m_screenCaptureManager(nullptr)
{

}



void ApplicationManager::start()
{
    m_logger.info("Application Manager Started");


    m_screenCaptureManager = new ScreenCaptureManager(this);

    connect(
        m_screenCaptureManager,
        &ScreenCaptureManager::screenCaptureDetected,
        this,
        [this](QString processName)
        {
            QMessageBox::warning(
                nullptr,
                "Screen Recording Detected",
                "Screen recording software detected:\n"
                    + processName +
                    "\n\nVideo playback has been stopped."
                );

            if(m_mainWindow)
            {
                m_mainWindow->stopVideo();
            }
        }
        );

    m_screenCaptureManager->startMonitoring();

    m_logger.info("Application: " + m_config.appName());
    m_logger.info("Version: " + m_config.appVersion());

    connect(
        &m_license,
        &LicenseManager::validationFinished,
        this,
        [this](
            bool success,
            const QString &message
            )
        {
            if (success)
            {
                m_logger.info(
                    "License Valid"
                    );

                m_mainWindow =
                    new MainWindow();

                m_mainWindow->show();

                return;
            }


            m_logger.error(
                message
                );


            m_activationWindow =
                new ActivationWindow();

            m_activationWindow->setAttribute(
                Qt::WA_DeleteOnClose
                );


            connect(
                m_activationWindow,
                &ActivationWindow::licenseActivated,
                this,
                [this]()
                {
                    m_logger.info(
                        "License Activated"
                        );

                    if (!m_mainWindow)
                    {
                        m_mainWindow =
                            new MainWindow();

                        m_mainWindow->show();
                    }

                    if (m_activationWindow)
                    {
                        m_activationWindow->close();

                        m_activationWindow =
                            nullptr;
                    }
                }
                );


            m_activationWindow->show();
            m_activationWindow->raise();
            m_activationWindow->activateWindow();
        }
        );


    m_license.validateLicenseAsync();
}