#include "CourseManager.h"


CourseManager::CourseManager()
{
    Course autocad;

    autocad.setId("AutoCAD");
    autocad.setName("AutoCAD Professional Course");

    autocad.setShortDescription(
        "2D Drafting and Architectural Design"
        );

    autocad.setDescription(
        "Complete AutoCAD course covering "
        "architectural drawings, drafting techniques "
        "and practical engineering projects."
        );

    autocad.setLessonsCount(35);
    autocad.setDuration("20 Hours");
    autocad.setVersion("1.0");

    autocad.setActivated(true);

    courses.append(autocad);



    Course sap;

    sap.setId("SAP2000");
    sap.setName("SAP2000 Structural Analysis Course");

    sap.setShortDescription(
        "Structural Analysis and Design"
        );

    sap.setDescription(
        "Learn structural modeling, analysis and "
        "design using SAP2000 with practical examples."
        );

    sap.setLessonsCount(40);
    sap.setDuration("25 Hours");
    sap.setVersion("1.0");

    sap.setActivated(false);

    courses.append(sap);



    Course revit;

    revit.setId("Revit");
    revit.setName("Revit BIM Course");

    revit.setShortDescription(
        "BIM Modeling and Architecture"
        );

    revit.setDescription(
        "Professional Revit training for BIM modeling "
        "and architectural projects."
        );

    revit.setLessonsCount(30);
    revit.setDuration("18 Hours");
    revit.setVersion("1.0");

    revit.setActivated(false);

    courses.append(revit);
}


CourseManager& CourseManager::instance()
{
    static CourseManager manager;
    return manager;
}


QList<Course> CourseManager::getCourses() const
{
    return courses;
}


Course* CourseManager::getCourseById(const QString& id)
{
    for (Course &course : courses)
    {
        if (course.getId() == id)
        {
            return &course;
        }
    }

    return nullptr;
}


void CourseManager::addCourse(const Course& course)
{
    courses.append(course);
}