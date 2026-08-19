#include "OfflineCacheManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QDebug>


QString OfflineCacheManager::cacheRootPath()
{
    QString basePath =
        QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation
            );


    QString path =
        QDir(basePath)
            .filePath("OfflineCache");


    QDir().mkpath(path);


    return path;
}


QString OfflineCacheManager::lessonDirectory(
    const QString& lessonId)
{
    QString path =
        QDir(cacheRootPath())
            .filePath(lessonId);


    QDir().mkpath(path);


    return path;
}


QString OfflineCacheManager::lessonFilePath(
    const QString& lessonId)
{
    return QDir(
               lessonDirectory(lessonId)
               ).filePath(
            "video.dat"
            );
}


bool OfflineCacheManager::createOfflineFile(
    const QString& lessonId,
    const QString& sourceFile)
{
    if (lessonId.isEmpty())
        return false;


    if (!QFile::exists(sourceFile))
    {
        qDebug()
        << "Offline source does not exist:"
        << sourceFile;

        return false;
    }


    QString destination =
        lessonFilePath(lessonId);


    if (QFile::exists(destination))
    {
        QFile::remove(destination);
    }


    if (!QFile::copy(
            sourceFile,
            destination))
    {
        qDebug()
        << "Cannot create offline file";

        return false;
    }


    qDebug()
        << "Offline file created:"
        << destination;


    return true;
}


bool OfflineCacheManager::isAvailable(
    const QString& lessonId)
{
    QString path =
        lessonFilePath(lessonId);


    return QFile::exists(path);
}


bool OfflineCacheManager::saveFirstOpenedTime(
    const QString& lessonId)
{
    QString directory =
        lessonDirectory(lessonId);


    QFile file(
        QDir(directory)
            .filePath("metadata.dat")
        );


    if (!file.open(
            QIODevice::WriteOnly))
    {
        return false;
    }


    QDataStream stream(&file);


    QDateTime now =
        QDateTime::currentDateTimeUtc();


    stream << now;


    file.close();


    qDebug()
        << "First opened time saved:"
        << now;


    return true;
}


QDateTime OfflineCacheManager::firstOpenedTime(
    const QString& lessonId)
{
    QString path =
        QDir(
            lessonDirectory(lessonId)
            ).filePath(
                "metadata.dat"
                );


    QFile file(path);


    if (!file.open(
            QIODevice::ReadOnly))
    {
        return {};
    }


    QDataStream stream(&file);


    QDateTime time;


    stream >> time;


    file.close();


    return time;
}


QDateTime OfflineCacheManager::expirationTime(
    const QString& lessonId)
{
    QDateTime first =
        firstOpenedTime(lessonId);


    if (!first.isValid())
        return {};


    return first.addMSecs(
        OFFLINE_DURATION_MS
        );
}


bool OfflineCacheManager::isExpired(
    const QString& lessonId)
{
    QDateTime expiration =
        expirationTime(lessonId);


    if (!expiration.isValid())
        return true;


    QDateTime now =
        QDateTime::currentDateTimeUtc();


    return now >= expiration;
}


bool OfflineCacheManager::removeLesson(
    const QString& lessonId)
{
    QString directory =
        lessonDirectory(lessonId);


    QDir dir(directory);


    if (!dir.exists())
        return true;


    bool result =
        dir.removeRecursively();


    if (result)
    {
        qDebug()
        << "Offline lesson removed:"
        << lessonId;
    }


    return result;
}


bool OfflineCacheManager::prepareLesson(
    const QString& lessonId,
    const QString& sourceFile)
{
    if (lessonId.isEmpty())
        return false;


    // ---------------------------------
    // 1. يوجد Offline Cache
    // ---------------------------------

    if (isAvailable(lessonId))
    {
        // ---------------------------------
        // 1A. انتهت الصلاحية
        // ---------------------------------

        if (isExpired(lessonId))
        {
            qDebug()
            << "Offline cache expired:"
            << lessonId;


            if (!removeLesson(lessonId))
            {
                qDebug()
                << "Cannot remove expired lesson";

                return false;
            }


            return false;
        }


        // ---------------------------------
        // 1B. ما زال صالحًا
        // ---------------------------------

        qDebug()
            << "Using existing offline cache:"
            << lessonId;


        qDebug()
            << "Expires At:"
            << expirationTime(lessonId);


        return true;
    }


    // ---------------------------------
    // 2. لا يوجد Offline Cache
    // ---------------------------------

    if (!QFile::exists(sourceFile))
    {
        qDebug()
        << "Source file does not exist:"
        << sourceFile;

        return false;
    }


    // ---------------------------------
    // 3. إنشاء Offline Cache
    // ---------------------------------

    if (!createOfflineFile(
            lessonId,
            sourceFile))
    {
        return false;
    }


    // ---------------------------------
    // 4. تسجيل أول فتح
    // ---------------------------------

    if (!saveFirstOpenedTime(
            lessonId))
    {
        removeLesson(lessonId);

        return false;
    }


    qDebug()
        << "Offline cache prepared:"
        << lessonId;


    qDebug()
        << "First Opened:"
        << firstOpenedTime(
               lessonId
               );


    qDebug()
        << "Expires At:"
        << expirationTime(
               lessonId
               );


    return true;
}