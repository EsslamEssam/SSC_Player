#include "ProgressManager.h"


ProgressManager::ProgressManager()
{
    m_progress = m_storage.load();
}


ProgressManager& ProgressManager::instance()
{
    static ProgressManager manager;

    return manager;
}



void ProgressManager::setCurrentLesson(
    const QString &lessonId
    )
{
    m_currentLessonId = lessonId;
}



QString ProgressManager::currentLesson() const
{
    return m_currentLessonId;
}



double ProgressManager::currentProgress() const
{
    if(m_currentLessonId.isEmpty())
        return 0.0;


    return m_progress
        .value(m_currentLessonId)
        .progress();
}



void ProgressManager::updateProgress(
    qint64 position,
    qint64 duration
    )
{

    if(m_currentLessonId.isEmpty())
        return;


    double percent = 0.0;


    if(duration > 0)
    {
        percent =
            (double(position) / duration) * 100.0;
    }


    bool completed =
        percent >= 95.0;



    qint64 oldPosition =
        m_progress
            .value(m_currentLessonId)
            .lastPosition();


    qint64 maxPosition =
        qMax(oldPosition, position);



    double maxPercent = 0.0;


    if(duration > 0)
    {
        maxPercent =
            (double(maxPosition) / duration) * 100.0;
    }


    bool maxCompleted =
        maxPercent >= 95.0;



    m_progress[m_currentLessonId] =
        LessonProgress(
            m_currentLessonId,
            maxPosition,
            maxPercent,
            maxCompleted
            );


    saveProgress();
}




LessonProgress ProgressManager::progress(
    const QString &lessonId
    ) const
{
    return m_progress.value(
        lessonId
        );
}




bool ProgressManager::isCompleted(
    const QString &lessonId
    ) const
{
    return m_progress
        .value(lessonId)
        .completed();
}



void ProgressManager::saveProgress()
{
    m_storage.save(m_progress);
}



void ProgressManager::loadProgress()
{
    m_progress = m_storage.load();
}