#include "filemanager.h"
#include <QFile>
#include <QProcess>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrent>

FileManager::FileManager(QObject *parent) :
    QObject(parent)
{

}

QByteArray FileManager::readFile(const QString &filePath){

    QByteArray fileData;
    QFile file(filePath);

    if(file.open(QIODevice::ReadOnly)){
        fileData = file.readAll();
        file.close();
    }

    return fileData;

}

bool FileManager::replaceOrCreateFile(const QString &filePath, const QByteArray &data){

    QFile file(filePath);

    if(file.exists() && !file.remove()){
        return false;
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        return false;
    }

    file.write(data);
    file.close();

    return true;

}

bool FileManager::createFolder(const QString &dir){

    QDir folder;

    if(!folder.exists(dir)){
        if(!folder.mkpath(dir)){
            qDebug()<<"Error al crear la carpeta en: "<<dir;
            return false;
        }
    }

    return true;

}

bool FileManager::deleteRecursively(const QString &dir){

    if(!errorCopyFiles.isEmpty()) return false;

    QDir folderDir(dir);

    if(folderDir.exists()){
        if(!folderDir.removeRecursively()){
            errorCopyFiles << "Error al eliminar la carpeta: " + dir;
            return false;
        }
    }

    return true;
}

void FileManager::descompressZipFile(const QString &zipFilePath, const QString &destinationPath){

    QFuture<void> future = QtConcurrent::run([=](){

        QStringList command = QStringList()
                              << "Expand-Archive"
                              << "-Path" << "\"" + zipFilePath +"\""
                              << "-DestinationPath" << "\"" + destinationPath + "\"";

        int exitCode = QProcess::execute("powershell", command);

        QMetaObject::invokeMethod(this, [this, exitCode]() {
                emit descompressFinished(exitCode);
            }, Qt::QueuedConnection);

    });

}

bool FileManager::searchFile(const QString &baseDir, const QFileInfo &fileSearched){

    QString tempFolderName = "tempUpdate"; //neet to ignore this folder
    QDir dir(baseDir);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    QFileInfoList entries = dir.entryInfoList();

    for(const QFileInfo &entry : entries){

        if(tempFolderName != entry.fileName()){

            if(entry.isDir()){

                QString newDir = dir.filePath(entry.fileName());
                if(searchFile(newDir, fileSearched)) return true;

            } else if (entry.isFile()){

                if(entry.fileName() == fileSearched.fileName()){
                    copyFile(fileSearched.absoluteFilePath(), entry.absoluteFilePath());
                    return true;
                }
            }

        }

    }

    return false;

}

void FileManager::copyFile(const QString &source, const QString &target){

    QFile targetFile(target);
    QString error;

    if(targetFile.exists() && !targetFile.remove()){
        error = "Error al eliminar el archivo: " + target;
        errorCopyFiles << error;
        return;
    }

    if(!QFile::copy(source, target)){
        error = "Error al copiar el archivo: " + source;
        errorCopyFiles << error;
    }

}

QFileInfoList FileManager::getDirEntries(const QString &dir){

    QDir tempUpdateDir(dir);
    QFileInfoList entries;

    if(tempUpdateDir.exists()){

        tempUpdateDir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
        entries = tempUpdateDir.entryInfoList();

    }

    return tempUpdateDir.entryInfoList();

}

bool FileManager::jsonIsValid(const QByteArray &data){

    QJsonParseError parseError;
    QJsonDocument::fromJson(data, &parseError);

    if(parseError.error != QJsonParseError::NoError){
        return false;
    }
    return true;
}

bool FileManager::startApp(const QString &appPath) {

    if (!QFile::exists(appPath)) return false;

    return QProcess::startDetached(appPath);
}


//getters

const QStringList&  FileManager::getErrorCopyFiles(){
    return errorCopyFiles;
}
