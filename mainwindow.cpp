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
    scene = new QGraphicsScene(this);
    rotationTimer = new QTimer(this);
    spinner = new LoadingItem();

    QTimer::singleShot(0, this, [this]{
        initConfig();
        getLatestAppVersion();
        initLoadingItem();
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
        ui->label->setText("Error al abrir el archivo de configuración");
        return;
    }

    getConfigJsonInfo(fileBytes);
}

void MainWindow::getConfigJsonInfo(QByteArray &fileBytes){

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileBytes, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        ui->label->setText("Error al leer el archivo de configuración!");
        return;
    }

    if(!jsonDoc.isObject()){
        ui->label->setText("Formato del archivo de configuración incorrecto");
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

    if(endpoints.empty()) return;

    Endpoint &endpoint = endpoints[0];
    QUrl url(hostName + endpoint.getRoute());
    QNetworkRequest request(url);
    QNetworkReply *reply = nullptr;

    if(endpoint.getMethod() == "GET"){
        reply = manager->get(request);
    }

    if(reply){
        ui->label->setText("Obteniendo la ultima versión...");
        connect(reply, &QNetworkReply::finished, this, &MainWindow::latestAppVersionRequestFinished);
    } else {
        ui->label->setText("No se pudo realizar la solicitud al servidor!");
    }

}

void MainWindow::latestAppVersionRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        ui->label->setText("Error en la respuesta del servidor");
        reply->deleteLater();
        return;
    }

    ui->label->setText("Procesando respuesta...");

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);

    if(jsonDoc.isObject()){
        QJsonObject jsonObj = jsonDoc.object();
        latestVersion = jsonObj["data"].toString();

        if(isUpdateRequired(latestVersion)){
            ui->label->setText("Descargando nueva version...");
            downloadNewUpdate(latestVersion);
        } else{
            ui->label->setText("No hay nuevas actualizaciones!");
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
        ui->label->setText("Descargando la version: '" + latestVersion + "'...");
        connect(reply, &QNetworkReply::finished, this, &MainWindow::downloadNewUpdateRequestFinished);
    } else {
        ui->label->setText("No se pudo realizar la solicitud de descarga!");
    }

}

void MainWindow::downloadNewUpdateRequestFinished(){

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error() != QNetworkReply::NoError){
        qDebug()<<"Hubo un error en la respuesta de la descarga";
        ui->label->setText("Error al descargar la nueva versión!");
        return;
    }

    ui->label->setText("Procesando respuesta...");

    QVariant header = reply->header(QNetworkRequest::ContentDispositionHeader);

    if(header.isValid()){

        QString contentDisposition = header.toString();

        if(contentDisposition.isEmpty()){
            ui->label->setText("Error en la respuesta. No hay nombre asignado");
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

        } else {
            ui->label->setText("Error del servidor. No se pudo identificar el nombre del archivo");
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
            ui->label->setText("No se pudo eliminar el archivo: " + fileName);
            return false;
        }
    }

    if (exeFile.open(QFile::WriteOnly | QFile::Truncate)) {
        exeFile.write(fileBytes);
        exeFile.close();
        ui->label->setText("Actualización finalizada correctamente!.");
    } else {
        ui->label->setText("Error al intentar escribir el archivo: " + fileName);
        return false;
    }

    return true;

}

bool MainWindow::saveZip(QString& fileName, QByteArray& fileBytes){

    QString appPath = QCoreApplication::applicationDirPath();
    QString tempFolderPath = appPath + "/tempUpdate";
    QDir tempFolder;

    if(!tempFolder.exists(tempFolderPath)){
        if(!tempFolder.mkpath(tempFolderPath)){
            ui->label->setText("Error al crear la carpeta temporal de actualización!");
            return false;
        }
    }

    QFile zipFile(tempFolderPath + "/" + fileName);
    if(!zipFile.open(QFile::WriteOnly)){
        ui->label->setText("Error al abrir el zip de la nueva actualización!");
        return false;
    }

    zipFile.write(fileBytes);
    zipFile.close();

    QStringList command = QStringList()
                          << "Expand-Archive"
                          << "-Path" << "\"" + tempFolderPath + "/" + fileName + "\""
                          << "-DestinationPath" << "\"" + tempFolderPath + "\"";

    int exitCode = QProcess::execute("powershell", command);

    if (exitCode == 0) {
        ui->label->setText("Descompresion exitosa! Copiando archivos...!");
        applyUpdateFiles();
        deleteTempUpdateFolder();
        ui->label->setText("Actualización finalizada correctamente!.");
    } else {
        ui->label->setText("Error al descomprimir el archivo de actualización!");
        return false;
    }

    return true;

}

void MainWindow::applyUpdateFiles(){

    QDir updaterDir(QCoreApplication::applicationDirPath());
    QDir tempUpdateDir(updaterDir.filePath("tempUpdate"));
    QFileInfoList entries;

    if(!tempUpdateDir.exists()){
        ui->label->setText("No se encontró la carpeta de actualización!");
        return;
    }

    tempUpdateDir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    entries = tempUpdateDir.entryInfoList();

    for(const QFileInfo& entry : entries){

        if(!entry.fileName().endsWith("zip")){
            if(!searchFile(updaterDir.path(), entry)){
                copyFile(entry.absoluteFilePath(), updaterDir.filePath(entry.fileName()));
            }
        }

    }

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
        ui->label->setText("Error al eliminar el archivo: " + target);
        return;
    }

    if(!QFile::copy(source, target)){
        ui->label->setText("Error al copiar el archivo" + source);
    }

}

void MainWindow::updateLocalVersion(){

    QString updaterPath = QCoreApplication::applicationDirPath();
    QFile updaterConfig(updaterPath + "/updater-config.json");
    QByteArray fileBytes;
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;

    if(!updaterConfig.open(QIODevice::ReadOnly)){
        ui->label->setText("Error al leer el archivo de configuración para actualización!");
        return;
    }

    fileBytes = updaterConfig.readAll();
    updaterConfig.close();

    jsonDoc = QJsonDocument::fromJson(fileBytes);

    if(!jsonDoc.isObject()){
        ui->label->setText("Archivo de configuración con formato incorrecto!");
        return;
    }

    jsonObj = jsonDoc.object();
    jsonObj["mainAppVersion"] = latestVersion;

    jsonDoc.setObject(jsonObj);

    if(!updaterConfig.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug()<<"Error al abrir el archivo para reescritura";
        ui->label->setText("Error al intentar actualizar el archivo de configuración!");
        return;
    }

    updaterConfig.write(jsonDoc.toJson(QJsonDocument::Indented));
    updaterConfig.close();

}

void MainWindow::deleteTempUpdateFolder(){

    QDir tempUpdateFolderDir(QDir(QCoreApplication::applicationDirPath()).filePath("tempUpdate"));
    if(tempUpdateFolderDir.exists()){
        if(!tempUpdateFolderDir.removeRecursively()){
            ui->label->setText("Error al eliminar la carpeta temporal!");
        }
    }

}

void MainWindow::initLoadingItem(){


    ui->graphicsView->setScene(scene);
    scene->addItem(spinner);

    spinner->setPos(0,0);
    scene->setSceneRect(-50, -50, 100, 100);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    connect(rotationTimer, &QTimer::timeout, this ,[=](){
        spinner->setRotation(spinner->rotation()+5);
    });

    rotationTimer->start(30);

}
