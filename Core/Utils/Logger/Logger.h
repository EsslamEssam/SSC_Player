#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

class Logger : public QObject
{
    Q_OBJECT

public:

    explicit Logger(QObject *parent = nullptr);

    void info(const QString &message);
    void error(const QString &message);

};

#endif // LOGGER_H
