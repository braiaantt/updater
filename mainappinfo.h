#ifndef MAINAPPINFO_H
#define MAINAPPINFO_H

#include <QString>

class MainAppInfo
{
public:
    MainAppInfo();

    //getters
    const double& getVersion();
    const QString& getFileName();

    //setters
    void setVersion(const double &version);
    void setFileName(const QString &fileName);

private:
    double version;
    QString fileName;
};

#endif // MAINAPPINFO_H
