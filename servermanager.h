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
    void downloadNewVersion();
    void sendLog(QString &log);

    //setters
    void setHostName(QString& hostName);
    void setGetLatestVersionRoute(QString &getLatestVersionRoute);
    void setDownloadVersionRoute(QString &downloadVersionRoute);
    void setSendLogRoute(QString &sendLogRoute);
signals:
    void latestVersionReceived(double latestVersion);
    void downloadFinished(QString fileName, QByteArray data);
    void errorHasOcurred(QString error);

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
