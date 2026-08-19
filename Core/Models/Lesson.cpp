#include "Lesson.h"


Lesson::Lesson()
{
    order = 0;
    completed = false;
}


// Getters

QString Lesson::getId() const
{
    return id;
}

QString Lesson::getCourseId() const
{
    return courseId;
}


QString Lesson::getTitle() const
{
    return title;
}


QString Lesson::getDescription() const
{
    return description;
}


QString Lesson::getVideoUrl() const
{
    return videoUrl;
}


int Lesson::getOrder() const
{
    return order;
}


QString Lesson::getDuration() const
{
    return duration;
}


bool Lesson::isCompleted() const
{
    return completed;
}



// Setters

void Lesson::setId(const QString &id)
{
    this->id = id;
}

void Lesson::setCourseId(const QString &courseId)
{
    this->courseId = courseId;
}


void Lesson::setTitle(const QString &title)
{
    this->title = title;
}


void Lesson::setDescription(const QString &description)
{
    this->description = description;
}


void Lesson::setVideoUrl(const QString &videoUrl)
{
    this->videoUrl = videoUrl;
}


void Lesson::setOrder(int order)
{
    this->order = order;
}


void Lesson::setDuration(const QString &duration)
{
    this->duration = duration;
}


void Lesson::setCompleted(bool completed)
{
    this->completed = completed;
}