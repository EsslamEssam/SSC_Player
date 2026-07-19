#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>


class ConfigManager : public QObject
{
    Q_OBJECT

public:

    explicit ConfigManager(QObject *parent = nullptr);

    QString appName() const;
    QString appVersion() const;
    QString serverUrl() const;

};


#endif // CONFIGMANAGER_H
