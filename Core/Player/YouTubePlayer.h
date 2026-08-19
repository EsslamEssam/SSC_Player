#ifndef YOUTUBEPLAYER_H
#define YOUTUBEPLAYER_H

#include <QObject>
#include <QString>
#include <QUrl>


class QWebEngineView;
class QWebChannel;

class YouTubePlayer : public QObject
{
    Q_OBJECT

public:

    explicit YouTubePlayer(
        QWebEngineView *view,
        QObject *parent = nullptr
        );

    ~YouTubePlayer();

    void loadVideo(
        const QString &videoId
        );

    void play();

    void pause();

    void stop();

    void setPosition(
        qint64 position
        );

    void seekTo(
        int seconds
        );

    void setVolume(
        int volume
        );

    void setFullScreen(
        bool enabled
        );


    // --- الإضافة الجديدة هنا: استقبل التحديثات من JavaScript ---
public slots:
    void onJsReady();
    void onJsStateChanged(int state);
    void onJsPositionChanged(double seconds);
    void onJsDurationChanged(double seconds);
    void onJsError(const QString &message);


signals:

    void ready();

    void playing();

    void paused();

    void ended();

    void positionChanged(
        qint64 position
        );

    void durationChanged(
        qint64 duration
        );

    void error(
        const QString &message
        );


private:

    void loadPlayerPage();

    void runJavaScript(
        const QString &script
        );


private:

    QWebEngineView *m_view;

    QWebChannel *m_channel;

    QString m_videoId;

    bool m_pageLoaded;

    bool m_pendingPlayCommand;

    bool m_pendingPauseCommand;

    bool m_playerReady;
};

#endif // YOUTUBEPLAYER_H
