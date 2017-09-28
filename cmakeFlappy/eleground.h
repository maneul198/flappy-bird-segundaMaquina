#ifndef ELEGROUND_H
#define ELEGROUND_H

#include "gameelement.h"

class EleGround : public GameElement
{
    Q_OBJECT
private:
    int currentFrame;
    int positionX;
    double speedX;
public:
    explicit EleGround(QObject *parent = 0);
    void init();
    void draw(QPainter *);
    void logic();
    void setSpeedX(double);
    double getSpeedX();
};

#endif // ELEGROUND_H
