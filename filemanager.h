#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QDir>

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);
    QByteArray readFile(QString &filePath);
    bool replaceOrCreateFile(QString &filePath, QByteArray &data);
    bool createFolder(QString &dir);
    bool descompressZipFile(QString &zipFilePath, QString &destinationPath);
    bool searchFile(QString &path, const QFileInfo &fileSearched);
    QFileInfoList getDirEntries(QString &dir);
};

#endif // FILEMANAGER_H
