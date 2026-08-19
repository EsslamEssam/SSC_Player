#ifndef VIDEOCRYPTOMANAGER_H
#define VIDEOCRYPTOMANAGER_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QVector>

class VideoCryptoManager
{
public:

    VideoCryptoManager();

    bool open(
        const QString& encryptedFile
        );

    QByteArray readChunk();

    bool seekToChunk(
        qint64 chunkIndex
        );

    qint64 chunkCount() const;

    qint64 currentChunk() const;

    qint64 chunkSize(
        qint64 chunkIndex
        ) const;

    qint64 totalSize() const;

    bool isOpen() const;

    void close();


private:

    struct ChunkInfo
    {
        qint64 dataPosition;
        qint64 encryptedSize;
        qint64 originalSize;

        QByteArray iv;
        QByteArray tag;
    };


    QFile m_file;

    QByteArray m_key;

    QString m_videoId;

    qint64 m_chunkSize;

    qint64 m_currentChunk;

    qint64 m_totalSize;

    QVector<ChunkInfo> m_chunks;
};

#endif