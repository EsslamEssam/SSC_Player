#include "LessonProgress.h"

LessonProgress::LessonProgress()
    : m_lastPosition(0),
    m_progress(0.0),
    m_completed(false)
{
}

LessonProgress::LessonProgress(
    const QString &lessonId,
    qint64 lastPosition,
    double progress,
    bool completed)
    : m_lessonId(lessonId),
    m_lastPosition(lastPosition),
    m_progress(progress),
    m_completed(completed)
{
}

QString LessonProgress::lessonId() const
{
    return m_lessonId;
}

void LessonProgress::setLessonId(const QString &id)
{
    m_lessonId = id;
}

qint64 LessonProgress::lastPosition() const
{
    return m_lastPosition;
}

void LessonProgress::setLastPosition(qint64 position)
{
    m_lastPosition = position;
}

double LessonProgress::progress() const
{
    return m_progress;
}

void LessonProgress::setProgress(double value)
{
    m_progress = value;
}

bool LessonProgress::completed() const
{
    return m_completed;
}

void LessonProgress::setCompleted(bool value)
{
    m_completed = value;
}