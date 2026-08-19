#ifndef LESSON_H
#define LESSON_H

#include <QString>


class Lesson
{

public:

    Lesson();


    // Getters

    QString getId() const;

    QString getCourseId() const;

    QString getTitle() const;

    QString getDescription() const;

    QString getVideoUrl() const;

    int getOrder() const;

    QString getDuration() const;

    bool isCompleted() const;



    // Setters

    void setId(const QString &id);

    void setCourseId(const QString &courseId);

    void setTitle(const QString &title);

    void setDescription(const QString &description);

    void setVideoUrl(const QString &videoUrl);

    void setOrder(int order);

    void setDuration(const QString &duration);

    void setCompleted(bool completed);



private:

    QString id;

    QString courseId;

    QString title;

    QString description;

    QString videoUrl;

    int order;

    QString duration;

    bool completed;

};


#endif // LESSON_H
