#include "VideoManager.h"


VideoManager::VideoManager()
{

}


QString VideoManager::getVideoPath(const Lesson &lesson)
{
    Q_UNUSED(lesson);

    // Temporary encrypted video path
    // Later this will come from database/server

    return QString::fromUtf8(
        "D:/SCCP_Player/TestVideos/test.enc"
        );
}