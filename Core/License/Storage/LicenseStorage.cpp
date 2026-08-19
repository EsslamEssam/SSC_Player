#include "LicenseStorage.h"

#include <QSettings>


LicenseStorage::LicenseStorage(QObject *parent)
    : QObject(parent)
{
}


// ============================================================
// SAVE LICENSE
// ============================================================

bool LicenseStorage::saveLicense(
    const QString &licenseKey,
    const QString &deviceId
    )
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

    return settings.status()
           == QSettings::NoError;
}


// ============================================================
// GET LICENSE KEY
// ============================================================

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


// ============================================================
// GET DEVICE ID
// ============================================================

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


// ============================================================
// SAVE ACTIVATION DATE
// ============================================================

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

    return settings.status()
           == QSettings::NoError;
}


// ============================================================
// GET ACTIVATION DATE
// ============================================================

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


// ============================================================
// SAVE LAST SERVER VALIDATION
// ============================================================

bool LicenseStorage::saveLastServerValidation(
    const QString &date
    )
{
    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );

    settings.setValue(
        "License/LastServerValidation",
        date
        );

    settings.sync();

    return settings.status()
           == QSettings::NoError;
}


// ============================================================
// GET LAST SERVER VALIDATION
// ============================================================

QString LicenseStorage::getLastServerValidation() const
{
    QSettings settings(
        "SCPP",
        "ClientPlayer"
        );

    return settings.value(
                       "License/LastServerValidation"
                       ).toString();
}