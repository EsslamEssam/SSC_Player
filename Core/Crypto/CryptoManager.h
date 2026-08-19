#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QString>
#include <QByteArray>

class CryptoManager
{
public:
    static QString sha256(const QString& text);
    static QByteArray sha256(const QByteArray& data);

    static QByteArray generateRandomBytes(int length);

    static QByteArray generateAESKey();   // 32 bytes
    static QByteArray generateAESIV();    // 16 bytes

    // AES-CBC
    static QByteArray aesEncrypt(
        const QByteArray& data,
        const QByteArray& key,
        const QByteArray& iv);

    static QByteArray aesDecrypt(
        const QByteArray& encryptedData,
        const QByteArray& key,
        const QByteArray& iv);


    // AES-GCM للفيديو
    static QByteArray aesGCMEncrypt(
        const QByteArray& data,
        const QByteArray& key,
        QByteArray& iv,
        QByteArray& tag);


    static QByteArray aesGCMDecrypt(
        const QByteArray& encryptedData,
        const QByteArray& key,
        const QByteArray& iv,
        const QByteArray& tag);
};

#endif