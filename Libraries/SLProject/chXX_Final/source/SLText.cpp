//#############################################################################
//  File:      SLText.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include <SLText.h>
#include <SLScene.h>
#include <SLGroup.h>
#include <SLGLShaderProg.h>
#include <SLGLState.h>

//-----------------------------------------------------------------------------
/*! 
The ctor sets all members and translates to the min. position.
*/
SLText::SLText(SLstring text, SLTexFont* font, SLCol4f color, 
               SLfloat maxWidth, SLfloat lineHeightFactor) 
               : SLShape("Text_"+text.substr(0,10))
{  assert(font);
   _font  = font;
   _text  = text;
   _color = color;
   _maxW  = maxWidth;
   _lineH = lineHeightFactor;
}
//-----------------------------------------------------------------------------
//! SLText::shapeInit sets the transparency flag of the AABB
void SLText::shapeInit(SLSceneView* sv)
{  
   (void)sv; // avoid unused parameter warning
   _aabb.hasAlpha(true);
}
//-----------------------------------------------------------------------------
/*! 
SLText::shapeCopy returns a deep copy of the SLText object. 
*/
SLShape* SLText::shapeCopy()
{  SLText* copy = new SLText(_text, _font, _color);
   return copy;
}
//-----------------------------------------------------------------------------
/*! 
SLText::shapeDraw draws the text buffer objects
*/
void SLText::shapeDraw(SLSceneView* sv)
{ 
   if (_drawBits.get(SL_DB_HIDDEN) || !stateGL->blend()) return;
   
   // create buffer object for text once
   if (!_bufP.id() && !_bufT.id() && !_bufI.id())
   {  _font->buildTextBuffers(&_bufP, &_bufT, &_bufI, _text, 
                              _maxW, _lineH, _color);
   }
   
   // Enable & build font texture with active OpenGL context
   _font->bindActive();

   // Setup shader
   SLGLShaderProg* sp = SLScene::current->shaderProgs(FontTex);
   SLGLState* state = SLGLState::getInstance();
   sp->useProgram();
   sp->uniformMatrix4fv("u_mvpMatrix", 1,
                        (SLfloat*)state->mvpMatrix());
   sp->uniform4fv("u_textColor", 1, (float*)&_color);
   sp->uniform1i("u_texture0", 0);
   
   // bind buffers and draw 
   _bufP.bindAndEnableAttrib(sp->getAttribLocation("a_position"));
   _bufT.bindAndEnableAttrib(sp->getAttribLocation("a_texCoord"));
   
   _bufI.bindAndDrawElementsAs(SL_TRIANGLES, _text.length()*2*3);
   
   _bufP.disableAttribArray();
   _bufT.disableAttribArray();

   // For debug purpose
   //sp = SLScene::current->shaderProgs(ColorUniform);
   //sp->useProgram();
   //sp->uniformMatrix4fv("u_mvpMatrix",1,(SLfloat*)state->mvpMatrix());
   //sp->uniform4fv("u_color", 1, (float*)&SLCol4f::GREEN);
   //_bufP.bindAndEnableAttrib(sp->getAttribLocation("a_position"));
   //_bufT.bindAndEnableAttrib(sp->getAttribLocation("a_texCoord"));
   //_bufI.bindAndDrawElementsAs(SL_LINES, _text.length()*2*3);
   //_bufP.disableAttribArray();
   //_bufT.disableAttribArray();
}
//-----------------------------------------------------------------------------
/*! 
SLButton::updateStats updates the parent groups statistics.
*/
void SLText::updateStats(SLGroup* parent)
{  assert(parent != 0);
   parent->numBytes += sizeof(SLText); 
   parent->numBytes += _text.length();  
   parent->numShapes++;
   parent->numTriangles += _text.length()*2 + 2;
}
//-----------------------------------------------------------------------------
/*! 
SLText::buildAABB builds and returns the axis-aligned bounding box.
*/
SLAABBox& SLText::buildAABB()
{  
   SLVec2f size = _font->calcTextSize(_text);
   
   // calculate min & max in object space
   SLVec3f minOS(0, 0, -0.01f);
   SLVec3f maxOS(size.x, size.y, 0.01f);
   
   // apply world matrix: this overwrites the AABB of the group
   _aabb.fromOStoWS(minOS, maxOS, _wm);
   
   return _aabb;
}
//-----------------------------------------------------------------------------
