//#############################################################################
//  File:      qtMain.cpp
//  Purpose:   Implements the main routine with the Qt application instance
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <QtGui/QApplication>
#include <qgl.h>
#include <qtGLWidget.h>
//-----------------------------------------------------------------------------
/*!
The main procedure holding the Qt application instance as well as the main
window instance of our class qt4QMainWindow.
*/
int main(int argc, char *argv[])
{  
   // main Qt application instance
   QApplication app(argc, argv);
   app.setApplicationName("cube");
   app.setApplicationVersion("0.1");

   // on Mac OSX the sample buffers must be turned on for antialiasing
   QGLFormat format;
   format.defaultFormat();
   format.setSampleBuffers(true);
   format.setSwapInterval(1);
   //format.setVersion(2,1);
   //QGLFormat::setDefaultFormat(format);

   // create OpenGL widget
   qtGLWidget myGLWidget(format, 0);
   myGLWidget.resize(640, 480);
   myGLWidget.show();
   
   // let's rock ...
   return app.exec();
}
//-----------------------------------------------------------------------------
