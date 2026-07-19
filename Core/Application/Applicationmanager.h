#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>

#include "../Utils/Logger/Logger.h"
#include "../Settings/ConfigManager.h"
#include "../License/LicenseManager.h"


class ApplicationManager : public QObject
{
    Q_OBJECT

public:

    explicit ApplicationManager(QObject *parent = nullptr);

    void start();


private:

    Logger m_logger;
    ConfigManager m_config;
    LicenseManager m_license;

};


#endif // APPLICATIONMANAGER_H
