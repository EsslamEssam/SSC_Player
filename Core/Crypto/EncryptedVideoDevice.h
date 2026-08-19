#ifndef ENCRYPTEDVIDEODEVICE_H
#define ENCRYPTEDVIDEODEVICE_H

#include <QIODevice>
#include <QByteArray>

#include "VideoCryptoManager.h"


class EncryptedVideoDevice : public QIODevice
{
    Q_OBJECT

public:

    explicit EncryptedVideoDevice(
        QObject* parent = nullptr
        );


    bool openEncryptedVideo(
        const QString& encryptedFile
        );


    void close() override;


    qint64 size() const override;


protected:

    qint64 readData(
        char* data,
        qint64 maxSize
        ) override;


    qint64 writeData(
        const char* data,
        qint64 maxSize
        ) override;


public:

    bool seek(
        qint64 pos
        ) override;


private:

    bool loadChunk(
        qint64 chunkIndex
        );


    VideoCryptoManager m_videoCrypto;


    QByteArray m_currentData;

    qint64 m_currentDataPosition;

    qint64 m_currentGlobalPosition;

    qint64 m_totalSize;
};

#endif