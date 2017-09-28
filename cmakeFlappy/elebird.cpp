#include "elebird.h"
#include <QDebug>

EleBird::EleBird(QObject *parent) :
    GameElement(parent)
{
    this->init();
    connect(&timer,SIGNAL(timeout()),this,SLOT(updateFrame()));
}

void EleBird::init()
{
    this->loadFrame();
    this->lastFrame = 0;
    this->currentFrame = 1;
    this->angle = 0;
    this->speedX = 7.0;
    this->speedY = 0;
    this->time= 0;
    this->bindRect.setRect(58,220.0,49.0,49.0);
    this->timer.start(150);
}

void EleBird::logic()
{
    if(!this->enabledLogic )
    {
        return;
    }

    //Y
    //this->speedY += this->increaseY;
    this->speedY = speedY + downAceleration * (time / 1000);
    time+= 20;

    //Angle
    if(this->speedY > 7.0 )
    {
        this->speedY = 7.0;
        this->angle = 25;
    }else{
        this->angle = 0;
    }

    this->bindRect.translate(0, this->speedY);

    //limit
    if(this->bindRect.top() < -this->bindRect.height())
        this->bindRect.moveTop(-this->bindRect.height());

    if(this->bindRect.bottom() > 415.0)
    {
        this->bindRect.moveBottom(415.0);
        this->timer.stop();
    }

    return;
}

void EleBird::draw(QPainter *painter)
{
    if( !this->enabledDraw )
        return;

    painter->save();
    painter->translate(this->bindRect.center());
    painter->rotate(this->angle);
    painter->translate(-this->bindRect.center());
    painter->drawPixmap(this->bindRect.x(),
                this->bindRect.y(),
                this->bindRect.width(),this->bindRect.height(),
                this->pixmapList[this->currentFrame]);
    painter->restore();
}

void EleBird::loadFrame()
{
    switch(qrand()%3)
    {
    case 0:
        this->clearAllFrame();
        this->addFrame(QPixmap(this->url + "image/bird0_0.png"));
        this->addFrame(QPixmap(this->url + "image/bird0_1.png"));
        this->addFrame(QPixmap(this->url + "image/bird0_2.png"));
        break;
    case 1:
        this->clearAllFrame();
        this->addFrame(QPixmap(this->url + "image/bird1_0.png"));
        this->addFrame(QPixmap(this->url + "image/bird1_1.png"));
        this->addFrame(QPixmap(this->url + "image/bird1_2.png"));
        break;
    case 2:
        this->clearAllFrame();
        this->addFrame(QPixmap(this->url + "image/bird2_0.png"));
        this->addFrame(QPixmap(this->url + "image/bird2_1.png"));
        this->addFrame(QPixmap(this->url + "image/bird2_2.png"));
        break;
    }
}

void EleBird::birdUp()
{
    this->speedY= upVelocity;
    time= 0;
}

void EleBird::birdUp(double up){
    this->speedY= up;
    time= 0;
}

void EleBird::updateFrame()
{
    if( !this->enabledDraw )
        return;

    if(this->currentFrame > this->lastFrame)
    {
        this->lastFrame = this->currentFrame;
        this->currentFrame++;
    }
    else if(this->currentFrame < this->lastFrame)
    {
        this->lastFrame = this->currentFrame;
        this->currentFrame--;
    }

    if(this->currentFrame < 0 || this->currentFrame > 2)
        this->currentFrame = 1;
}


void EleBird::setUpVelocity(double velocity){
    upVelocity= velocity;
}

void EleBird::setDownAceleration(double aceleration){
    downAceleration= aceleration;
}
