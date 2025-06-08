#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filemanager.h"
#include "servermanager.h"
#include "mainappinfo.h"
#include "loadingitem.h"
#include "quittimer.h"
#include <QGraphicsScene>
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
    void showErrorMessageAndQuit(const QString &errors);
    void updateLabelLogs(const QString &text);
    void timerFinished();
    void onDescompressFinished(int exitCode);

private slots:

    void on_pushButtonCancel_clicked();

private:
    Ui::MainWindow *ui;
    FileManager *fileManager;
    ServerManager *serverManager;
    QuitTimer *quitAppTimer;
    MainAppInfo mainAppInfo;
    QString tempFolderName;
    QGraphicsScene *scene;
    LoadingItem *spinner;
    QTimer *rotationTimer;
    int secsToQuit;

    void readUpdaterConfigFile();
    void initMainAppInfo(QJsonObject &mainAppInfoObj);
    void initServerManager(QJsonObject &serverInfoObj);
    void connectSignals();
    void initUpdate();
    void downloadNewUpdate(QString&);
    void installExe(const QString &fileName, const QByteArray &data);
    void installZip(const QString &fileName, const QByteArray &data);
    void updateLocalVersion();
    void updateFinishedSuccessfully(const QString &text);
    void initLoadingItem();
    void quitUpdater(const QString &text);

};
#endif // MAINWINDOW_H
