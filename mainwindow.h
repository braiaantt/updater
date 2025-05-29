#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filemanager.h"
#include "servermanager.h"
#include "mainappinfo.h""
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QFileInfo>
#include <QGraphicsScene>
#include "loadingitem.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void latestAppVersionRequestFinished(double latestVersion);
    void downloadNewUpdateRequestFinished(QString fileName, QByteArray data);

private slots:

    void on_pushButtonCancel_clicked();

private:
    Ui::MainWindow *ui;
    FileManager *fileManager;
    ServerManager *serverManager;
    MainAppInfo mainAppInfo;
    QString tempFolderName;
    QGraphicsScene *scene;
    LoadingItem *spinner;
    QTimer *rotationTimer;
    int secsToQuit;

    void readUpdaterConfigFile();
    void initMainAppInfo(QJsonObject &mainAppInfoObj);
    void initServerManager(QJsonObject &serverInfoObj);
    void initFileManager();
    void connectSignals();
    void initUpdate();
    void downloadNewUpdate(QString&);
    void updateLocalVersion();
    void initLoadingItem();
    void quitUpdater();

};
#endif // MAINWINDOW_H
