#ifndef PROGRESSSTORAGE_H
#define PROGRESSSTORAGE_H

#include <QMap>
#include <QString>

#include "LessonProgress.h"


class ProgressStorage
{
public:

    ProgressStorage();


    bool save(
        const QMap<QString, LessonProgress> &progress
        );


    QMap<QString, LessonProgress> load();

private:

    QString m_filePath;
};


#endif // PROGRESSSTORAGE_H