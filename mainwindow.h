#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <QLabel>
#include <QVideoWidget>
#include <QWidget>
#include <QMediaPlayer>
#include "Core/Crypto/EncryptedVideoDevice.h"
#include <QWebEngineView>
#include "Core/Player/YouTubePlayer.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class VideoPlayer;
class QVideoWidget;
class QPushButton;
class QSlider;
class QMenu;
class QToolButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool eventFilter(QObject *obj, QEvent *event) override;

    void stopVideo();

    void toggleYouTubePlayPause();

private:

    void setupLocalControls();

    void setupYouTubeControls();

    void updatePlaybackRateUi(
        double rate
        );

    void setPlaybackMode(bool useYouTube);

    void updateVideoLayout();

    Ui::MainWindow *ui;

    QWidget *volumePopupWindow;

    VideoPlayer *m_videoPlayer;

    QWebEngineView *m_youtubeView;

    YouTubePlayer *m_youtubePlayer;

    // YouTube is the active source. Set through setPlaybackMode(false)
    // when the preserved local/encrypted path is intentionally re-enabled.
    bool m_usingYouTube = true;

    bool m_youtubeIsPlaying = false;

    QString currentCourseId;

    QString formatTime(qint64 milliseconds);

    qint64 m_videoDuration = 0;

    bool m_isSeeking = false;

    bool m_isFullScreen = false;

    QVideoWidget *m_videoWidget;

    QTimer *controlsTimer;

    qint64 m_maxWatchedPosition = 0;

    qint64 m_pendingSeekPosition = 0;

    // Local controls stay available for a future local/encrypted mode.
    // They are hidden while YouTube is the primary player.
    QWidget *m_localControlBar = nullptr;
    QPushButton *m_localPlayButton = nullptr;
    QPushButton *m_localPauseButton = nullptr;
    QPushButton *m_localFullScreenButton = nullptr;
    QSlider *m_localProgressSlider = nullptr;
    QLabel *m_localTimeLabel = nullptr;
    QSlider *m_localVolumeSlider = nullptr;

    QToolButton *m_speedButton = nullptr;
    QMenu *m_speedMenu = nullptr;
    double m_playbackRate = 1.0;



protected:

    void resizeEvent(QResizeEvent *event) override;
};

#endif // MAINWINDOW_H
