#include "ServerClient.h"

#include "ServerConfig.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QDebug>


ServerClient::ServerClient(QObject *parent)
    : QObject(parent)
{
}


// ============================================================
// GET
// ============================================================

void ServerClient::get(
    const QString &endpoint
    )
{
    const QString url =
        ServerConfig::baseUrl() + endpoint;


    qDebug()
        << "SERVER GET URL:"
        << url;


    QNetworkRequest request;

    request.setUrl(
        QUrl(url)
        );


    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );


    const QByteArray publishableKey =
        ServerConfig::publishableKey().toUtf8();


    request.setRawHeader(
        "apikey",
        publishableKey
        );


    request.setRawHeader(
        "Authorization",
        QByteArray("Bearer ")
            + publishableKey
        );


    QNetworkReply *reply =
        m_networkManager.get(request);


    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [this, reply]()
        {
            const QByteArray response =
                reply->readAll();


            const QVariant status =
                reply->attribute(
                    QNetworkRequest::HttpStatusCodeAttribute
                    );


            qDebug()
                << "SERVER HTTP STATUS:"
                << status;


            qDebug()
                << "SERVER NETWORK ERROR:"
                << reply->error();


            qDebug()
                << "SERVER ERROR STRING:"
                << reply->errorString();


            qDebug()
                << "SERVER RESPONSE:"
                << response;


            const bool success =
                reply->error()
                == QNetworkReply::NoError;


            emit requestFinished(
                success,
                response
                );


            reply->deleteLater();
        }
        );
}


// ============================================================
// POST
// ============================================================

void ServerClient::post(
    const QString &endpoint,
    const QJsonObject &data
    )
{
    const QString url =
        ServerConfig::baseUrl() + endpoint;


    qDebug()
        << "SERVER POST URL:"
        << url;


    const QByteArray publishableKey =
        ServerConfig::publishableKey().toUtf8();


    qDebug()
        << "SERVER PUBLISHABLE KEY EXISTS:"
        << !publishableKey.isEmpty();


    QNetworkRequest request;

    request.setUrl(
        QUrl(url)
        );


    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );


    request.setRawHeader(
        "apikey",
        publishableKey
        );


    request.setRawHeader(
        "Authorization",
        QByteArray("Bearer ")
            + publishableKey
        );


    const QByteArray body =
        QJsonDocument(data).toJson(
            QJsonDocument::Compact
            );


    qDebug()
        << "SERVER POST BODY:"
        << body;


    QNetworkReply *reply =
        m_networkManager.post(
            request,
            body
            );


    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [this, reply]()
        {
            const QByteArray response =
                reply->readAll();


            const QVariant status =
                reply->attribute(
                    QNetworkRequest::HttpStatusCodeAttribute
                    );


            qDebug()
                << "SERVER HTTP STATUS:"
                << status;


            qDebug()
                << "SERVER NETWORK ERROR:"
                << reply->error();


            qDebug()
                << "SERVER ERROR STRING:"
                << reply->errorString();


            qDebug()
                << "SERVER RESPONSE:"
                << response;


            const bool success =
                reply->error()
                == QNetworkReply::NoError;


            emit requestFinished(
                success,
                response
                );


            reply->deleteLater();
        }
        );
}