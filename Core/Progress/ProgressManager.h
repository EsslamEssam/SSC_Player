#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

#include <QMap>
#include <QString>

#include "LessonProgress.h"
#include "ProgressStorage.h"


class ProgressManager
{
public:

    static ProgressManager& instance();


    void setCurrentLesson(
        const QString &lessonId
        );


    QString currentLesson() const;

    double currentProgress() const;

    QMap<QString, LessonProgress> allProgress() const;

    void loadProgress();

    void saveProgress();


    void updateProgress(
        qint64 position,
        qint64 duration
        );


    LessonProgress progress(
        const QString &lessonId
        ) const;


    bool isCompleted(
        const QString &lessonId
        ) const;




private:

    ProgressManager();


    QString m_currentLessonId;


    QMap<QString, LessonProgress> m_progress;


    ProgressStorage m_storage;
};


#endif // PROGRESSMANAGER_H