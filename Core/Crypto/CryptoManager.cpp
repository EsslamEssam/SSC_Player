#include "CryptoManager.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#include <QDebug>

QString CryptoManager::sha256(const QString& text)
{
    QByteArray input = text.toUtf8();

    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(input.constData()),
        input.size(),
        hash);

    QString result;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        result += QString("%1")
        .arg(hash[i], 2, 16, QLatin1Char('0'));
    }

    return result;
}

QByteArray CryptoManager::sha256(const QByteArray& data)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(data.constData()),
        data.size(),
        hash);

    return QByteArray(
        reinterpret_cast<char*>(hash),
        SHA256_DIGEST_LENGTH);
}

QByteArray CryptoManager::generateRandomBytes(int length)
{
    QByteArray bytes(length, 0);

    if (RAND_bytes(
            reinterpret_cast<unsigned char*>(bytes.data()),
            length) != 1)
    {
        return {};
    }

    return bytes;
}

QByteArray CryptoManager::generateAESKey()
{
    return generateRandomBytes(32);
}

QByteArray CryptoManager::generateAESIV()
{
    return generateRandomBytes(16);
}

QByteArray CryptoManager::aesEncrypt(
    const QByteArray& data,
    const QByteArray& key,
    const QByteArray& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
        return {};

    QByteArray encrypted(data.size() + EVP_MAX_BLOCK_LENGTH, 0);

    int outLen1 = 0;
    int outLen2 = 0;

    if (EVP_EncryptInit_ex(
            ctx,
            EVP_aes_256_cbc(),
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    if (EVP_EncryptUpdate(
            ctx,
            reinterpret_cast<unsigned char*>(encrypted.data()),
            &outLen1,
            reinterpret_cast<const unsigned char*>(data.constData()),
            data.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    if (EVP_EncryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char*>(encrypted.data()) + outLen1,
            &outLen2) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    encrypted.resize(outLen1 + outLen2);

    EVP_CIPHER_CTX_free(ctx);

    return encrypted;
}

QByteArray CryptoManager::aesDecrypt(
    const QByteArray& encryptedData,
    const QByteArray& key,
    const QByteArray& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
        return {};

    QByteArray decrypted(
        encryptedData.size(),
        0
        );

    int outLen1 = 0;
    int outLen2 = 0;


    if (EVP_DecryptInit_ex(
            ctx,
            EVP_aes_256_cbc(),
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(iv.constData()))
        != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    if (EVP_DecryptUpdate(
            ctx,
            reinterpret_cast<unsigned char*>(decrypted.data()),
            &outLen1,
            reinterpret_cast<const unsigned char*>(encryptedData.constData()),
            encryptedData.size())
        != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    if (EVP_DecryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char*>(decrypted.data()) + outLen1,
            &outLen2)
        != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }


    decrypted.resize(outLen1 + outLen2);


    EVP_CIPHER_CTX_free(ctx);


    return decrypted;
}



QByteArray CryptoManager::aesGCMEncrypt(
    const QByteArray& data,
    const QByteArray& key,
    QByteArray& iv,
    QByteArray& tag)
{

    EVP_CIPHER_CTX* ctx =
        EVP_CIPHER_CTX_new();


    if(!ctx)
        return {};



    iv =
        generateRandomBytes(12);



    QByteArray encrypted(
        data.size() + 16,
        0);



    int outLen = 0;
    int finalLen = 0;



    if(EVP_EncryptInit_ex(
            ctx,
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    if(EVP_CIPHER_CTX_ctrl(
            ctx,
            EVP_CTRL_GCM_SET_IVLEN,
            iv.size(),
            nullptr) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    if(EVP_EncryptInit_ex(
            ctx,
            nullptr,
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    if(EVP_EncryptUpdate(
            ctx,
            reinterpret_cast<unsigned char*>(encrypted.data()),
            &outLen,
            reinterpret_cast<const unsigned char*>(data.constData()),
            data.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    if(EVP_EncryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char*>(encrypted.data()) + outLen,
            &finalLen) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    encrypted.resize(outLen + finalLen);



    tag.resize(16);



    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_GET_TAG,
        16,
        tag.data());



    EVP_CIPHER_CTX_free(ctx);


    return encrypted;

}



QByteArray CryptoManager::aesGCMDecrypt(
    const QByteArray& encryptedData,
    const QByteArray& key,
    const QByteArray& iv,
    const QByteArray& tag)
{

    EVP_CIPHER_CTX* ctx =
        EVP_CIPHER_CTX_new();


    if(!ctx)
        return {};



    QByteArray decrypted(
        encryptedData.size(),
        0);



    int outLen = 0;
    int finalLen = 0;



    if(EVP_DecryptInit_ex(
            ctx,
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }



    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_SET_IVLEN,
        iv.size(),
        nullptr);



    EVP_DecryptInit_ex(
        ctx,
        nullptr,
        nullptr,
        reinterpret_cast<const unsigned char*>(key.constData()),
        reinterpret_cast<const unsigned char*>(iv.constData()));



    EVP_DecryptUpdate(
        ctx,
        reinterpret_cast<unsigned char*>(decrypted.data()),
        &outLen,
        reinterpret_cast<const unsigned char*>(encryptedData.constData()),
        encryptedData.size());



    EVP_CIPHER_CTX_ctrl(
        ctx,
        EVP_CTRL_GCM_SET_TAG,
        tag.size(),
        const_cast<char*>(tag.constData()));



    if(EVP_DecryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char*>(decrypted.data()) + outLen,
            &finalLen) <= 0)
    {
        EVP_CIPHER_CTX_free(ctx);

        qDebug()
            << "GCM authentication failed";

        return {};
    }



    decrypted.resize(outLen + finalLen);



    EVP_CIPHER_CTX_free(ctx);


    return decrypted;

}