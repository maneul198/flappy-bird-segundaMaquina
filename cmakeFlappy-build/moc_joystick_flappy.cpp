/****************************************************************************
** Meta object code from reading C++ file 'joystick_flappy.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../cmakeFlappy/joystick_flappy.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'joystick_flappy.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_joystick_flappy_t {
    QByteArrayData data[6];
    char stringdata0[59];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_joystick_flappy_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_joystick_flappy_t qt_meta_stringdata_joystick_flappy = {
    {
QT_MOC_LITERAL(0, 0, 15), // "joystick_flappy"
QT_MOC_LITERAL(1, 16, 14), // "buttom_pressed"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 6), // "button"
QT_MOC_LITERAL(4, 39, 15), // "joystick_change"
QT_MOC_LITERAL(5, 55, 3) // "num"

    },
    "joystick_flappy\0buttom_pressed\0\0button\0"
    "joystick_change\0num"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_joystick_flappy[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,

       0        // eod
};

void joystick_flappy::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        joystick_flappy *_t = static_cast<joystick_flappy *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->buttom_pressed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->joystick_change((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (joystick_flappy::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&joystick_flappy::buttom_pressed)) {
                *result = 0;
            }
        }
        {
            typedef void (joystick_flappy::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&joystick_flappy::joystick_change)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject joystick_flappy::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_joystick_flappy.data,
      qt_meta_data_joystick_flappy,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *joystick_flappy::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *joystick_flappy::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_joystick_flappy.stringdata0))
        return static_cast<void*>(const_cast< joystick_flappy*>(this));
    return QThread::qt_metacast(_clname);
}

int joystick_flappy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void joystick_flappy::buttom_pressed(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void joystick_flappy::joystick_change(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
