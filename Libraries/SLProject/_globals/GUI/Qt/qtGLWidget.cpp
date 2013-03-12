//#############################################################################
//  File:      qtGLWidget.cpp
//  Purpose:   Implementation of the Qt4 QGLWidget derived window class.
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <qtGLWidget.h>
#include <SLInterface.h>
#include <QMouseEvent>
#include <QtGui/QApplication>

//-----------------------------------------------------------------------------
/*!
qtGLWidget Constructor: Creates the QMenus and attaches them to the
appropriate action.
*/
qtGLWidget::qtGLWidget(QGLFormat &format, QWidget* parent) :
            QGLWidget(format, parent)
{
   setFocusPolicy(Qt::StrongFocus); // to receive keyboard focus
   setAutoBufferSwap(false);        // for correct framerate calculation (see paintGL)
}

//-----------------------------------------------------------------------------
qtGLWidget::~qtGLWidget()
{
}

//-----------------------------------------------------------------------------
/*!
initializeGL: Event handler called on the initializeGL event of the window. 
This event is called once before the first resizeGL event and is used for 
OpenGL initialization.
*/
void qtGLWidget::initializeGL()
{
   // Initialize Scene Library
   slInit(640, 480, 142, (SLCmd)SL_STARTSCENE,
          "../_globals/oglsl/",
          "../_data/models/3DS/",
          "../_data/images/textures/");
}
//-----------------------------------------------------------------------------
/*!
resizeGL: Event handler called on the resize event of the window. This event is
called once before the paintGL event.
*/
void qtGLWidget::resizeGL(int w,int h)
{
}
//-----------------------------------------------------------------------------
/*!
paintGL: Paint event handler that passes the event to the according paint 
method of the SLScene class. For accurate frame rate meassurement we have to 
take the time after the OpenGL frame buffer swapping. Because this is normally
done by Qt we have to disable this with setAutoBufferSwap(false). Qt provides
an OS independent swap method. After that we can call our calcFPS method.
*/
void qtGLWidget::paintGL()
{
   bool updated = slPaint();

   swapBuffers();    // Qt swaps the backbuffer to the front
   
   // Build caption string with scene name and fps
   slGetWindowTitle().c_str();
   
   // Simply call update for constant repaint. Never call paintGL directly
   if (updated) update();
}
//-----------------------------------------------------------------------------
/*!
mousePressEvent: Event handler called when a mouse buttons where pressed.
*/
void qtGLWidget::mousePressEvent(QMouseEvent *e)
{
   SLKey modifiers = KeyNone;
   if (e->modifiers() & Qt::SHIFT) modifiers = KeyShift;
   if (e->modifiers() & Qt::CTRL)  modifiers = (SLKey)(modifiers|KeyCtrl);
   if (e->modifiers() & Qt::ALT)   modifiers = (SLKey)(modifiers|KeyAlt);

   if (e->button()==Qt::LeftButton)
   {  if (slMouseDown(ButtonLeft, e->x(),e->y(), modifiers)) updateGL();
   } else
   if (e->button()==Qt::RightButton) 
   {  if (slMouseDown(ButtonRight, e->x(),e->y(), modifiers)) updateGL();
   } else 
   if (e->button()==Qt::MidButton) 
   {  if (slMouseDown(ButtonMiddle, e->x(),e->y(), modifiers)) updateGL();
   }
}
//-----------------------------------------------------------------------------
/*!
mouseReleaseEvent: Event handler called when a mouse buttons where released.
*/
void qtGLWidget::mouseReleaseEvent(QMouseEvent *e)
{
   SLKey modifiers = KeyNone;
   if (e->modifiers() & Qt::SHIFT) modifiers = KeyShift;
   if (e->modifiers() & Qt::CTRL)  modifiers = (SLKey)(modifiers|KeyCtrl);
   if (e->modifiers() & Qt::ALT)   modifiers = (SLKey)(modifiers|KeyAlt);

   if (e->button()==Qt::LeftButton)
   {  if (slMouseUp(ButtonLeft, e->x(),e->y(), modifiers)) updateGL();
   } else 
   if (e->button()==Qt::RightButton) 
   {  if (slMouseUp(ButtonRight, e->x(),e->y(), modifiers)) updateGL();
   } else 
   if (e->button()==Qt::MidButton) 
   {  if (slMouseUp(ButtonMiddle, e->x(),e->y(), modifiers)) updateGL();
   }
}
//-----------------------------------------------------------------------------
/*!
mouseMoveEvent: Event handler called when the mouse is moving.
*/
void qtGLWidget::mouseMoveEvent(QMouseEvent *e)
{
   SLuint modifiers = KeyNone;
   if (e->modifiers() & Qt::SHIFT) modifiers = KeyShift;
   if (e->modifiers() & Qt::CTRL)  modifiers = (SLKey)(modifiers|KeyCtrl);
   if (e->modifiers() & Qt::ALT)   modifiers = (SLKey)(modifiers|KeyAlt);

   if (slMouseMove(e->x(), e->y())) updateGL();
}
//-----------------------------------------------------------------------------
/*!
wheelEvent: Before the mouse wheel event is passed over to the according event
handler of SLScene the key modifiers are evaluated.
*/
void qtGLWidget::wheelEvent(QWheelEvent *e)
{
   SLKey modifiers = KeyNone;
   if (e->modifiers() & Qt::SHIFT) modifiers = KeyShift;
   if (e->modifiers() & Qt::CTRL)  modifiers = (SLKey)(modifiers|KeyCtrl);
   if (e->modifiers() & Qt::ALT)   modifiers = (SLKey)(modifiers|KeyAlt);

   if (slMouseWheel(e->delta(), modifiers)) updateGL();
} 
//-----------------------------------------------------------------------------
/*!
keyPressEvent: The keyPressEvent is translated into SLKey constants and 
forewarded to SLScene::onKeyPress event handler.
*/
void qtGLWidget::keyPressEvent(QKeyEvent* e)
{
   SLKey modifiers = KeyNone;
   if (e->modifiers() & Qt::SHIFT) modifiers = KeyShift;
   if (e->modifiers() & Qt::CTRL)  modifiers = (SLKey)(modifiers|KeyCtrl);
   if (e->modifiers() & Qt::ALT)   modifiers = (SLKey)(modifiers|KeyAlt);

   SLKey key=KeyNone;
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

   if (key==KeyEsc) QCoreApplication::quit();
   if (slKeyPress(key, modifiers)) updateGL();
}
//-----------------------------------------------------------------------------
