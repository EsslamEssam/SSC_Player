#ifndef LICENSESTORAGE_H
#define LICENSESTORAGE_H

#include <QObject>
#include <QString>


class LicenseStorage : public QObject
{
    Q_OBJECT

public:

    explicit LicenseStorage(QObject *parent = nullptr);


    bool saveLicense(
        const QString &licenseKey,
        const QString &deviceId
        );


    QString getLicenseKey() const;

    QString getDeviceId() const;


    bool saveActivationDate(
        const QString &date
        );


    QString getActivationDate() const;

private:

    QString m_licenseKey;
    QString m_deviceId;

};


#endif // LICENSESTORAGE_H