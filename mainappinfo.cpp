#include "mainappinfo.h"

MainAppInfo::MainAppInfo()
{

}

//getters
const QString& MainAppInfo::getFileName(){
    return fileName;
}

const double& MainAppInfo::getVersion(){
    return version;
}

//setters

void MainAppInfo::setFileName(QString& _fileName){
    fileName = _fileName;
}

void MainAppInfo::setVersion(double& _version){
    version = _version;
}
