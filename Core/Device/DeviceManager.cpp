#include "DeviceManager.h"

#include <QCryptographicHash>
#include <QProcess>
#include <QSettings>

namespace
{
QString cleanText(QString text)
{
    text = text.trimmed();
    text.remove('\r');
    text.remove('\n');
    return text;
}

QString runPowerShellCommand(const QString &command)
{
    QProcess process;
    process.start(
        "powershell.exe",
        QStringList()
            << "-NoProfile"
            << "-ExecutionPolicy" << "Bypass"
            << "-Command" << command
        );

    if (!process.waitForFinished(5000))
        return QString();

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    return cleanText(output);
}
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
}

QString DeviceManager::getMachineGuid() const
{
    QSettings registry(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography",
        QSettings::NativeFormat
        );

    return cleanText(registry.value("MachineGuid").toString());
}

QString DeviceManager::getMotherboardSerial() const
{
    QString serial = runPowerShellCommand(
        "(Get-CimInstance Win32_BaseBoard | Select-Object -ExpandProperty SerialNumber)"
        );

    if (serial.isEmpty())
        serial = "UNKNOWN";

    return serial;
}

QString DeviceManager::getBiosSerial() const
{
    QString serial = runPowerShellCommand(
        "(Get-CimInstance Win32_BIOS | Select-Object -ExpandProperty SerialNumber)"
        );

    if (serial.isEmpty())
        serial = "UNKNOWN";

    return serial;
}

QString DeviceManager::getSystemUUID() const
{
    QString uuid = runPowerShellCommand(
        "(Get-CimInstance Win32_ComputerSystemProduct | Select-Object -ExpandProperty UUID)"
        );

    if (uuid.isEmpty())
        uuid = "UNKNOWN";

    return uuid;
}

QString DeviceManager::sha256(const QString &text) const
{
    QByteArray hash = QCryptographicHash::hash(
        text.toUtf8(),
        QCryptographicHash::Sha256
        );

    return QString::fromLatin1(hash.toHex());
}

QString DeviceManager::getDeviceId() const
{
    QString raw;

    raw += "MachineGuid:" + getMachineGuid() + "\n";
    raw += "MotherboardSerial:" + getMotherboardSerial() + "\n";
    raw += "BiosSerial:" + getBiosSerial() + "\n";
    raw += "SystemUUID:" + getSystemUUID() + "\n";

    return sha256(raw);
}