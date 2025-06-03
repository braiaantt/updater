#include "quittimer.h"

QuitTimer::QuitTimer(QObject *parent) :
    QTimer(parent)
{
    connect(this, QTimer::timeout, this, QuitTimer::updateLabelAndSecs);
    secsToQuit = 5;
    this->setInterval(1000);
}

void QuitTimer::startContdown(){

    updateLabelAndSecs();
    this->start();

}

void QuitTimer::updateLabelAndSecs(){

    if(secsToQuit == 0){

        this->stop();
        emit timerFinished();

    } else {

        emit updateLabel("Cerrando app en..." + QString::number(secsToQuit));
        secsToQuit--;

    }

}
