/****************************************************************************
** Meta object code from reading C++ file 'helperservice.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/helper/helperservice.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'helperservice.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_HelperService_t {
    QByteArrayData data[20];
    char stringdata0[217];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HelperService_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HelperService_t qt_meta_stringdata_HelperService = {
    {
QT_MOC_LITERAL(0, 0, 13), // "HelperService"
QT_MOC_LITERAL(1, 14, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 30, 22), // "com.debmanager.Helper1"
QT_MOC_LITERAL(3, 53, 6), // "Output"
QT_MOC_LITERAL(4, 60, 0), // ""
QT_MOC_LITERAL(5, 61, 11), // "transaction"
QT_MOC_LITERAL(6, 73, 4), // "text"
QT_MOC_LITERAL(7, 78, 8), // "Finished"
QT_MOC_LITERAL(8, 87, 8), // "exitCode"
QT_MOC_LITERAL(9, 96, 7), // "message"
QT_MOC_LITERAL(10, 104, 20), // "InstallLocalPackages"
QT_MOC_LITERAL(11, 125, 5), // "paths"
QT_MOC_LITERAL(12, 131, 15), // "InstallPackages"
QT_MOC_LITERAL(13, 147, 5), // "names"
QT_MOC_LITERAL(14, 153, 14), // "RemovePackages"
QT_MOC_LITERAL(15, 168, 5), // "purge"
QT_MOC_LITERAL(16, 174, 11), // "UpdateCache"
QT_MOC_LITERAL(17, 186, 7), // "Upgrade"
QT_MOC_LITERAL(18, 194, 11), // "distUpgrade"
QT_MOC_LITERAL(19, 206, 10) // "AutoRemove"

    },
    "HelperService\0D-Bus Interface\0"
    "com.debmanager.Helper1\0Output\0\0"
    "transaction\0text\0Finished\0exitCode\0"
    "message\0InstallLocalPackages\0paths\0"
    "InstallPackages\0names\0RemovePackages\0"
    "purge\0UpdateCache\0Upgrade\0distUpgrade\0"
    "AutoRemove"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HelperService[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       8,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // classinfo: key, value
       1,    2,

 // signals: name, argc, parameters, tag, flags
       3,    2,   56,    4, 0x06 /* Public */,
       7,    3,   61,    4, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   68,    4, 0x0a /* Public */,
      12,    1,   71,    4, 0x0a /* Public */,
      14,    2,   74,    4, 0x0a /* Public */,
      16,    0,   79,    4, 0x0a /* Public */,
      17,    1,   80,    4, 0x0a /* Public */,
      19,    0,   83,    4, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    5,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::QString,    5,    8,    9,

 // slots: parameters
    QMetaType::QString, QMetaType::QStringList,   11,
    QMetaType::QString, QMetaType::QStringList,   13,
    QMetaType::QString, QMetaType::QStringList, QMetaType::Bool,   13,   15,
    QMetaType::QString,
    QMetaType::QString, QMetaType::Bool,   18,
    QMetaType::QString,

       0        // eod
};

void HelperService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HelperService *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->Output((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->Finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 2: { QString _r = _t->InstallLocalPackages((*reinterpret_cast< const QStringList(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 3: { QString _r = _t->InstallPackages((*reinterpret_cast< const QStringList(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 4: { QString _r = _t->RemovePackages((*reinterpret_cast< const QStringList(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 5: { QString _r = _t->UpdateCache();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 6: { QString _r = _t->Upgrade((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 7: { QString _r = _t->AutoRemove();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HelperService::*)(const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HelperService::Output)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (HelperService::*)(const QString & , int , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HelperService::Finished)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject HelperService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_HelperService.data,
    qt_meta_data_HelperService,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *HelperService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HelperService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HelperService.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QDBusContext"))
        return static_cast< QDBusContext*>(this);
    return QObject::qt_metacast(_clname);
}

int HelperService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void HelperService::Output(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void HelperService::Finished(const QString & _t1, int _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
