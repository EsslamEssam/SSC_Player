#ifndef KEYMANAGER_H
#define KEYMANAGER_H

#include <QString>
#include <QByteArray>

class KeyManager
{
public:

    static QByteArray createKey();

    static bool saveVideoKey(
        const QString& videoId,
        const QByteArray& key
        );

    static QByteArray loadVideoKey(
        const QString& videoId
        );

private:

    static QString keyPath(
        const QString& videoId
        );

    static QByteArray getStorageKey();

    static QByteArray encryptKey(
        const QByteArray& key
        );

    static QByteArray decryptKey(
        const QByteArray& encryptedKey
        );
};

#endif