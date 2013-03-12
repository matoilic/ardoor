//#############################################################################
//  File:      qt4Raytrace.cpp
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <stdafx.h>

#include "qt4Raytrace.h"
#include "qt4SceneView.h"

int qt4Raytrace::_currentID = 1;

//-----------------------------------------------------------------------------
/*! 
Creates and shows the QWidget window with a title bar, a system menu as a 
dialog window. The allocated memory is automatically released when the user
closes the window.
*/
qt4Raytrace::qt4Raytrace(QWidget* parent,
                         int width, 
                         int height) : QWidget(0, Qt::Tool)
{  _parent = parent;
   _image = 0;
   setWindowTitle("Ray Tracing");
   setAttribute(Qt::WA_DeleteOnClose);
   init(width, height);
}
//-----------------------------------------------------------------------------
qt4Raytrace::~qt4Raytrace()
{  //printf("~qt4Raytrace\n");  
   if (_image) delete _image;
   qt4SceneView* sv = (qt4SceneView*)_parent;
   sv->wndRT = 0;
}
//-----------------------------------------------------------------------------
//! Initializes the RT window and its image object
void qt4Raytrace::init(int width, int height)
{  
   if (_image) delete _image;
   _image = new QImage(width,height,QImage::Format_RGB32);
   resize(width,height);
   
   // get the main window of the sceneview widget
   QWidget* p = (QWidget*)_parent->parent();
   move(p->x()+p->frameGeometry().width()+5,
        p->geometry().y());
}
//-----------------------------------------------------------------------------
/*!
This method is called by the Ray Tracing algorithm. It only draws the pixel and
stores it in the image object.
*/
void qt4Raytrace::setPixel(int x, int y, SLCol4f color)
{  color *= 255.0f;
   _image->setPixel(x, y, qRgb(color.x, color.y, color.z));
}
//-----------------------------------------------------------------------------
/*!
This method is called by the Ray Tracing algorithm. It only draws the pixel and
stores it in the image object.
*/
void qt4Raytrace::getPixel(int x, int y, SLCol4f &color)
{  QRgb col;
   col = _image->pixel(x, y);
   color.r = (SLfloat)qRed  (col) / 255.0f;
   color.g = (SLfloat)qGreen(col) / 255.0f;
   color.b = (SLfloat)qBlue (col) / 255.0f;
   color.a = 0.0f;
}
//-----------------------------------------------------------------------------
/*!
Saves the image as a PNG file on disc with a unique name.
*/
void qt4Raytrace::saveAsPNG(int depth)
{  
   _id = _currentID++;
   char filename[255];  
   sprintf(filename,"Raytrace_%d_%d.png", _id, depth);
      
   if (_image->save(filename,"PNG")) 
   {  printf("\nImage file name   : %s",filename);
   } else SL_WARN_MSG("Image file writing failed.");
   update();
}
//-----------------------------------------------------------------------------
/*!
Paint event just for repainting the rendered image
*/
void qt4Raytrace::paintEvent(QPaintEvent* e)
{  
   (void)e;                   // avoid unused parameter warning
   _painter.begin(this);      // begin the painter here once is a lot faster.
   _painter.drawImage(0,0, *_image);
   _painter.end();
}
//-----------------------------------------------------------------------------
