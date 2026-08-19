#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
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



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_videoWidget(nullptr)
{
    ui->setupUi(this);

    //يوتيوب
    m_youtubeView =
        new QWebEngineView(
            ui->videoDisplay
            );

    m_youtubeView->setGeometry(
        ui->videoDisplay->rect()
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

            if (m_videoWidget)
            {
                m_videoWidget->hide();
            }

            // عند تشغيل يوتيوب:

            // ضبط قياسات الـ View
            m_youtubeView->setGeometry(0, 0, ui->videoDisplay->width(), ui->videoDisplay->height());

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
            overlay->setStyleSheet("background: transparent;");
            overlay->setGeometry(0, 0, m_youtubeView->width(), m_youtubeView->height());
            overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            overlay->raise();
            overlay->show();
            // -------------------------------------------------------------

            // تشغيل الفيديو المطلوب
            m_youtubePlayer->loadVideo("LYBNEWuSP04");
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



#ifdef Q_OS_WIN

    HWND hwnd = (HWND)winId();

    SetWindowDisplayAffinity(
        hwnd,
        WDA_EXCLUDEFROMCAPTURE
        );

#endif


    this->setMouseTracking(true);
    this->installEventFilter(this);

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

    connect(
        ui->playButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (m_usingYouTube)
            {
                m_youtubePlayer->play();
            }
            else
            {
                m_videoPlayer->play();
            }
        }
        );


    connect(
        ui->pauseButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (m_usingYouTube)
            {
                m_youtubePlayer->pause();
            }
            else
            {
                m_videoPlayer->pause();
            }
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
            });

    connect(
        ui->volumeSlider,
        &QSlider::valueChanged,
        this,
        [this](int value)
        {
            if (m_usingYouTube)
            {
                m_youtubePlayer->setVolume(value);
            }
            else
            {
                m_videoPlayer->setVolume(
                    value / 100.0f
                    );
            }
        }
        );


    connect(m_videoPlayer,
            &VideoPlayer::durationChanged,
            this,
            [=](qint64 duration)
            {
                ui->progressSlider->setMaximum(duration);
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
        }
        );


    connect(m_videoPlayer,
            &VideoPlayer::positionChanged,
            this,
            [=](qint64 position)
            {
                if (!m_isSeeking)
                {
                    ui->progressSlider->setValue(position);
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
                m_isSeeking = true;
            });


    connect(
        ui->progressSlider,
        &QSlider::sliderReleased,
        this,
        [this]()
        {
            m_isSeeking = false;

            qint64 position =
                ui->progressSlider->value();

            // ==========================================
            // YouTube
            // ==========================================
            if (m_usingYouTube)
            {
                if (position > m_maxWatchedPosition)
                {
                    position = m_maxWatchedPosition;

                    ui->progressSlider->setValue(
                        static_cast<int>(position)
                        );
                }

                m_youtubePlayer->setPosition(position);

                return;
            }

            // ==========================================
            // Local Video
            // ==========================================
            if (position > m_maxWatchedPosition)
            {
                position = m_maxWatchedPosition;

                ui->progressSlider->setValue(
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
                m_videoDuration = duration;

                ui->timeLabel->setText(
                    "00:00 / " + formatTime(duration)
                    );
            });


    connect(m_videoPlayer,
            &VideoPlayer::positionChanged,
            this,
            [=](qint64 position)
            {
                ui->timeLabel->setText(
                    formatTime(position)
                    + " / "
                    + formatTime(m_videoDuration)
                    );
            });


    connect(m_videoPlayer,
            &VideoPlayer::videoReady,
            this,
            [=]()
            {
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


MainWindow::~MainWindow()
{
    delete ui;
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (obj == m_youtubeView ||
            obj == m_videoWidget ||
            obj == ui->videoDisplay)
        {
            if (!m_isFullScreen)
            {
                ui->coursePanel->hide();

                this->showFullScreen();

                m_isFullScreen = true;

                ui->controlBar->show();
                ui->controlBar->raise();
            }
            else
            {
                this->showNormal();

                ui->coursePanel->show();

                m_isFullScreen = false;

                ui->controlBar->show();
                ui->controlBar->raise();
            }

            return true;
        }
    }


    if(event->type() == QEvent::MouseMove)
    {
        if(m_isFullScreen)
        {
            ui->controlBar->show();

            controlsTimer->start(3000);
        }
        else
        {
            ui->controlBar->show();

            controlsTimer->stop();
        }
    }


    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent =
            static_cast<QKeyEvent*>(event);


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

            ui->videoDisplay->setGeometry(
                ui->videoFrame->rect()
                );

            if (m_youtubeView)
            {
                m_youtubeView->setGeometry(
                    ui->videoDisplay->rect()
                    );

                m_youtubeView->raise();
            }

            ui->controlBar->setGeometry(
                0,
                ui->videoFrame->height()
                    - ui->controlBar->height(),
                ui->videoFrame->width(),
                ui->controlBar->height()
                );

            ui->controlBar->raise();

            return true;
        }
    }


    return QMainWindow::eventFilter(obj, event);
}


//يوتيوب
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);


    // =====================================================
    // Video display
    // =====================================================

    ui->videoDisplay->setGeometry(
        ui->videoFrame->rect()
        );


    // =====================================================
    // YouTube display
    // =====================================================

    if (m_youtubeView &&
        m_youtubeView->isVisible())
    {
        m_youtubeView->setGeometry(
            ui->videoDisplay->rect()
            );

        m_youtubeView->raise();


        QWidget *overlay =
            m_youtubeView->findChild<QWidget*>(
                "youtubeOverlay"
                );

        if (overlay)
        {
            overlay->setGeometry(
                0,
                0,
                m_youtubeView->width(),
                m_youtubeView->height()
                );

            overlay->raise();
        }
    }


    // =====================================================
    // Control Bar
    // =====================================================

    ui->controlBar->setGeometry(
        0,
        ui->videoFrame->height()
            - ui->controlBar->height(),
        ui->videoFrame->width(),
        ui->controlBar->height()
        );


    ui->controlBar->raise();
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
    if(m_videoPlayer)
    {
        m_videoPlayer->pause();
    }
}


void MainWindow::toggleYouTubePlayPause()
{
    static bool youtubePlaying = false;

    if (!m_youtubePlayer)
        return;

    if (youtubePlaying)
    {
        m_youtubePlayer->pause();
        youtubePlaying = false;

        qDebug()
            << "[YOUTUBE CONTROL]"
            << "Pause";
    }
    else
    {
        m_youtubePlayer->play();
        youtubePlaying = true;

        qDebug()
            << "[YOUTUBE CONTROL]"
            << "Play";
    }
}
