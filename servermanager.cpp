#include "servermanager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

ServerManager::ServerManager(QObject *parent) :
    QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
{

}

void ServerManager::getLatestVersion(){

    QUrl url(hostName +getLatestVersionRoute);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &ServerManager::getLatestVersionRequestFinished);

    runningRequests.insert(reply);

}

void ServerManager::downloadNewVersion(){

    QUrl url(hostName + downloadVersionRoute);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &ServerManager::downloadNewVersionRequestFinished);

    runningRequests.insert(reply);

}

void ServerManager::sendLog(const QString &log){

    QJsonObject json;
    json["message"] = log;
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    QUrl url(hostName + sendLogRoute);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, &ServerManager::sendLogRequestFinished);

    runningRequests.insert(reply);

}

void ServerManager::getLatestVersionRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(replyHasError(reply)){
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);

    if(!jsonDoc.isObject()){
        emit errorHasOcurred("Error al obtener la última version. Respuesta del servidor mal formateada!");
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
        emit errorHasOcurred("Versión obtenida del servidor no válida!");
    }

    reply->deleteLater();

}

void ServerManager::downloadNewVersionRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(replyHasError(reply)){
        reply->deleteLater();
        return;
    }

    QVariant header = reply->header(QNetworkRequest::ContentDispositionHeader);

    if(!header.isValid()){
        emit errorHasOcurred("Error en la respuesta. Header invalido!");
        reply->deleteLater();
        return;
    }

    QString contentDisposition = header.toString();

    if(contentDisposition.isEmpty()){
        emit errorHasOcurred("Error en la respuesta. Header valido pero sin contenido!");
        reply->deleteLater();
        return;
    }

    static QRegularExpression regex(R"regex(filename="?([^";]+)"?)regex");
    QRegularExpressionMatch match = regex.match(contentDisposition);

    if (!match.hasMatch()) {
        emit errorHasOcurred("Error en la respuesta. No se encontró el nombre del archivo");
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

    replyHasError(reply);

    reply->deleteLater();

}

bool ServerManager::replyHasError(QNetworkReply *reply){

    if(reply->error() == QNetworkReply::OperationCanceledError){

        runningRequests.remove(reply);

        if(runningRequests.isEmpty()){
            emit readyToQuit();
        }
        return true;
    }

    if(reply->error() != QNetworkReply::NoError){
        emit errorHasOcurred(reply->errorString());
        return true;
    }

    return false;

}

void ServerManager::cancelRequests(){

    if(runningRequests.isEmpty()){
        emit readyToQuit();
        return;
    }

    for (QSet<QNetworkReply*>::const_iterator it = runningRequests.constBegin(); it != runningRequests.constEnd(); ++it) {
        QNetworkReply *reply = *it;
        if (reply && reply->isRunning())
            reply->abort();
    }

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
