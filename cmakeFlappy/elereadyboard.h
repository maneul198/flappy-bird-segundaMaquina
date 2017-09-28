#ifndef ELEREADYBOARD_H
#define ELEREADYBOARD_H

#include "gameelement.h"
#include <QTimer>

class EleReadyBoard : public GameElement
{
    Q_OBJECT
public:
    explicit EleReadyBoard(QObject *parent = 0);
    void init();
    void draw(QPainter *);
    void logic();
    void mostrarIngreseDinero(bool);
signals:

private:
    bool show;
    QTimer *timer;

public slots:

};

#endif // ELEREADYBOARD_H
