#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <QObject>
#include <QString>

class DeviceManager;
class LicenseStorage;
class ServerClient;

class LicenseManager : public QObject
{
    Q_OBJECT

public:

    explicit LicenseManager(QObject *parent = nullptr);

    bool validateLicense();

    void validateLicenseAsync();

    bool isLicenseValid();

    void activateLicense(
        const QString &licenseKey
        );

    QString getDeviceId() const;

signals:

    void activationFinished(
        bool success,
        const QString &message
        );

    void validationFinished(
        bool success,
        const QString &message
        );

private:

    enum class PendingRequest
    {
        None,
        Activate,
        Validate
    };

    bool validateOfflineGracePeriod() const;

    void handleServerResponse(
        bool success,
        const QByteArray &response
        );

    DeviceManager *m_device;

    LicenseStorage *m_storage;

    ServerClient *m_serverClient;

    PendingRequest m_pendingRequest =
        PendingRequest::None;
};

#endif // LICENSEMANAGER_H