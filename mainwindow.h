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

    Ui::MainWindow *ui;

    QWidget *volumePopupWindow;

    VideoPlayer *m_videoPlayer;

    QWebEngineView *m_youtubeView;

    YouTubePlayer *m_youtubePlayer;

    bool m_usingYouTube = true;

    QString currentCourseId;

    QString formatTime(qint64 milliseconds);

    qint64 m_videoDuration = 0;

    bool m_isSeeking = false;

    bool m_isFullScreen = false;

    QVideoWidget *m_videoWidget;

    QTimer *controlsTimer;

    qint64 m_maxWatchedPosition = 0;

    qint64 m_pendingSeekPosition = 0;



protected:

    void resizeEvent(QResizeEvent *event) override;
};

#endif // MAINWINDOW_H
