//#############################################################################
//  File:      qt4MainWindow.cpp
//  Purpose:   Implementation of the QMainWindow & SLScene derived window class
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <stdafx.h>

#include "qt4MainWindow.h"
#include "qt4SceneView.h"

//-----------------------------------------------------------------------------
qt4MainWindow::qt4MainWindow(int argc, char** argv) : SLScene(this, "MyScene")
{  
   (void)argc; // not used
   (void)argv; // not used

   // on Mac OSX the sample buffers must be turned on for antialiasing
   QGLFormat fmt;
   fmt.defaultFormat();
   fmt.setSampleBuffers(true);
   fmt.setSwapInterval(1);
   fmt.setProfile(QGLFormat::CompatibilityProfile);
   QGLFormat::setDefaultFormat(fmt);

   // create central widget
   _activeSceneView = new qt4SceneView(this, (SLScene*)this, "MySceneViewName");
   setCentralWidget(_activeSceneView);
   
   SLScene::activeSV((SLSceneView*)_activeSceneView); // must be before onLoadScene
   SLScene::onLoadScene();
   
   // create _actions
   _actQuit = new QAction(tr("&Quit"), this);
   _actQuit->setShortcut(tr("Ctrl+Q"));
   connect(_actQuit, SIGNAL(triggered()), this, SLOT(close()));

   _actAboutKeys = new QAction(tr("About Keys"), this);
   connect(_actAboutKeys, SIGNAL(triggered()), this, SLOT(onAboutKeys()));

   _actAboutOpenGL = new QAction(tr("About OpenGL"), this);
   connect(_actAboutOpenGL, SIGNAL(triggered()), this, SLOT(onAboutOpenGL()));
   
   _actAboutQt = new QAction(tr("About &Qt"), this);
   connect(_actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
   
   _actAbout = new QAction(tr("&About"), this);
   connect(_actAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
   
   // create menus
   _mnuFile = menuBar()->addMenu(tr("&File"));
   _mnuFile->addAction(_actQuit);
   _mnuHelp = menuBar()->addMenu(tr("&Help"));
   _mnuHelp->addAction(_actAboutKeys);
   _mnuHelp->addAction(_actAboutOpenGL);
   _mnuHelp->addAction(_actAboutQt);
   _mnuHelp->addAction(_actAbout);
}
//-----------------------------------------------------------------------------
qt4MainWindow::~qt4MainWindow()
{  
   //printf("~--------------------------------------------------------------\n");
   //printf("~Destruction:\n");
   //printf("~qt4MainWindow\n");      
}
//-----------------------------------------------------------------------------
//! The size of widgets is available with this event but not before.
void qt4MainWindow::showEvent(QShowEvent * event)
{
   #ifdef SL_OS_MACOSX
   setGeometry(100,100,320,240);
   #else
   //setGeometry(100,100,320,240 + menuBar()->height());
   setGeometry(100,100,640, 480 + menuBar()->height());
   #endif
}
//-----------------------------------------------------------------------------
void qt4MainWindow::onAboutKeys()
{  
   QMessageBox::about(this, 
      tr("About Keyboard and Mouse Control"),
      tr("Mouse Left Button: camera trackball rotation\n"
         "Mouse Middle Button: camera translation left/right, up/down\n"
         "Mouse Right Button: show context menu for changing display modes\n"
         "Mousewheel: camera forward/backwards\n"
         "Cursor left/right: camera left/right\n"
         "Cursor up/down: camera forward/backwards\n"
         "Page up/down: camera up/down\n"
         "HOME: reset to initial settings\n"
         "ALT & Mousewheel: changes the view angle\n"
         "CTRL/SHIFT & cursor keys: precise rotation\n"));
}
//-----------------------------------------------------------------------------
void qt4MainWindow::onAboutOpenGL()
{  
   SLstring info = SLScene::openglInfoString();
   QMessageBox::about(this, "About OpenGL", info.c_str());
}
//-----------------------------------------------------------------------------
void qt4MainWindow::onAbout()
{  
   QMessageBox::about(this, tr("About this Scene Library Application"),
      tr("Demonstates a specific feature of the Scene Library.\n"
         "See the readme file for details.\n"
         "THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY\n"
         "AND WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.\n"
         "2001-2010 Marcus Hudritsch"));
}
//----------------------------------------------------------------------------
