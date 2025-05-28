#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filemanager.h"
#include "servermanager.h"
#include "endpoint.h"
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
    void downloadNewUpdateRequestFinished();

private slots:

    void on_pushButtonCancel_clicked();

private:
    Ui::MainWindow *ui;
    FileManager *fileManager;
    ServerManager *serverManager;
    QVector<Endpoint> endpoints;
    QString hostName;
    QString mainAppVersion;
    QString latestVersion;
    QNetworkAccessManager *manager;
    QNetworkReply *currentReply;
    QGraphicsScene *scene;
    LoadingItem *spinner;
    QTimer *rotationTimer;
    int secsToQuit;

    void initServerManager();
    void initFileManager();
    void initUpdate();
    void getConfigJsonInfo(QByteArray&);
    void getLatestAppVersion();
    bool isUpdateRequired(QString&);
    void downloadNewUpdate(QString&);
    bool saveExe(QString&, QByteArray&);
    bool saveZip(QString&, QByteArray&);
    void applyUpdateFiles();
    bool searchFile(QString, const QFileInfo&) const;
    void copyFile(const QString&, const QString&) const;
    void deleteTempUpdateFolder();
    void updateLocalVersion();
    void initLoadingItem();
    void quitUpdater();

};
#endif // MAINWINDOW_H
