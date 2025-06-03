#ifndef QUITTIMER_H
#define QUITTIMER_H

#include <QObject>
#include <QTimer>

class QuitTimer : public QTimer {
    Q_OBJECT
public:
    explicit QuitTimer(QParent *parent = nullptr);
};

#endif // QUITTIMER_H
