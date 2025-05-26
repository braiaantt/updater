#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "endpoint.h"
#include <QtNetwork/QNetworkAccessManager>
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
    void latestAppVersionRequestFinished();
    void downloadNewUpdateRequestFinished();

private:
    Ui::MainWindow *ui;
    QVector<Endpoint> endpoints;
    QString hostName;
    QString mainAppVersion;
    QString latestVersion;
    QNetworkAccessManager *manager;
    QGraphicsScene *scene;
    LoadingItem *spinner;
    QTimer *rotationTimer;

    void initConfig();
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

};
#endif // MAINWINDOW_H
