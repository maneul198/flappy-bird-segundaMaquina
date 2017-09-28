#include "elepipe.h"
#include <QDebug>
#include <iostream>
using namespace std;

ElePipe::ElePipe(int pos, QObject *parent) :
    GameElement(parent)
{
    this->startPosition = pos;
    this->loadFrame();
    this->init();
}

ElePipe::ElePipe(bool random, int pos, QObject *parent) :
    GameElement(parent)
{
    this->startPosition = pos;
    this->loadFrame();
    randomColor= random;
    this->init();
}

void ElePipe::init()
{
    this->currentFrame = 0;
    this->pipePassEmited = false;
    this->pipeRect[above].setRect(576 + this->startPosition*175.6,  -271.0 + qrand() % 200 ,   52.0,   321.0);
    this->pipeRect[following].setRect(576 + this->startPosition*175.6,  this->pipeRect[above].bottom() + 100.0, 52.0, 321.0);
    loadFrame();
    occilationValue= qPow(-1, qrand() % 2);
    typeOccilation= qPow(-1, qrand() % 2 );
    occilationRange= 0;
}

void ElePipe::logic()
{
    if(!this->enabledLogic)
        return;

    if(occilation){
        if(occilationRange > 70 || occilationRange < (typeOccilation == 1 ? -70 : 0)){
            occilationValue= occilationValue * -1;
        }

        occilationRange+= occilationValue ;

        this->pipeRect[above].translate(-this->speedX, occilationValue * typeOccilation);
        this->pipeRect[following].translate(-this->speedX, occilationValue);

    }else{

        this->pipeRect[above].translate(-this->speedX, 0);
        this->pipeRect[following].translate(-this->speedX, 0);
    }

    if(this->pipeRect[above].right() < 0)
    {
        this->init();
        this->pipeRect[above].moveTo(474.0,-271.0 + qrand()%200);
        this->pipeRect[following].moveTo(474.0,this->pipeRect[above].bottom() + 100.0);
        this->pipePassEmited = false;
    }

    if(this->pipeRect[above].left() < 20 && this->pipePassEmited == false)
    {
        emit pipePass();
        this->pipePassEmited = true;
    }
}

void ElePipe::draw(QPainter *painter)
{
    painter->drawPixmap(this->pipeRect[above].x(),
                        this->pipeRect[above].y(),
                        this->pipeRect[above].width(),
                        this->pipeRect[above].height(),
                        this->pixmapList[0]);
    painter->drawPixmap(this->pipeRect[following].x(),
                        this->pipeRect[following].y(),
                        this->pipeRect[following].width(),
                        this->pipeRect[following].height(),
                        this->pixmapList[1]);
}

void ElePipe::loadFrame()
{
    this->clearAllFrame();
    this->addFrame(QPixmap(this->url + "image/pipe_down.png"));
    this->addFrame(QPixmap(this->url + "image/pipe_up.png"));

    if(randomColor){
        int r= qrand() % 2;
        if(r == 0){
            this->clearAllFrame();
            this->addFrame(QPixmap(this->url + "image/pipe2_down.png"));
            this->addFrame(QPixmap(this->url + "image/pipe2_up.png"));
        }
    }
}

QRectF& ElePipe::getRect(PipeType type)
{
        return this->pipeRect[type];
}

void ElePipe::setSpeedX(double speed){
    speedX= speed;
}

double ElePipe::getSpeedX(){
    return speedX;
}

void ElePipe::setOccilation(bool oc){
    occilation= oc;
}

void ElePipe::setRandomColor(bool random){
    randomColor= random;
}
