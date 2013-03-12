/****************************************************************************
** Meta object code from reading C++ file 'qt4MainWindow.h'
**
** Created: Wed 22. Sep 08:08:55 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <stdafx.h>
#include "qt4MainWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qt4MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qt4MainWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      29,   14,   14,   14, 0x08,
      45,   14,   14,   14, 0x08,
      61,   55,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_qt4MainWindow[] = {
    "qt4MainWindow\0\0onAboutKeys()\0"
    "onAboutOpenGL()\0onAbout()\0event\0"
    "showEvent(QShowEvent*)\0"
};

const QMetaObject qt4MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_qt4MainWindow,
      qt_meta_data_qt4MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qt4MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qt4MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qt4MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qt4MainWindow))
        return static_cast<void*>(const_cast< qt4MainWindow*>(this));
    if (!strcmp(_clname, "SLScene"))
        return static_cast< SLScene*>(const_cast< qt4MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int qt4MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onAboutKeys(); break;
        case 1: onAboutOpenGL(); break;
        case 2: onAbout(); break;
        case 3: showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
