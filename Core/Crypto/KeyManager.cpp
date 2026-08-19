#include "KeyManager.h"

#include "CryptoManager.h"

#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>


QByteArray KeyManager::createKey()
{
    return CryptoManager::generateAESKey();
}


// ============================================================
// Key file path
// ============================================================

QString KeyManager::keyPath(
    const QString& videoId)
{
    QString folder =
        QCoreApplication::applicationDirPath()
        + "/Data/Keys";

    QDir dir(folder);

    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    return folder + "/" + videoId + ".key";
}


// ============================================================
// Storage key
// ============================================================

QByteArray KeyManager::getStorageKey()
{
    /*
     * DEVELOPMENT VERSION
     *
     * This key protects the Video Key stored on disk.
     *
     * Later this will be replaced with a stronger
     * device-bound key.
     */

    const QString storageSecret =
        "SCPP_PLAYER_KEY_STORAGE_SECRET_2026";

    return CryptoManager::sha256(
        storageSecret.toUtf8()
        );
}


// ============================================================
// Encrypt Video Key before saving
// ============================================================

QByteArray KeyManager::encryptKey(
    const QByteArray& key)
{
    if (key.size() != 32)
    {
        qDebug()
        << "[KEY MANAGER] Invalid key size.";

        return {};
    }

    QByteArray storageKey =
        getStorageKey();

    if (storageKey.size() != 32)
    {
        qDebug()
        << "[KEY MANAGER] Invalid storage key.";

        return {};
    }

    QByteArray iv =
        CryptoManager::generateAESIV();

    if (iv.size() != 16)
    {
        qDebug()
        << "[KEY MANAGER] Failed to generate IV.";

        return {};
    }

    QByteArray encrypted =
        CryptoManager::aesEncrypt(
            key,
            storageKey,
            iv
            );

    if (encrypted.isEmpty())
    {
        qDebug()
        << "[KEY MANAGER] Key encryption failed.";

        return {};
    }

    /*
     * File format:
     *
     * [16 bytes IV]
     * [AES-256-CBC encrypted 32-byte Video Key]
     */

    QByteArray result;

    result.append(iv);
    result.append(encrypted);

    return result;
}


// ============================================================
// Decrypt Video Key after loading
// ============================================================

QByteArray KeyManager::decryptKey(
    const QByteArray& encryptedKey)
{
    if (encryptedKey.size() <= 16)
    {
        qDebug()
        << "[KEY MANAGER] Invalid encrypted key.";

        return {};
    }

    QByteArray iv =
        encryptedKey.left(16);

    QByteArray encryptedData =
        encryptedKey.mid(16);

    QByteArray storageKey =
        getStorageKey();

    if (storageKey.size() != 32)
    {
        qDebug()
        << "[KEY MANAGER] Invalid storage key.";

        return {};
    }

    QByteArray key =
        CryptoManager::aesDecrypt(
            encryptedData,
            storageKey,
            iv
            );

    if (key.size() != 32)
    {
        qDebug()
        << "[KEY MANAGER] Key decryption failed.";

        return {};
    }

    return key;
}


// ============================================================
// Save encrypted Video Key
// ============================================================

bool KeyManager::saveVideoKey(
    const QString& videoId,
    const QByteArray& key)
{
    if (videoId.isEmpty())
    {
        qDebug()
        << "[KEY MANAGER] Empty video ID.";

        return false;
    }

    if (key.size() != 32)
    {
        qDebug()
        << "[KEY MANAGER] Invalid video key size:"
        << key.size();

        return false;
    }

    QByteArray encryptedKey =
        encryptKey(key);

    if (encryptedKey.isEmpty())
    {
        return false;
    }

    QFile file(
        keyPath(videoId)
        );

    if (!file.open(
            QIODevice::WriteOnly))
    {
        qDebug()
        << "[KEY MANAGER] Cannot save video key.";

        return false;
    }

    qint64 written =
        file.write(encryptedKey);

    file.close();

    if (written != encryptedKey.size())
    {
        qDebug()
        << "[KEY MANAGER] Failed to write complete key.";

        return false;
    }

    qDebug()
        << "[KEY MANAGER] Encrypted video key saved:"
        << videoId;

    return true;
}


// ============================================================
// Load and decrypt Video Key
// ============================================================

QByteArray KeyManager::loadVideoKey(
    const QString& videoId)
{
    if (videoId.isEmpty())
    {
        qDebug()
        << "[KEY MANAGER] Empty video ID.";

        return {};
    }

    QFile file(
        keyPath(videoId)
        );

    if (!file.open(
            QIODevice::ReadOnly))
    {
        qDebug()
        << "[KEY MANAGER] Cannot load video key:"
        << keyPath(videoId);

        return {};
    }

    QByteArray encryptedKey =
        file.readAll();

    file.close();

    qDebug()
        << "[KEY MANAGER] Encrypted key file size:"
        << encryptedKey.size();

    QByteArray key =
        decryptKey(encryptedKey);

    if (key.size() != 32)
    {
        qDebug()
        << "[KEY MANAGER] Invalid decrypted video key.";

        return {};
    }

    qDebug()
        << "[KEY MANAGER] Video key loaded and decrypted:"
        << videoId;

    return key;
}