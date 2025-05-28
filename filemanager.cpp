#include "filemanager.h"
#include <QFile>
#include <QProcess>

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

    if(file.exists()){
        if(!file.remove()){
            qDebug()<<"Error al remover el archivo";
            return false;
        }
    }

    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        file.write(data);
        file.close();
    }

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

bool FileManager::descompressZipFile(const QString &zipFilePath, const QString &destinationPath){

    QStringList command = QStringList()
                          << "Expand-Archive"
                          << "-Path" << "\"" + zipFilePath +"\""
                          << "-DestinationPath" << "\"" + destinationPath + "\"";

    int exitCode = QProcess::execute("powershell", command);

    if(exitCode == 0) return true;

    return false;

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

    if(targetFile.exists() && !targetFile.remove()){
        qDebug()<<"Error al eliminar el archivo: "<<target;
        return;
    }

    if(!QFile::copy(source, target)){
        qDebug()<<"Error al copiar el archivo: "<<source;
    }

}
