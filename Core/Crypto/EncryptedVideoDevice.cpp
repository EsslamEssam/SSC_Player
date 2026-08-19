#include "EncryptedVideoDevice.h"

#include <QDebug>
#include <cstring>


EncryptedVideoDevice::EncryptedVideoDevice(
    QObject* parent)
    : QIODevice(parent)
    , m_currentDataPosition(0)
    , m_currentGlobalPosition(0)
    , m_totalSize(0)
{
}


bool EncryptedVideoDevice::openEncryptedVideo(
    const QString& encryptedFile)
{
    if (isOpen())
    {
        close();
    }


    if (!m_videoCrypto.open(
            encryptedFile))
    {
        qDebug()
        << "EncryptedVideoDevice:"
        << "Cannot open video";

        return false;
    }


    m_currentData.clear();

    m_currentDataPosition = 0;

    m_currentGlobalPosition = 0;


    m_totalSize =
        m_videoCrypto.totalSize();


    if (m_totalSize <= 0)
    {
        m_videoCrypto.close();

        return false;
    }


    if (!loadChunk(0))
    {
        m_videoCrypto.close();

        return false;
    }


    if (!QIODevice::open(
            QIODevice::ReadOnly))
    {
        m_videoCrypto.close();

        return false;
    }


    return true;
}


bool EncryptedVideoDevice::loadChunk(
    qint64 chunkIndex)
{
    if (!m_videoCrypto.seekToChunk(
            chunkIndex))
    {
        return false;
    }


    m_currentData =
        m_videoCrypto.readChunk();


    qDebug()
        << "[SECURITY TEST] Decrypted chunk exists in RAM."
        << "Size:"
        << m_currentData.size();


    if (m_currentData.isEmpty())
    {
        return false;
    }


    m_currentDataPosition = 0;


    return true;
}


qint64 EncryptedVideoDevice::readData(
    char* data,
    qint64 maxSize)
{
    if (!isOpen())
        return -1;


    if (maxSize <= 0)
        return 0;


    qint64 totalRead = 0;


    while (totalRead < maxSize)
    {
        if (m_currentDataPosition >=
            m_currentData.size())
        {
            qint64 nextChunk =
                m_videoCrypto.currentChunk();


            if (nextChunk >=
                m_videoCrypto.chunkCount())
            {
                break;
            }


            if (!loadChunk(nextChunk))
            {
                break;
            }
        }


        qint64 available =
            m_currentData.size()
            - m_currentDataPosition;


        qint64 wanted =
            maxSize - totalRead;


        qint64 amount =
            qMin(
                available,
                wanted
                );


        std::memcpy(
            data + totalRead,
            m_currentData.constData()
                + m_currentDataPosition,
            static_cast<size_t>(
                amount
                )
            );


        m_currentDataPosition +=
            amount;


        m_currentGlobalPosition +=
            amount;


        totalRead +=
            amount;
    }


    return totalRead;
}


qint64 EncryptedVideoDevice::writeData(
    const char*,
    qint64)
{
    return -1;
}


qint64 EncryptedVideoDevice::size() const
{
    return m_totalSize;
}


void EncryptedVideoDevice::close()
{
    QIODevice::close();


    m_videoCrypto.close();


    m_currentData.clear();


    m_currentDataPosition = 0;

    m_currentGlobalPosition = 0;

    m_totalSize = 0;
}


bool EncryptedVideoDevice::seek(
    qint64 pos)
{
    if (!isOpen())
        return false;


    if (pos < 0 ||
        pos > m_totalSize)
    {
        return false;
    }


    if (pos == m_totalSize)
    {
        m_currentGlobalPosition =
            pos;

        m_currentData.clear();

        m_currentDataPosition = 0;

        return QIODevice::seek(pos);
    }


    const qint64 CHUNK_SIZE =
        1024 * 1024;


    qint64 chunkIndex =
        pos / CHUNK_SIZE;


    qint64 offsetInsideChunk =
        pos % CHUNK_SIZE;


    if (chunkIndex >=
        m_videoCrypto.chunkCount())
    {
        return false;
    }


    if (!loadChunk(chunkIndex))
    {
        return false;
    }


    if (offsetInsideChunk >
        m_currentData.size())
    {
        return false;
    }


    m_currentDataPosition =
        offsetInsideChunk;


    m_currentGlobalPosition =
        pos;


    return QIODevice::seek(pos);
}