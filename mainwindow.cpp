#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QHBoxLayout>
#include <QLayout>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <utility>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QTime>
#include <QFont>
#include <QRandomGenerator>
#include <QPoint>
#include <QMediaPlayer>
#include <QBuffer>
#include <QAbstractButton>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QPushButton>
#include <QRegularExpression>
#include <QSlider>
#include <QToolButton>

#include "Core/Managers/CourseManager.h"
#include "Core/Managers/LessonManager.h"
#include "Core/Models/Lesson.h"
#include "Core/Managers/VideoManager.h"
#include "Core/Player/VideoPlayer.h"
#include "Core/Progress/ProgressManager.h"
#include "Core/Progress/LessonProgress.h"
#include "Core/Crypto/CryptoManager.h"
#include "Core/Crypto/KeyManager.h"
#include "Core/Crypto/EncryptionManager.h"
#include "Core/Crypto/VideoCryptoManager.h"
#include "Core/Crypto/EncryptedVideoDevice.h"
#include "Core/Offline/OfflineCacheManager.h"


namespace
{
// Temporary test video used by every lesson while YouTube playback is being verified.
constexpr auto kDefaultYouTubeVideoId = "clR1RITS18E";
constexpr bool kUseSingleYouTubeTestVideo = true;

QString youtubeVideoIdFromValue(const QString &value)
{
    const QString trimmedValue = value.trimmed();

    const QRegularExpression idPattern(
        QStringLiteral("^[A-Za-z0-9_-]{11}$")
        );

    if (idPattern.match(trimmedValue).hasMatch())
    {
        return trimmedValue;
    }

    const QRegularExpression urlPattern(
        QStringLiteral(
            "(?:v=|youtu\\.be/|youtube(?:-nocookie)?\\.com/"
            "(?:embed/|shorts/))([A-Za-z0-9_-]{11})"
            )
        );

    const QRegularExpressionMatch match =
        urlPattern.match(trimmedValue);

    if (match.hasMatch())
    {
        return match.captured(1);
    }

    // Existing lesson records still contain local/encrypted identifiers.
    // Keep the current test video as a safe YouTube fallback until each
    // lesson receives its real YouTube ID.
    return QString::fromLatin1(kDefaultYouTubeVideoId);
}

QIcon makePlayIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    QPolygonF triangle;
    triangle << QPointF(10.0, 7.0)
             << QPointF(24.0, 16.0)
             << QPointF(10.0, 25.0);

    painter.drawPolygon(triangle);
    return QIcon(pixmap);
}

QIcon makePauseIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(8.0, 7.0, 6.0, 18.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(18.0, 7.0, 6.0, 18.0), 2.0, 2.0);

    return QIcon(pixmap);
}

QIcon makeVolumeIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(color);

    QPainterPath speaker;
    speaker.moveTo(7.0, 13.0);
    speaker.lineTo(12.0, 13.0);
    speaker.lineTo(18.0, 8.0);
    speaker.lineTo(18.0, 24.0);
    speaker.lineTo(12.0, 19.0);
    speaker.lineTo(7.0, 19.0);
    speaker.closeSubpath();
    painter.drawPath(speaker);

    painter.setBrush(Qt::NoBrush);
    painter.drawArc(QRectF(15.0, 10.0, 13.0, 12.0), -55 * 16, 110 * 16);

    return QIcon(pixmap);
}

QIcon makeFullscreenIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter.drawLine(QPointF(7.0, 12.0), QPointF(7.0, 7.0));
    painter.drawLine(QPointF(7.0, 7.0), QPointF(12.0, 7.0));
    painter.drawLine(QPointF(20.0, 7.0), QPointF(25.0, 7.0));
    painter.drawLine(QPointF(25.0, 7.0), QPointF(25.0, 12.0));
    painter.drawLine(QPointF(7.0, 20.0), QPointF(7.0, 25.0));
    painter.drawLine(QPointF(7.0, 25.0), QPointF(12.0, 25.0));
    painter.drawLine(QPointF(20.0, 25.0), QPointF(25.0, 25.0));
    painter.drawLine(QPointF(25.0, 25.0), QPointF(25.0, 20.0));

    return QIcon(pixmap);
}

QIcon makeSpeedIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QRectF(6.0, 6.0, 20.0, 20.0));
    painter.drawLine(QPointF(16.0, 16.0), QPointF(16.0, 10.0));
    painter.drawLine(QPointF(16.0, 16.0), QPointF(21.0, 19.0));

    QPolygonF arrow;
    arrow << QPointF(22.0, 7.0)
          << QPointF(27.0, 8.0)
          << QPointF(24.0, 12.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawPolygon(arrow);

    return QIcon(pixmap);
}

void styleControlButton(
    QAbstractButton *button,
    const QIcon &icon,
    const QString &toolTip)
{
    if (!button)
    {
        return;
    }

    button->setText(QString());
    button->setIcon(icon);
    button->setIconSize(QSize(21, 21));
    button->setToolTip(toolTip);
    button->setAccessibleName(toolTip);
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::StrongFocus);
    button->setStyleSheet(
        "QPushButton { background: #202b38; color: #eef4fb;"
        "border: 1px solid #344454; border-radius: 8px; padding: 0; }"
        "QPushButton:hover { background: #2b3b4d; border-color: #4d6982; }"
        "QPushButton:pressed { background: #17212c; }"
        "QPushButton:focus { border: 1px solid #72b7ff; }"
        );
}

QString formatPlaybackRate(double rate)
{
    return QString::number(rate, 'g', 3) + QStringLiteral("×");
}
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_videoWidget(nullptr)
{
    ui->setupUi(this);

    //يوتيوب
    m_youtubeView =
        new QWebEngineView(
            ui->videoFrame
            );

    m_youtubeView->setGeometry(
        ui->videoDisplay->geometry()
        );

    m_youtubeView->hide();

    m_youtubeView->installEventFilter(this);

    m_youtubePlayer =
        new YouTubePlayer(
            m_youtubeView,
            this
            );



    //Gemini
    connect(
        m_youtubePlayer,
        &YouTubePlayer::ready,
        this,
        [this]()
        {
            qDebug() << "[YOUTUBE TEST]" << "YouTube player is ready.";

            // إخفاء شاشة الفيديو المحلي
            ui->videoDisplay->show();

            setPlaybackMode(true);
            m_youtubeIsPlaying = false;

            if (m_videoWidget)
            {
                m_videoWidget->hide();
            }

            // عند تشغيل يوتيوب:

            // ضبط قياسات الـ View
            m_youtubeView->setGeometry(
                ui->videoDisplay->geometry()
                );

            m_youtubeView->show();
            m_youtubeView->raise();

            // -------------------------------------------------------------
            // 👇 إنشاء وتحديد حجم الـ Overlay الشفاف داخل m_youtubeView نفسه
            // وليس داخل videoFrame حتى لا يغطي أزرار التفاعل الخارجية
            // -------------------------------------------------------------
            // إنشاء الطبقة الشفافة فوق يوتيوب داخل videoDisplay لمنع الضغط على شعارات يوتيوب
            QWidget *overlay = m_youtubeView->findChild<QWidget*>("youtubeOverlay");
            if (!overlay) {
                overlay = new QWidget(m_youtubeView);
                overlay->setObjectName("youtubeOverlay");
            }
            overlay->installEventFilter(this);
            overlay->setStyleSheet("background: transparent;");
            overlay->setGeometry(0, 0, m_youtubeView->width(), m_youtubeView->height());
            overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            overlay->raise();
            overlay->show();

            // Keep the native YouTube controls above the WebEngine view after
            // the view/overlay is recreated or raised.
            ui->controlBar->raise();
            // -------------------------------------------------------------

            // تشغيل الفيديو المطلوب
            m_youtubePlayer->loadVideo(
                QString::fromLatin1(kDefaultYouTubeVideoId)
                );
        }
        );


    //Gemini


    connect(
        m_youtubePlayer,
        &YouTubePlayer::error,
        this,
        [](const QString &message)
        {
            qDebug()
            << "[YOUTUBE TEST ERROR]"
            << message;
        }
        );

    //يوتيوب


    // ================================================================
    // LOCAL / ENCRYPTED PATH (parked while YouTube is primary)
    // ================================================================
    // This block is intentionally kept intact for a future local mode.
    if (!m_usingYouTube)
    {
        if (!QFile::exists("D:/SCCP_Player/TestVideos/test.enc"))
        {
            bool videoEncrypted =
                EncryptionManager::encryptVideoFile(
                    "D:/SCCP_Player/TestVideos/test.mp4",
                    "D:/SCCP_Player/TestVideos/test.enc",
                    "lesson_001"
                    );

            qDebug()
                << "Video Encrypt:"
                << videoEncrypted;
        }
        else
        {
            qDebug()
                << "Encrypted video already exists. Skipping encryption.";
        }

        QString testLessonId =
            "offline_test";

        QString encryptedSource =
            "D:/SCCP_Player/TestVideos/test.enc";

        bool offlineReady =
            OfflineCacheManager::prepareLesson(
                testLessonId,
                encryptedSource
                );

        qDebug()
            << "Offline Ready:"
            << offlineReady;

        QString offlinePath =
            OfflineCacheManager::lessonFilePath(
                testLessonId
                );

        qDebug()
            << "Offline Path:"
            << offlinePath;

        if (offlineReady)
        {
            qDebug()
                << "Offline video is valid.";

            EncryptedVideoDevice* device =
                new EncryptedVideoDevice(this);

            bool opened =
                device->openEncryptedVideo(
                    offlinePath
                    );

            qDebug()
                << "Offline Encrypted Device Open:"
                << opened;

            if (opened)
            {
                qDebug()
                    << "Offline Video Size:"
                    << device->size();

                QByteArray testData =
                    device->read(1024);

                qDebug()
                    << "Offline Read Size:"
                    << testData.size();

                bool seeked =
                    device->seek(0);

                qDebug()
                    << "Offline Seek:"
                    << seeked;
            }
        }
        else
        {
            qDebug()
                << "Offline video is not available.";
        }
    }



#ifdef Q_OS_WIN

    HWND hwnd = (HWND)winId();

    SetWindowDisplayAffinity(
        hwnd,
        WDA_EXCLUDEFROMCAPTURE
        );

#endif


    this->setMouseTracking(true);

    // Capture video keyboard shortcuts regardless of which control currently
    // owns focus (button, slider, WebEngine view, or one of its children).
    qApp->installEventFilter(this);

    ui->videoDisplay->setGeometry(
        ui->videoFrame->rect()
        );

    ui->controlBar->setParent(ui->videoFrame);

    ui->controlBar->raise();


    controlsTimer = new QTimer(this);

    controlsTimer->setSingleShot(true);

    connect(controlsTimer,
            &QTimer::timeout,
            this,
            [=]()
            {
                if(m_isFullScreen)
                {
                    ui->controlBar->hide();
                }
            });
    volumePopupWindow = new QWidget(this);

    volumePopupWindow->setWindowFlags(
        Qt::Popup | Qt::FramelessWindowHint
        );


    QVBoxLayout *volumeLayout = new QVBoxLayout(volumePopupWindow);

    volumeLayout->setContentsMargins(8, 8, 8, 8);

    volumeLayout->addWidget(ui->volumeSlider);


    volumePopupWindow->setLayout(volumeLayout);


    volumePopupWindow->hide();


    connect(ui->volumeButton,
            &QPushButton::clicked,
            this,
            [=]()
            {
                // حساب مكان زر الصوت على الشاشة
                QPoint pos =
                    ui->volumeButton->mapToGlobal(
                        QPoint(
                            0,
                            -volumePopupWindow->height() - 10
                            )
                        );


                // حدود نافذة البرنامج
                QRect windowRect = this->geometry();


                // لو خرج من فوق النافذة يظهر تحت الزر
                if (pos.y() < windowRect.top())
                {
                    pos.setY(
                        ui->volumeButton->mapToGlobal(
                                            QPoint(
                                                0,
                                                ui->volumeButton->height()
                                                )
                                            ).y()
                        );
                }


                // لو خرج من ناحية اليمين يرجعه للداخل
                if (pos.x() + volumePopupWindow->width() > windowRect.right())
                {
                    pos.setX(
                        windowRect.right()
                        - volumePopupWindow->width()
                        );
                }


                volumePopupWindow->move(pos);


                volumePopupWindow->setVisible(
                    !volumePopupWindow->isVisible()
                    );
            });
    // Video Player setup

    m_videoPlayer = new VideoPlayer(this);

    // The local controls are kept separate and parked while YouTube is primary.
    setupLocalControls();
    setupYouTubeControls();

    connect(
        ui->playButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!m_usingYouTube || !m_youtubePlayer)
                return;

            m_youtubePlayer->play();
        }
        );


    connect(
        ui->pauseButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!m_usingYouTube || !m_youtubePlayer)
                return;

            m_youtubePlayer->pause();
        }
        );


    connect(ui->fullScreenButton,
            &QPushButton::clicked,
            this,
            [=]()
            {
                QHBoxLayout *mainLayout =
                    qobject_cast<QHBoxLayout*>(
                        ui->centralwidget->layout()
                        );


                if (!m_isFullScreen)
                {
                    ui->coursePanel->hide();


                    if(mainLayout)
                    {
                        mainLayout->setContentsMargins(0,0,0,0);
                        mainLayout->setSpacing(0);

                        mainLayout->setStretch(0,0);
                        mainLayout->setStretch(1,1);
                    }


                    this->showFullScreen();


                    ui->controlBar->show();
                    ui->controlBar->raise();


                    m_isFullScreen = true;
                }
                else
                {
                    this->showNormal();


                    ui->coursePanel->show();


                    if(mainLayout)
                    {
                        mainLayout->setStretch(0,1);
                        mainLayout->setStretch(1,1);

                        mainLayout->setContentsMargins(0,0,0,0);
                        mainLayout->setSpacing(0);
                    }


                    m_isFullScreen = false;
                }

                // Let Qt finish the window/layout transition first, then
                // resize the video and its overlay from the final geometry.
                QTimer::singleShot(
                    0,
                    this,
                    [this]()
                    {
                        updateVideoLayout();
                    }
                    );
            });

    connect(
        ui->volumeSlider,
        &QSlider::valueChanged,
        this,
        [this](int value)
        {
            if (!m_usingYouTube || !m_youtubePlayer)
                return;

            m_youtubePlayer->setVolume(value);
        }
        );


    connect(m_videoPlayer,
            &VideoPlayer::durationChanged,
            this,
            [=](qint64 duration)
            {
                if (m_usingYouTube)
                    return;

                if (m_localProgressSlider)
                {
                    m_localProgressSlider->setMinimum(0);
                    m_localProgressSlider->setMaximum(
                        static_cast<int>(duration)
                        );
                }
            });


    connect(
        m_youtubePlayer,
        &YouTubePlayer::durationChanged,
        this,
        [this](qint64 duration)
        {
            if (!m_usingYouTube)
                return;

            ui->progressSlider->setMinimum(0);
            ui->progressSlider->setMaximum(
                static_cast<int>(duration)
                );

            m_videoDuration = duration;

            ui->timeLabel->setText(
                "00:00 / "
                + formatTime(duration)
                );

            if (m_pendingSeekPosition > 0)
            {
                const qint64 resumePosition =
                    qMin(m_pendingSeekPosition, duration);

                m_pendingSeekPosition = 0;

                if (resumePosition > 0)
                {
                    m_youtubePlayer->setPosition(resumePosition);
                }
            }
        }
        );


    connect(m_videoPlayer,
            &VideoPlayer::positionChanged,
            this,
            [=](qint64 position)
            {
                if (m_usingYouTube)
                    return;

                if (!m_isSeeking)
                {
                    if (m_localProgressSlider)
                    {
                        m_localProgressSlider->setValue(
                            static_cast<int>(position)
                            );
                    }
                }


                if(position > m_maxWatchedPosition)
                {
                    m_maxWatchedPosition = position;
                }


                qDebug()
                    << "PlayerPosition:"
                    << position
                    << "MaxWatched:"
                    << m_maxWatchedPosition;


                ProgressManager::instance()
                    .updateProgress(
                        m_maxWatchedPosition,
                        m_videoPlayer->duration()
                        );
            });

        //YouTube تحكمات
    connect(
        m_youtubePlayer,
        &YouTubePlayer::positionChanged,
        this,
        [this](qint64 position)
        {
            if (!m_usingYouTube)
                return;

            if (!m_isSeeking)
            {
                ui->progressSlider->setValue(
                    static_cast<int>(position)
                    );
            }

            ui->timeLabel->setText(
                formatTime(position)
                + " / "
                + formatTime(m_videoDuration)
                );

            if (position > m_maxWatchedPosition)
            {
                m_maxWatchedPosition = position;
            }

            if (m_videoDuration > 0)
            {
                ProgressManager::instance()
                    .updateProgress(
                        m_maxWatchedPosition,
                        m_videoDuration
                        );
            }
        }
        );

    connect(
        m_youtubePlayer,
        &YouTubePlayer::playing,
        this,
        [this]()
        {
            if (!m_usingYouTube)
                return;

            m_youtubeIsPlaying = true;

            qDebug()
                << "[YOUTUBE CONTROL]"
                << "Playing";
        }
        );


    connect(
        m_youtubePlayer,
        &YouTubePlayer::paused,
        this,
        [this]()
        {
            if (!m_usingYouTube)
                return;

            m_youtubeIsPlaying = false;

            qDebug()
                << "[YOUTUBE CONTROL]"
                << "Paused";
        }
        );


    connect(
        m_youtubePlayer,
        &YouTubePlayer::ended,
        this,
        [this]()
        {
            if (!m_usingYouTube)
                return;

            m_youtubeIsPlaying = false;

            qDebug()
                << "[YOUTUBE CONTROL]"
                << "Ended";
        }
        );
        //YouTube تحكمات


    connect(ui->progressSlider,
            &QSlider::sliderMoved,
            this,
            [=](int position)
            {
                if (!m_usingYouTube)
                    return;

                qDebug()
                << "Slider to:"
                << position
                << "Max:"
                << m_maxWatchedPosition;

                if(position > m_maxWatchedPosition)
                {
                    ui->progressSlider->setValue(
                        m_maxWatchedPosition
                        );


                    return;
                }
            });

    connect(ui->progressSlider,
            &QSlider::sliderPressed,
            this,
            [=]()
            {
                if (!m_usingYouTube)
                    return;

                m_isSeeking = true;
            });


    connect(
        ui->progressSlider,
        &QSlider::sliderReleased,
        this,
        [this]()
        {
            m_isSeeking = false;

            if (!m_usingYouTube)
                return;

            qint64 position =
                ui->progressSlider->value();

            if (position > m_maxWatchedPosition)
            {
                position = m_maxWatchedPosition;

                ui->progressSlider->setValue(
                    static_cast<int>(position)
                    );
            }

            if (m_youtubePlayer)
            {
                m_youtubePlayer->setPosition(position);
            }
        });


    // ================================================================
    // LOCAL CONTROLS (kept separate; hidden while YouTube is primary)
    // ================================================================
    connect(m_localProgressSlider,
            &QSlider::sliderMoved,
            this,
            [this](int position)
            {
                if (m_usingYouTube)
                    return;

                if (position > m_maxWatchedPosition)
                {
                    m_localProgressSlider->setValue(
                        static_cast<int>(m_maxWatchedPosition)
                        );
                }
            });

    connect(m_localProgressSlider,
            &QSlider::sliderPressed,
            this,
            [this]()
            {
                if (!m_usingYouTube)
                {
                    m_isSeeking = true;
                }
            });

    connect(m_localProgressSlider,
            &QSlider::sliderReleased,
            this,
            [this]()
            {
                m_isSeeking = false;

                if (m_usingYouTube || !m_videoPlayer)
                    return;

                qint64 position =
                    m_localProgressSlider->value();

                if (position > m_maxWatchedPosition)
                {
                    position = m_maxWatchedPosition;

                    m_localProgressSlider->setValue(
                        static_cast<int>(position)
                        );
                }

                m_videoPlayer->setPosition(position);
            });


    connect(m_videoPlayer,
            &VideoPlayer::durationChanged,
            this,
            [=](qint64 duration)
            {
                if (m_usingYouTube)
                    return;

                m_videoDuration = duration;

                if (m_localTimeLabel)
                {
                    m_localTimeLabel->setText(
                        "00:00 / " + formatTime(duration)
                        );
                }
            });


    connect(m_videoPlayer,
            &VideoPlayer::positionChanged,
            this,
            [=](qint64 position)
            {
                if (m_usingYouTube)
                    return;

                if (m_localTimeLabel)
                {
                    m_localTimeLabel->setText(
                        formatTime(position)
                        + " / "
                        + formatTime(m_videoDuration)
                        );
                }
            });


    connect(m_videoPlayer,
            &VideoPlayer::videoReady,
            this,
            [=]()
            {
                if (m_usingYouTube)
                    return;

                if(m_pendingSeekPosition > 0)
                {
                    qDebug()
                    << "Seeking to:"
                    << m_pendingSeekPosition;

                    m_videoPlayer->setPosition(
                        m_pendingSeekPosition
                        );

                    m_pendingSeekPosition = 0;
                }

                m_videoPlayer->play();
            });


    QVBoxLayout *layout =
        qobject_cast<QVBoxLayout*>(ui->videoDisplay->layout());


    if (layout)
    {
        layout->setContentsMargins(0, 0, 0, 0);

        m_videoWidget = new QVideoWidget(ui->videoDisplay);

        m_videoWidget->setAspectRatioMode(
            Qt::KeepAspectRatio
            );
        m_videoWidget->setMouseTracking(true);

        m_videoWidget->installEventFilter(this);

        layout->addWidget(m_videoWidget);

        m_videoPlayer->setVideoOutput(m_videoWidget);

        if (m_usingYouTube)
        {
            m_videoWidget->hide();
        }

    }

    // User info
    ui->userInfoLabel->setText("User: Guest\nLicense: Not Activated");


    // Load courses from CourseManager

const QList<Course>& courses = CourseManager::instance().getCourses();

    for (const Course &course : courses)
    {
        QString status = course.isActivated()
        ? "Activated"
        : "Not Activated";

        QString icon = course.isActivated()
                           ? "📘"
                           : "🔒";

        QListWidgetItem *item =
            new QListWidgetItem(
                icon + "  " + course.getName()
                + "\n     " + status
                );

        item->setData(Qt::UserRole, course.getId());

        ui->courseList->addItem(item);
    }


    // Course selection test
    connect(ui->courseList, &QListWidget::itemClicked,
            this, [=](QListWidgetItem *item)
            {
                QString courseID = item->data(Qt::UserRole).toString();

                currentCourseId = courseID;

                Course *course =
                    CourseManager::instance().getCourseById(courseID);

                if (!course)
                    return;

                bool activated = course->isActivated();


                // Change page
                ui->mainStack->setCurrentWidget(ui->courseDetailsPage);


                // Set course information from Course object

                ui->courseNameLabel->setText(course->getName());


                ui->courseDescription->setText(
                    course->getDescription()
                    );


                // License status
                if (activated)
                {
                    ui->activationStatusLabel->setText("✓ Activated");
                    ui->activationStatusLabel->setStyleSheet(
                        "QLabel { color: #00C853; font-weight: bold; }"
                        );

                    ui->startCourseButton->setVisible(true);
                    ui->contactButton->setVisible(false);
                }
                else
                {
                    ui->activationStatusLabel->setText("🔒 Not Activated");
                    ui->activationStatusLabel->setStyleSheet(
                        "QLabel { color: #FF9800; font-weight: bold; }"
                        );

                    ui->startCourseButton->setVisible(false);
                    ui->contactButton->setVisible(true);
                }
            });

    connect(ui->backToCoursesButton, &QPushButton::clicked,
            this, [=]()
            {
                ui->mainStack->setCurrentWidget(ui->coursesPage);
            });

    connect(ui->backToDetailsButton, &QPushButton::clicked,
            this, [=]()
            {
                ui->mainStack->setCurrentWidget(ui->courseDetailsPage);
            });

    connect(ui->startCourseButton, &QPushButton::clicked,
            this, [=]()
            {

                ui->lessonList->clear();


                QList<Lesson> lessons =
                    LessonManager::instance()
                        .getLessonsByCourseId(currentCourseId);


                for (const Lesson &lesson : std::as_const(lessons))
                {

                    bool completed =
                        ProgressManager::instance()
                            .isCompleted(
                                lesson.getId()
                                );


                    bool unlocked =
                        LessonManager::instance()
                            .isLessonUnlocked(
                                lesson.getId()
                                );



                    QString icon;


                    if(completed)
                    {
                        icon = "✅";
                    }
                    else if(unlocked)
                    {
                        icon = "▶";
                    }
                    else
                    {
                        icon = "🔒";
                    }



                    QListWidgetItem *item =
                        new QListWidgetItem(
                            icon
                            + "  "
                            + lesson.getTitle()
                            + "\n     "
                            + lesson.getDuration()
                            );


                    item->setData(
                        Qt::UserRole,
                        lesson.getId()
                        );


                    item->setData(
                        Qt::UserRole + 1,
                        unlocked
                        );


                    ui->lessonList->addItem(item);
                }




                ui->mainStack->setCurrentWidget(
                    ui->lessonsPage
                    );

            });

    // ⬇️ ده الكود الجديد اللي هتشغل بيه كليك على الدروس عشان تشغل الفيديو
    connect(ui->lessonList, &QListWidget::itemClicked, this, [=](QListWidgetItem *item) {
        QString lessonId = item->data(Qt::UserRole).toString();
        bool isUnlocked = item->data(Qt::UserRole + 1).toBool();

        if (!isUnlocked) {
            QMessageBox::warning(this, "Lesson Locked", "Please complete the previous lessons first to unlock this lesson.");
            return;
        }

        const Lesson *lesson = LessonManager::instance().getLessonById(lessonId);
        if (!lesson) return;

        ProgressManager::instance().setCurrentLesson(lessonId);
        LessonProgress prog = ProgressManager::instance().progress(lessonId);

        m_maxWatchedPosition = prog.lastPosition();
        m_pendingSeekPosition = prog.lastPosition();

        // ============================================================
        // YouTube is the active source.
        // The local/encrypted path below remains intact for re-enabling
        // the local mode later, but must not run in the current mode.
        // ============================================================
        if (m_usingYouTube)
        {
            m_videoDuration = 0;
            m_youtubeIsPlaying = false;

            ui->progressSlider->setMinimum(0);
            ui->progressSlider->setMaximum(0);
            ui->progressSlider->setValue(0);
            ui->timeLabel->setText("00:00 / 00:00");

            setPlaybackMode(true);

            const QString youtubeVideoId =
                kUseSingleYouTubeTestVideo
                    ? QString::fromLatin1(kDefaultYouTubeVideoId)
                    : youtubeVideoIdFromValue(
                          lesson->getVideoUrl()
                          );

            m_youtubePlayer->loadVideo(youtubeVideoId);

            return;
        }

        // ============================================================
        // LOCAL / ENCRYPTED PLAYBACK (preserved for future local mode)
        // ============================================================
        VideoManager vm;

        QString encryptedPath =
            vm.getVideoPath(*lesson);

        qDebug()
            << "[ENCRYPTED PLAYBACK]"
            << "Encrypted path:"
            << encryptedPath;


        EncryptedVideoDevice *encryptedDevice =
            new EncryptedVideoDevice();


        bool opened =
            encryptedDevice->openEncryptedVideo(
                encryptedPath
                );


        qDebug()
            << "[ENCRYPTED PLAYBACK]"
            << "Device opened:"
            << opened;


        if (!opened)
        {
            QMessageBox::critical(
                this,
                "Playback Error",
                "Could not open encrypted video."
                );

            delete encryptedDevice;

            return;
        }


        qDebug()
            << "[ENCRYPTED PLAYBACK]"
            << "Device size:"
            << encryptedDevice->size();


        m_videoPlayer->setEncryptedSource(
            encryptedDevice,
            QUrl("lesson.mp4")
            );
//تبع اخر دالة خالص الاضافية
        //QTimer::singleShot(
        //    1000,
        //    this,
        //    &MainWindow::testOfflineMediaPlayback
        //    );
    });
} // 👈 قفلة دالة الـ Constructor (MainWindow::MainWindow)


void MainWindow::setupLocalControls()
{
    if (m_localControlBar || !ui || !ui->videoFrame)
        return;

    // This bar is deliberately separate from the active YouTube controls.
    // It remains hidden until the local/encrypted mode is explicitly enabled.
    m_localControlBar = new QWidget(ui->videoFrame);
    m_localControlBar->setObjectName("localControlBar");
    m_localControlBar->setAttribute(Qt::WA_StyledBackground, true);
    m_localControlBar->setStyleSheet(
        "QWidget { background-color: #1e1e1e; }"
        "QPushButton { background-color: #2b2b2b; color: white;"
        "border: 1px solid #444444; border-radius: 6px;"
        "font-size: 14px; }"
        "QPushButton:hover { background-color: #3d3d3d; }"
        "QPushButton:pressed { background-color: #555555; }"
        "QLabel { color: white; font-size: 12px; }"
        );

    m_localControlBar->setGeometry(
        ui->controlBar->geometry()
        );

    QHBoxLayout *layout =
        new QHBoxLayout(m_localControlBar);

    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(8);

    m_localPlayButton =
        new QPushButton(m_localControlBar);
    m_localPlayButton->setObjectName("localPlayButton");
    m_localPlayButton->setFixedSize(45, 40);
    styleControlButton(
        m_localPlayButton,
        makePlayIcon(QColor("#eef4fb")),
        QStringLiteral("تشغيل الفيديو المحلي")
        );

    m_localPauseButton =
        new QPushButton(m_localControlBar);
    m_localPauseButton->setObjectName("localPauseButton");
    m_localPauseButton->setFixedSize(45, 40);
    styleControlButton(
        m_localPauseButton,
        makePauseIcon(QColor("#eef4fb")),
        QStringLiteral("إيقاف الفيديو المحلي مؤقتًا")
        );

    m_localProgressSlider =
        new QSlider(Qt::Horizontal, m_localControlBar);
    m_localProgressSlider->setObjectName("localProgressSlider");
    m_localProgressSlider->setRange(0, 0);

    m_localTimeLabel =
        new QLabel(QStringLiteral("00:00 / 00:00"), m_localControlBar);
    m_localTimeLabel->setObjectName("localTimeLabel");
    m_localTimeLabel->setMinimumWidth(100);
    m_localTimeLabel->setAlignment(Qt::AlignCenter);

    m_localVolumeSlider =
        new QSlider(Qt::Horizontal, m_localControlBar);
    m_localVolumeSlider->setObjectName("localVolumeSlider");
    m_localVolumeSlider->setRange(0, 100);
    m_localVolumeSlider->setValue(50);
    m_localVolumeSlider->setFixedWidth(120);

    m_localFullScreenButton =
        new QPushButton(m_localControlBar);
    m_localFullScreenButton->setObjectName("localFullScreenButton");
    m_localFullScreenButton->setFixedSize(45, 40);
    styleControlButton(
        m_localFullScreenButton,
        makeFullscreenIcon(QColor("#eef4fb")),
        QStringLiteral("ملء الشاشة")
        );

    layout->addWidget(m_localPlayButton);
    layout->addWidget(m_localPauseButton);
    layout->addWidget(m_localProgressSlider, 1);
    layout->addWidget(m_localTimeLabel);
    layout->addWidget(m_localVolumeSlider);
    layout->addWidget(m_localFullScreenButton);

    connect(
        m_localPlayButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!m_usingYouTube && m_videoPlayer)
            {
                m_videoPlayer->play();
            }
        }
        );

    connect(
        m_localPauseButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!m_usingYouTube && m_videoPlayer)
            {
                m_videoPlayer->pause();
            }
        }
        );

    connect(
        m_localVolumeSlider,
        &QSlider::valueChanged,
        this,
        [this](int value)
        {
            if (!m_usingYouTube && m_videoPlayer)
            {
                m_videoPlayer->setVolume(value / 100.0f);
            }
        }
        );

    connect(
        m_localFullScreenButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (ui && ui->fullScreenButton)
            {
                ui->fullScreenButton->click();
            }
        }
        );

    m_localControlBar->hide();
}


void MainWindow::setupYouTubeControls()
{
    const QColor iconColor("#eef4fb");

    styleControlButton(
        ui->playButton,
        makePlayIcon(iconColor),
        QStringLiteral("تشغيل الفيديو")
        );

    styleControlButton(
        ui->pauseButton,
        makePauseIcon(iconColor),
        QStringLiteral("إيقاف الفيديو مؤقتًا")
        );

    styleControlButton(
        ui->volumeButton,
        makeVolumeIcon(iconColor),
        QStringLiteral("الصوت")
        );

    styleControlButton(
        ui->fullScreenButton,
        makeFullscreenIcon(iconColor),
        QStringLiteral("ملء الشاشة")
        );

    m_speedButton = new QToolButton(ui->controlBar);
    m_speedButton->setObjectName(QStringLiteral("speedButton"));
    m_speedButton->setFixedSize(76, 40);
    m_speedButton->setIcon(makeSpeedIcon(iconColor));
    m_speedButton->setIconSize(QSize(18, 18));
    m_speedButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_speedButton->setPopupMode(QToolButton::InstantPopup);
    m_speedButton->setToolTip(
        QStringLiteral("سرعة تشغيل الفيديو")
        );
    m_speedButton->setAccessibleName(
        QStringLiteral("سرعة تشغيل الفيديو")
        );
    m_speedButton->setCursor(Qt::PointingHandCursor);
    m_speedButton->setFocusPolicy(Qt::StrongFocus);
    m_speedButton->setStyleSheet(
        "QToolButton { background: #202b38; color: #eef4fb;"
        "border: 1px solid #344454; border-radius: 8px;"
        "padding: 0 8px; font-size: 13px; font-weight: 600; }"
        "QToolButton:hover { background: #2b3b4d; border-color: #4d6982; }"
        "QToolButton:pressed { background: #17212c; }"
        "QToolButton:focus { border: 1px solid #72b7ff; }"
        );

    m_speedMenu = new QMenu(m_speedButton);
    m_speedMenu->setStyleSheet(
        "QMenu { background: #1b2632; color: #eef4fb;"
        "border: 1px solid #344454; padding: 6px; }"
        "QMenu::item { padding: 8px 30px 8px 12px; border-radius: 6px; }"
        "QMenu::item:selected { background: #2b4d6b; }"
        "QMenu::item:checked { color: #8bc7ff; font-weight: 600; }"
        );

    const QList<double> playbackRates =
        {0.5, 0.75, 1.0, 1.25, 1.5, 2.0};

    for (const double rate : playbackRates)
    {
        QAction *action =
            m_speedMenu->addAction(formatPlaybackRate(rate));

        action->setCheckable(true);
        action->setData(rate);

        connect(
            action,
            &QAction::triggered,
            this,
            [this, rate]()
            {
                if (!m_usingYouTube || !m_youtubePlayer)
                {
                    return;
                }

                m_youtubePlayer->setPlaybackRate(rate);
            }
            );
    }

    m_speedButton->setMenu(m_speedMenu);

    QHBoxLayout *controlsLayout = nullptr;
    QLayout *barLayout = ui->controlBar->layout();

    if (barLayout && barLayout->count() > 0)
    {
        QLayoutItem *firstItem = barLayout->itemAt(0);

        if (firstItem)
        {
            controlsLayout =
                qobject_cast<QHBoxLayout*>(firstItem->layout());
        }
    }

    if (controlsLayout)
    {
        const int fullScreenIndex =
            controlsLayout->indexOf(ui->fullScreenButton);

        controlsLayout->insertWidget(
            fullScreenIndex >= 0
                ? fullScreenIndex
                : controlsLayout->count(),
            m_speedButton
            );
    }

    connect(
        m_youtubePlayer,
        &YouTubePlayer::playbackRateChanged,
        this,
        [this](double rate)
        {
            updatePlaybackRateUi(rate);
        }
        );

    updatePlaybackRateUi(1.0);
}


void MainWindow::updatePlaybackRateUi(double rate)
{
    m_playbackRate = rate;

    if (m_speedButton)
    {
        m_speedButton->setText(formatPlaybackRate(rate));
    }

    if (!m_speedMenu)
    {
        return;
    }

    for (QAction *action : m_speedMenu->actions())
    {
        const double actionRate = action->data().toDouble();
        action->setChecked(qAbs(actionRate - rate) < 0.01);
    }
}


void MainWindow::setPlaybackMode(bool useYouTube)
{
    m_usingYouTube = useYouTube;

    if (ui)
    {
        // The original control bar is now explicitly the YouTube bar.
        ui->playButton->setVisible(useYouTube);
        ui->pauseButton->setVisible(useYouTube);
        ui->progressSlider->setVisible(useYouTube);
        ui->timeLabel->setVisible(useYouTube);
        ui->volumeButton->setVisible(useYouTube);
    }

    if (m_speedButton)
    {
        m_speedButton->setVisible(useYouTube);
    }

    if (m_localControlBar)
    {
        m_localControlBar->setGeometry(
            ui->controlBar->geometry()
            );
        m_localControlBar->setVisible(!useYouTube);

        if (!useYouTube)
        {
            m_localControlBar->raise();
        }
    }

    if (m_videoWidget)
    {
        m_videoWidget->setVisible(!useYouTube);
    }

    if (m_youtubeView)
    {
        m_youtubeView->setVisible(useYouTube);

        if (useYouTube)
        {
            m_youtubeView->raise();
        }
    }

    if (volumePopupWindow && !useYouTube)
    {
        volumePopupWindow->hide();
    }
}


void MainWindow::updateVideoLayout()
{
    if (!ui || !ui->videoFrame || !ui->videoDisplay)
        return;

    // Re-activate the layouts after showFullScreen()/showNormal() so the
    // geometry below is based on the final restored window size.
    if (ui->centralwidget && ui->centralwidget->layout())
    {
        ui->centralwidget->layout()->activate();
    }

    if (ui->videoFrame->layout())
    {
        ui->videoFrame->layout()->activate();
    }

    ui->videoDisplay->setGeometry(
        ui->videoFrame->rect()
        );

    if (m_youtubeView && m_youtubeView->isVisible())
    {
        m_youtubeView->setGeometry(
            ui->videoDisplay->geometry()
            );

        m_youtubeView->raise();

        QWidget *overlay =
            m_youtubeView->findChild<QWidget*>(
                "youtubeOverlay"
                );

        if (overlay)
        {
            overlay->setGeometry(
                m_youtubeView->rect()
                );

            overlay->raise();
        }
    }

    ui->controlBar->setGeometry(
        0,
        ui->videoFrame->height()
            - ui->controlBar->height(),
        ui->videoFrame->width(),
        ui->controlBar->height()
        );

    ui->controlBar->raise();

    if (m_localControlBar)
    {
        m_localControlBar->setGeometry(
            ui->controlBar->geometry()
            );

        if (m_localControlBar->isVisible())
        {
            m_localControlBar->raise();
        }
    }
}


MainWindow::~MainWindow()
{
    if (qApp)
    {
        qApp->removeEventFilter(this);
    }

    delete ui;
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *targetWidget = qobject_cast<QWidget*>(obj);
    const bool belongsToThisWindow =
        obj == this ||
        (targetWidget &&
         (targetWidget == this || this->isAncestorOf(targetWidget)));

    if (!belongsToThisWindow)
    {
        return QMainWindow::eventFilter(obj, event);
    }

    if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (obj == m_youtubeView ||
            obj == m_videoWidget ||
            obj == ui->videoDisplay ||
            (obj && obj->objectName() == "youtubeOverlay"))
        {
            if (ui->fullScreenButton)
            {
                // Use the same path as the dedicated button so double-click
                // cannot leave the central layout in a different state.
                ui->fullScreenButton->click();
            }

            return true;
        }
    }


    if(event->type() == QEvent::MouseMove)
    {
        if(m_isFullScreen)
        {
            ui->controlBar->show();
            ui->controlBar->raise();

            controlsTimer->start(3000);
        }
        else
        {
            ui->controlBar->show();
            ui->controlBar->raise();

            controlsTimer->stop();
        }
    }


    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent =
            static_cast<QKeyEvent*>(event);

        const int key = keyEvent->key();
        const bool isVideoToggleKey =
            key == Qt::Key_Space ||
            key == Qt::Key_Left ||
            key == Qt::Key_Right;

        const bool speedMenuIsOpen =
            m_speedMenu && m_speedMenu->isVisible();

        // These keys belong to the application's custom video controls.
        // Consume both press and release so the embedded YouTube player
        // never receives them as native YouTube shortcuts.
        if (m_usingYouTube &&
            isVideoToggleKey &&
            !speedMenuIsOpen)
        {
            if (event->type() == QEvent::KeyPress &&
                !keyEvent->isAutoRepeat())
            {
                toggleYouTubePlayPause();
            }

            return true;
        }

        if (event->type() == QEvent::KeyRelease)
        {
            return QMainWindow::eventFilter(obj, event);
        }


        // =========================
        // SPACE = PLAY / PAUSE
        // =========================

        if (keyEvent->key() == Qt::Key_Space)
        {
            if (m_usingYouTube)
            {
                toggleYouTubePlayPause();
            }
            else
            {
                if (m_videoPlayer->isPlaying())
                {
                    m_videoPlayer->pause();
                }
                else
                {
                    m_videoPlayer->play();
                }
            }

            return true;
        }


        // =========================
        // ESC = EXIT FULLSCREEN
        // =========================

        if (keyEvent->key() == Qt::Key_Escape &&
            m_isFullScreen)
        {
            this->showNormal();

            ui->coursePanel->show();

            m_isFullScreen = false;

            QTimer::singleShot(
                0,
                this,
                [this]()
                {
                    updateVideoLayout();
                }
                );

            return true;
        }
    }


    return QMainWindow::eventFilter(obj, event);
}


//يوتيوب
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    updateVideoLayout();
}



QString MainWindow::formatTime(qint64 milliseconds)
{
    qint64 totalSeconds = milliseconds / 1000;

    qint64 hours = totalSeconds / 3600;

    qint64 minutes = (totalSeconds % 3600) / 60;

    qint64 seconds = totalSeconds % 60;


    if (hours > 0)
    {
        return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
    else
    {
        return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
}


void MainWindow::stopVideo()
{
    if (m_usingYouTube)
    {
        if (m_youtubePlayer)
        {
            m_youtubePlayer->stop();
            m_youtubeIsPlaying = false;
        }

        return;
    }

    if (m_videoPlayer)
    {
        m_videoPlayer->pause();
    }
}


void MainWindow::toggleYouTubePlayPause()
{
    if (!m_youtubePlayer || !m_usingYouTube)
        return;

    if (m_youtubeIsPlaying)
    {
        m_youtubePlayer->pause();
        m_youtubeIsPlaying = false;

        qDebug()
            << "[YOUTUBE CONTROL]"
            << "Pause";
    }
    else
    {
        m_youtubePlayer->play();
        m_youtubeIsPlaying = true;

        qDebug()
            << "[YOUTUBE CONTROL]"
            << "Play";
    }
}
