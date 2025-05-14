#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>

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

    getServerInfo(fileBytes);

}
