//#############################################################################
//  File:      qt4SceneView.h
//  Purpose:   Declaration of the QGLWidget derive window class.
//             In general all the event hadlers will foreward the events to the 
//             appropriate event handlers of the SLSceneView class
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED
//
//  Moccing:   Add unter the headerfile properties under Custom Build Step: 
//  Cmd line:  $(QTDIR)\bin\moc.exe "$(InputPath)" -o "$(InputDir)moc_$(InputName).cpp"
//  Outputs:   "$(InputDir)moc_$(InputName).cpp"
//#############################################################################

#ifndef QT4SCENEVIEWWIDGET_H
#define QT4SCENEVIEWWIDGET_H

#include <stdafx.h>
#include <SLSceneView.h>  // must be included before <QtOpenGL>

class SLScene;
class qt4MainWindow;
class qt4Raytrace;

//-----------------------------------------------------------------------------
//! Qt QGLWidget class (for OpenGL rendering) containing a sceneview.
/*!
qt4SceneView: provides the GUI functionality for OpenGL rendering and is 
therefore derived from QGLWidget. It forewards all rendering and interaction
tasks to the according methods of the class SLSceneView
*/
class qt4SceneView : public QGLWidget, public SLSceneView
{  
   Q_OBJECT
   public:              qt4SceneView      (QWidget* parent, 
                                           SLScene* scene,
                                           const char* name = 0);
                       ~qt4SceneView      ();
   public slots:
            void        onDrawingBitHide  ();
            void        onDrawingBitMesh  ();
            void        onDrawingBitFlat  ();
            void        onDrawingBitCullOff();
            void        onDrawingBitNormal();
            void        onDrawingBitAABBGr();
            void        onDrawingBitAABB  ();
            void        onDrawingBitTexOff();
            void        onDrawingBitGLSLOff();
            void        onDrawingBitVoxel ();
            
            void        onProjectionPersp ();
            void        onProjectionOrtho ();
            void        onRestoreCamera   ();
            
            void        onDoAutoRepaint   (bool isOn);
            void        onDoAntiAliasing  (bool isOn);
            void        onDoFrustumCulling(bool isOn);
   public:
            void        initializeGL      ();
            void        resizeGL          (int w,int h);
            void        paintGL           ();
            void        mousePressEvent   (QMouseEvent *e);
            void        mouseReleaseEvent (QMouseEvent *e);
            void        mouseMoveEvent    (QMouseEvent *e);
            void        wheelEvent        (QWheelEvent *e); 
            void        keyPressEvent     (QKeyEvent   *e);
   
   
   #ifdef SL_RAY1
            // Raytrace window
            qt4Raytrace* wndRT;
               
   // callback for the GUI interface                                      
   static   void SL_STDCALL RTWndCreate(const void* parentWidget, int w, int h);
   static   void SL_STDCALL RTWndUpdate(const void* parentWidget, float percent);
   static   void SL_STDCALL RTWndSetPix(const void* parentWidget, int x, int y, const SLCol4f color);
   static   void SL_STDCALL RTWndGetPix(const void* parentWidget, int x, int y, SLCol4f &color);
   static   void SL_STDCALL RTWndSaveIm(const void* parentWidget, int depth);
   #endif
   
   private:
            qt4MainWindow* myMainWnd;
            QMenu*      _mnuPopup;
            QAction*       _actDrawingBitHide;
            QAction*       _actDrawingBitFlat;
            QAction*       _actDrawingBitMesh;
            QAction*       _actDrawingBitNormal;
            QAction*       _actDrawingBitAABBGr;
            QAction*       _actDrawingBitAABB;
            QAction*       _actDrawingBitTexOff;
            QAction*       _actDrawingBitGLSLOff;
            QAction*       _actDrawingBitVoxel;
            QAction*       _actDrawingBitCullOff;
            QMenu*         _mnuProjection;
            QAction*          _actProjectionPersp;
            QAction*          _actProjectionOrtho;
            QAction*       _actRestoreCamera;
            QAction*       _actDoAutoRepaint;
            QAction*       _actDoAntiAliasing;
            QAction*       _actDoFrustumCulling;
};
//-----------------------------------------------------------------------------
#endif
