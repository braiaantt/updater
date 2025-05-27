#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QString>
#include <QNetworkAccessManager>

class ServerManager : public QObject
{
    Q_OBJECT

public:
    explicit ServerManager(QObject *parent = nullptr);

    void getLatestVersion();
    void downloadNewVersion(QString& version);
    void sendLog(QString &log);

    //setters
    void setHostName(QString& hostName);
    void setGetLatestVersionRoute(QString &getLatestVersionRoute);
    void setDownloadVersionRoute(QString &downloadVersionRoute);
    void setSendLogRoute(QString &sendLogRoute);
private:
    QString hostName;
    QString getLatestVersionRoute;
    QString downloadVersionRoute;
    QString sendLogRoute;

    QNetworkAccessManager *networkManager;

    void getLatestVersionRequestFinished();
    void downloadNewVersionRequestFinished();
    void sendLogRequestFinished();
};

#endif // SERVERMANAGER_H
