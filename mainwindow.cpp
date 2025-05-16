#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QDir>
#include <QProcess>

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

    //get main app version
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
            downloadNewUpdate(latestVersion);
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

void MainWindow::downloadNewUpdate(QString &latestVersion){

    Endpoint &endpoint = endpoints[1];
    QUrl url(hostName + endpoint.getRoute() + "/" + latestVersion);
    QNetworkRequest request(url);
    QNetworkReply *reply = nullptr;

    if(endpoint.getMethod() == "GET"){
        reply = manager->get(request);
    }

    if(reply){
        connect(reply, &QNetworkReply::finished, this, &MainWindow::downloadNewUpdateRequestFinished);
    } else {
        qDebug()<<"No se ha podido realizar la solicitud";
    }

}

void MainWindow::downloadNewUpdateRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Hubo un error en la respuesta de la descarga";
        return;
    }

    QVariant header = reply->header(QNetworkRequest::ContentDispositionHeader);

    if(header.isValid()){

        QString contentDisposition = header.toString();

        if(contentDisposition.isEmpty()){
            qDebug()<<"El nombre del archivo no es valido!";
            return;
        }

        static QRegularExpression regex(R"regex(filename="?([^";]+)"?)regex");
        QRegularExpressionMatch match = regex.match(contentDisposition);

        if (match.hasMatch()) {
            QString fileName = match.captured(1);
            QByteArray fileBytes = reply->readAll();

            if(fileName.endsWith(".exe")){

                saveExe(fileName, fileBytes);

            } else if(fileName.endsWith(".zip")){

                //saveZip

            } else {
                qDebug()<<"extension de archivo no valida!";
            }

        }

    }

}

void MainWindow::saveExe(QString& fileName, QByteArray& fileBytes){

    QString updaterPath = QCoreApplication::applicationDirPath();
    QString exePath = updaterPath + "/" + fileName;
    QFile exeFile(exePath);

    if (exeFile.exists()) {
        if (!exeFile.remove()) {
            qDebug() << "No se pudo eliminar el archivo existente:" << exePath;
            return;
        }
    }

    if (exeFile.open(QFile::WriteOnly | QFile::Truncate)) {
        exeFile.write(fileBytes);
        exeFile.close();
        qDebug() << "Archivo .exe guardado en:" << exePath;
    } else {
        qDebug() << "No se pudo abrir el archivo para escribir:" << exePath;
    }

}

void MainWindow::saveZip(QString& fileName, QByteArray& fileBytes){

    QString appPath = QCoreApplication::applicationDirPath();
    QString tempFolderPath = appPath + "/tempUpdate";
    QDir tempFolder;

    if(!tempFolder.exists(tempFolderPath)){
        if(!tempFolder.mkpath(tempFolderPath)){
            qDebug()<<"carpeta temporal creada con exito!";
        } else {
            qDebug()<<"error al crear la carpeta temporal!";
        }
    } else {
        qDebug()<<"la carpeta temporal ya existe!";
    }

    QFile zipFile(tempFolderPath + "/" + fileName);
    if(zipFile.open(QFile::WriteOnly)){

        zipFile.write(fileBytes);
        zipFile.close();

    } else {
        qDebug()<<"Error al crear o abrir el zipFile";
    }

    QStringList command = QStringList()
                          << "Expand-Archive"
                          << "-Path" << "\"" + appPath + "/" + fileName + "\""
                          << "-DestinationPath" << "\"" + tempFolderPath + "\"";

    int exitCode = QProcess::execute("powershell", command);

    if (exitCode == 0) {
        qDebug() << "Descompresión exitosa!";
    } else {
        qDebug()<< "Error al descomprimir el archivo";
    }


}
