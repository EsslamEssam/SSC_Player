#ifndef LESSONPROGRESS_H
#define LESSONPROGRESS_H

#include <QString>

class LessonProgress
{
public:

    LessonProgress();

    LessonProgress(
        const QString &lessonId,
        qint64 lastPosition,
        double progress,
        bool completed
        );

    QString lessonId() const;
    void setLessonId(const QString &id);

    qint64 lastPosition() const;
    void setLastPosition(qint64 position);

    double progress() const;
    void setProgress(double value);

    bool completed() const;
    void setCompleted(bool value);

private:

    QString m_lessonId;

    qint64 m_lastPosition;

    double m_progress;

    bool m_completed;
};

#endif