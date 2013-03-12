//#############################################################################
//  File:      qt4Main.cpp
//  Purpose:   Implements the main routine with the Qt application instance
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <stdafx.h>
#include "qt4MainWindow.h"

//-----------------------------------------------------------------------------
/*!
The main procedure holding the Qt application instance as well as the main
window instance of our class qt4MainWindow.
*/
int main(int argc, char *argv[])
{  
   // main Qt application instance
   QApplication app(argc, argv);
   
   // add plugins folder to the library path
   app.addLibraryPath(app.applicationDirPath() + "/plugins");
   
   // create one main window and show it
   qt4MainWindow mainWin(argc, argv);
   mainWin.show();
   
   // let's rock ...
   return app.exec();
}
//-----------------------------------------------------------------------------
