#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "endpoint.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QVector<Endpoint> endpoints;

    void initConfig();
    void getServerInfo(QByteArray&);

};
#endif // MAINWINDOW_H
