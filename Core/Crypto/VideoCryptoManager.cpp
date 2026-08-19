#include "VideoCryptoManager.h"

#include "KeyManager.h"
#include "CryptoManager.h"

#include <QDataStream>
#include <QDebug>


VideoCryptoManager::VideoCryptoManager()
    : m_chunkSize(0)
    , m_currentChunk(0)
    , m_totalSize(0)
{
}


bool VideoCryptoManager::open(
    const QString& encryptedFile)
{
    close();


    m_file.setFileName(
        encryptedFile
        );


    if (!m_file.open(
            QIODevice::ReadOnly))
    {
        qDebug()
        << "Cannot open encrypted video:"
        << encryptedFile;

        return false;
    }


    QDataStream stream(&m_file);


    QByteArray magic;

    stream >> magic;


    if (magic != "SCPPVID")
    {
        qDebug()
        << "Invalid encrypted video format";

        close();

        return false;
    }


    quint32 version;

    stream >> version;


    if (version != 1)
    {
        qDebug()
        << "Unsupported video version:"
        << version;

        close();

        return false;
    }


    stream >> m_videoId;


    stream >> m_chunkSize;


    if (m_chunkSize <= 0)
    {
        qDebug()
        << "Invalid chunk size";

        close();

        return false;
    }


    m_key =
        KeyManager::loadVideoKey(
            m_videoId
            );


    qDebug() << "Video ID:" << m_videoId;
    qDebug() << "Video Key Size:" << m_key.size();



    if (m_key.size() != 32)
    {
        qDebug()
        << "Invalid video key";

        close();

        return false;
    }


    m_chunks.clear();

    m_totalSize = 0;


    while (!stream.atEnd())
    {
        ChunkInfo info;


        quint64 originalSize = 0;

        stream >> originalSize;


        if (stream.status() !=
            QDataStream::Ok)
        {
            break;
        }


        info.originalSize =
            static_cast<qint64>(
                originalSize
                );


        stream >> info.iv;


        if (stream.status() !=
            QDataStream::Ok)
        {
            break;
        }


        stream >> info.tag;


        if (stream.status() !=
            QDataStream::Ok)
        {
            break;
        }


        quint64 encryptedSize = 0;

        stream >> encryptedSize;


        if (stream.status() !=
            QDataStream::Ok)
        {
            break;
        }


        info.encryptedSize =
            static_cast<qint64>(
                encryptedSize
                );


        info.dataPosition =
            m_file.pos();


        if (info.encryptedSize < 0 ||
            info.originalSize < 0)
        {
            close();

            return false;
        }


        if (info.dataPosition +
                info.encryptedSize >
            m_file.size())
        {
            qDebug()
            << "Invalid chunk boundaries";

            close();

            return false;
        }


        m_chunks.append(info);


        m_totalSize +=
            info.originalSize;


        if (!m_file.seek(
                info.dataPosition +
                info.encryptedSize))
        {
            close();

            return false;
        }


        stream.device()->seek(
            m_file.pos()
            );
    }


    m_currentChunk = 0;


    if (m_chunks.isEmpty())
    {
        qDebug()
        << "No video chunks found";

        close();

        return false;
    }


    qDebug()
        << "Video opened:"
        << m_videoId;


    qDebug()
        << "Chunk size:"
        << m_chunkSize;


    qDebug()
        << "Chunk count:"
        << m_chunks.size();


    qDebug()
        << "Video total size:"
        << m_totalSize;


    return true;
}


QByteArray VideoCryptoManager::readChunk()
{
    if (!isOpen())
        return {};


    if (m_currentChunk >=
        m_chunks.size())
    {
        return {};
    }


    const ChunkInfo& info =
        m_chunks.at(
            static_cast<int>(
                m_currentChunk
                )
            );


    if (!m_file.seek(
            info.dataPosition))
    {
        qDebug()
        << "Cannot seek to chunk";

        return {};
    }


    QByteArray encryptedData =
        m_file.read(
            info.encryptedSize
            );


    if (encryptedData.size() !=
        info.encryptedSize)
    {
        qDebug()
        << "Cannot read encrypted chunk";

        return {};
    }


    QByteArray decrypted =
        CryptoManager::aesGCMDecrypt(
            encryptedData,
            m_key,
            info.iv,
            info.tag
            );


    qDebug()
        << "[SECURITY TEST] Chunk decrypted in RAM."
        << "Chunk:"
        << m_currentChunk
        << "Size:"
        << decrypted.size();


    if (decrypted.size() !=
        info.originalSize)
    {
        qDebug()
        << "Invalid decrypted chunk";

        return {};
    }


    ++m_currentChunk;


    return decrypted;
}


bool VideoCryptoManager::seekToChunk(
    qint64 chunkIndex)
{
    if (chunkIndex < 0 ||
        chunkIndex >= m_chunks.size())
    {
        return false;
    }


    m_currentChunk =
        chunkIndex;


    return true;
}


qint64 VideoCryptoManager::chunkCount() const
{
    return m_chunks.size();
}


qint64 VideoCryptoManager::currentChunk() const
{
    return m_currentChunk;
}


qint64 VideoCryptoManager::chunkSize(
    qint64 chunkIndex) const
{
    if (chunkIndex < 0 ||
        chunkIndex >= m_chunks.size())
    {
        return 0;
    }


    return m_chunks.at(
                       static_cast<int>(
                           chunkIndex
                           )
                       ).originalSize;
}


qint64 VideoCryptoManager::totalSize() const
{
    return m_totalSize;
}


bool VideoCryptoManager::isOpen() const
{
    return m_file.isOpen();
}


void VideoCryptoManager::close()
{
    if (m_file.isOpen())
    {
        m_file.close();
    }


    m_key.clear();

    m_videoId.clear();

    m_chunkSize = 0;

    m_currentChunk = 0;

    m_totalSize = 0;

    m_chunks.clear();
}