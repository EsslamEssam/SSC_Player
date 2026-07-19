#include "ApplicationManager.h"


ApplicationManager::ApplicationManager(QObject *parent)
    : QObject(parent)
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


    m_license.activateLicense("TEST-1234");


    if(m_license.validateLicense())
    {
        m_logger.info("License Valid");
    }
    else
    {
        m_logger.error("License Invalid");
    }
}