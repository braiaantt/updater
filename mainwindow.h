#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "endpoint.h"
#include <QtNetwork/QNetworkAccessManager>

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
    QNetworkAccessManager *manager;

    void initConfig();
    void getConfigJsonInfo(QByteArray&);
    void getLatestAppVersion();
    bool isUpdateRequired(QString&);
    void downloadNewUpdate(QString&);
    void saveExe(QString&, QByteArray&);
    void saveZip(QString&, QByteArray&);

};
#endif // MAINWINDOW_H
