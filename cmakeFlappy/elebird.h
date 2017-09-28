#ifndef ELEBIRD_H
#define ELEBIRD_H

#include <QTimer>
#include <qmath.h>
#include "gameelement.h"


class EleBird : public GameElement
{
    Q_OBJECT
private:
    int lastFrame;
    int currentFrame;
    double speedY;
    double speedX;
    double time;
    double upVelocity;
    double downAceleration;
    int angle;
    QTimer timer;

    void loadFrame();

public:
    explicit EleBird(QObject *parent = 0);
    void init();
    void logic();
    void draw(QPainter *);
    void birdUp();
    void birdUp(double);
    void setUpVelocity(double);
    void setDownAceleration(double);

signals:

public slots:
    void updateFrame();
};

#endif // ELEBIRD_H
