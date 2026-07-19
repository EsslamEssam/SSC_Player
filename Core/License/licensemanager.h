#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <QObject>
#include <QString>


class DeviceManager;
class LicenseStorage;


class LicenseManager : public QObject
{
    Q_OBJECT

public:

    explicit LicenseManager(QObject *parent = nullptr);

    bool validateLicense();

    bool isLicenseValid();

    bool activateLicense(
        const QString &licenseKey
        );

    QString getDeviceId() const;


private:

    DeviceManager *m_device;

    LicenseStorage *m_storage;

};


#endif // LICENSEMANAGER_H