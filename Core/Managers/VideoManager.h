#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QString>

#include "../Models/Lesson.h"


class VideoManager
{
public:

    VideoManager();


    QString getVideoPath(const Lesson &lesson);

};


#endif // VIDEOMANAGER_H