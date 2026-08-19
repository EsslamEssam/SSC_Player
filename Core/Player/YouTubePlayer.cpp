#include "YouTubePlayer.h"

#include <QWebEngineView>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QUrl>
#include <QDebug>
#include <QStringList>


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
    , m_playbackRate(1.0)
    , m_playbackQuality(QStringLiteral("auto"))
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

    // The controls live in the native Qt window, not inside the web page.
    // Allow those controls to call YouTube's play API without being rejected
    // by Chromium's "user gesture" autoplay policy.
    m_view->settings()->setAttribute(
        QWebEngineSettings::PlaybackRequiresUserGesture,
        false
        );

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

    installYouTubeFrameBridge();

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


void YouTubePlayer::installYouTubeFrameBridge()
{
    if (!m_view || !m_view->page())
    {
        return;
    }

    // The IFrame API no longer accepts a requested quality. The native
    // quality menu still exists inside YouTube's child iframe, so this
    // script provides a narrowly scoped bridge for the custom Qt button.
    // It runs in the child frame's own DOM and keeps the native controls
    // hidden except for the short, programmatic quality-selection action.
    const QString source = QString::fromUtf8(R"JS(
(function()
{
    if (window.top === window ||
        window.__sscQualityBridgeInstalled)
    {
        return;
    }

    window.__sscQualityBridgeInstalled = true;

    var styleId = "ssc-youtube-hidden-controls";
    var commandClass = "ssc-quality-command";

    function installStyle()
    {
        if (document.getElementById(styleId))
        {
            return;
        }

        var style = document.createElement("style");
        style.id = styleId;
        style.textContent =
            ".ytp-chrome-top,"
            + ".ytp-chrome-bottom,"
            + ".ytp-title-link,"
            + ".ytp-title,"
            + ".ytp-share-button,"
            + ".ytp-youtube-button,"
            + ".ytp-watermark,"
            + ".ytp-pause-overlay,"
            + ".ytp-ce-element,"
            + ".ytp-show-cards-title,"
            + ".html5-endscreen,"
            + ".ytp-upnext {"
            + "display:none!important;"
            + "opacity:0!important;"
            + "visibility:hidden!important;"
            + "pointer-events:none!important;"
            + "}"
            + "." + commandClass + " .ytp-chrome-bottom {"
            + "display:block!important;"
            + "opacity:1!important;"
            + "visibility:visible!important;"
            + "pointer-events:auto!important;"
            + "}"
            + "." + commandClass
            + " .ytp-chrome-bottom .ytp-settings-button {"
            + "display:block!important;"
            + "opacity:1!important;"
            + "visibility:visible!important;"
            + "pointer-events:auto!important;"
            + "}";

        if (document.head)
        {
            document.head.appendChild(style);
        }
        else if (document.documentElement)
        {
            document.documentElement.appendChild(style);
        }
    }

    function playerRoot()
    {
        return document.querySelector(".html5-video-player")
            || document.body;
    }

    function setCommandMode(enabled)
    {
        var root = playerRoot();

        if (!root)
        {
            return;
        }

        if (enabled)
        {
            root.classList.add(commandClass);
        }
        else
        {
            root.classList.remove(commandClass);
        }
    }

    function isVisible(element)
    {
        if (!element)
        {
            return false;
        }

        var style = window.getComputedStyle(element);
        var rect = element.getBoundingClientRect();

        return style.display !== "none" &&
            style.visibility !== "hidden" &&
            rect.width > 0 &&
            rect.height > 0;
    }

    function itemText(element)
    {
        return [
            element.getAttribute("aria-label"),
            element.getAttribute("data-label"),
            element.getAttribute("data-tooltip-text"),
            element.textContent
        ]
            .filter(function(value)
            {
                return value;
            })
            .join(" ")
            .trim()
            .toLowerCase();
    }

    function findMenuItem(matcher)
    {
        var items = document.querySelectorAll(
            ".ytp-menuitem, [role=\"menuitem\"]"
            );

        for (var i = 0; i < items.length; ++i)
        {
            if (isVisible(items[i]) &&
                matcher.test(itemText(items[i])))
            {
                return items[i];
            }
        }

        return null;
    }

    function qualityMatcher(quality)
    {
        switch (String(quality || "auto").toLowerCase())
        {
            case "small":
                return /144p|small/i;

            case "medium":
                return /240p|medium/i;

            case "large":
                return /360p|large/i;

            case "hd480":
                return /480p|hd480/i;

            case "hd720":
                return /720p|hd720/i;

            case "hd1080":
                return /1080p|hd1080/i;

            case "highres":
                return /1440p|2160p|4320p|highres|8k/i;

            case "auto":
            default:
                return /auto|automatic|تلقائي/i;
        }
    }

    function reportResult(quality, success)
    {
        if (window.parent)
        {
            window.parent.postMessage(
                {
                    source: "ssc-youtube-frame",
                    type: "quality-result",
                    quality: String(quality || "auto"),
                    success: !!success
                },
                "*"
                );
        }
    }

    function finishCommand(quality, success)
    {
        setCommandMode(false);
        reportResult(quality, success);
    }

    function qualityOptionIsSelected(element)
    {
        if (!element)
        {
            return false;
        }

        return element.classList.contains("ytp-menuitem-checked") ||
            element.classList.contains("ytp-menuitem-selected") ||
            element.getAttribute("aria-checked") === "true" ||
            element.getAttribute("aria-selected") === "true" ||
            !!element.querySelector(
                ".ytp-menuitem-checked, .ytp-menuitem-selected, "
                + "[aria-checked=\"true\"], [aria-selected=\"true\"]"
                );
    }

    function verifyQualitySelection(quality, option, attempt)
    {
        if (qualityOptionIsSelected(option))
        {
            finishCommand(quality, true);
            return;
        }

        if (attempt < 12)
        {
            window.setTimeout(
                function()
                {
                    verifyQualitySelection(
                        quality,
                        option,
                        attempt + 1
                        );
                },
                150
                );
        }
        else
        {
            finishCommand(quality, false);
        }
    }

    function selectQualityOption(quality, attempt)
    {
        var qualityOption = findMenuItem(
            qualityMatcher(quality)
            );

        if (!qualityOption)
        {
            if (attempt < 15)
            {
                window.setTimeout(
                    function()
                    {
                        selectQualityOption(
                            quality,
                            attempt + 1
                            );
                    },
                    120
                    );
            }
            else
            {
                finishCommand(quality, false);
            }

            return;
        }

        qualityOption.click();

        window.setTimeout(
            function()
            {
                verifyQualitySelection(
                    quality,
                    qualityOption,
                    0
                    );
            },
            180
            );
    }

    function openQualityMenu(quality, attempt)
    {
        var qualityItem = findMenuItem(
            /quality|الجودة|qualité|qualidade|qualität/i
            );

        if (!qualityItem)
        {
            if (attempt < 15)
            {
                window.setTimeout(
                    function()
                    {
                        openQualityMenu(
                            quality,
                            attempt + 1
                            );
                    },
                    120
                    );
            }
            else
            {
                finishCommand(quality, false);
            }

            return;
        }

        qualityItem.click();
        selectQualityOption(quality, 0);
    }

    function chooseQuality(quality, attempt)
    {
        installStyle();

        var settingsButton =
            document.querySelector(
                ".ytp-settings-button, "
                + "[aria-label*=\"Settings\"], "
                + "[data-tooltip-target-id=\"ytp-settings-button\"]"
                );

        if (!settingsButton)
        {
            if (attempt < 12)
            {
                window.setTimeout(
                    function()
                    {
                        chooseQuality(quality, attempt + 1);
                    },
                    100
                    );
            }
            else
            {
                finishCommand(quality, false);
            }

            return;
        }

        setCommandMode(true);
        settingsButton.click();

        window.setTimeout(
            function()
            {
                openQualityMenu(quality, 0);
            },
            120
            );
    }

    installStyle();

    window.addEventListener(
        "message",
        function(event)
        {
            var data = event.data;

            if (!data ||
                data.source !== "ssc-player" ||
                data.type !== "set-quality")
            {
                return;
            }

            chooseQuality(
                String(data.quality || "auto"),
                0
                );
        }
        );
})();
)JS");

    QWebEngineScript frameBridge;
    frameBridge.setName(
        QStringLiteral("ssc-youtube-quality-bridge")
        );
    frameBridge.setInjectionPoint(
        QWebEngineScript::DocumentCreation
        );
    frameBridge.setWorldId(
        QWebEngineScript::MainWorld
        );
    frameBridge.setRunsOnSubFrames(true);
    frameBridge.setSourceCode(source);

    m_view->page()->scripts().insert(frameBridge);
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

var requestedPlaybackRate = 1.0;

var requestedPlaybackQuality = "auto";

function reportQualityCommandResult(quality, success)
{
    if (qtBridge && qtBridge.onJsPlaybackQualityCommandResult)
    {
        qtBridge.onJsPlaybackQualityCommandResult(
            String(quality || "auto"),
            !!success
            );
    }
}

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

function applyRequestedPlaybackRate()
{
    if (player &&
        youtubePlayerReady &&
        player.setPlaybackRate)
    {
        player.setPlaybackRate(
            requestedPlaybackRate
            );
    }
}

function sendFrameQualityCommand(quality, attempt)
{
    var iframe =
        document.querySelector("#player iframe");

    if (!iframe || !iframe.contentWindow)
    {
        if (attempt < 20)
        {
            window.setTimeout(
                function()
                {
                    sendFrameQualityCommand(quality, attempt + 1);
                },
                100
                );
        }
        else
        {
            reportQualityCommandResult(quality, false);
        }

        return;
    }

    iframe.contentWindow.postMessage(
        {
            source: "ssc-player",
            type: "set-quality",
            quality: quality
        },
        "*"
        );
}

function applyRequestedPlaybackQuality()
{
    if (!player || !youtubePlayerReady)
    {
        return;
    }

    // YouTube's IFrame API no longer applies setPlaybackQuality(). The only
    // honest path available here is the native quality menu inside the child
    // iframe; its result is reported only after the selected menu item is
    // visibly marked as selected.
    sendFrameQualityCommand(
        requestedPlaybackQuality,
        0
        );
}

function onYouTubeFrameMessage(event)
{
    var data = event.data;

    if (!data ||
        data.source !== "ssc-youtube-frame" ||
        data.type !== "quality-result" ||
        !qtBridge)
    {
        return;
    }

    reportQualityCommandResult(
        String(data.quality || "auto"),
        !!data.success
        );
}

window.addEventListener(
    "message",
    onYouTubeFrameMessage
    );

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

                    // Keep the native settings menu available as the
                    // backend for the custom quality button. The injected
                    // child-frame stylesheet keeps it hidden from users.
                    controls: 1,

                    // Keep keyboard control in the native Qt player only.
                    disablekb: 1,

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


                    onPlaybackRateChange:
                        onPlaybackRateChange,


                    onPlaybackQualityChange:
                        onPlaybackQualityChange,


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

    applyRequestedPlaybackRate();

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

function onPlaybackRateChange(event)
{
    if (qtBridge && qtBridge.onJsPlaybackRateChanged)
    {
        qtBridge.onJsPlaybackRateChanged(event.data);
    }
}

function onPlaybackQualityChange(event)
{
    if (qtBridge && qtBridge.onJsPlaybackQualityChanged)
    {
        qtBridge.onJsPlaybackQualityChanged(event.data);
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

    // Loading a new video resets the playback rate to 1. Re-apply the
    // selected rate after YouTube finishes loading the new video.
    window.setTimeout(
        applyRequestedPlaybackRate,
        250
        );

    if (requestedPlaybackQuality !== "auto")
    {
        window.setTimeout(
            applyRequestedPlaybackQuality,
            500
            );
    }
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


function setVideoPlaybackRate(rate)
{
    requestedPlaybackRate = Number(rate);

    applyRequestedPlaybackRate();
}


function setVideoPlaybackQuality(quality)
{
    requestedPlaybackQuality =
        String(quality || "auto").toLowerCase();

    applyRequestedPlaybackQuality();
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

            runJavaScript(
                QString("setVideoPlaybackRate(%1);").arg(
                    QString::number(m_playbackRate, 'f', 2)
                    )
                );

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

void YouTubePlayer::onJsPlaybackRateChanged(double rate)
{
    if (rate <= 0.0)
    {
        return;
    }

    m_playbackRate = rate;
    emit playbackRateChanged(rate);
}

void YouTubePlayer::onJsPlaybackQualityChanged(const QString &quality)
{
    const QString normalized = quality.trimmed().toLower();

    if (normalized.isEmpty())
    {
        return;
    }

    m_playbackQuality = normalized;
    emit playbackQualityChanged(normalized);
}

void YouTubePlayer::onJsPlaybackQualityCommandResult(
    const QString &quality,
    bool success)
{
    if (success)
    {
        onJsPlaybackQualityChanged(quality);
    }
    else
    {
        emit playbackQualityChangeFailed(quality);
    }
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

void YouTubePlayer::setPlaybackRate(double rate)
{
    if (rate < 0.25 || rate > 2.0)
    {
        return;
    }

    m_playbackRate = rate;

    if (m_view && m_view->page())
    {
        m_view->page()->runJavaScript(
            QString("setVideoPlaybackRate(%1);").arg(
                QString::number(rate, 'f', 2)
                )
            );
    }
}

void YouTubePlayer::setPlaybackQuality(const QString &quality)
{
    const QString normalized = quality.trimmed().toLower();

    const QStringList supportedQualities =
        {
            QStringLiteral("auto"),
            QStringLiteral("small"),
            QStringLiteral("medium"),
            QStringLiteral("large"),
            QStringLiteral("hd480"),
            QStringLiteral("hd720"),
            QStringLiteral("hd1080"),
            QStringLiteral("highres")
        };

    if (!supportedQualities.contains(normalized))
    {
        return;
    }

    if (m_view && m_view->page())
    {
        m_view->page()->runJavaScript(
            QString("setVideoPlaybackQuality('%1');")
                .arg(normalized)
            );
    }
}
