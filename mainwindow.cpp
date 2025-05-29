#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this)), rotationTimer(new QTimer(this)), spinner(new LoadingItem())
    , secsToQuit(4), tempFolderName("tempUpdate")

{
    ui->setupUi(this);

    QTimer::singleShot(0, this, [this]{
        readUpdaterConfigFile();
        connectSignals();
        initUpdate();
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

    if(fileBytes.isEmpty()){
        qDebug()<<"Error al leer el archivo de configuracion!";
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileBytes);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonObject mainAppInfoObj = jsonDoc["mainAppInfo"].toObject();
    QJsonObject serverInfoObj = jsonDoc["serverInfo"].toObject();

    initMainAppInfo(mainAppInfoObj);
    initServerManager(serverInfoObj);
    initFileManager();

}

void MainWindow::initMainAppInfo(QJsonObject &mainAppInfoObj){

    QString fileName = mainAppInfoObj["fileName"].toString();
    double version = mainAppInfoObj["version"].toDouble();

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

}

void MainWindow::initUpdate(){

    serverManager->getLatestVersion();

}

void MainWindow::latestAppVersionRequestFinished(double latestVersion){

    if(latestVersion > mainAppInfo.getVersion()){
        serverManager->downloadNewVersion();
        mainAppInfo.setVersion(latestVersion);
    }

}

void MainWindow::downloadNewUpdateRequestFinished(QString fileName, QByteArray data){

    QString appDirPath = QCoreApplication::applicationDirPath();

    if(fileName.endsWith(".exe")){

        QString filePath = appDirPath + "/" + fileName;
        if (fileManager->replaceOrCreateFile(filePath, data)) updateLocalVersion();

    } else if(fileName.endsWith(".zip")){

        QString tempUpdateFolder = appDirPath + "/" + tempFolderName;
        QString zipFilePath = tempUpdateFolder + "/" + fileName;

        if(fileManager->createFolder(tempUpdateFolder) &&
           fileManager->replaceOrCreateFile(zipFilePath, data) &&
           fileManager->descompressZipFile(zipFilePath, tempUpdateFolder))
        {

            QDir tempUpdateDir(tempUpdateFolder);
            tempUpdateDir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
            QFileInfoList entries = tempUpdateDir.entryInfoList();

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
                //show errors;
            } else {
                updateLocalVersion();
            }

        }

    } else {
        qDebug()<<"extension de archivo no valida!";
    }

}

void MainWindow::updateLocalVersion(){

    QString updaterConfigPath = QCoreApplication::applicationDirPath() + "/updater-config.json";
    QByteArray data = fileManager->readFile(updaterConfigPath);

    if(data.isEmpty()){
        qDebug()<<"Error al leer el archivo de configuracion!";
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

    if(currentReply && currentReply->isRunning()){
        currentReply->abort();
        quitUpdater();
        ui->pushButtonCancel->setEnabled(false);

    }

}

void MainWindow::quitUpdater(){

    QTimer *quitTimer = new QTimer(this);
    quitTimer->setInterval(1000);

    connect(quitTimer, &QTimer::timeout, this, [=]{

        if(secsToQuit == 0){

            quitTimer->stop();
            QCoreApplication::quit();

        }
        ui->label->setText("Actualización detenida.\nCerrando en " + QString::number(secsToQuit));
        secsToQuit--;
    });

    ui->label->setText("Actualización detenida.\nCerrando en " + QString::number(secsToQuit));
    quitTimer->start();

}
