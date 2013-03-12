//#############################################################################
//  File:      SLText.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLTEXT_H
#define SLTEXT_H

#include <stdafx.h>
#include "SLShape.h"
#include "SLAABBox.h"
#include "SLGLBuffer.h"
#include <SLTexFont.h>

class SLSceneView;
class SLGroup;
class SLRay;

//-----------------------------------------------------------------------------
//! SLText creates a mesh using a textured font from SLTexFont    
class SLText: public SLShape 
{  public:                     
                              SLText(SLstring text,
                                     SLTexFont* font = SLTexFont::font09,
                                     SLCol4f txtCol = SLCol4f::WHITE,
                                     SLfloat maxWidth = 0.0f,
                                     SLfloat lineHeightFactor = 1.3f);
                                           
                             ~SLText(){;}
               
               void           shapeInit   (SLSceneView* sv);
               void           shapeDraw   (SLSceneView* sv);
               SLShape*       shapeCopy   ();
               void           updateStats (SLGroup* parent);
               SLAABBox&      buildAABB   ();
               SLbool         shapeHit    (SLRay* ray){return false;}
               
               void           preShade    (SLRay* ray){;}
               
               // Getters
               SLstring       text        (){return _text;}
               SLCol4f        color       (){return _color;}
               SLVec2f        size        (){return _font->calcTextSize(_text,
                                                                        _maxW,
                                                                        _lineH);}
               SLint          length      (){return _text.length();}
               
   protected:    
               SLstring    _text;      //!< Text of the button
               SLTexFont*  _font;      //!< Font pointer of the preloaded font
               SLCol4f     _color;     //!< RGBA-Color of the text
               SLfloat     _maxW;      //!< Max. width in pix. for wrapped text
               SLfloat     _lineH;     //!< Line height factor for wrapped text

               SLGLBuffer  _bufP;      //!< Buffer for text vertex positions
               SLGLBuffer  _bufT;      //!< Buffer for text vertex texCoords
               SLGLBuffer  _bufI;      //!< Buffer for text vertex indexes
};
//-----------------------------------------------------------------------------
#endif //SLSPHERE_H

