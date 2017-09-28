#ifndef ELEPIPE_H
#define ELEPIPE_H

#include "gameelement.h"
#include <qmath.h>

enum PipeType{above=0,
              following=1};

class ElePipe : public GameElement
{
    Q_OBJECT
private:
    int currentFrame;
    double speedX;
    int startPosition;
    QRectF pipeRect[2];
    bool pipePassEmited;
    double occilationRange;
    int occilationValue;
    bool occilation;
    int typeOccilation;
    bool randomColor;
    void loadFrame();

public:
    explicit ElePipe(int pos= 0, QObject *parent = 0);
    ElePipe(bool randomColor, int pos= 0, QObject *parent = 0);
    void init();
    void draw(QPainter *);
    void logic();
    void setSpeedX(double);
    double getSpeedX();
    void setOccilation(bool);
    void setRandomColor(bool);


    QRectF &getRect(PipeType);
signals:
    void pipePass();

};

#endif // ELEPIPE_H
