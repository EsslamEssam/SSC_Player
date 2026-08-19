#ifndef LESSONMANAGER_H
#define LESSONMANAGER_H

#include <QList>
#include "../Models/Lesson.h"


class LessonManager
{

public:

    static LessonManager& instance();


    QList<Lesson> getLessons() const;


    void addLesson(const Lesson &lesson);


    QList<Lesson> getLessonsByCourseId(const QString &courseId) const;


    const Lesson* getLessonById(const QString &lessonId) const;


    bool isLessonUnlocked(
        const QString &lessonId
        ) const;



private:

    LessonManager();


    QList<Lesson> lessons;

};


#endif // LESSONMANAGER_H
