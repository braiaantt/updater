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
    QByteArray readFile(const QString &filePath);
    bool replaceOrCreateFile(const QString &filePath, const QByteArray &data);
    bool createFolder(const QString &dir);
    bool descompressZipFile(const QString &zipFilePath, const QString &destinationPath);
    bool searchFile(const QString &baseDir, const QFileInfo &fileSearched);
    void copyFile(const QString &source, const QString &target);
    QFileInfoList getDirEntries(const QString &dir);
};

#endif // FILEMANAGER_H
