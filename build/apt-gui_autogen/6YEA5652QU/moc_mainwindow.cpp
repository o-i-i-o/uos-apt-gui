/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.19)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.19. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[26];
    char stringdata0[449];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 18), // "refreshPackageList"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 19), // "searchLocalPackages"
QT_MOC_LITERAL(4, 51, 22), // "removeSelectedPackages"
QT_MOC_LITERAL(5, 74, 20), // "installLocalPackages"
QT_MOC_LITERAL(6, 95, 18), // "autoRemovePackages"
QT_MOC_LITERAL(7, 114, 18), // "onOperationStarted"
QT_MOC_LITERAL(8, 133, 7), // "message"
QT_MOC_LITERAL(9, 141, 19), // "onOperationProgress"
QT_MOC_LITERAL(10, 161, 19), // "onOperationFinished"
QT_MOC_LITERAL(11, 181, 7), // "success"
QT_MOC_LITERAL(12, 189, 25), // "onPackageSelectionChanged"
QT_MOC_LITERAL(13, 215, 13), // "onItemChanged"
QT_MOC_LITERAL(14, 229, 17), // "QTableWidgetItem*"
QT_MOC_LITERAL(15, 247, 4), // "item"
QT_MOC_LITERAL(16, 252, 19), // "getSelectedPackages"
QT_MOC_LITERAL(17, 272, 20), // "searchOnlinePackages"
QT_MOC_LITERAL(18, 293, 29), // "installSelectedOnlinePackages"
QT_MOC_LITERAL(19, 323, 14), // "updateAptCache"
QT_MOC_LITERAL(20, 338, 13), // "upgradeSystem"
QT_MOC_LITERAL(21, 352, 31), // "onOnlinePackageSelectionChanged"
QT_MOC_LITERAL(22, 384, 19), // "onOnlineItemChanged"
QT_MOC_LITERAL(23, 404, 25), // "getSelectedOnlinePackages"
QT_MOC_LITERAL(24, 430, 12), // "onTabChanged"
QT_MOC_LITERAL(25, 443, 5) // "index"

    },
    "MainWindow\0refreshPackageList\0\0"
    "searchLocalPackages\0removeSelectedPackages\0"
    "installLocalPackages\0autoRemovePackages\0"
    "onOperationStarted\0message\0"
    "onOperationProgress\0onOperationFinished\0"
    "success\0onPackageSelectionChanged\0"
    "onItemChanged\0QTableWidgetItem*\0item\0"
    "getSelectedPackages\0searchOnlinePackages\0"
    "installSelectedOnlinePackages\0"
    "updateAptCache\0upgradeSystem\0"
    "onOnlinePackageSelectionChanged\0"
    "onOnlineItemChanged\0getSelectedOnlinePackages\0"
    "onTabChanged\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  109,    2, 0x08 /* Private */,
       3,    0,  110,    2, 0x08 /* Private */,
       4,    0,  111,    2, 0x08 /* Private */,
       5,    0,  112,    2, 0x08 /* Private */,
       6,    0,  113,    2, 0x08 /* Private */,
       7,    1,  114,    2, 0x08 /* Private */,
       9,    1,  117,    2, 0x08 /* Private */,
      10,    2,  120,    2, 0x08 /* Private */,
      12,    0,  125,    2, 0x08 /* Private */,
      13,    1,  126,    2, 0x08 /* Private */,
      16,    0,  129,    2, 0x08 /* Private */,
      17,    0,  130,    2, 0x08 /* Private */,
      18,    0,  131,    2, 0x08 /* Private */,
      19,    0,  132,    2, 0x08 /* Private */,
      20,    0,  133,    2, 0x08 /* Private */,
      21,    0,  134,    2, 0x08 /* Private */,
      22,    1,  135,    2, 0x08 /* Private */,
      23,    0,  138,    2, 0x08 /* Private */,
      24,    1,  139,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   11,    8,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::QStringList,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::QStringList,
    QMetaType::Void, QMetaType::Int,   25,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->refreshPackageList(); break;
        case 1: _t->searchLocalPackages(); break;
        case 2: _t->removeSelectedPackages(); break;
        case 3: _t->installLocalPackages(); break;
        case 4: _t->autoRemovePackages(); break;
        case 5: _t->onOperationStarted((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->onOperationProgress((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->onOperationFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 8: _t->onPackageSelectionChanged(); break;
        case 9: _t->onItemChanged((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 10: { QStringList _r = _t->getSelectedPackages();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->searchOnlinePackages(); break;
        case 12: _t->installSelectedOnlinePackages(); break;
        case 13: _t->updateAptCache(); break;
        case 14: _t->upgradeSystem(); break;
        case 15: _t->onOnlinePackageSelectionChanged(); break;
        case 16: _t->onOnlineItemChanged((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 17: { QStringList _r = _t->getSelectedOnlinePackages();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->onTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
