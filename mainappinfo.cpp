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

void MainAppInfo::setFileName(const QString& _fileName){
    fileName = _fileName;
}

void MainAppInfo::setVersion(const double& _version){
    version = _version;
}
