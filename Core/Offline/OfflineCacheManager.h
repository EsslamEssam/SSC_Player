#ifndef OFFLINECACHEMANAGER_H
#define OFFLINECACHEMANAGER_H

#include <QString>
#include <QDateTime>


class OfflineCacheManager
{
public:

    static QString cacheRootPath();


    static QString lessonDirectory(
        const QString& lessonId
        );


    static QString lessonFilePath(
        const QString& lessonId
        );


    static bool createOfflineFile(
        const QString& lessonId,
        const QString& sourceFile
        );


    static bool isAvailable(
        const QString& lessonId
        );


    static bool isExpired(
        const QString& lessonId
        );


    static bool saveFirstOpenedTime(
        const QString& lessonId
        );


    static QDateTime firstOpenedTime(
        const QString& lessonId
        );


    static QDateTime expirationTime(
        const QString& lessonId
        );


    static bool removeLesson(
        const QString& lessonId
        );


    static bool prepareLesson(
        const QString& lessonId,
        const QString& sourceFile
        );


private:

    static constexpr qint64 OFFLINE_DURATION_MS =
        60LL * 24LL * 60LL * 60LL * 1000LL;
};

#endif