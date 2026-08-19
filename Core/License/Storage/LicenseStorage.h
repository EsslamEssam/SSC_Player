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

    // --------------------------------------------------------
    // Last successful server validation
    // --------------------------------------------------------

    bool saveLastServerValidation(
        const QString &date
        );

    QString getLastServerValidation() const;

private:
};

#endif // LICENSESTORAGE_H