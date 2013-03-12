//#############################################################################
//  File:      qt4MainWindow.h
//  Purpose:   Declaration of the QMainWindow & SLScene derived window class
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//
//  Moccing:   Add under the headerfile properties under Custom Build Step: 
//  Cmd line:  $(QTDIR)\bin\moc.exe "$(InputPath)" -o "$(InputDir)moc_$(InputName).cpp"
//  Outputs:   "$(InputDir)moc_$(InputName).cpp"
//#############################################################################

#ifndef QT4MAINWINDOW_H
#define QT4MAINWINDOW_H

#include <stdafx.h>
#include <SLScene.h>

class QMenu;
class QAction;
class qt4SceneView;
class qt4SceneTree;
class QDockWidget;

//-----------------------------------------------------------------------------
/*!
qt4MainWindow: The Qt main window class derived from QMainWindow and our 
SLScene class so that it has access to the scene wide valid parameters.
It would be possible to have several views of the same scene.
*/
class qt4MainWindow : public QMainWindow, public SLScene
{
   Q_OBJECT
   public:
                              qt4MainWindow(int argc=0, char* argv[]=0);
                             ~qt4MainWindow();
               
   private slots:
               void           onAboutKeys();
               void           onAboutOpenGL();
               void           onAbout();
               void           showEvent(QShowEvent* event);
   private:
               qt4SceneView*  _activeSceneView;

               QMenu*         _mnuFile;
               QMenu*         _mnuHelp;
               
               QAction*       _actQuit;
               QAction*       _actAboutCPU;
               QAction*       _actAboutKeys;
               QAction*       _actAboutOpenGL;
               QAction*       _actAboutQt;
               QAction*       _actAbout;
};
//-----------------------------------------------------------------------------
#endif
