#include "LessonManager.h"
#include "../Progress/ProgressManager.h"


LessonManager::LessonManager()
{
    Lesson lesson1;

    lesson1.setId("AutoCAD_001");
    lesson1.setCourseId("AutoCAD");
    lesson1.setTitle("Introduction to AutoCAD");

    lesson1.setDescription(
        "Introduction to the interface and basic tools."
        );

    lesson1.setVideoUrl(
        "encrypted_video_autocad_001"
        );

    lesson1.setOrder(1);
    lesson1.setDuration("35 Minutes");

    lessons.append(lesson1);



    Lesson lesson2;

    lesson2.setId("AutoCAD_002");
    lesson2.setCourseId("AutoCAD");
    lesson2.setTitle("Drawing Tools");

    lesson2.setDescription(
        "Learn basic drawing and modification commands."
        );

    lesson2.setVideoUrl(
        "encrypted_video_autocad_002"
        );

    lesson2.setOrder(2);
    lesson2.setDuration("50 Minutes");

    lessons.append(lesson2);



    Lesson lesson3;

    lesson3.setId("AutoCAD_003");
    lesson3.setCourseId("AutoCAD");
    lesson3.setTitle("Architectural Project");

    lesson3.setDescription(
        "Create a complete architectural project."
        );

    lesson3.setVideoUrl(
        "encrypted_video_autocad_003"
        );

    lesson3.setOrder(3);
    lesson3.setDuration("1 Hour");

    lessons.append(lesson3);
}



LessonManager& LessonManager::instance()
{
    static LessonManager manager;

    return manager;
}



QList<Lesson> LessonManager::getLessons() const
{
    return lessons;
}



void LessonManager::addLesson(const Lesson &lesson)
{
    lessons.append(lesson);
}



QList<Lesson> LessonManager::getLessonsByCourseId(
    const QString &courseId) const
{
    QList<Lesson> result;


    for (const Lesson &lesson : lessons)
    {
        if (lesson.getCourseId() == courseId)
        {
            result.append(lesson);
        }
    }


    return result;
}



const Lesson* LessonManager::getLessonById(const QString &lessonId) const
{
    for (const Lesson &lesson : lessons)
    {
        if (lesson.getId() == lessonId)
        {
            return &lesson;
        }
    }

    return nullptr;
}



bool LessonManager::isLessonUnlocked(
    const QString &lessonId
    ) const
{
    const Lesson *currentLesson =
        getLessonById(lessonId);


    if(!currentLesson)
        return false;


    // أول درس مفتوح دائمًا
    if(currentLesson->getOrder() == 1)
        return true;



    int previousOrder =
        currentLesson->getOrder() - 1;



    for(const Lesson &lesson : lessons)
    {
        if(lesson.getCourseId() == currentLesson->getCourseId()
            &&
            lesson.getOrder() == previousOrder)
        {

            return ProgressManager::instance()
            .isCompleted(
                lesson.getId()
                );
        }
    }


    return false;
}