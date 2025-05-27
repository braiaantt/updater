#include "servermanager.h"

ServerManager::ServerManager()
{

}
//setters

void ServerManager::setHostName(QString &_hostName){
    hostName = _hostName;
}

void ServerManager::setGetVersionRoute(QString &_versionRoute){
    getVersionRoute = _versionRoute;
}

void ServerManager::setDownloadVersionRoute(QString &_downloadVersionRoute){
    downloadVersionRoute = _downloadVersionRoute;
}

void ServerManager::setSendLogRoute(QString &_sendLogRoute){
    sendLogRoute = _sendLogRoute;
}

