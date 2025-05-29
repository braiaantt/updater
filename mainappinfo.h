#ifndef MAINAPPINFO_H
#define MAINAPPINFO_H

#include <QString>

class MainAppInfo
{
public:
    MainAppInfo();

    //getters
    const QString& getVersion();
    const QString& getFileName();

    //setters
    void setVersion();
    void setFileName();

private:
    QString version;
    QString fileName;
};

#endif // MAINAPPINFO_H
