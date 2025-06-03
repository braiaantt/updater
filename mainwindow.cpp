#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this)), rotationTimer(new QTimer(this)), spinner(new LoadingItem()), quitAppTimer(new QuitTimer(this))
    , secsToQuit(4), tempFolderName("tempUpdate")

{
    ui->setupUi(this);

    QTimer::singleShot(0, this, [this]{
        initFileManager();
        readUpdaterConfigFile();
        initLoadingItem();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readUpdaterConfigFile(){

    QString updaterConfigPath = QCoreApplication::applicationDirPath() + "/updater-config.json";
    QByteArray fileBytes = fileManager->readFile(updaterConfigPath);

    if(fileBytes.isEmpty() || !fileManager->jsonIsValid(fileBytes)){
        showErrorMessageAndQuit("Error al leer el archivo de configuracion!");
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileBytes);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonObject mainAppInfoObj = jsonObj["mainAppInfo"].toObject();
    QJsonObject serverInfoObj = jsonObj["serverInfo"].toObject();

    initMainAppInfo(mainAppInfoObj);
    initServerManager(serverInfoObj);
    connectSignals();
    initUpdate();

}

void MainWindow::initMainAppInfo(QJsonObject &mainAppInfoObj){

    QString fileName = mainAppInfoObj["fileName"].toString();
    double version = mainAppInfoObj["version"].toString().toDouble();

    mainAppInfo.setFileName(fileName);
    mainAppInfo.setVersion(version);

}

void MainWindow::initServerManager(QJsonObject &serverInfoObj){

    serverManager = new ServerManager(this);

    QString hostName = serverInfoObj["hostName"].toString();
    QString getLatestVersionRoute = serverInfoObj["getLatestVersionRoute"].toString();
    QString downloadVersionRoute = serverInfoObj["downloadVersionRoute"].toString();
    QString sendLogRoute = serverInfoObj["sendLogRoute"].toString();

    serverManager->setHostName(hostName);
    serverManager->setGetLatestVersionRoute(getLatestVersionRoute);
    serverManager->setDownloadVersionRoute(downloadVersionRoute);
    serverManager->setSendLogRoute(sendLogRoute);

}

void MainWindow::initFileManager(){
    fileManager = new FileManager(this);
}

void MainWindow::connectSignals(){

    connect(serverManager, &ServerManager::latestVersionReceived, this, &MainWindow::latestAppVersionRequestFinished);
    connect(serverManager, &ServerManager::downloadFinished, this, &MainWindow::downloadNewUpdateRequestFinished);
    connect(serverManager, &ServerManager::errorHasOcurred, this, &MainWindow::showErrorMessageAndQuit);

}

void MainWindow::initUpdate(){

    serverManager->getLatestVersion();
    ui->labelLogs->setText("Obteniendo última versión");

}

void MainWindow::latestAppVersionRequestFinished(double latestVersion){

    if(latestVersion > mainAppInfo.getVersion()){
        serverManager->sendLog("LOG: Iniciando descarga de la version: " + QString::number(latestVersion));
        ui->labelLogs->setText("Descargando nueva versión...");
        serverManager->downloadNewVersion();
        mainAppInfo.setVersion(latestVersion);
    }

}

void MainWindow::downloadNewUpdateRequestFinished(QString fileName, QByteArray data){

    QString appDirPath = QCoreApplication::applicationDirPath();
    serverManager->sendLog("LOG: Descarga finalizada correctamente");

    if(fileName.endsWith(".exe")){
        serverManager->sendLog("LOG: Instalando ejecutable...");
        ui->labelLogs->setText("Instalando actualización...");
        QString filePath = appDirPath + "/" + fileName;
        if (!fileManager->replaceOrCreateFile(filePath, data)){
            serverManager->sendLog("LOG: Hubo un error al instalar el ejecutable!");
            showErrorMessageAndQuit("Error al crear o copiar el archivo: " + fileName);
        } else{
            updateLocalVersion();
            serverManager->sendLog("LOG: Instalacion finalizada correctamente!");
        }

    } else if(fileName.endsWith(".zip")){
        serverManager->sendLog("LOG: Instalando zip...");
        ui->labelLogs->setText("Instalando archivos...");
        QString tempUpdateFolder = appDirPath + "/" + tempFolderName;
        QString zipFilePath = tempUpdateFolder + "/" + fileName;

        if(!fileManager->createFolder(tempUpdateFolder) ||
           !fileManager->replaceOrCreateFile(zipFilePath, data) ||
           !fileManager->descompressZipFile(zipFilePath, tempUpdateFolder))
        {
            QStringList errors = fileManager->getErrorCopyFiles();
            QString message = errors.join("\n");
            serverManager->sendLog(message);
            showErrorMessageAndQuit(message);
            return;
        }

        QFileInfoList entries = fileManager->getDirEntries(tempUpdateFolder);

        for(const QFileInfo &entry : entries){
            if(entry.fileName() != fileName){

                if(!fileManager->searchFile(appDirPath, entry)){

                    QString target = appDirPath + "/" + fileName;
                    fileManager->copyFile(entry.absoluteFilePath(), target);

                }

            }
        }

        if(!fileManager->deleteRecursively(tempUpdateFolder)){
            QStringList errors = fileManager->getErrorCopyFiles();
            QString message = "Error al instalar los siguientes archivos:\n" + errors.join("\n");
            serverManager->sendLog("LOG:" + message);
            showErrorMessageAndQuit(message);
            return;
        }

        serverManager->sendLog("LOG: Instalacion de archivos finalizada correctamente!");
        updateLocalVersion();

    } else {
        serverManager->sendLog("LOG: Archivo enviado no valido!");
        showErrorMessageAndQuit("Archivo recibido del servidor no valido!");
    }

    quitUpdater("Instalacion finalizada correctamente!");

}

void MainWindow::updateLocalVersion(){

    QString updaterConfigPath = QCoreApplication::applicationDirPath() + "/updater-config.json";
    QByteArray data = fileManager->readFile(updaterConfigPath);

    if(data.isEmpty()){
        serverManager->sendLog("No se pudo actualizar la version local!");
        showErrorMessageAndQuit("Error al actualizar la version local. Nueva version: " + QString::number(mainAppInfo.getVersion()));
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonObject jsonObj = jsonDoc.object();

    jsonObj = jsonDoc.object();
    jsonObj["mainAppVersion"] = mainAppInfo.getVersion();

    jsonDoc.setObject(jsonObj);

    QByteArray newData = jsonDoc.toJson(QJsonDocument::Indented);
    fileManager->replaceOrCreateFile(updaterConfigPath, newData);

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

void MainWindow::on_pushButtonCancel_clicked()
{

    quitAppTimer->startContdown();

}

void MainWindow::showErrorMessageAndQuit(const QString &errorMessage){

    QMessageBox::warning(this,"Error",errorMessage);

    quitUpdater("");

}
