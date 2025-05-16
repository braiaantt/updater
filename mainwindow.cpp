#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initConfig(){

    QString appPath = QCoreApplication::applicationDirPath();
    QFile file(appPath + "updater-config.json");
    QByteArray fileBytes;

    if(file.open(QFile::ReadOnly)){

        fileBytes = file.readAll();
        file.close();

    } else {
        qDebug()<<"error al abrir el archivo: "<<file.fileName();
        return;
    }

    getConfigJsonInfo(fileBytes);

}

void MainWindow::getConfigJsonInfo(QByteArray &fileBytes){

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileBytes, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Error al leer el archivo JSON:" << parseError.errorString();
        return;
    }

    if(!jsonDoc.isObject()){
        qDebug()<<"Formato del archivo de configuración incorrecto";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    //get hostName
    hostName = jsonObj["hostName"].toString();

    //get main app name
    mainAppVersion = jsonObj["mainAppVersion"].toString();

    //get endpoints
    if(jsonObj["endpoints"].isArray()){
        QJsonArray array = jsonObj["endpoints"].toArray();

        for(const QJsonValue &arrayValue : array){

            const QJsonObject &arrayObject = arrayValue.toObject();

            QString route = arrayObject["route"].toString();
            QString method = arrayObject["method"].toString();

            Endpoint endpoint(route, method);
            endpoints << endpoint;

        }

    } else {

        QJsonObject endpointObj = jsonObj["endpoints"].toObject();

        QString route = endpointObj["route"].toString();
        QString method = endpointObj["method"].toString();

        Endpoint endpoint(route, method);
        endpoints << endpoint;

    }

}

void MainWindow::getLatestAppVersion(){

    Endpoint &endpoint = endpoints[0];
    QUrl url(hostName + endpoint.getRoute());
    QNetworkRequest request(url);
    QNetworkReply *reply = nullptr;

    if(endpoint.getMethod() == "GET"){
        reply = manager->get(request);
    }

    if(reply){
        connect(reply, &QNetworkReply::finished, this, &MainWindow::latestAppVersionRequestFinished);
    } else {
        qDebug()<<"No se ha podido realizar la solicitud";
    }

}

void MainWindow::latestAppVersionRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Hubo un error en la respuesta";
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);

    if(jsonDoc.isObject()){
        QJsonObject jsonObj = jsonDoc.object();
        QString latestVersion = jsonObj["data"].toString();

        if(isUpdateRequired(latestVersion)){
            downloadNewUpdate();
        }

    }

    reply->deleteLater();

}

bool MainWindow::isUpdateRequired(QString &strLatestVersion){

    double latestVersion = strLatestVersion.toDouble();
    double currentVersion = mainAppVersion.toDouble();

    if(latestVersion > currentVersion) return true;

    return false;

}
