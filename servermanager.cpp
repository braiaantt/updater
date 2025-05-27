#include "servermanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

ServerManager::ServerManager(QObject *parent) :
    QObject(parent)
{

}

void ServerManager::getLatestVersion(){

    QUrl url(hostName + "/" +getLatestVersionRoute);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &ServerManager::getLatestVersionRequestFinished);

}

void ServerManager::downloadNewVersion(QString &version){

    QUrl url(hostName + "/" + getLatestVersionRoute);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &ServerManager::downloadNewVersionRequestFinished);

}

void ServerManager::sendLog(QString &log){

    QByteArray postData = log.toUtf8();
    QUrl url(hostName + "/" + sendLogRoute);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    QNetworkReply *reply = networkManager->post(request, postData);

    connect(reply, &QNetworkReply::finished, this, &ServerManager::sendLogRequestFinished);

}

void ServerManager::getLatestVersionRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Hubo un error en la respuesta al obtener la ultima version";
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);

    if(!jsonDoc.isObject()){
        qDebug()<<"Respuesta mal formateada!";
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QString latestVersion = jsonObj["data"].toString();
    bool isDouble = false;
    double version = latestVersion.toDouble(&isDouble);

    if(isDouble){
        emit latestVersionReceived(version);
    } else {
        //emit error
    }

    reply->deleteLater();

}

void ServerManager::downloadNewVersionRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Hubo un error en la respuesta de la descarga";
        reply->deleteLater();
        return;
    }

    QVariant header = reply->header(QNetworkRequest::ContentDispositionHeader);

    if(!header.isValid()){
        qDebug()<<"El header de la respuesta no es valido!";
        reply->deleteLater();
        return;
    }

    QString contentDisposition = header.toString();

    if(contentDisposition.isEmpty()){
        qDebug()<<"Error en la respuesta. No hay nombre de archivo asignado";
        reply->deleteLater();
        return;
    }

    static QRegularExpression regex(R"regex(filename="?([^";]+)"?)regex");
    QRegularExpressionMatch match = regex.match(contentDisposition);

    if (!match.hasMatch()) {
        qDebug()<<"Error en la respuesta. No se encontró el nombre de archivo";
        reply->deleteLater();
        return;
    }

    QString fileName = match.captured(1);
    QByteArray fileBytes = reply->readAll();

    emit downloadFinished(fileName, fileBytes);

    reply->deleteLater();

}

void ServerManager::sendLogRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Error en la solicitud de log";
    }

    reply->deleteLater();

}

//setters
void ServerManager::setHostName(QString &_hostName){
    hostName = _hostName;
}

void ServerManager::setGetLatestVersionRoute(QString &_versionRoute){
    getLatestVersionRoute = _versionRoute;
}

void ServerManager::setDownloadVersionRoute(QString &_downloadVersionRoute){
    downloadVersionRoute = _downloadVersionRoute;
}

void ServerManager::setSendLogRoute(QString &_sendLogRoute){
    sendLogRoute = _sendLogRoute;
}
