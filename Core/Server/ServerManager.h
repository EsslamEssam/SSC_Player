#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>
#include <QString>

class ServerClient;

class ServerManager : public QObject
{
    Q_OBJECT

public:

    explicit ServerManager(QObject *parent = nullptr);

    void validateLicense(
        const QString &licenseKey,
        const QString &deviceId
        );

    void activateLicense(
        const QString &licenseKey,
        const QString &deviceId
        );

signals:

    void licenseValidated(
        bool valid
        );

    void licenseActivated(
        bool success
        );

private:

    ServerClient *m_client;
};

#endif // SERVERMANAGER_H