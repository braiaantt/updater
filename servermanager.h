#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QString>

class ServerManager
{
public:
    ServerManager();

private:
    QString hostName;
    QString getVersionRoute;
    QString downloadVersionRooute;
    QString sendLogRoute;
};

#endif // SERVERMANAGER_H
