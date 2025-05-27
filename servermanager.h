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
    void setGetVersionRoute(QString &versionRoute);
    void setDownloadVersionRoute(QString &downloadVersionRoute);
    void setSendLogRoute(QString &sendLogRoute);

private:
    QString hostName;
    QString getVersionRoute;
    QString downloadVersionRooute;
    QString sendLogRoute;
};

#endif // SERVERMANAGER_H
