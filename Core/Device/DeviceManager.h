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

    QString m_deviceId;

};


#endif // DEVICEMANAGER_H