#ifndef QUITTIMER_H
#define QUITTIMER_H

#include <QObject>
#include <QTimer>

class QuitTimer : public QTimer {
    Q_OBJECT
signals:
    void updateLabel(const QString &);
    void timerFinished();

public:
    explicit QuitTimer(QObject *parent = nullptr);
    void startContdown();

private:
    int secsToQuit;
};

#endif // QUITTIMER_H
