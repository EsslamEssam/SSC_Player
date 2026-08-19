#include "ServerManager.h"

#include "ServerClient.h"

#include <QJsonObject>


ServerManager::ServerManager(QObject *parent)
    : QObject(parent)
{
    m_client = new ServerClient(this);
}


void ServerManager::validateLicense(
    const QString &licenseKey,
    const QString &deviceId
    )
{
    Q_UNUSED(licenseKey)
    Q_UNUSED(deviceId)

    emit licenseValidated(false);
}


void ServerManager::activateLicense(
    const QString &licenseKey,
    const QString &deviceId
    )
{
    Q_UNUSED(licenseKey)
    Q_UNUSED(deviceId)

    emit licenseActivated(false);
}