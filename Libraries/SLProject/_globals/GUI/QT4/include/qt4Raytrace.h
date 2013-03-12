//#############################################################################
//  File:      qtRaytrace.h
//  Author:    Marcus Hudritsch
//  Date:      18-SEP-10 (HS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#ifndef QRAYTRACE_H
#define QRAYTRACE_H

#include <stdafx.h>
#include <SLScene.h>

//-----------------------------------------------------------------------------
//! Simple window widget for ray tracing output.
/*!
Simple window widget derived from QWidget for ray tracing output. 
This window shows the tracing progress during the process as well as the 
finished image afterwards.
*/
class qt4Raytrace : public QWidget
{  public:
                        qt4Raytrace (QWidget* parent,
                                     int width, 
                                     int height);
                       ~qt4Raytrace ();
                       
            void        init        (int width, int height);
            void        setPixel    (int x, int y, SLCol4f  color);
            void        getPixel    (int x, int y, SLCol4f &color);
            void        saveAsPNG   (int depth);
            void        paintEvent  (QPaintEvent* e);
            
            // Getters
            int         id          () {return _id;}

   private:
            QWidget*    _parent;    //!< Parent widget of this
            QPainter    _painter;   //!< QPainter for drawing points
            QImage*     _image;     //!< raytraced image
            int         _id;        //!< unique number beginning with 0
   static   int         _currentID; //!< NO. of tracing for unique naming
};
//-----------------------------------------------------------------------------
#endif

