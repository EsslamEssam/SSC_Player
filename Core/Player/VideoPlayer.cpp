#include "VideoPlayer.h"
#include "../Crypto/EncryptedVideoDevice.h"

#include <QVideoWidget>
#include <QUrl>
#include <QIODevice>
#include <QDebug>


VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent)
    , m_player(nullptr)
    , m_audioOutput(nullptr)
    , m_videoWidget(nullptr)
    , m_encryptedDevice(nullptr)
    , m_pendingEncryptedDevice(nullptr)
{
    m_audioOutput = new QAudioOutput(this);

    m_player = new QMediaPlayer(this);


    //اختبار قرائة MP4
    qDebug() << "[MEDIA TEST] QMediaPlayer created:"
             << m_player;

    qDebug() << "[MEDIA TEST] QMediaPlayer error:"
             << m_player->error();

    qDebug() << "[MEDIA TEST] QMediaPlayer mediaStatus:"
             << m_player->mediaStatus();
    //اختبار قرائة MP4


    m_player->setAudioOutput(m_audioOutput);


    connect(m_player, &QMediaPlayer::positionChanged,
            this,
            [this](qint64 position)
            {
                emit positionChanged(position);
            });


    connect(m_player, &QMediaPlayer::durationChanged,
            this,
            [this](qint64 duration)
            {
                emit durationChanged(duration);
            });


    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this,
            [this](QMediaPlayer::PlaybackState state)
            {
                if (state == QMediaPlayer::PlayingState)
                {
                    emit videoStarted();
                }
                else if (state == QMediaPlayer::PausedState)
                {
                    emit videoPaused();
                }
                else if (state == QMediaPlayer::StoppedState)
                {
                    emit videoStopped();
                }
            });

//اختبار قرائة mp4
    connect(
        m_player,
        &QMediaPlayer::errorOccurred,
        this,
        [](QMediaPlayer::Error error,
           const QString& errorString)
        {
            qDebug()
            << "[MEDIA TEST] QMediaPlayer ERROR:"
            << error
            << errorString;
        }
        );

    connect(
        m_player,
        &QMediaPlayer::mediaStatusChanged,
        this,
        [](QMediaPlayer::MediaStatus status)
        {
            qDebug()
            << "[MEDIA TEST] Media Status:"
            << status;
        }
        );
    //نهايته


    connect(
        m_player,
        &QMediaPlayer::mediaStatusChanged,
        this,
        [this](QMediaPlayer::MediaStatus status)
        {
            qDebug()
            << "[ENCRYPTED PLAYER] Media status:"
            << status;


            if (status == QMediaPlayer::LoadedMedia)
            {
                emit videoReady();
            }


            if (status == QMediaPlayer::EndOfMedia)
            {
                emit videoFinished();
            }


            /*
         * The old media source has now been released.
         * It is finally safe to delete the old device
         * and attach the pending device.
         */

            if (status == QMediaPlayer::NoMedia &&
                m_pendingEncryptedDevice)
            {
                qDebug()
                << "[ENCRYPTED PLAYER]"
                << "Old media released.";

                /*
             * Delete old device.
             */

                if (m_encryptedDevice)
                {
                    qDebug()
                    << "[ENCRYPTED PLAYER]"
                    << "Deleting old encrypted device:"
                    << m_encryptedDevice;

                    m_encryptedDevice->close();

                    m_encryptedDevice->deleteLater();

                    m_encryptedDevice =
                        nullptr;
                }


                /*
             * Move pending device to active device.
             */

                m_encryptedDevice =
                    m_pendingEncryptedDevice;

                m_pendingEncryptedDevice =
                    nullptr;


                QUrl newUrl =
                    m_pendingSourceUrl;

                m_pendingSourceUrl =
                    QUrl();


                qDebug()
                    << "[ENCRYPTED PLAYER]"
                    << "Attaching new encrypted device:"
                    << m_encryptedDevice;


                m_player->setSourceDevice(
                    m_encryptedDevice,
                    newUrl
                    );


                qDebug()
                    << "[ENCRYPTED PLAYER]"
                    << "New encrypted source assigned.";
            }
        });
}


VideoPlayer::~VideoPlayer()
{
    if (m_player)
    {
        m_player->stop();
        m_player->setSource(QUrl());
    }


    if (m_encryptedDevice)
    {
        m_encryptedDevice->close();
        delete m_encryptedDevice;
        m_encryptedDevice = nullptr;
    }


    if (m_pendingEncryptedDevice)
    {
        m_pendingEncryptedDevice->close();
        delete m_pendingEncryptedDevice;
        m_pendingEncryptedDevice = nullptr;
    }
}


void VideoPlayer::setVideoOutput(QVideoWidget *videoWidget)
{
    m_player->setVideoOutput(videoWidget);
}


void VideoPlayer::play()
{
    m_player->play();
}


void VideoPlayer::pause()
{
    m_player->pause();
}


void VideoPlayer::stop()
{
    m_player->stop();
}


void VideoPlayer::setSource(const QString &filePath)
{
    qDebug() << "[MEDIA TEST] setSource - 1";

    QUrl url = QUrl::fromLocalFile(filePath);

    qDebug() << "[MEDIA TEST] setSource - 2 URL:"
             << url;

    qDebug() << "[MEDIA TEST] setSource - 3 before QMediaPlayer::setSource";

    m_player->setSource(url);

    qDebug() << "[MEDIA TEST] setSource - 4 after QMediaPlayer::setSource";
}



void VideoPlayer::setEncryptedSource(
    QIODevice *device,
    const QUrl &sourceUrl)
{
    qDebug()
    << "[ENCRYPTED PLAYER] ===== setEncryptedSource =====";

    if (!device)
    {
        qDebug()
        << "[ENCRYPTED PLAYER] Device is NULL";

        return;
    }

    EncryptedVideoDevice *encryptedDevice =
        qobject_cast<EncryptedVideoDevice*>(device);

    if (!encryptedDevice)
    {
        qDebug()
        << "[ENCRYPTED PLAYER]"
        << "Device is not EncryptedVideoDevice.";

        return;
    }

    if (!encryptedDevice->isOpen() ||
        !encryptedDevice->isReadable())
    {
        qDebug()
        << "[ENCRYPTED PLAYER]"
        << "Device must be open and readable.";

        return;
    }

    qDebug()
        << "[ENCRYPTED PLAYER] New device:"
        << encryptedDevice;

    /*
     * If another encrypted video is currently loaded,
     * do NOT delete its device immediately.
     *
     * QMediaPlayer is asynchronous and may still be
     * using the old device.
     */

    if (m_encryptedDevice)
    {
        qDebug()
        << "[ENCRYPTED PLAYER]"
        << "Previous encrypted device exists.";

        /*
         * Store the new device temporarily.
         */
        m_pendingEncryptedDevice =
            encryptedDevice;

        m_pendingSourceUrl =
            sourceUrl;

        /*
         * Stop current playback.
         */
        m_player->stop();

        /*
         * Clear current media source.
         *
         * We wait for NoMedia before deleting
         * the old encrypted device.
         */
        m_player->setSource(QUrl());

        qDebug()
            << "[ENCRYPTED PLAYER]"
            << "Waiting for old media to be released.";

        return;
    }

    /*
     * No previous encrypted device.
     * We can use the new one immediately.
     */

    m_encryptedDevice =
        encryptedDevice;

    qDebug()
        << "[ENCRYPTED PLAYER]"
        << "Encrypted device stored:"
        << m_encryptedDevice;

    m_player->setSourceDevice(
        m_encryptedDevice,
        sourceUrl
        );

    qDebug()
        << "[ENCRYPTED PLAYER]"
        << "New encrypted source assigned.";
}




qint64 VideoPlayer::currentPosition() const
{
    return m_player->position();
}


qint64 VideoPlayer::duration() const
{
    return m_player->duration();
}


void VideoPlayer::setPosition(qint64 position)
{
    m_player->setPosition(position);
}


void VideoPlayer::setVolume(float volume)
{
    m_audioOutput->setVolume(volume);
}


float VideoPlayer::volume() const
{
    return m_audioOutput->volume();
}


bool VideoPlayer::isPlaying() const
{
    return m_player->playbackState()
    == QMediaPlayer::PlayingState;
}