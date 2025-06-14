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
    , fileManager(new FileManager(this)), serverManager(new ServerManager(this)), tempFolderName("tempUpdate"), updateCancelled(false)

{
    ui->setupUi(this);

    QTimer::singleShot(0, this, [this]{
        connectSignals();
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
    initUpdate();

}

void MainWindow::initMainAppInfo(QJsonObject &mainAppInfoObj){

    QString fileName = mainAppInfoObj["fileName"].toString();
    double version = mainAppInfoObj["version"].toString().toDouble();

    mainAppInfo.setFileName(fileName);
    mainAppInfo.setVersion(version);

}

void MainWindow::initServerManager(QJsonObject &serverInfoObj){

    QString hostName = serverInfoObj["hostName"].toString();
    QString getLatestVersionRoute = serverInfoObj["getLatestVersionRoute"].toString();
    QString downloadVersionRoute = serverInfoObj["downloadVersionRoute"].toString();
    QString sendLogRoute = serverInfoObj["sendLogRoute"].toString();

    serverManager->setHostName(hostName);
    serverManager->setGetLatestVersionRoute(getLatestVersionRoute);
    serverManager->setDownloadVersionRoute(downloadVersionRoute);
    serverManager->setSendLogRoute(sendLogRoute);

}

void MainWindow::connectSignals(){

    connect(serverManager, &ServerManager::latestVersionReceived, this, &MainWindow::latestAppVersionRequestFinished);
    connect(serverManager, &ServerManager::downloadFinished, this, &MainWindow::downloadNewUpdateRequestFinished);
    connect(serverManager, &ServerManager::errorHasOcurred, this, &MainWindow::showErrorMessageAndQuit);
    connect(fileManager, &FileManager::descompressFinished, this, &MainWindow::onDescompressFinished);
    connect(quitAppTimer, &QuitTimer::updateLabel, this, &MainWindow::updateLabelLogs);
    connect(quitAppTimer, &QuitTimer::timerFinished, this, &MainWindow::timerFinished);
    connect(serverManager, &ServerManager::readyToQuit, quitAppTimer, &QuitTimer::startContdown);

}

void MainWindow::initUpdate(){

    updateLabelLogs("Obteniendo última versión...");

    QTimer::singleShot(450, this, [=](){
        if(updateCancelled) return;
        serverManager->getLatestVersion();
    });

}

void MainWindow::latestAppVersionRequestFinished(double latestVersion){

    if(latestVersion > mainAppInfo.getVersion()){

        serverManager->sendLog("LOG: Iniciando descarga de la version: " + QString::number(latestVersion));
        updateLabelLogs("Descargando nueva versión...");
        mainAppInfo.setVersion(latestVersion);

        QTimer::singleShot(450, this, [=](){
            if(updateCancelled) return;
            ui->pushButtonCancel->setEnabled(false);
            serverManager->downloadNewVersion();
        });

    } else {
        ui->pushButtonCancel->setEnabled(false);
        updateFinishedSuccessfully("No hay actualizaciones disponibles!");
    }

}

void MainWindow::downloadNewUpdateRequestFinished(QString fileName, QByteArray data){

    serverManager->sendLog("LOG: Descarga finalizada correctamente. Instalando...");
    updateLabelLogs("Descarga finalizada. Instalando...");

    QTimer::singleShot(450, this, [=](){

        if(fileName.endsWith(".exe")){

            installExe(fileName, data);

        } else if(fileName.endsWith(".zip")){

            installZip(fileName, data);

        } else {
            serverManager->sendLog("LOG: Archivo enviado no valido!");
            showErrorMessageAndQuit("Archivo recibido del servidor no valido!");
            return;
        }
    });

}

void MainWindow::installExe(const QString &fileName, const QByteArray &data){

    QString appDirPath = QCoreApplication::applicationDirPath();

    QString filePath = appDirPath + "/" + fileName;
    if (!fileManager->replaceOrCreateFile(filePath, data)){
        serverManager->sendLog("LOG: Error al crear o copiar el archivo: " + fileName);
        showErrorMessageAndQuit("Error al crear o copiar el archivo: " + fileName);
        return;
    }

    updateLocalVersion();

    QTimer::singleShot(450, this, [=](){
        updateFinishedSuccessfully("Instalación finalizada correctamente!");
    });

}

void MainWindow::installZip(const QString &fileName, const QByteArray &data){

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString tempUpdateFolder = appDirPath + "/" + tempFolderName;
    QString zipFilePath = tempUpdateFolder + "/" + fileName;

    if(!fileManager->createFolder(tempUpdateFolder) ||
        !fileManager->replaceOrCreateFile(zipFilePath, data))
    {
        QStringList errors = fileManager->getErrorCopyFiles();
        QString message = errors.join("\n");
        serverManager->sendLog(message);
        showErrorMessageAndQuit(message);
        return;
    }

    fileManager->descompressZipFile(zipFilePath, tempUpdateFolder);

}

void MainWindow::onDescompressFinished(int exitCode){

    if(exitCode != 0){
        showErrorMessageAndQuit("Hubo un error al descomprimir el archivo de actualización!");
    }

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString tempUpdateFolder = appDirPath + "/" + tempFolderName;
    QFileInfoList entries = fileManager->getDirEntries(tempUpdateFolder);

    for(const QFileInfo &entry : entries){
        if(!entry.fileName().endsWith("zip")){

            if(!fileManager->searchFile(appDirPath, entry)){

                QString target = appDirPath + "/" + entry.fileName();
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

    updateLocalVersion();
    QTimer::singleShot(450, this, [=](){
        updateFinishedSuccessfully("Instalación finalizada correctamente!");
    });

}

void MainWindow::updateFinishedSuccessfully(const QString &text){

    serverManager->sendLog("LOG:" + text);

    QPixmap pixmap(":/resources/checked-success-svgrepo-com.svg");
    ui->labelShowResult->setPixmap(pixmap);
    ui->labelUpdateState->setText(text);
    ui->stackedWidget->setCurrentIndex(1);

    quitAppTimer->startContdown();

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
    QJsonObject mainAppInfoObj = jsonObj["mainAppInfo"].toObject();

    mainAppInfoObj["version"] = QString::number(mainAppInfo.getVersion());
    jsonObj["mainAppInfo"] = mainAppInfoObj;

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

    updateCancelled = true;
    ui->pushButtonCancel->setEnabled(false);
    serverManager->cancelRequests();
    ui->labelLogs->setText("Cancelando...");

}

void MainWindow::showErrorMessageAndQuit(const QString &errorMessage){

    ui->pushButtonCancel->setEnabled(false);

    QPixmap pixmap(":/resources/error-svgrepo-com.svg");
    ui->labelShowResult->setPixmap(pixmap);
    ui->labelUpdateState->setText("Error en la actualización");
    ui->stackedWidget->setCurrentIndex(1);

    QMessageBox::warning(this,"Error",errorMessage);
    quitAppTimer->startContdown();

}

void MainWindow::updateLabelLogs(const QString& message){

    ui->labelLogs->setText(message);

}

void MainWindow::timerFinished() {

    QString mainAppPath = QCoreApplication::applicationDirPath() + QDir::separator() + mainAppInfo.getFileName();

    if (!fileManager->startApp(mainAppPath)) {
        QMessageBox::critical(this,"Error","Error al iniciar la aplicación principal!");
    }

    QCoreApplication::quit();
}

