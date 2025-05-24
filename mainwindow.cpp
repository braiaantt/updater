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
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    manager = new QNetworkAccessManager(this);

    QTimer::singleShot(0, this, [this]{
        //initConfig();
        //getLatestAppVersion();
        applyUpdateFiles();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initConfig(){

    QString appPath = QCoreApplication::applicationDirPath();
    QFile file(appPath + "/updater-config.json");
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
        qDebug()<<"Solicitando ultima version de la aplicación...";
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

    qDebug()<<"Procesando respuesta...";

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);

    if(jsonDoc.isObject()){
        QJsonObject jsonObj = jsonDoc.object();
        latestVersion = jsonObj["data"].toString();

        if(isUpdateRequired(latestVersion)){
            qDebug()<<"descargando nueva version...";
            downloadNewUpdate(latestVersion);
        } else{
            qDebug()<<"no hay descarga requerida!";
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
        qDebug()<<"realizando descarga de la nueva version... ("<<latestVersion<<")";
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

    qDebug()<<"procesando respuesta...";

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

                if(saveExe(fileName, fileBytes)) updateLocalVersion();

            } else if(fileName.endsWith(".zip")){

                if(saveZip(fileName, fileBytes)) updateLocalVersion();

            } else {
                qDebug()<<"extension de archivo no valida!";
            }

        }

    } else {
        qDebug()<<"Header no valido!";
    }

}

bool MainWindow::saveExe(QString& fileName, QByteArray& fileBytes){

    QString updaterPath = QCoreApplication::applicationDirPath();
    QString exePath = updaterPath + "/" + fileName;
    QFile exeFile(exePath);

    if(exeFile.exists()) {
        if (!exeFile.remove()) {
            qDebug() << "No se pudo eliminar el archivo existente:" << exePath;
            return false;
        }
    }

    if (exeFile.open(QFile::WriteOnly | QFile::Truncate)) {
        exeFile.write(fileBytes);
        exeFile.close();
        qDebug() << "Archivo .exe guardado en:" << exePath;
    } else {
        qDebug() << "No se pudo abrir el archivo para escribir:" << exePath;
        return false;
    }

    return true;

}

bool MainWindow::saveZip(QString& fileName, QByteArray& fileBytes){

    QString appPath = QCoreApplication::applicationDirPath();
    QString tempFolderPath = appPath + "/tempUpdate";
    QDir tempFolder;

    if(!tempFolder.exists(tempFolderPath)){
        if(tempFolder.mkpath(tempFolderPath)){
            qDebug()<<"carpeta temporal creada con exito!";
        } else {
            qDebug()<<"error al crear la carpeta temporal!";
            return false;
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
                          << "-Path" << "\"" + tempFolderPath + "/" + fileName + "\""
                          << "-DestinationPath" << "\"" + tempFolderPath + "\"";

    int exitCode = QProcess::execute("powershell", command);

    if (exitCode == 0) {
        qDebug() << "Descompresión exitosa!";
        applyUpdateFiles();

    } else {
        qDebug()<< "Error al descomprimir el archivo";
        return false;
    }

    return true;

}

void MainWindow::applyUpdateFiles(){

    QDir updaterDir(QCoreApplication::applicationDirPath());
    QDir tempUpdateDir(updaterDir.filePath("tempUpdate"));
    QFileInfoList entries;

    if(!tempUpdateDir.exists()){
        qDebug()<<"El path: "<<tempUpdateDir.path()<<" no existe!";
        return;
    }

    tempUpdateDir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    entries = tempUpdateDir.entryInfoList();

    for(const QFileInfo& entry : entries){

        if(!entry.fileName().endsWith("zip")){
            if(!searchFile(updaterDir.path(), entry)){
                qDebug()<<"Creando archivo: "<<entry.fileName();
                copyFile(entry.absoluteFilePath(), updaterDir.filePath(entry.fileName()));
            }
        }

    }

    qDebug()<<"Reemplazo de archivos finalizado!";

}

bool MainWindow::searchFile(QString path, const QFileInfo& fileSearched) const{

    QFileInfoList entries;
    QDir dir(path);

    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    entries = dir.entryInfoList();

    for(const QFileInfo &entry : entries){

        QString name = entry.fileName();

        if(name != "tempUpdate"){
            if(entry.isDir()){

                if(searchFile(dir.filePath(name), fileSearched)) return true;

            } else if(entry.isFile()){

                if(name == fileSearched.fileName()){
                    copyFile(fileSearched.absoluteFilePath(),entry.absoluteFilePath());
                    return true;
                }
            }
        }

    }

    return false;

}

void MainWindow::copyFile(const QString& source, const QString& target) const{

    QFile targetFile(target);

    if(targetFile.exists() && !targetFile.remove()){
        qDebug()<<"Hubo un error al eliminar el archivo: "<<target;
    }

    if(!QFile::copy(source, target)){
        qDebug()<<"Error al copiar el archivo: "<<source;
    } else {
        qDebug()<<"Archivo: "<<source<<" copiado correctamente!";
    }

}

void MainWindow::updateLocalVersion(){

    QString updaterPath = QCoreApplication::applicationDirPath();
    QFile updaterConfig(updaterPath + "/updater-config.json");
    QByteArray fileBytes;
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;

    if(!updaterConfig.open(QIODevice::ReadOnly)){
        qDebug()<<"Error al intentar leer el archivo de configuración. Actualizacion de nueva version incompleta.";
        return;
    }

    fileBytes = updaterConfig.readAll();
    updaterConfig.close();

    jsonDoc = QJsonDocument::fromJson(fileBytes);

    if(!jsonDoc.isObject()){
        qDebug()<<"Archivo de configuración con formato incorrecto.";
        return;
    }

    jsonObj = jsonDoc.object();
    jsonObj["mainAppVersion"] = latestVersion;

    jsonDoc.setObject(jsonObj);

    if(!updaterConfig.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug()<<"Error al abrir el archivo para reescritura";
        return;
    }

    updaterConfig.write(jsonDoc.toJson(QJsonDocument::Indented));
    updaterConfig.close();

    qDebug()<<"Archivo de configuración actualizado correctamente!";

}
