#include "Course.h"


Course::Course()
{
    lessonsCount = 0;
    activated = false;
}


// Getters

QString Course::getId() const
{
    return id;
}


QString Course::getName() const
{
    return name;
}


QString Course::getShortDescription() const
{
    return shortDescription;
}


QString Course::getDescription() const
{
    return description;
}


QString Course::getThumbnail() const
{
    return thumbnail;
}


QString Course::getCoverImage() const
{
    return coverImage;
}


int Course::getLessonsCount() const
{
    return lessonsCount;
}


QString Course::getDuration() const
{
    return duration;
}


bool Course::isActivated() const
{
    return activated;
}


QString Course::getVersion() const
{
    return version;
}



// Setters

void Course::setId(const QString &id)
{
    this->id = id;
}


void Course::setName(const QString &name)
{
    this->name = name;
}


void Course::setShortDescription(const QString &shortDescription)
{
    this->shortDescription = shortDescription;
}


void Course::setDescription(const QString &description)
{
    this->description = description;
}


void Course::setThumbnail(const QString &thumbnail)
{
    this->thumbnail = thumbnail;
}


void Course::setCoverImage(const QString &coverImage)
{
    this->coverImage = coverImage;
}


void Course::setLessonsCount(int lessonsCount)
{
    this->lessonsCount = lessonsCount;
}


void Course::setDuration(const QString &duration)
{
    this->duration = duration;
}


void Course::setActivated(bool activated)
{
    this->activated = activated;
}


void Course::setVersion(const QString &version)
{
    this->version = version;
}

const QList<Lesson>& Course::getLessons() const
{
    return lessons;
}


void Course::addLesson(const Lesson &lesson)
{
    lessons.append(lesson);
}