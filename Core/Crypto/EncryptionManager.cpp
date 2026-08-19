#include "EncryptionManager.h"

#include "CryptoManager.h"
#include "KeyManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>


bool EncryptionManager::encryptFile(
    const QString& inputFile,
    const QString& outputFile,
    const QString& videoId)
{

    QFile input(inputFile);


    if(!input.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open input file";
        return false;
    }



    QByteArray data =
        input.readAll();


    input.close();



    QByteArray key =
        KeyManager::createKey();


    if(!KeyManager::saveVideoKey(
            videoId,
            key))
    {
        qDebug()
        << "Cannot save video key";

        return false;
    }



    QByteArray iv =
        CryptoManager::generateAESIV();



    QByteArray tag;


    QByteArray encrypted =
        CryptoManager::aesGCMEncrypt(
            data,
            key,
            iv,
            tag
            );



    QFile output(outputFile);



    if(!output.open(QIODevice::WriteOnly))
    {
        qDebug() << "Cannot create output file";
        return false;
    }



    QDataStream stream(&output);



    stream << QByteArray("SCPPENC");


    stream << quint32(2);

    stream << videoId;

    stream << iv;

    stream << tag;

    stream << encrypted;



    output.close();



    return true;
}




bool EncryptionManager::decryptFile(
    const QString& inputFile,
    const QString& outputFile)
{

    QFile input(inputFile);


    if(!input.open(QIODevice::ReadOnly))
    {
        qDebug()
        << "Cannot open encrypted file";

        return false;
    }



    QDataStream stream(&input);



    QByteArray magic;


    stream >> magic;



    if(magic != "SCPPENC")
    {
        qDebug()
        << "Invalid encrypted file";

        input.close();

        return false;
    }



    quint32 version;


    stream >> version;



    QString videoId;

    stream >> videoId;


    QByteArray iv;

    stream >> iv;


    QByteArray tag;

    stream >> tag;


    QByteArray encrypted;

    stream >> encrypted;



    input.close();



    QByteArray key =
        KeyManager::loadVideoKey(videoId);



    if(key.isEmpty())
    {
        qDebug()
        << "Video key not found";

        return false;
    }



    QByteArray decrypted =
        CryptoManager::aesGCMDecrypt(
            encrypted,
            key,
            iv,
            tag
            );



    if(decrypted.isEmpty())
    {
        qDebug()
        << "Decrypt failed";

        return false;
    }



    QFile output(outputFile);



    if(!output.open(QIODevice::WriteOnly))
    {
        qDebug()
        << "Cannot create output file";

        return false;
    }



    output.write(decrypted);


    output.close();



    return true;

}


bool EncryptionManager::encryptVideoFile(
    const QString& inputFile,
    const QString& outputFile,
    const QString& videoId)
{
    const qint64 CHUNK_SIZE =
        1024 * 1024;


    QFile input(inputFile);


    if (!input.open(QIODevice::ReadOnly))
    {
        qDebug()
        << "Cannot open video input file:"
        << inputFile;

        return false;
    }


    QByteArray key =
        KeyManager::createKey();


    if (!KeyManager::saveVideoKey(
            videoId,
            key))
    {
        qDebug()
        << "Cannot save video key";

        input.close();

        return false;
    }


    QFile output(outputFile);


    if (!output.open(QIODevice::WriteOnly))
    {
        qDebug()
        << "Cannot create encrypted video file:"
        << outputFile;

        input.close();

        return false;
    }


    QDataStream stream(&output);


    stream << QByteArray("SCPPVID");

    stream << quint32(1);

    stream << videoId;

    stream << quint64(CHUNK_SIZE);


    while (!input.atEnd())
    {
        QByteArray chunk =
            input.read(CHUNK_SIZE);


        if (chunk.isEmpty())
        {
            break;
        }


        QByteArray iv;

        QByteArray tag;


        QByteArray encrypted =
            CryptoManager::aesGCMEncrypt(
                chunk,
                key,
                iv,
                tag
                );


        if (encrypted.isEmpty())
        {
            qDebug()
            << "Video chunk encryption failed";

            input.close();

            output.close();

            return false;
        }


        stream << quint64(chunk.size());


        stream << iv;


        stream << tag;


        stream << quint64(encrypted.size());


        output.write(encrypted);


        if (output.error() !=
            QFileDevice::NoError)
        {
            qDebug()
            << "Cannot write encrypted chunk";

            input.close();

            output.close();

            return false;
        }
    }


    input.close();

    output.close();


    qDebug()
        << "Video encryption completed:"
        << outputFile;


    return true;
}