#include "LicenseManager.h"

#include "../Device/DeviceManager.h"
#include "Storage/LicenseStorage.h"

#include <QDebug>


LicenseManager::LicenseManager(QObject *parent)
    : QObject(parent)
{
    m_device = new DeviceManager(this);

    m_storage = new LicenseStorage(this);
}



bool LicenseManager::activateLicense(const QString &licenseKey)
{
    if(licenseKey == "TEST-1234")
    {
        return m_storage->saveLicense(
            licenseKey,
            m_device->getDeviceId()
            );
    }

    return false;
}



bool LicenseManager::validateLicense()
{
    QString savedKey =
        m_storage->getLicenseKey();


    QString savedDevice =
        m_storage->getDeviceId();


    qDebug() << "Device ID:" << m_device->getDeviceId();


    if(savedKey.isEmpty())
    {
        return false;
    }


    if(savedDevice != m_device->getDeviceId())
    {
        return false;
    }


    return true;
}



QString LicenseManager::getDeviceId() const
{
    return m_device->getDeviceId();
}

bool LicenseManager::isLicenseValid()
{
    return validateLicense();
}