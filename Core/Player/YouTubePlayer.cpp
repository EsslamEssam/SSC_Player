#include "YouTubePlayer.h"

#include <QWebEngineView>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QUrl>
#include <QDebug>


YouTubePlayer::YouTubePlayer(
    QWebEngineView *view,
    QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_channel(nullptr)
    , m_videoId()
    , m_pageLoaded(false)
    , m_pendingPlayCommand(false)
    , m_pendingPauseCommand(false)
    , m_playerReady(false)
{
    if (!m_view)
    {
        qDebug()
        << "[YOUTUBE] WebEngineView is NULL.";

        return;
    }

    //Gemini
    // حظر القائمة الجانبية (الزر الأيمن) لمنع الوصول لأدوات الفحص
    m_view->setContextMenuPolicy(Qt::NoContextMenu);

    m_channel =
        new QWebChannel(
            m_view->page()
            );

    m_channel->registerObject(
        "youtubePlayer",
        this
        );

    m_view->page()->setWebChannel(
        m_channel
        );


    loadPlayerPage();
}


YouTubePlayer::~YouTubePlayer()
{
    m_pageLoaded = false;
    m_playerReady = false;

    if (m_view)
    {
        m_view->setUrl(QUrl());
    }
}


void YouTubePlayer::loadPlayerPage()
{
    if (!m_view)
        return;


    const QString html = R"HTML(
<!DOCTYPE html>

<html>

<head>

<meta charset="utf-8">

<meta
    name="referrer"
    content="strict-origin-when-cross-origin"
>

<script src="qrc:///qtwebchannel/qwebchannel.js"></script>

//Gemini
<style>
html, body {
    margin: 0;
    padding: 0;
    width: 100%;
    height: 100%;
    overflow: hidden;
    background: black;
}

/* حاوية المشغل مع تكبير خفيف لقص الأطراف التي تحتوي على الشعارات */
#player-container {
    position: relative;
    width: 100%;
    height: 100%;
    overflow: hidden;
}

#player {
    position: absolute;
    /* تكبير العرض والارتفاع بنسبة بسيطة لقص الشريط العلوي والسفلي */
    top: -6%;
    left: -2%;
    width: 104%;
    height: 112%;
    pointer-events: none; /* يمنع الضغط بالماوس نهائياً داخل الفيديو */
}

/* إخفاء كافة طبقات يوتيوب عبر CSS */
.ytp-chrome-top,
.ytp-chrome-bottom,
.ytp-title-link,
.ytp-title,
.ytp-share-button,
.ytp-youtube-button,
.ytp-watermark,
.ytp-pause-overlay,
.ytp-ce-element,
.ytp-show-cards-title,
.html5-endscreen,
.ytp-upnext {
    display: none !important;
    opacity: 0 !important;
    visibility: hidden !important;
    pointer-events: none !important;
}
</style>
//Gemini


</head>

//Gemini
<body>
    <div id="player-container">
        <div id="player"></div>
    </div>
//Gemini

<script>

var player = null;

var requestedVideoId = "";

var qtBridge = null;

var youtubePlayerReady = false;

var qtReadyNotified = false;

var progressTimer = null;

var lastReportedDuration = -1;

var pendingPlay = false;

var pendingPause = false;

function notifyQtPlayerReady()
{
    if (!youtubePlayerReady ||
        !qtBridge ||
        qtReadyNotified)
    {
        return;
    }

    if (qtBridge.onJsReady)
    {
        qtReadyNotified = true;
        qtBridge.onJsReady();
    }
}

function reportProgress()
{
    if (!player || !qtBridge)
    {
        return;
    }

    var duration = player.getDuration();

    if (duration > 0 &&
        duration !== lastReportedDuration)
    {
        lastReportedDuration = duration;

        if (qtBridge.onJsDurationChanged)
        {
            qtBridge.onJsDurationChanged(duration);
        }
    }

    var state = player.getPlayerState();

    if (state === YT.PlayerState.PLAYING ||
        state === YT.PlayerState.PAUSED ||
        state === YT.PlayerState.BUFFERING ||
        state === YT.PlayerState.ENDED)
    {
        if (qtBridge.onJsPositionChanged)
        {
            qtBridge.onJsPositionChanged(
                player.getCurrentTime()
                );
        }
    }
}

function startProgressReporting()
{
    if (progressTimer !== null)
    {
        return;
    }

    progressTimer = setInterval(
        reportProgress,
        250
        );
}

function setupQtBridge()
{
    if (typeof QWebChannel === "undefined" ||
        typeof qt === "undefined" ||
        !qt.webChannelTransport)
    {
        console.error(
            "[YOUTUBE] Qt WebChannel is unavailable."
            );

        return;
    }

    new QWebChannel(
        qt.webChannelTransport,
        function(channel)
        {
            qtBridge =
                channel.objects.youtubePlayer;

            notifyQtPlayerReady();
        }
        );
}

//Gemini
// 👇 هنا تضع كود الـ JavaScript للحذف التلقائي للعناصر
/* مراقبة وحذف أشرطة العناوين والشعارات تلقائياً فور تكوينها */
function hideYoutubeElements() {
    const selectors = [
        '.ytp-chrome-top',
        '.ytp-title',
        '.ytp-share-button',
        '.ytp-youtube-button',
        '.ytp-watermark',
        '.ytp-pause-overlay',
        '.ytp-ce-element'
    ];

    selectors.forEach(selector => {
        const els = document.querySelectorAll(selector);
        els.forEach(el => {
            el.style.display = 'none';
            el.style.opacity = '0';
            el.remove(); // حذف العنصر نهائياً من الـ DOM
        });
    });
}

// تشغيل المراقبة كل 200 مللي ثانية
setInterval(hideYoutubeElements, 200);
//Gemini

/* ============================================================
   Load YouTube IFrame API
   ============================================================ */

function loadYouTubeAPI()
{
    var tag =
        document.createElement("script");


    tag.src =
        "https://www.youtube.com/iframe_api";


    var firstScriptTag =
        document.getElementsByTagName(
            "script"
        )[0];


    firstScriptTag
        .parentNode
        .insertBefore(
            tag,
            firstScriptTag
        );
}


/* ============================================================
   YouTube API ready
   ============================================================ */

function onYouTubeIframeAPIReady()
{
    console.log(
        "[YOUTUBE] IFrame API ready"
    );


    createPlayer();
}


/* ============================================================
   Create player
   ============================================================ */

function createPlayer()
{
    player =
        new YT.Player(
            "player",
            {
                width: "100%",
                height: "100%",


                videoId:
                    requestedVideoId,


                playerVars:
                {
                    autoplay: 0,

                    controls: 0,

                    rel: 0,

                    playsinline: 1,

                    enablejsapi: 1,

                    origin:
                        "https://app.scpp-player.local"
                },


                events:
                {
                    onReady:
                        onPlayerReady,


                    onStateChange:
                        onPlayerStateChange,


                    onError:
                        onPlayerError
                }
            }
        );
}


/* ============================================================
   Player ready
   ============================================================ */

function onPlayerReady(event)
{
    console.log(
        "[YOUTUBE] Player ready"
    );

    youtubePlayerReady = true;

    if (pendingPlay)
    {
        pendingPlay = false;
        player.playVideo();
    }

    if (pendingPause)
    {
        pendingPause = false;
        player.pauseVideo();
    }

    startProgressReporting();

    notifyQtPlayerReady();


    if (window.youtubeReady)
    {
        window.youtubeReady();
    }
}


/* ============================================================
   Player state
   ============================================================ */

function onPlayerStateChange(event)
{
    if (!player)
        return;

    if (qtBridge && qtBridge.onJsStateChanged)
    {
        qtBridge.onJsStateChanged(event.data);
    }


    if (event.data ===
        YT.PlayerState.PLAYING)
    {
        if (window.youtubePlaying)
        {
            window.youtubePlaying();
        }
    }


    if (event.data ===
        YT.PlayerState.PAUSED)
    {
        if (window.youtubePaused)
        {
            window.youtubePaused();
        }
    }


    if (event.data ===
        YT.PlayerState.ENDED)
    {
        if (window.youtubeEnded)
        {
            window.youtubeEnded();
        }
    }
}


/* ============================================================
   Player error
   ============================================================ */

function onPlayerError(event)
{
    console.log(
        "[YOUTUBE] Player error:"
        + event.data
    );

    if (qtBridge && qtBridge.onJsError)
    {
        qtBridge.onJsError(String(event.data));
    }


    if (window.youtubeError)
    {
        window.youtubeError(
            String(event.data)
        );
    }
}


/* ============================================================
   Load video
   ============================================================ */

function setVideo(videoId)
{
    requestedVideoId =
        videoId;

    lastReportedDuration = -1;


    if (!player)
        return;


    player.loadVideoById(
        videoId
    );
}


/* ============================================================
   Controls
   ============================================================ */

function playVideo()
{
    if (player && youtubePlayerReady)
    {
        player.playVideo();
    }
    else
    {
        pendingPlay = true;
        pendingPause = false;
    }
}


function pauseVideo()
{
    if (player && youtubePlayerReady)
    {
        player.pauseVideo();
    }
    else
    {
        pendingPause = true;
        pendingPlay = false;
    }
}


function stopVideo()
{
    if (player)
    {
        player.stopVideo();
    }
}


function seekVideo(positionSeconds)
{
    if (player)
    {
        player.seekTo(
            positionSeconds,
            true
        );
    }
}


function setVideoVolume(volume)
{
    if (player)
    {
        player.setVolume(
            volume
        );
    }
}


function getCurrentTime()
{
    if (!player)
        return 0;


    return player.getCurrentTime();
}


function getDuration()
{
    if (!player)
        return 0;


    return player.getDuration();
}


/* ============================================================
   Start API
   ============================================================ */

setupQtBridge();

loadYouTubeAPI();

</script>

</body>

</html>
)HTML";


    /*
     * IMPORTANT:
     *
     * The HTML is loaded with a real HTTPS
     * base URL so QWebEngine has a proper
     * web origin instead of a local file /
     * opaque origin.
     */

    const QUrl baseUrl =
        QUrl(
            "https://app.scpp-player.local/"
            );

    connect(
        m_view,
        &QWebEngineView::loadFinished,
        this,
        [this](bool ok)
        {
            if (!ok)
            {
                emit error(
                    "YouTube player page failed to load."
                    );

                return;
            }

            qDebug()
                << "[YOUTUBE]"
                << "Player HTML loaded.";

            m_pageLoaded = true;

            if (!m_videoId.isEmpty())
            {
                runJavaScript(
                    QString(
                        "requestedVideoId = '%1';"
                        "setVideo('%1');"
                        )
                        .arg(m_videoId)
                    );
            }

            if (m_pendingPlayCommand)
            {
                m_pendingPlayCommand = false;
                runJavaScript("playVideo();");
            }

            if (m_pendingPauseCommand)
            {
                m_pendingPauseCommand = false;
                runJavaScript("pauseVideo();");
            }

            // Loading the page is enough to start the existing
            // YouTube flow. WebChannel is only used for telemetry.
            emit ready();
        }
        );

    m_view->setHtml(
        html,
        baseUrl
        );
}


void YouTubePlayer::loadVideo(
    const QString &videoId)
{
    if (videoId.isEmpty())
    {
        emit error(
            "YouTube video ID is empty."
            );

        return;
    }


    m_videoId =
        videoId;


    qDebug()
        << "[YOUTUBE]"
        << "Loading video:"
        << videoId;

    if (!m_pageLoaded)
    {
        return;
    }


    QString script =
        QString(
            "requestedVideoId = '%1';"
            "setVideo('%1');"
            )
            .arg(
                videoId
                );


    runJavaScript(
        script
        );
}


//void YouTubePlayer::play()
//{
   // runJavaScript(
     //   "playVideo();"
       // );
//}


//void YouTubePlayer::pause()
//{
//    runJavaScript(
//        "pauseVideo();"
//        );
//}


void YouTubePlayer::stop()
{
    if (m_pageLoaded && m_view && m_view->page())
    {
        m_view->page()->runJavaScript("stopVideo();");
    }
}


void YouTubePlayer::setPosition(
    qint64 position)
{
    double seconds =
        static_cast<double>(
            position
            ) / 1000.0;


    runJavaScript(
        QString(
            "seekVideo(%1);"
            ).arg(
                QString::number(
                    seconds,
                    'f',
                    3
                    )
                )
        );
}


//void YouTubePlayer::setVolume(
//    int volume)
//{
 //   if (volume < 0)
 //       volume = 0;

 //   if (volume > 100)
 //       volume = 100;
//
//
 //   runJavaScript(
 //       QString(
  //          "setVideoVolume(%1);"
 //           ).arg(volume)
  //      );
//}


void YouTubePlayer::setFullScreen(
    bool enabled)
{
    Q_UNUSED(enabled);

    /*
     * Fullscreen integration will be handled
     * with the application's existing
     * fullscreen system in the next stage.
     */
}


void YouTubePlayer::runJavaScript(
    const QString &script)
{
    if (!m_view)
        return;


    m_view->page()->runJavaScript(
        script
        );
}




//Gemini
void YouTubePlayer::onJsReady()
{
    m_playerReady = true;
}

void YouTubePlayer::onJsStateChanged(int state)
{
    if (state == 1) emit playing();
    else if (state == 2) emit paused();
    else if (state == 0) emit ended();
}

void YouTubePlayer::onJsPositionChanged(double seconds)
{
    emit positionChanged(static_cast<qint64>(seconds * 1000));
}

void YouTubePlayer::onJsDurationChanged(double seconds)
{
    if (seconds <= 0)
    {
        return;
    }

    emit durationChanged(static_cast<qint64>(seconds * 1000));
}

void YouTubePlayer::onJsError(const QString &message)
{
    emit error(message);
}

void YouTubePlayer::play() {
    if (!m_pageLoaded)
    {
        m_pendingPlayCommand = true;
        m_pendingPauseCommand = false;
        return;
    }

    if (m_view && m_view->page()) {
        m_view->page()->runJavaScript("playVideo();");
    }
}

void YouTubePlayer::pause() {
    if (!m_pageLoaded)
    {
        m_pendingPauseCommand = true;
        m_pendingPlayCommand = false;
        return;
    }

    if (m_view && m_view->page()) {
        m_view->page()->runJavaScript("pauseVideo();");
    }
}

void YouTubePlayer::seekTo(int seconds) {
    if (m_view && m_view->page()) {
        m_view->page()->runJavaScript(
            QString("if(player && player.seekTo) { player.seekTo(%1, true); }").arg(seconds)
            );
    }
}

void YouTubePlayer::setVolume(int volume) {
    if (m_view && m_view->page()) {
        m_view->page()->runJavaScript(
            QString("if(player && player.setVolume) { player.setVolume(%1); }").arg(volume)
            );
    }
}
