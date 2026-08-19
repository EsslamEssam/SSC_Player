#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>

#include <QIODevice>
#include <QUrl>

#include "../Crypto/EncryptedVideoDevice.h"

class QVideoWidget;

class VideoPlayer : public QObject
{
    Q_OBJECT

public:

    explicit VideoPlayer(QObject *parent = nullptr);

    ~VideoPlayer();

    void setVideoOutput(QVideoWidget *videoWidget);

    void play();
    void pause();
    void stop();

    void setSource(const QString &filePath);

    void setEncryptedSource(
        QIODevice *device,
        const QUrl &sourceUrl
        );

    qint64 currentPosition() const;
    qint64 duration() const;

    void setPosition(qint64 position);

    void setVolume(float volume);
    float volume() const;

    bool isPlaying() const;


signals:

    void videoStarted();
    void videoPaused();
    void videoStopped();
    void videoFinished();
    void videoReady();

    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);


private:

    QMediaPlayer *m_player;

    QAudioOutput *m_audioOutput;

    QVideoWidget* m_videoWidget;

    EncryptedVideoDevice* m_encryptedDevice;

    EncryptedVideoDevice* m_pendingEncryptedDevice;
    QUrl m_pendingSourceUrl;

};

#endif // VIDEOPLAYER_H