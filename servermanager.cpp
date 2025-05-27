#include "servermanager.h"
#include <QUrl>

ServerManager::ServerManager()
{

}
//setters

void ServerManager::setHostName(QString &_hostName){
    hostName = _hostName;
}

void ServerManager::setGetLatestVersionRoute(QString &_versionRoute){
    getLatestVersionRoute = _versionRoute;
}

void ServerManager::setDownloadVersionRoute(QString &_downloadVersionRoute){
    downloadVersionRoute = _downloadVersionRoute;
}

void ServerManager::setSendLogRoute(QString &_sendLogRoute){
    sendLogRoute = _sendLogRoute;
}

