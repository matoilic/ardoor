/****************************************************************************
** Meta object code from reading C++ file 'qt4SceneView.h'
**
** Created: Wed 22. Sep 08:08:55 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <stdafx.h>
#include "qt4SceneView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qt4SceneView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qt4SceneView[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,
      33,   13,   13,   13, 0x0a,
      52,   13,   13,   13, 0x0a,
      71,   13,   13,   13, 0x0a,
      93,   13,   13,   13, 0x0a,
     114,   13,   13,   13, 0x0a,
     135,   13,   13,   13, 0x0a,
     154,   13,   13,   13, 0x0a,
     175,   13,   13,   13, 0x0a,
     197,   13,   13,   13, 0x0a,
     217,   13,   13,   13, 0x0a,
     237,   13,   13,   13, 0x0a,
     257,   13,   13,   13, 0x0a,
     280,  275,   13,   13, 0x0a,
     302,  275,   13,   13, 0x0a,
     325,  275,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_qt4SceneView[] = {
    "qt4SceneView\0\0onDrawingBitHide()\0"
    "onDrawingBitMesh()\0onDrawingBitFlat()\0"
    "onDrawingBitCullOff()\0onDrawingBitNormal()\0"
    "onDrawingBitAABBGr()\0onDrawingBitAABB()\0"
    "onDrawingBitTexOff()\0onDrawingBitGLSLOff()\0"
    "onDrawingBitVoxel()\0onProjectionPersp()\0"
    "onProjectionOrtho()\0onRestoreCamera()\0"
    "isOn\0onDoAutoRepaint(bool)\0"
    "onDoAntiAliasing(bool)\0onDoFrustumCulling(bool)\0"
};

const QMetaObject qt4SceneView::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_qt4SceneView,
      qt_meta_data_qt4SceneView, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qt4SceneView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qt4SceneView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qt4SceneView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qt4SceneView))
        return static_cast<void*>(const_cast< qt4SceneView*>(this));
    if (!strcmp(_clname, "SLSceneView"))
        return static_cast< SLSceneView*>(const_cast< qt4SceneView*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int qt4SceneView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onDrawingBitHide(); break;
        case 1: onDrawingBitMesh(); break;
        case 2: onDrawingBitFlat(); break;
        case 3: onDrawingBitCullOff(); break;
        case 4: onDrawingBitNormal(); break;
        case 5: onDrawingBitAABBGr(); break;
        case 6: onDrawingBitAABB(); break;
        case 7: onDrawingBitTexOff(); break;
        case 8: onDrawingBitGLSLOff(); break;
        case 9: onDrawingBitVoxel(); break;
        case 10: onProjectionPersp(); break;
        case 11: onProjectionOrtho(); break;
        case 12: onRestoreCamera(); break;
        case 13: onDoAutoRepaint((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: onDoAntiAliasing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: onDoFrustumCulling((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
