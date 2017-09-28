/****************************************************************************
** Meta object code from reading C++ file 'id003_lib_v3.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../id003_lib_v3.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'id003_lib_v3.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ID003_Lib_V3_t {
    QByteArrayData data[20];
    char stringdata0[243];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ID003_Lib_V3_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ID003_Lib_V3_t qt_meta_stringdata_ID003_Lib_V3 = {
    {
QT_MOC_LITERAL(0, 0, 12), // "ID003_Lib_V3"
QT_MOC_LITERAL(1, 13, 11), // "entroDinero"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 6), // "dinero"
QT_MOC_LITERAL(4, 33, 11), // "stackerFull"
QT_MOC_LITERAL(5, 45, 15), // "stackerRetirado"
QT_MOC_LITERAL(6, 61, 10), // "noteJammed"
QT_MOC_LITERAL(7, 72, 16), // "billeteroPausado"
QT_MOC_LITERAL(8, 89, 13), // "intentoFraude"
QT_MOC_LITERAL(9, 103, 16), // "fallaEnBilletero"
QT_MOC_LITERAL(10, 120, 16), // "falla_especifica"
QT_MOC_LITERAL(11, 137, 19), // "errorEnComunicacion"
QT_MOC_LITERAL(12, 157, 15), // "comandoInvalido"
QT_MOC_LITERAL(13, 173, 19), // "estadoIndeterminado"
QT_MOC_LITERAL(14, 193, 13), // "openPortError"
QT_MOC_LITERAL(15, 207, 6), // "TITOIN"
QT_MOC_LITERAL(16, 214, 11), // "abrirPuerto"
QT_MOC_LITERAL(17, 226, 5), // "char*"
QT_MOC_LITERAL(18, 232, 3), // "dev"
QT_MOC_LITERAL(19, 236, 6) // "apagar"

    },
    "ID003_Lib_V3\0entroDinero\0\0dinero\0"
    "stackerFull\0stackerRetirado\0noteJammed\0"
    "billeteroPausado\0intentoFraude\0"
    "fallaEnBilletero\0falla_especifica\0"
    "errorEnComunicacion\0comandoInvalido\0"
    "estadoIndeterminado\0openPortError\0"
    "TITOIN\0abrirPuerto\0char*\0dev\0apagar"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ID003_Lib_V3[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      12,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x06 /* Public */,
       4,    0,   87,    2, 0x06 /* Public */,
       5,    0,   88,    2, 0x06 /* Public */,
       6,    0,   89,    2, 0x06 /* Public */,
       7,    0,   90,    2, 0x06 /* Public */,
       8,    0,   91,    2, 0x06 /* Public */,
       9,    1,   92,    2, 0x06 /* Public */,
      11,    0,   95,    2, 0x06 /* Public */,
      12,    0,   96,    2, 0x06 /* Public */,
      13,    0,   97,    2, 0x06 /* Public */,
      14,    0,   98,    2, 0x06 /* Public */,
      15,    0,   99,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    1,  100,    2, 0x0a /* Public */,
      19,    0,  103,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Bool, 0x80000000 | 17,   18,
    QMetaType::Void,

       0        // eod
};

void ID003_Lib_V3::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ID003_Lib_V3 *_t = static_cast<ID003_Lib_V3 *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->entroDinero((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->stackerFull(); break;
        case 2: _t->stackerRetirado(); break;
        case 3: _t->noteJammed(); break;
        case 4: _t->billeteroPausado(); break;
        case 5: _t->intentoFraude(); break;
        case 6: _t->fallaEnBilletero((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 7: _t->errorEnComunicacion(); break;
        case 8: _t->comandoInvalido(); break;
        case 9: _t->estadoIndeterminado(); break;
        case 10: _t->openPortError(); break;
        case 11: _t->TITOIN(); break;
        case 12: { bool _r = _t->abrirPuerto((*reinterpret_cast< char*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 13: _t->apagar(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ID003_Lib_V3::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::entroDinero)) {
                *result = 0;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::stackerFull)) {
                *result = 1;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::stackerRetirado)) {
                *result = 2;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::noteJammed)) {
                *result = 3;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::billeteroPausado)) {
                *result = 4;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::intentoFraude)) {
                *result = 5;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)(unsigned int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::fallaEnBilletero)) {
                *result = 6;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::errorEnComunicacion)) {
                *result = 7;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::comandoInvalido)) {
                *result = 8;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::estadoIndeterminado)) {
                *result = 9;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::openPortError)) {
                *result = 10;
            }
        }
        {
            typedef void (ID003_Lib_V3::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ID003_Lib_V3::TITOIN)) {
                *result = 11;
            }
        }
    }
}

const QMetaObject ID003_Lib_V3::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_ID003_Lib_V3.data,
      qt_meta_data_ID003_Lib_V3,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ID003_Lib_V3::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ID003_Lib_V3::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ID003_Lib_V3.stringdata0))
        return static_cast<void*>(const_cast< ID003_Lib_V3*>(this));
    return QThread::qt_metacast(_clname);
}

int ID003_Lib_V3::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void ID003_Lib_V3::entroDinero(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ID003_Lib_V3::stackerFull()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void ID003_Lib_V3::stackerRetirado()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void ID003_Lib_V3::noteJammed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void ID003_Lib_V3::billeteroPausado()
{
    QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}

// SIGNAL 5
void ID003_Lib_V3::intentoFraude()
{
    QMetaObject::activate(this, &staticMetaObject, 5, Q_NULLPTR);
}

// SIGNAL 6
void ID003_Lib_V3::fallaEnBilletero(unsigned int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ID003_Lib_V3::errorEnComunicacion()
{
    QMetaObject::activate(this, &staticMetaObject, 7, Q_NULLPTR);
}

// SIGNAL 8
void ID003_Lib_V3::comandoInvalido()
{
    QMetaObject::activate(this, &staticMetaObject, 8, Q_NULLPTR);
}

// SIGNAL 9
void ID003_Lib_V3::estadoIndeterminado()
{
    QMetaObject::activate(this, &staticMetaObject, 9, Q_NULLPTR);
}

// SIGNAL 10
void ID003_Lib_V3::openPortError()
{
    QMetaObject::activate(this, &staticMetaObject, 10, Q_NULLPTR);
}

// SIGNAL 11
void ID003_Lib_V3::TITOIN()
{
    QMetaObject::activate(this, &staticMetaObject, 11, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
