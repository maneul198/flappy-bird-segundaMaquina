#ifndef MYKEYEVENTS
#define MYKEYEVENTS

#include <QCoreApplication>
#include <QApplication>
#include <QKeyEvent>
#include <QWidget>
#include <QObject>
#include <QWindow>


void enter_pressed(){
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress
                                 , Qt::Key_Enter
                                 , Qt::NoModifier
                                 , QKeySequence(Qt::Key_Enter).toString());

    QCoreApplication::postEvent(QApplication::focusWidget(), k);
}

void left_pressed(){
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress
                                 , Qt::Key_Left
                                 , Qt::NoModifier
                                 , QKeySequence(Qt::Key_Left).toString());

    QCoreApplication::postEvent(QApplication::focusWidget(), k);

}

void right_pressed(){
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress
                                 , Qt::Key_Right
                                 , Qt::NoModifier
                                 , QKeySequence(Qt::Key_Right).toString());

    QCoreApplication::postEvent(QApplication::focusWidget(), k);

}

void space_pressed(){
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress
                                 , Qt::Key_Space
                                 , Qt::NoModifier
                                 , QKeySequence(Qt::Key_Space).toString());

    qDebug() << QApplication::focusWidget() << endl;

    //QCoreApplication::postEvent(a, k);
    QCoreApplication::postEvent(QApplication::focusWidget(), k);

}
#endif
