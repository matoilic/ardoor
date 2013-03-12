//#############################################################################
//  File:      qtGLWidget.h
//  Purpose:   Declaration of the QGLWidget derive window class.
//             In general all the event hadlers will foreward the events to the 
//             appropriate event handlers of the SLSceneView class
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED
//#############################################################################

#ifndef QTGLWIDGET_H
#define QTGLWIDGET_H

#include <QtOpenGL/QGLWidget>

//-----------------------------------------------------------------------------
//! Qt QGLWidget class (for OpenGL rendering) for OpenGL display
/*!
qtGLWidget provides the GUI functionality for OpenGL rendering and is
therefore derived from QGLWidget. It forewards all rendering and interaction
tasks to the according methods of the interface in slInterface
*/
class qtGLWidget : public QGLWidget
{
   public:              qtGLWidget        (QGLFormat &format, QWidget* parent);
                       ~qtGLWidget        ();
   public:
            void        initializeGL      ();
            void        resizeGL          (int w,int h);
            void        paintGL           ();
            void        mousePressEvent   (QMouseEvent *e);
            void        mouseReleaseEvent (QMouseEvent *e);
            void        mouseMoveEvent    (QMouseEvent *e);
            void        wheelEvent        (QWheelEvent *e); 
            void        keyPressEvent     (QKeyEvent   *e);
};
//-----------------------------------------------------------------------------
#endif
