#include "elereadyboard.h"

EleReadyBoard::EleReadyBoard(QObject *parent) :
    GameElement(parent)
{
    this->addFrame(QPixmap(this->url + "image/text_ready.png"));
    this->addFrame(QPixmap(this->url + "image/tutorial.png"));
    this->addFrame(QPixmap(this->url + "image/ingrese_dinero.png"));
    this->init();
    show= false;
    timer= new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [=](){ show= !show; });
}

void EleReadyBoard::init()
{
}

void EleReadyBoard::logic()
{
    if(!this->enabledLogic)
        return;
}

void EleReadyBoard::draw(QPainter *painter)
{
    if(!this->enabledDraw)
        return;

    if(show){
        painter->drawPixmap(45.0, 60.0, 197.0, 63.0,
                this->pixmapList[2]);
    }

    painter->drawPixmap(45.0,145.0,197.0,63.0,
                        this->pixmapList[0]);
    painter->drawPixmap(86.5,220.0,115.0,99.0,
                        this->pixmapList[1]);
}

void EleReadyBoard::mostrarIngreseDinero(bool b){
    if(b){
        timer->start();
    }
    else{
        timer->stop();
        show= false;
    }
}
