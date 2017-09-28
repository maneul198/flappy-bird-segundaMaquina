/****************************************************************************
** Meta object code from reading C++ file 'enginesv.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../enginesv.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'enginesv.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_EngineSV_t {
    QByteArrayData data[4];
    char stringdata0[38];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EngineSV_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EngineSV_t qt_meta_stringdata_EngineSV = {
    {
QT_MOC_LITERAL(0, 0, 8), // "EngineSV"
QT_MOC_LITERAL(1, 9, 14), // "engineFinished"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 12) // "engineStoped"

    },
    "EngineSV\0engineFinished\0\0engineStoped"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EngineSV[] = {

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
       1,    0,   24,    2, 0x06 /* Public */,
       3,    0,   25,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void EngineSV::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        EngineSV *_t = static_cast<EngineSV *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->engineFinished(); break;
        case 1: _t->engineStoped(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (EngineSV::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&EngineSV::engineFinished)) {
                *result = 0;
            }
        }
        {
            typedef void (EngineSV::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&EngineSV::engineStoped)) {
                *result = 1;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject EngineSV::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_EngineSV.data,
      qt_meta_data_EngineSV,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *EngineSV::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EngineSV::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_EngineSV.stringdata0))
        return static_cast<void*>(const_cast< EngineSV*>(this));
    return QObject::qt_metacast(_clname);
}

int EngineSV::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void EngineSV::engineFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void EngineSV::engineStoped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
