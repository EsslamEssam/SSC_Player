#ifndef COURSE_H
#define COURSE_H

#include <QString>
#include <QList>
#include "Lesson.h"


class Course
{

public:

    Course();


    // Getters

    QString getId() const;

    QString getName() const;

    QString getShortDescription() const;

    QString getDescription() const;

    QString getThumbnail() const;

    QString getCoverImage() const;

    int getLessonsCount() const;

    QString getDuration() const;

    bool isActivated() const;

    QString getVersion() const;

    const QList<Lesson>& getLessons() const;




    // Setters

    void setId(const QString &id);

    void setName(const QString &name);

    void setShortDescription(const QString &shortDescription);

    void setDescription(const QString &description);

    void setThumbnail(const QString &thumbnail);

    void setCoverImage(const QString &coverImage);

    void setLessonsCount(int lessonsCount);

    void setDuration(const QString &duration);

    void setActivated(bool activated);

    void setVersion(const QString &version);

    void addLesson(const Lesson &lesson);



private:

    QString id;

    QString name;

    QString shortDescription;

    QString description;

    QString thumbnail;

    QString coverImage;

    int lessonsCount;

    QString duration;

    bool activated;

    QString version;

    QList<Lesson> lessons;
};


#endif // COURSE_H
