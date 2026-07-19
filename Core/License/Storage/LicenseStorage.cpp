#include "LicenseStorage.h"

#include <QSettings>

#include <QDateTime>


LicenseStorage::LicenseStorage(QObject *parent)
    : QObject(parent)
{

}


bool LicenseStorage::saveLicense(
    const QString &licenseKey,
    const QString &deviceId)
{

    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );


    settings.setValue(
        "License/Key",
        licenseKey
        );


    settings.setValue(
        "License/Device",
        deviceId
        );


    settings.sync();


    return true;
}



QString LicenseStorage::getLicenseKey() const
{

    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );


    return settings.value(
                       "License/Key"
                       ).toString();

}



QString LicenseStorage::getDeviceId() const
{

    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );


    return settings.value(
                       "License/Device"
                       ).toString();

}



bool LicenseStorage::saveActivationDate(
    const QString &date
    )
{
    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );


    settings.setValue(
        "License/ActivationDate",
        date
        );


    settings.sync();


    return true;
}



QString LicenseStorage::getActivationDate() const
{

    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );


    return settings.value(
                       "License/ActivationDate"
                       ).toString();

}