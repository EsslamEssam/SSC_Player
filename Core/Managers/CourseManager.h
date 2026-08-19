#ifndef COURSEMANAGER_H
#define COURSEMANAGER_H

#include <QList>
#include "../Models/Course.h"

class CourseManager
{
public:

    static CourseManager& instance();

    QList<Course> getCourses() const;

    Course* getCourseById(const QString& id);

    void addCourse(const Course& course);


private:

    CourseManager();

    QList<Course> courses;
};

#endif // COURSEMANAGER_H
