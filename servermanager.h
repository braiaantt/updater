#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QString>
#include <QNetworkReply>

class ServerManager
{
public:
    ServerManager();
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
};

#endif // SERVERMANAGER_H
