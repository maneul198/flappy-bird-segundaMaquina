#include "awardPrize.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include <QApplication>


awardPrize::awardPrize(QWidget *parent):
    QMainWindow(parent),
    setting("/home/" + qgetenv("USER") + "/.config/flappyBirdConfig/productos.ini",
            QSettings::IniFormat)
{
    active= false;
    manager = new ProductsManager();
    loadProducs();

    connect(manager, SIGNAL(delivered(Product*)), this, SLOT(displayInfo()));

    connect(manager, &ProductsManager::deliveredNumberHook,this, [=](uint producto){
        qDebug() << "HOOK #: " << producto << ";Nombre" << manager->product(producto) << endl;
        std::ostringstream oss;
        oss << "HOOK #: " << producto << ";Nombre: " << manager->product(producto)->name().toStdString() << endl;
        //oss << producto;
        configurationFile::writeOnFile("/home/" + qgetenv("USER") + "/.config/flappyBirdConfig/entregasPremios",
                                         oss.str().c_str()  );

        emit selectedPrize();

    });

    connect(manager, &ProductsManager::noProduct, this, [=](){
        a = new QMessageBox("EROOR", "Lo sentimos el producto seleccionado no esta disponible",
                                         QMessageBox::NoIcon, QMessageBox::NoButton,
                                         QMessageBox::NoButton, QMessageBox::NoButton, this);
        a->show();

        QTimer::singleShot(5000, this, [=](){
            a->close();
        });

    });
    init();

}

void awardPrize::init(){
    resize(540, 760);
    setStyleSheet("background-image: url(" + GameElement::url + "/image/fondo_productos.png)");
    double BUTTON_WIDTH = width() / 3;
    double BUTTON_HEIGH = height() / 4;
    double HORIZONTAL_BUTTON_SEPARATION = BUTTON_WIDTH / 3;
    double VERTICAL_BUTTON_SEPARATION = BUTTON_HEIGH / 5;

    for(int i= 0; i < 4; i++){
        QPushButton * button = new QPushButton(this);

        button->setGeometry(QRect(BUTTON_WIDTH * (i % 2) + HORIZONTAL_BUTTON_SEPARATION * ((i % 2) + 1),
                                  BUTTON_HEIGH * (i / 2)  + VERTICAL_BUTTON_SEPARATION * ((i / 2)),
                                  BUTTON_WIDTH, BUTTON_HEIGH));

        //button->setEnabled(false);
        buttonList.append(button);

        connect(button, &QPushButton::pressed, this, [=](){
            manager->turnHook(orden[i]);
            //emit selectedPrize();
        });
    }

    BUTTON_WIDTH = width() / 4;
    BUTTON_HEIGH = BUTTON_WIDTH;
    HORIZONTAL_BUTTON_SEPARATION = BUTTON_WIDTH / 4;
    VERTICAL_BUTTON_SEPARATION = BUTTON_HEIGH / 4;

    for(int i= 0; i < 6; i++){
        QPushButton * button = new QPushButton(this);

        button->setGeometry(QRect(BUTTON_WIDTH * (i % 3) + HORIZONTAL_BUTTON_SEPARATION * ((i % 3) + 1),
                                  BUTTON_HEIGH * ( 3 + i / 3)  + VERTICAL_BUTTON_SEPARATION * ((i / 3)),
                                  BUTTON_WIDTH, BUTTON_HEIGH));

        //button->setEnabled(false);
        buttonList.append(button);



        connect(button, &QPushButton::pressed, this, [=](){
            manager->turnHook(orden[4 + i]);
            //emit selectedPrize();
        });
    }

    for(int i= 0; i < 10; i++){
        std::ostringstream oss;
        oss << "Producto" << orden[i] << ".nombre";
        QString image= setting.value(oss.str().c_str()).toString();
        oss.str("");
        buttonList.at(i)->setStyleSheet("QPushButton{border-image:url("
                                               + GameElement::url + "image/products/" + image + ");" +
                                               "} QPushButton:focus{margin: 2px 2px 2px 2px;}"
                                               "QPushButton{margin: 15px 15px 15px 15px}");
    }
}

void awardPrize::resizeEvent(QResizeEvent *){
    setStyleSheet("background-image: url(" + GameElement::url + "/image/fondo_productos.png)");
    double BUTTON_WIDTH = width() / 3;
    double BUTTON_HEIGH = height() / 4;
    double HORIZONTAL_BUTTON_SEPARATION = BUTTON_WIDTH / 3;
    double VERTICAL_BUTTON_SEPARATION = BUTTON_HEIGH / 5;

    for(int i= 0; i < 4; i++){
        QPushButton * button = buttonList.at(i);

        button->setGeometry(QRect(BUTTON_WIDTH * (i % 2) + HORIZONTAL_BUTTON_SEPARATION * ((i % 2) + 1),
            BUTTON_HEIGH * (i / 2)  + VERTICAL_BUTTON_SEPARATION * ((i / 2)),
            BUTTON_WIDTH, BUTTON_HEIGH));

    }

    BUTTON_WIDTH = width() / 4;
    BUTTON_HEIGH = BUTTON_WIDTH;
    HORIZONTAL_BUTTON_SEPARATION = BUTTON_WIDTH / 4;
    VERTICAL_BUTTON_SEPARATION = BUTTON_HEIGH / 4;

    for(int i= 0; i < 3; i++){
        QPushButton * button = buttonList.at(4 + i);
        button->setGeometry(QRect(BUTTON_WIDTH * (i % 3) + HORIZONTAL_BUTTON_SEPARATION * ((i % 3) + 1),
                                  BUTTON_HEIGH * ( 3 + i / 3)  + 300,
                                  BUTTON_WIDTH, BUTTON_HEIGH));
    }

    for(int i= 3; i < 6; i++){
        QPushButton * button = buttonList.at(4 + i);
        button->setGeometry(QRect(BUTTON_WIDTH * (i % 3) + HORIZONTAL_BUTTON_SEPARATION * ((i % 3) + 1),
                                  BUTTON_HEIGH * ( 3 + i / 3)  + 400,
                                  BUTTON_WIDTH, BUTTON_HEIGH));
    }

}

void awardPrize::enablePrizes(int num){
    for(int i= 0; i < 10; i++){
        buttonList.at(i)->setEnabled(false);
    }
    int a;
    int b;

    if(num == 3){
        a= 4;
        b= 10;
    }else if(num == 6){
        a= 2;
        b= 4;
    }else if(num == 9){
        a= 0;
        b= 2;
    }

    for(int i= a; i < b; i++){
        buttonList.at(i)->setEnabled(true);
    }

}

ProductsManager *awardPrize::getManager()
{
   return manager;
}

void awardPrize::displayInfo()
{
    if(active || !this->isVisible()) return;
    //soundSwooshing->play();
    a = new QMessageBox("FELICIDADES", "FELICIDADES RECOJA SU PREMIO",
                                     QMessageBox::NoIcon, QMessageBox::NoButton,
                                     QMessageBox::NoButton, QMessageBox::NoButton, this);
    a->show();
    active= true;

    QTimer::singleShot(5000, this, [=](){
        a->close();
        close();
        active= false;
    });
}

void awardPrize::loadProducs(){
    for(int i=0; i < 10; i++){
        //Product p(this);
        Product *p = new Product(this);
        connect(p, &Product::countChanged, this, [=](){
            std::ostringstream oss;
            oss << "Producto" << i << ".existencias";
            setting.setValue(oss.str().c_str(), p->count());
        });

        std::ostringstream oss;
        oss << "Producto" << i << ".nombre";
        p->setName(setting.value(oss.str().c_str()).toString());
        oss.str("");
        oss << "Producto" << i << ".existencias";
        p->setCount(setting.value(oss.str().c_str()).toInt());
        manager->addProduct(p);
        qDebug() << p->name() << p->count() << endl;
    }

}
