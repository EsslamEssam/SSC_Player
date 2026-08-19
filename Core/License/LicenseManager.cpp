#include "LicenseManager.h"

#include "../Device/DeviceManager.h"
#include "Storage/LicenseStorage.h"
#include "../Server/ServerClient.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>


namespace
{

constexpr qint64 OFFLINE_GRACE_PERIOD_SECONDS =
    3LL * 24LL * 60LL * 60LL;

}


// ============================================================
// CONSTRUCTOR
// ============================================================

LicenseManager::LicenseManager(QObject *parent)
    : QObject(parent)
{
    m_device =
        new DeviceManager(this);

    m_storage =
        new LicenseStorage(this);

    m_serverClient =
        new ServerClient(this);


    // One single handler for all server requests.
    connect(
        m_serverClient,
        &ServerClient::requestFinished,
        this,
        [this](
            bool success,
            const QByteArray &response
            )
        {
            handleServerResponse(
                success,
                response
                );
        }
        );
}


// ============================================================
// HANDLE SERVER RESPONSE
// ============================================================

void LicenseManager::handleServerResponse(
    bool success,
    const QByteArray &response
    )
{
    qDebug()
    << "License Server Success:"
    << success;

    qDebug()
        << "License Server Response:"
        << response;


    const PendingRequest request =
        m_pendingRequest;

    m_pendingRequest =
        PendingRequest::None;


    // --------------------------------------------------------
    // NETWORK ERROR
    // --------------------------------------------------------

    if (!success)
    {
        qDebug()
        << "[LICENSE] Server network error.";


        // ----------------------------------------------------
        // Validation failed because there is no internet.
        // Try offline grace period.
        // ----------------------------------------------------

        if (request ==
            PendingRequest::Validate)
        {
            qDebug()
            << "[LICENSE] Trying offline grace period.";


            if (validateOfflineGracePeriod())
            {
                qDebug()
                << "[LICENSE] Offline validation successful.";

                emit validationFinished(
                    true,
                    "License valid (offline mode)."
                    );

                return;
            }


            qDebug()
                << "[LICENSE] Offline grace period failed.";


            emit validationFinished(
                false,
                "Could not connect to license server."
                );

            return;
        }


        // ----------------------------------------------------
        // Activation network error
        // ----------------------------------------------------

        emit activationFinished(
            false,
            "Could not connect to license server."
            );

        return;
    }


    // --------------------------------------------------------
    // PARSE JSON
    // --------------------------------------------------------

    QJsonParseError parseError;

    const QJsonDocument document =
        QJsonDocument::fromJson(
            response,
            &parseError
            );


    if (
        parseError.error !=
            QJsonParseError::NoError
        ||
        !document.isObject()
        )
    {
        if (request ==
            PendingRequest::Validate)
        {
            emit validationFinished(
                false,
                "Invalid server response."
                );
        }
        else
        {
            emit activationFinished(
                false,
                "Invalid server response."
                );
        }

        return;
    }


    const QJsonObject result =
        document.object();


    const bool serverSuccess =
        result
            .value("success")
            .toBool();


    const QString message =
        result
            .value("message")
            .toString();


    // ========================================================
    // VALIDATION RESPONSE
    // ========================================================

    if (request ==
        PendingRequest::Validate)
    {
        if (!serverSuccess)
        {
            qDebug()
            << "[LICENSE] Server rejected license:"
            << message;


            emit validationFinished(
                false,
                message.isEmpty()
                    ? "License Invalid"
                    : message
                );

            return;
        }


        // ----------------------------------------------------
        // Confirm server returned the correct device
        // ----------------------------------------------------

        const QString serverDevice =
            result
                .value("device_id")
                .toString();


        const QString currentDevice =
            m_device->getDeviceId();


        if (
            serverDevice.isEmpty()
            ||
            serverDevice != currentDevice
            )
        {
            qDebug()
            << "[LICENSE] Server device mismatch.";

            emit validationFinished(
                false,
                "License is activated on another device."
                );

            return;
        }


        // ----------------------------------------------------
        // Save successful server validation time
        // ----------------------------------------------------

        const QString now =
            QDateTime::currentDateTimeUtc()
                .toString(Qt::ISODate);


        if (
            !m_storage->saveLastServerValidation(
                now
                )
            )
        {
            qDebug()
            << "[LICENSE] Warning:"
            << "Could not save last server validation.";
        }


        qDebug()
            << "[LICENSE] Server validation successful.";

        qDebug()
            << "[LICENSE] Last server validation:"
            << now;


        emit validationFinished(
            true,
            message.isEmpty()
                ? "License validated successfully."
                : message
            );

        return;
    }


    // ========================================================
    // ACTIVATION RESPONSE
    // ========================================================

    if (request ==
        PendingRequest::Activate)
    {
        if (!serverSuccess)
        {
            qDebug()
            << "[LICENSE] Activation rejected:"
            << message;


            emit activationFinished(
                false,
                message.isEmpty()
                    ? "License activation failed."
                    : message
                );

            return;
        }


        const QString licenseKey =
            result
                .value("license_key")
                .toString();


        const QString deviceId =
            result
                .value("device_id")
                .toString();


        if (
            licenseKey.isEmpty()
            ||
            deviceId.isEmpty()
            )
        {
            emit activationFinished(
                false,
                "Invalid license data received."
                );

            return;
        }


        // ----------------------------------------------------
        // Save license locally
        // ----------------------------------------------------

        if (
            !m_storage->saveLicense(
                licenseKey,
                deviceId
                )
            )
        {
            emit activationFinished(
                false,
                "License activated, but could not be saved locally."
                );

            return;
        }


        // ----------------------------------------------------
        // Save successful server validation
        // ----------------------------------------------------

        const QString now =
            QDateTime::currentDateTimeUtc()
                .toString(Qt::ISODate);


        if (
            !m_storage->saveLastServerValidation(
                now
                )
            )
        {
            qDebug()
            << "[LICENSE] Warning:"
            << "Could not save last server validation.";
        }


        qDebug()
            << "[LICENSE] License activation successful.";


        emit activationFinished(
            true,
            message.isEmpty()
                ? "License activated successfully."
                : message
            );

        return;
    }
}


// ============================================================
// ACTIVATE LICENSE
// ============================================================

void LicenseManager::activateLicense(
    const QString &licenseKey
    )
{
    const QString trimmedKey =
        licenseKey.trimmed();


    if (trimmedKey.isEmpty())
    {
        emit activationFinished(
            false,
            "Please enter a license key."
            );

        return;
    }


    const QString deviceId =
        m_device->getDeviceId();


    if (deviceId.isEmpty())
    {
        emit activationFinished(
            false,
            "Could not generate device ID."
            );

        return;
    }


    QJsonObject data;

    data["action"] =
        "activate";

    data["license_key"] =
        trimmedKey;

    data["device_id"] =
        deviceId;


    qDebug()
        << "[LICENSE] Activating license:"
        << trimmedKey;

    qDebug()
        << "[LICENSE] Device ID:"
        << deviceId;


    m_pendingRequest =
        PendingRequest::Activate;


    m_serverClient->post(
        "functions/v1/super-worker",
        data
        );
}


// ============================================================
// OFFLINE GRACE PERIOD
// ============================================================

bool LicenseManager::validateOfflineGracePeriod() const
{
    const QString savedKey =
        m_storage->getLicenseKey();

    const QString savedDevice =
        m_storage->getDeviceId();

    const QString lastValidation =
        m_storage->getLastServerValidation();

    const QString currentDevice =
        m_device->getDeviceId();


    qDebug()
        << "[LICENSE] Offline validation.";

    qDebug()
        << "[LICENSE] Saved License:"
        << savedKey;

    qDebug()
        << "[LICENSE] Saved Device:"
        << savedDevice;

    qDebug()
        << "[LICENSE] Current Device:"
        << currentDevice;

    qDebug()
        << "[LICENSE] Last Server Validation:"
        << lastValidation;


    // --------------------------------------------------------
    // Local license
    // --------------------------------------------------------

    if (
        savedKey.isEmpty()
        ||
        savedDevice.isEmpty()
        )
    {
        qDebug()
        << "[LICENSE] Offline validation failed:"
        << "No local license.";

        return false;
    }


    // --------------------------------------------------------
    // Device
    // --------------------------------------------------------

    if (
        savedDevice
        !=
        currentDevice
        )
    {
        qDebug()
        << "[LICENSE] Offline validation failed:"
        << "Device mismatch.";

        return false;
    }


    // --------------------------------------------------------
    // Previous server validation
    // --------------------------------------------------------

    if (lastValidation.isEmpty())
    {
        qDebug()
        << "[LICENSE] Offline validation failed:"
        << "No previous server validation.";

        return false;
    }


    const QDateTime validationTime =
        QDateTime::fromString(
            lastValidation,
            Qt::ISODate
            );


    if (!validationTime.isValid())
    {
        qDebug()
        << "[LICENSE] Offline validation failed:"
        << "Invalid validation date.";

        return false;
    }


    const QDateTime now =
        QDateTime::currentDateTimeUtc();


    const qint64 elapsedSeconds =
        validationTime.secsTo(now);


    qDebug()
        << "[LICENSE] Offline elapsed seconds:"
        << elapsedSeconds;


    // --------------------------------------------------------
    // Clock moved backwards
    // --------------------------------------------------------

    if (elapsedSeconds < 0)
    {
        qDebug()
        << "[LICENSE] Offline validation failed:"
        << "System clock moved backwards.";

        return false;
    }


    // --------------------------------------------------------
    // Grace period
    // --------------------------------------------------------

    if (
        elapsedSeconds
        >
        OFFLINE_GRACE_PERIOD_SECONDS
        )
    {
        qDebug()
        << "[LICENSE] Offline grace period expired.";

        return false;
    }


    qDebug()
        << "[LICENSE] Offline grace period accepted.";

    return true;
}


// ============================================================
// VALIDATE LICENSE ASYNC
// ============================================================

void LicenseManager::validateLicenseAsync()
{
    const QString savedKey =
        m_storage->getLicenseKey();

    const QString savedDevice =
        m_storage->getDeviceId();

    const QString currentDevice =
        m_device->getDeviceId();


    qDebug()
        << "[LICENSE] Saved License:"
        << savedKey;

    qDebug()
        << "[LICENSE] Saved Device:"
        << savedDevice;

    qDebug()
        << "[LICENSE] Current Device:"
        << currentDevice;


    // --------------------------------------------------------
    // Local validation
    // --------------------------------------------------------

    if (
        savedKey.isEmpty()
        ||
        savedDevice.isEmpty()
        ||
        savedDevice != currentDevice
        )
    {
        emit validationFinished(
            false,
            "Local license validation failed."
            );

        return;
    }


    QJsonObject data;

    data["action"] =
        "validate";

    data["license_key"] =
        savedKey;

    data["device_id"] =
        currentDevice;


    qDebug()
        << "[LICENSE] Sending validation request to server.";


    m_pendingRequest =
        PendingRequest::Validate;


    m_serverClient->post(
        "functions/v1/super-worker",
        data
        );
}


// ============================================================
// LOCAL VALIDATION
// ============================================================

bool LicenseManager::validateLicense()
{
    const QString savedKey =
        m_storage->getLicenseKey();

    const QString savedDevice =
        m_storage->getDeviceId();

    const QString currentDevice =
        m_device->getDeviceId();


    qDebug()
        << "[LICENSE] Saved License:"
        << savedKey;

    qDebug()
        << "[LICENSE] Saved Device:"
        << savedDevice;

    qDebug()
        << "[LICENSE] Current Device:"
        << currentDevice;


    if (
        savedKey.isEmpty()
        ||
        savedDevice.isEmpty()
        )
    {
        return false;
    }


    if (
        savedDevice
        !=
        currentDevice
        )
    {
        return false;
    }


    return true;
}


// ============================================================
// GET DEVICE ID
// ============================================================

QString LicenseManager::getDeviceId() const
{
    return m_device->getDeviceId();
}


// ============================================================
// IS LICENSE VALID
// ============================================================

bool LicenseManager::isLicenseValid()
{
    return validateLicense();
}