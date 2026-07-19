#include "DeviceManager.h"

#include <QSysInfo>


DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{

    m_deviceId = QSysInfo::machineUniqueId();

}


QString DeviceManager::getDeviceId() const
{
    return m_deviceId;
}