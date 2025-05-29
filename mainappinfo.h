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
    void setVersion(double &version);
    void setFileName(QString &fileName);

private:
    double version;
    QString fileName;
};

#endif // MAINAPPINFO_H
