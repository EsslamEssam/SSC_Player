#include "ProgressStorage.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>


ProgressStorage::ProgressStorage()
{
    QString folder =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
            );


    QDir dir(folder);

    if(!dir.exists())
    {
        dir.mkpath(".");
    }


    m_filePath =
        folder + "/progress.json";
}



bool ProgressStorage::save(
    const QMap<QString, LessonProgress> &progress
    )
{
    QJsonObject root;


    for(auto it = progress.begin();
         it != progress.end();
         ++it)
    {
        QJsonObject lesson;


        lesson["position"] =
            static_cast<qint64>(
                it.value().lastPosition()
                );


        lesson["progress"] =
            it.value().progress();


        lesson["completed"] =
            it.value().completed();



        root[it.key()] = lesson;
    }



    QFile file(m_filePath);


    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Cannot open progress file:"
                 << m_filePath;

        return false;
    }


    qDebug() << "Saving progress to:"
             << m_filePath;



    QJsonDocument doc(root);


    file.write(
        doc.toJson()
        );


    file.close();


    return true;
}



QMap<QString, LessonProgress>
ProgressStorage::load()
{
    QMap<QString, LessonProgress> result;


    QFile file(m_filePath);


    if(!file.exists())
        return result;


    if(!file.open(QIODevice::ReadOnly))
        return result;



    QJsonDocument doc =
        QJsonDocument::fromJson(
            file.readAll()
            );


    file.close();



    QJsonObject root =
        doc.object();



    const QStringList keys = root.keys();

    for(const QString &key : keys)
    {
        QJsonObject lesson =
            root[key].toObject();



        LessonProgress progress(
            key,
            lesson["position"].toInt(),
            lesson["progress"].toDouble(),
            lesson["completed"].toBool()
            );


        result[key] = progress;
    }


    return result;
}