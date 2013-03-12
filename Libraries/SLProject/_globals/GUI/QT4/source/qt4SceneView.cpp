//#############################################################################
//  File:      qt4SceneView.cpp
//  Purpose:   Implementation of the Qt4 QGLWidget derived window class.
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <stdafx.h>

#include "qt4SceneView.h"
#include "qt4Raytrace.h"
#include "qt4MainWindow.h"
#include "SLCamera.h"

class SLScene;

//-----------------------------------------------------------------------------
/*!
qt4SceneView Constructor: Creates the QMenus and attaches them to the
appropriate action.
*/
qt4SceneView::qt4SceneView(QWidget* parent,
                           SLScene* scene, 
                           const char* name)
             :QGLWidget(parent), SLSceneView(this, scene, name)
{  
   myMainWnd = (qt4MainWindow*)parent;
   
   #ifdef SL_RAY1
   // pass the callback function addresses to the sceneviews raytracer
   SLSceneView::raytracer()->guiRTWndCreate = (cbRTWndCreate) &RTWndCreate;
   SLSceneView::raytracer()->guiRTWndUpdate = (cbRTWndUpdate) &RTWndUpdate;
   SLSceneView::raytracer()->guiRTWndSetPix = (cbRTWndSetPix) &RTWndSetPix;
   SLSceneView::raytracer()->guiRTWndGetPix = (cbRTWndGetPix) &RTWndGetPix;
   SLSceneView::raytracer()->guiRTWndSaveIm = (cbRTWndSaveIm) &RTWndSaveIm;
   wndRT = 0;
   #endif
   
   // Create _actions for the context mnuPopup
   _actDrawingBitHide = new QAction(tr("Hide Objects [H]"), this);
   _actDrawingBitHide->setCheckable(true);
   connect(_actDrawingBitHide, SIGNAL(triggered(bool)), SLOT(onDrawingBitHide()));
   
   _actDrawingBitFlat = new QAction(tr("Use Flat Shading [F]"), this);
   _actDrawingBitFlat->setCheckable(true);
   connect(_actDrawingBitFlat, SIGNAL(triggered(bool)), SLOT(onDrawingBitFlat()));

   _actDrawingBitMesh = new QAction(tr("Show mesh as wireframe [M]"), this);
   _actDrawingBitMesh->setCheckable(true);
   connect(_actDrawingBitMesh, SIGNAL(triggered(bool)), SLOT(onDrawingBitMesh()));

   _actDrawingBitCullOff = new QAction(tr("Turn off face culling [C]"), this);
   _actDrawingBitCullOff->setCheckable(true);
   connect(_actDrawingBitCullOff, SIGNAL(triggered(bool)), SLOT(onDrawingBitCullOff()));

   _actDrawingBitNormal = new QAction(tr("Show Vertex Normals [N]"), this);
   _actDrawingBitNormal->setCheckable(true);
   connect(_actDrawingBitNormal, SIGNAL(triggered(bool)), SLOT(onDrawingBitNormal()));

   _actDrawingBitAABBGr = new QAction(tr("Show bounding boxes of Groups [R]"), this);
   _actDrawingBitAABBGr->setCheckable(true);
   connect(_actDrawingBitAABBGr, SIGNAL(triggered(bool)), SLOT(onDrawingBitAABBGr()));

   _actDrawingBitAABB = new QAction(tr("Show bounding boxes of shapes [B]"), this);
   _actDrawingBitAABB->setCheckable(true);
   connect(_actDrawingBitAABB, SIGNAL(triggered(bool)), SLOT(onDrawingBitAABB()));

   _actDrawingBitTexOff = new QAction(tr("Turn off Texture Mapping [T]"), this);
   _actDrawingBitTexOff->setCheckable(true);
   connect(_actDrawingBitTexOff, SIGNAL(triggered(bool)), SLOT(onDrawingBitTexOff()));
   
   _actDrawingBitGLSLOff = new QAction(tr("Turn off GLSL shaders [G]"), this);
   _actDrawingBitGLSLOff->setCheckable(true);
   connect(_actDrawingBitGLSLOff, SIGNAL(triggered(bool)), SLOT(onDrawingBitGLSLOff()));
   
   _actDrawingBitVoxel = new QAction(tr("Show Voxels [V]"), this);
   _actDrawingBitVoxel->setCheckable(true);
   connect(_actDrawingBitVoxel, SIGNAL(triggered(bool)), SLOT(onDrawingBitVoxel()));
   
   _actProjectionPersp = new QAction(tr("Perspective"), this);
   _actProjectionPersp->setCheckable(true);
   _actProjectionPersp->setChecked(true);
   connect(_actProjectionPersp, SIGNAL(triggered(bool)), SLOT(onProjectionPersp()));
   
   _actProjectionOrtho = new QAction(tr("Orthographic"), this);
   _actProjectionOrtho->setCheckable(true);
   connect(_actProjectionOrtho, SIGNAL(triggered(bool)), SLOT(onProjectionOrtho()));
   
   _actRestoreCamera = new QAction(tr("Restore camera view"), this);
   _actRestoreCamera->setCheckable(false);
   connect(_actRestoreCamera, SIGNAL(triggered(bool)), SLOT(onRestoreCamera()));
   
   _actDoAutoRepaint = new QAction(tr("Do Auto Repaint"), this);
   _actDoAutoRepaint->setCheckable(true);
   _actDoAutoRepaint->setChecked(SLSceneView::doAutoRepaint());
   connect(_actDoAutoRepaint, SIGNAL(triggered(bool)), SLOT(onDoAutoRepaint(bool)));
   
   _actDoAntiAliasing = new QAction(tr("Do Antialiasing"), this);
   _actDoAntiAliasing->setCheckable(true);
   _actDoAntiAliasing->setChecked(true);
   connect(_actDoAntiAliasing, SIGNAL(triggered(bool)), SLOT(onDoAntiAliasing(bool)));
   
   _actDoFrustumCulling = new QAction(tr("Do Frustum Culling"), this);
   _actDoFrustumCulling->setCheckable(true);
   _actDoFrustumCulling->setChecked(true);
   connect(_actDoFrustumCulling, SIGNAL(triggered(bool)), SLOT(onDoFrustumCulling(bool)));
   
   // build context menu
   _mnuPopup = new QMenu(this);
   
   _mnuPopup->addAction(_actDrawingBitHide);
   _mnuPopup->addAction(_actDrawingBitFlat);
   _mnuPopup->addAction(_actDrawingBitMesh);
   _mnuPopup->addAction(_actDrawingBitCullOff);
   #ifdef SL_MESH
   _mnuPopup->addAction(_actDrawingBitNormal);
   #endif
   #ifdef SL_AABB
   _mnuPopup->addAction(_actDrawingBitAABBGr);
   _mnuPopup->addAction(_actDrawingBitAABB);
   #endif
   #ifdef SL_MESH
   _mnuPopup->addAction(_actDrawingBitTexOff);
   #endif
   #ifdef SL_GLSL
   _mnuPopup->addAction(_actDrawingBitGLSLOff);
   #endif
   #ifdef SL_RAY2
   _mnuPopup->addAction(_actDrawingBitVoxel);
   #endif
   _mnuPopup->addSeparator();
   _mnuProjection = new QMenu("Projection", this);
   _mnuProjection->addAction(_actProjectionPersp);
   _mnuProjection->addAction(_actProjectionOrtho);
   _mnuPopup->addMenu(_mnuProjection);
   _mnuPopup->addAction(_actRestoreCamera);
   _mnuPopup->addSeparator();
   _mnuPopup->addAction(_actDoAutoRepaint);
   _mnuPopup->addAction(_actDoAntiAliasing);
   _mnuPopup->addAction(_actDoFrustumCulling);
   
   setFocusPolicy(Qt::StrongFocus); // to receive keyboard focus    
}

//-----------------------------------------------------------------------------
qt4SceneView::~qt4SceneView()
{ 
   #ifdef SL_RAY1
   //printf("~qt4SceneView\n");
   if (wndRT) 
   {  wndRT->close();
      delete wndRT;
   }
   #endif
}

//-----------------------------------------------------------------------------
//! onDrawingBitHide: hides objects and shows AABB
void qt4SceneView::onDrawingBitHide()
{  if (_actDrawingBitHide->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_HIDE);
      _actDrawingBitHide->setChecked(true);
      SLSceneView::setDrawingBit(SL_DB_BBOX);
      _actDrawingBitAABB->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_HIDE);
      _actDrawingBitHide->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitFlat: slot method turning on/off wire frame mode
void qt4SceneView::onDrawingBitFlat()
{  if (_actDrawingBitFlat->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_FLAT);
      _actDrawingBitFlat->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_FLAT);
      _actDrawingBitFlat->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitMesh: slot method turning on/off wire frame mode
void qt4SceneView::onDrawingBitMesh()
{  if (_actDrawingBitMesh->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_MESHWIRE);
      _actDrawingBitMesh->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_MESHWIRE);
      _actDrawingBitMesh->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitCullOff: slot method turning on/off face culling
void qt4SceneView::onDrawingBitCullOff()
{  if (_actDrawingBitCullOff->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_CULLOFF);
      _actDrawingBitCullOff->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_CULLOFF);
      _actDrawingBitCullOff->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitNormal: slot method turning on/off the vertex normals
void qt4SceneView::onDrawingBitNormal()
{  if (_actDrawingBitNormal->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_NORMALS);
      _actDrawingBitNormal->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_NORMALS);
      _actDrawingBitNormal->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitVoxel: slot method turning on/off the voxels
void qt4SceneView::onDrawingBitVoxel()
{  
   #ifdef SL_RAY2
   if (_actDrawingBitVoxel->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_VOXELS);
      _actDrawingBitVoxel->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_VOXELS);
      _actDrawingBitVoxel->setChecked(false);
   }
   updateGL();
   #endif
}
//-----------------------------------------------------------------------------
//! onDrawingBitAABBGr: slot method turning on/off AABBs of groups
void qt4SceneView::onDrawingBitAABBGr()
{  if (_actDrawingBitAABBGr->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_BBOXGROUP);
      _actDrawingBitAABBGr->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_BBOXGROUP);
      _actDrawingBitAABBGr->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitAABB: slot method turning on/off AABBs of shapes
void qt4SceneView::onDrawingBitAABB()
{  if (_actDrawingBitAABB->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_BBOX);
      _actDrawingBitAABB->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_BBOX);
      _actDrawingBitAABB->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitTexOff: slot method turning on/off Texture hiding
void qt4SceneView::onDrawingBitTexOff()
{  if (_actDrawingBitTexOff->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_TEXOFF);
      _actDrawingBitTexOff->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_TEXOFF);
      _actDrawingBitTexOff->setChecked(false);
   }
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDrawingBitGLSLOff: slot method turning on/off GLSL
void qt4SceneView::onDrawingBitGLSLOff()
{  
   #ifdef SL_GLSL
   if (_actDrawingBitGLSLOff->isChecked())
   {  SLSceneView::setDrawingBit(SL_DB_GLSLOFF);
      _actDrawingBitGLSLOff->setChecked(true);
   } else
   {  SLSceneView::delDrawingBit(SL_DB_GLSLOFF);
      _actDrawingBitGLSLOff->setChecked(false);
   }
   updateGL();
   #endif
}
//-----------------------------------------------------------------------------
//! onProjectionPersp: slot method for OpenGL perspective projection
void qt4SceneView::onProjectionPersp()
{  SLSceneView::onCommand(cmdProjectionPersp);
   _actProjectionPersp->setChecked(true);
   _actProjectionOrtho->setChecked(false);
   updateGL();
}
//-----------------------------------------------------------------------------
//! onProjectionOrtho: slot method for OpenGL orthographic projection
void qt4SceneView::onProjectionOrtho()
{  SLSceneView::onCommand(cmdProjectionOrtho);
   _actProjectionPersp->setChecked(false);
   _actProjectionOrtho->setChecked(true);
   updateGL();
}
//-----------------------------------------------------------------------------
//! onRestoreCamera: restores the original settings of the active camera
void qt4SceneView::onRestoreCamera()
{  SLSceneView::onCommand(cmdRestoreCamera);
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDoAutoRepaint: slot method for turning on constant repaints
void qt4SceneView::onDoAutoRepaint(bool isOn)
{  SLSceneView::doAutoRepaint(isOn);
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDoAntiAliasing: slot method for turning on/off hardware antialiasing
void qt4SceneView::onDoAntiAliasing(bool isOn)
{  if (isOn) SLSceneView::onCommand(cmdAntiAliasing_On);
   else SLSceneView::onCommand(cmdAntiAliasing_Off);
   updateGL();
}
//-----------------------------------------------------------------------------
//! onDoFrustumCulling: slot method for turning on/off software frustum culling
void qt4SceneView::onDoFrustumCulling(bool isOn)
{  if (isOn) SLSceneView::onCommand(cmdFrustumCulling_On);
   else SLSceneView::onCommand(cmdFrustumCulling_Off);
   updateGL();
}
//-----------------------------------------------------------------------------
/*!
initializeGL: Event handler called on the initializeGL event of the window. 
This event is called once before the first resizeGL event and is used for 
OpenGL initialization.
*/
void qt4SceneView::initializeGL()
{  SLSceneView::onInitializeGL();
   setAutoBufferSwap(false); // for correct framerate calculation (see paintGL)
}
//-----------------------------------------------------------------------------
/*!
resizeGL: Event handler called on the resize event of the window. This event is
called once before the paintGL event.
*/
void qt4SceneView::resizeGL(int w,int h)
{  SLSceneView::onResizeGL(w, h);
}
//-----------------------------------------------------------------------------
/*!
paintGL: Paint event handler that passes the event to the according paint 
method of the SLScene class. For accurate frame rate meassurement we have to 
take the time after the OpenGL frame buffer swapping. Because this is normally
done by Qt we have to disable this with setAutoBufferSwap(false). Qt provides
an OS independent swap method. After that we can call our calcFPS method.
*/
void qt4SceneView::paintGL()
{  SLSceneView::onPaintGL();
   swapBuffers();    // Qt swaps the backbuffer to the front
   
   // Build caption string with scene name and fps
   QString title;
   #ifdef SL_AABB
   title.sprintf("%s (fps: %4.1f, %u shapes rendered)", 
                 myMainWnd->name().c_str(), 
                 SLSceneView::calcFPS(),
                 SLSceneView::camera()->numRendered());
   #else
   title.sprintf("%s (fps: %4.1f)", 
                 myMainWnd->name().c_str(), 
                 SLSceneView::calcFPS());
   #endif
   myMainWnd->setWindowTitle(title);
   
   // Simply call update for constant repaint. Never call paintGL directly
   if (SLSceneView::doAutoRepaint()) update();
}
//-----------------------------------------------------------------------------
/*!
mousePressEvent: Event handler called when a mouse buttons where pressed.
*/
void qt4SceneView::mousePressEvent(QMouseEvent *e)
{  if (e->button()==Qt::LeftButton)  
   {  if (SLSceneView::onMouseDown(ButtonLeft, e->x(),e->y())) updateGL();
   } else
   if (e->button()==Qt::RightButton) 
   {  _mnuPopup->exec(e->globalPos());
   } else 
   if (e->button()==Qt::MidButton) 
   {   if (SLSceneView::onMouseDown(ButtonMiddle, e->x(),e->y())) updateGL();
   }
}
//-----------------------------------------------------------------------------
/*!
mouseReleaseEvent: Event handler called when a mouse buttons where released.
*/
void qt4SceneView::mouseReleaseEvent(QMouseEvent *e)
{  if (e->button()==Qt::LeftButton) 
   {  if (SLSceneView::onMouseUp(ButtonLeft, e->x(),e->y())) updateGL();
   } else 
   if (e->button()==Qt::RightButton) 
   {  if (SLSceneView::onMouseUp(ButtonRight, e->x(),e->y())) updateGL();
   } else 
   if (e->button()==Qt::MidButton) 
   {  if (SLSceneView::onMouseUp(ButtonMiddle, e->x(),e->y())) updateGL();
   }
}
//-----------------------------------------------------------------------------
/*!
mouseMoveEvent: Event handler called when the mouse is moving.
*/
void qt4SceneView::mouseMoveEvent(QMouseEvent *e)
{  if (SLSceneView::onMouseMove(e->x(),e->y())) updateGL();
}
//-----------------------------------------------------------------------------
/*!
wheelEvent: Before the mouse wheel event is passed over to the according event
handler of SLScene the key modifiers are evaluated.
*/
void qt4SceneView::wheelEvent(QWheelEvent *e)
{  SLuint mod = KeyNone;   
   if (e->modifiers() & Qt::SHIFT) mod = KeyShift;
   if (e->modifiers() & Qt::CTRL)  mod = (SLKey)(mod|KeyCtrl);
   if (e->modifiers() & Qt::ALT)   mod = (SLKey)(mod|KeyAlt);
   
   if (SLSceneView::onMouseWheel(e->delta(), mod)) updateGL();
} 
//-----------------------------------------------------------------------------
/*!
keyPressEvent: The keyPressEvent is translated into SLKey constants and 
forewarded to SLScene::onKeyPress event handler.
*/
void qt4SceneView::keyPressEvent(QKeyEvent* e)
{  SLKey mod=KeyNone, key=KeyNone;   

   if (e->key() >= ' ' && e->key() <= 'Z') key = (SLKey)e->key();
   switch (e->key())
   {  case Qt::Key_Escape:    key = KeyEsc;      break;
      case Qt::Key_Up:        key = KeyUp;       break;
      case Qt::Key_Down:      key = KeyDown;     break;
      case Qt::Key_Left:      key = KeyLeft;     break;
      case Qt::Key_Right:     key = KeyRight;    break;
      case Qt::Key_PageUp:    key = KeyPageUp;   break;
      case Qt::Key_PageDown:  key = KeyPageDown; break;
      case Qt::Key_Home:      key = KeyHome;     break;
      case Qt::Key_F1:        key = KeyF1;       break;
      case Qt::Key_F2:        key = KeyF2;       break;
      case Qt::Key_F3:        key = KeyF3;       break;
      case Qt::Key_F4:        key = KeyF4;       break;
      case Qt::Key_F5:        key = KeyF5;       break;
      case Qt::Key_F6:        key = KeyF6;       break;
      case Qt::Key_F7:        key = KeyF7;       break;
      case Qt::Key_F8:        key = KeyF8;       break;
      case Qt::Key_F9:        key = KeyF9;       break;
      case Qt::Key_F10:       key = KeyF10;      break;
      case Qt::Key_F11:       key = KeyF11;      break;
      case Qt::Key_F12:       key = KeyF12;      break;
      case Qt::Key_Tab:       key = KeyTab;      break;
   }
   
   //if (!e->isAutoRepeat())
   {  if (e->modifiers() & Qt::CTRL)  mod = (SLKey)(mod|KeyCtrl);
      if (e->modifiers() & Qt::ALT)   mod = (SLKey)(mod|KeyAlt);
      if (e->modifiers() & Qt::SHIFT) mod = (SLKey)(mod|KeyShift);
   }
   if (key)
   {  
      if (key=='H') 
      {  togDrawingBit(SL_DB_HIDE);  
         togDrawingBit(SL_DB_BBOX);
         _actDrawingBitFlat->setChecked(getDrawingBit(SL_DB_HIDE));
         _actDrawingBitFlat->setChecked(getDrawingBit(SL_DB_BBOX));
         updateGL();    
      } 
      
      if (key=='F') 
      {  togDrawingBit(SL_DB_FLAT);  
         _actDrawingBitFlat->setChecked(getDrawingBit(SL_DB_FLAT));
         updateGL();    
      } 
      
      if (key=='M') 
      {  togDrawingBit(SL_DB_MESHWIRE);
         _actDrawingBitMesh->setChecked(getDrawingBit(SL_DB_MESHWIRE));
         updateGL();
      }  
      
      if (key=='C') 
      {  togDrawingBit(SL_DB_CULLOFF);  
         _actDrawingBitFlat->setChecked(getDrawingBit(SL_DB_CULLOFF));
         updateGL();    
      }    
      
      if (key=='N') 
      {  togDrawingBit(SL_DB_NORMALS); 
         _actDrawingBitNormal->setChecked(getDrawingBit(SL_DB_NORMALS));
         updateGL();  
      }   
      
      if (key=='T') 
      {  togDrawingBit(SL_DB_TEXOFF);   
         _actDrawingBitTexOff->setChecked(getDrawingBit(SL_DB_TEXOFF));
         updateGL(); 
      }  
      
      #ifdef SL_GLSL
      if (key=='G') 
      {  togDrawingBit(SL_DB_GLSLOFF);
         _actDrawingBitGLSLOff->setChecked(getDrawingBit(SL_DB_GLSLOFF));
         updateGL();   
      }
      #endif
      
      if (key=='R') 
      {  togDrawingBit(SL_DB_BBOXGROUP); 
         _actDrawingBitAABBGr->setChecked(getDrawingBit(SL_DB_BBOXGROUP));
         updateGL();
      } 
      
      if (key=='B') 
      {  togDrawingBit(SL_DB_BBOX);   
         _actDrawingBitAABB->setChecked(getDrawingBit(SL_DB_BBOX));
         updateGL();   
      }
      
      #ifdef SL_RAY2
      if (key=='P') 
      {  togDrawingBit(SL_DB_VOXELS); 
         _actDrawingBitVoxel->setChecked(getDrawingBit(SL_DB_VOXELS));
         updateGL();   
      }
      #endif
       
      
      if (key==KeyEsc) qApp->quit();
      if (SLSceneView::onKeyPress((SLKey)(mod|key))) updateGL();
   }
}




//-----------------------------------------------------------------------------
// Ray Tracing callback functions
// Callbacks are used so that we do not need to include Qt headers in our RT
//-----------------------------------------------------------------------------
#ifdef SL_RAY1
//! RTWndCreate is called to create the RT window.
void qt4SceneView::RTWndCreate(const void* parentWidget, int w, int h)
{  assert(parentWidget);
   qt4SceneView* sv = (qt4SceneView*)parentWidget;
   if (sv->wndRT==0)
   {  sv->wndRT = new qt4Raytrace(sv, w, h);
      sv->wndRT->show();
   } else 
   {  sv->wndRT->init(w, h);
   }
}
//-----------------------------------------------------------------------------
//! RTWndUpdate is called to repaint the RT window 
void qt4SceneView::RTWndUpdate(const void* parentWidget, float percent)
{  assert(parentWidget);
   qt4SceneView* sv = (qt4SceneView*)parentWidget;
   QString title;
   title.sprintf("%s (%5.2f)", "Ray Tracing: ", percent);
   if (sv->wndRT) 
   {  sv->wndRT->setWindowTitle(title);
      sv->wndRT->update();
      QCoreApplication::processEvents();
   }
}
//-----------------------------------------------------------------------------
//! RTWndSetPix sets one pixel into image of the RT window
void qt4SceneView::RTWndSetPix(const void* parentWidget, int x, int y, 
                               const SLCol4f color)
{  assert(parentWidget);
   qt4SceneView* sv = (qt4SceneView*)parentWidget;
   sv->wndRT->setPixel(x, y, color);
}
//-----------------------------------------------------------------------------
//! RTWndGetPix gets one pixel into image of the RT window
void qt4SceneView::RTWndGetPix(const void* parentWidget, int x, int y, 
                               SLCol4f &color)
{  assert(parentWidget);
   qt4SceneView* sv = (qt4SceneView*)parentWidget;
   sv->wndRT->getPixel(x, y, color);
}
//-----------------------------------------------------------------------------
//! RTWndSaveIm saves the RT image
void qt4SceneView::RTWndSaveIm(const void* parentWidget, int depth)
{  assert(parentWidget);
   qt4SceneView* sv = (qt4SceneView*)parentWidget;
   if (sv->wndRT) sv->wndRT->saveAsPNG(depth);
}
//-----------------------------------------------------------------------------
#endif
