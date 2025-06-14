#ifndef QUITTIMER_H
#define QUITTIMER_H

#include <QObject>
#include <QTimer>

class QuitTimer : public QTimer {
    Q_OBJECT
signals:
    void updateLabel(const QString &);
    void timerFinished();

public slots:
    void updateLabelAndSecs();
    void startContdown();

public:
    explicit QuitTimer(QObject *parent = nullptr);

private:
    int secsToQuit;
};

#endif // QUITTIMER_H
