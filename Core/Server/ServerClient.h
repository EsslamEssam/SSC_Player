#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class ServerClient : public QObject
{
    Q_OBJECT

public:

    explicit ServerClient(QObject *parent = nullptr);

    void get(
        const QString &endpoint
        );

    void post(
        const QString &endpoint,
        const QJsonObject &data
        );

signals:

    void requestFinished(
        bool success,
        const QByteArray &response
        );

private:

    QNetworkAccessManager m_networkManager;
};

#endif // SERVERCLIENT_H