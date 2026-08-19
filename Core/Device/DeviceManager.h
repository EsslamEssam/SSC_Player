#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QString>

class DeviceManager : public QObject
{
    Q_OBJECT

public:

    explicit DeviceManager(QObject *parent = nullptr);

    QString getDeviceId() const;

private:

    QString getMachineGuid() const;

    QString getMotherboardSerial() const;

    QString getBiosSerial() const;

    QString getSystemUUID() const;

    QString sha256(const QString &text) const;
};

#endif // DEVICEMANAGER_H