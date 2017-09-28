/****************************************************************************
** Meta object code from reading C++ file 'product.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../cmakeFlappy/product.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'product.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Product_t {
    QByteArrayData data[15];
    char stringdata0[129];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Product_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Product_t qt_meta_stringdata_Product = {
    {
QT_MOC_LITERAL(0, 0, 7), // "Product"
QT_MOC_LITERAL(1, 8, 11), // "iconChanged"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 11), // "nameChanged"
QT_MOC_LITERAL(4, 33, 12), // "valueChanged"
QT_MOC_LITERAL(5, 46, 12), // "countChanged"
QT_MOC_LITERAL(6, 59, 7), // "setName"
QT_MOC_LITERAL(7, 67, 4), // "name"
QT_MOC_LITERAL(8, 72, 7), // "setIcon"
QT_MOC_LITERAL(9, 80, 4), // "icon"
QT_MOC_LITERAL(10, 85, 8), // "setValue"
QT_MOC_LITERAL(11, 94, 5), // "value"
QT_MOC_LITERAL(12, 100, 8), // "setCount"
QT_MOC_LITERAL(13, 109, 5), // "count"
QT_MOC_LITERAL(14, 115, 13) // "decreaseCount"

    },
    "Product\0iconChanged\0\0nameChanged\0"
    "valueChanged\0countChanged\0setName\0"
    "name\0setIcon\0icon\0setValue\0value\0"
    "setCount\0count\0decreaseCount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Product[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       4,   76, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    0,   60,    2, 0x06 /* Public */,
       4,    0,   61,    2, 0x06 /* Public */,
       5,    0,   62,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   63,    2, 0x0a /* Public */,
       8,    1,   66,    2, 0x0a /* Public */,
      10,    1,   69,    2, 0x0a /* Public */,
      12,    1,   72,    2, 0x0a /* Public */,
      14,    0,   75,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::UInt,   11,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void,

 // properties: name, type, flags
       9, QMetaType::QString, 0x00495103,
       7, QMetaType::QString, 0x00495103,
      11, QMetaType::UInt, 0x00495103,
      13, QMetaType::UInt, 0x00495103,

 // properties: notify_signal_id
       0,
       1,
       2,
       3,

       0        // eod
};

void Product::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Product *_t = static_cast<Product *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->iconChanged(); break;
        case 1: _t->nameChanged(); break;
        case 2: _t->valueChanged(); break;
        case 3: _t->countChanged(); break;
        case 4: _t->setName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->setIcon((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->setValue((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 7: _t->setCount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->decreaseCount(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Product::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Product::iconChanged)) {
                *result = 0;
            }
        }
        {
            typedef void (Product::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Product::nameChanged)) {
                *result = 1;
            }
        }
        {
            typedef void (Product::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Product::valueChanged)) {
                *result = 2;
            }
        }
        {
            typedef void (Product::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Product::countChanged)) {
                *result = 3;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        Product *_t = static_cast<Product *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->icon(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->name(); break;
        case 2: *reinterpret_cast< uint*>(_v) = _t->value(); break;
        case 3: *reinterpret_cast< uint*>(_v) = _t->count(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        Product *_t = static_cast<Product *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setIcon(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setName(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setValue(*reinterpret_cast< uint*>(_v)); break;
        case 3: _t->setCount(*reinterpret_cast< uint*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

const QMetaObject Product::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Product.data,
      qt_meta_data_Product,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Product::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Product::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Product.stringdata0))
        return static_cast<void*>(const_cast< Product*>(this));
    return QObject::qt_metacast(_clname);
}

int Product::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Product::iconChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void Product::nameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void Product::valueChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void Product::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
