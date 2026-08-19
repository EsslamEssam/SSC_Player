#include "Logger.h"

#include <QDebug>

#include <openssl/sha.h>
#include <QCryptographicHash>
#include <QDebug>


Logger::Logger(QObject *parent)
    : QObject(parent)
{

}


void Logger::info(const QString &message)
{
    qDebug() << "[INFO]" << message;
}


void Logger::error(const QString &message)
{
    qDebug() << "[ERROR]" << message;
}
