//#############################################################################
//  File:      SLGLState.cpp
//  Purpose:   Singleton class implementation for global OpenGL replacement
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

#include "SLGLState.h"

//-----------------------------------------------------------------------------
SLGLState* SLGLState::instance = NULL;
//-----------------------------------------------------------------------------
/*! Public static creator and getter function. Garantees the the static 
instance is created only once. The constructor is therefore private.
*/
SLGLState* SLGLState::getInstance()     
{  if(!instance)
   {  instance = new SLGLState();
      return instance;
   } else return instance;
}
//-----------------------------------------------------------------------------
/*! Public static destruction.
*/
void SLGLState::deleteInstance()     
{  delete instance;
   instance = 0;
}
//-----------------------------------------------------------------------------
/*! Private constructor should be called only once for a sigleton class.
*/
SLGLState::SLGLState()
{ 
   initAll();
}
//-----------------------------------------------------------------------------
/*! Intiallizes all states.
*/
void SLGLState::initAll()
{  
   viewMatrix.identity();
   modelViewMatrix.identity();
   projectionMatrix.identity();
   
   numLightsUsed = 0;
   
   for (SLint i=0; i<SL_MAX_LIGHTS; ++i)
   {  lightIsOn[i] = 0;
      lightPosWS[i] = SLVec4f(0,0,1,1);
      lightPosVS[i] = SLVec4f(0,0,1,1);
      lightAmbient[i] = SLCol4f::BLACK;
      lightDiffuse[i] = SLCol4f::BLACK;
      lightSpecular[i] = SLCol4f::BLACK;
      lightDirWS[i] = SLVec3f(0,0,-1);
      lightDirVS[i] = SLVec3f(0,0,-1);
      lightSpotCutoff[i] = 180.0f;
      lightSpotCosCut[i] = cos(SL_DEG2RAD*lightSpotCutoff[i]);
      lightSpotExp[i] = 1.0f;
      lightAtt[i].set(1.0f, 0.0f, 0.0f);
      lightDoAtt[i] = 0;
   }
   
   matAmbient     = SLCol4f::WHITE;
   matDiffuse     = SLCol4f::WHITE;      
   matSpecular    = SLCol4f::WHITE;     
   matEmissive    = SLCol4f::BLACK;     
   matShininess   = 100;
   
   fogIsOn = false;                 
   fogMode = GL_LINEAR;
   fogDensity = 0.2f;
   fogDistStart = 1.0f;
   fogDistEnd = 6.0f;
   fogColor = SLCol4f::BLACK;
   
   globalAmbientLight.set(0.2f,0.2f,0.2f,0.0f);
   
   _glVersion     = SLstring((char*)glGetString(GL_VERSION));
   _glVendor      = SLstring((char*)glGetString(GL_VENDOR));
   _glRenderer    = SLstring((char*)glGetString(GL_RENDERER));
   _glGLSLVersion = SLstring((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
   _glExtensions  = SLstring((char*)glGetString(GL_EXTENSIONS));
   
   //initialize states a unset
   _blend = false;
}
//-----------------------------------------------------------------------------
/*! The destructor only empties the stacks
*/
SLGLState::~SLGLState()
{  
   _modelViewMatrixStack.clear();
   _projectionMatrixStack.clear();
}
//-----------------------------------------------------------------------------
/*! One time initialization
*/
void SLGLState::onInitialize(SLCol4f clearColor)
{  
   // Reset all internal states
   initAll();

   // enable depth_test
   glDepthFunc(GL_LESS);

   // set blend function for classis transparency
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
     
   // set background color
   glClearColor(clearColor.r,
                clearColor.g,
                clearColor.b,
                clearColor.a);
}
//-----------------------------------------------------------------------------
/*! Builds the normal matrix by the inverse transposed modelview matrix. Only
the linear 3x3 submatrix of the modelview matrix with the rotation is inversed.
In 
*/
void SLGLState::buildNormalMatrix()
{  _normalMatrix.setMatrix(modelViewMatrix.mat3());
   _normalMatrix.invert();
   _normalMatrix.transpose();
}
//-----------------------------------------------------------------------------
/*! Builds the 4x4 inverse matrix and the 3x3 normal matrix from the modelview 
matrix. If only the normal matrix is needed use the method buildNormalMatrix
because inverses only the 3x3 submatrix of the modelview matrix.
*/
void SLGLState::buildInverseAndNormalMatrix()
{  
   _invModelViewMatrix.setMatrix(modelViewMatrix);
   _invModelViewMatrix.invert();
   _normalMatrix.setMatrix(_invModelViewMatrix.mat3());
   _normalMatrix.transpose();
}
//-----------------------------------------------------------------------------
/*! Returns the normal matrix as the transposed of the inverse modelview matrix
*/
const SLMat3f* SLGLState::normalMatrix()
{  
   return &_normalMatrix;
}
//-----------------------------------------------------------------------------
/*! Returns the inverse modelview matrix
*/
const SLMat4f* SLGLState::invModelViewMatrix()
{  
   return &_invModelViewMatrix;
}
//-----------------------------------------------------------------------------
/*! Returns the combined modelview projection matrix
*/
const SLMat4f* SLGLState::mvpMatrix()
{  _mvpMatrix.setMatrix(projectionMatrix);
   _mvpMatrix.multiply(modelViewMatrix);
   return &_mvpMatrix;
}
//-----------------------------------------------------------------------------
/*! Pushes the currente modelview matrix on its stack.
*/
void SLGLState::pushModelViewMatrix()
{  
   _modelViewMatrixStack.push_back(modelViewMatrix);
}
//-----------------------------------------------------------------------------
/*! Pops back the top matrix from the modelview matrix stack & sets it current
*/
void SLGLState::popModelViewMatrix()
{  
   modelViewMatrix = _modelViewMatrixStack.back();
   _modelViewMatrixStack.pop_back();
}
//-----------------------------------------------------------------------------  
/*! Pushes the currente projection matrix on its stack.
*/    
void SLGLState::pushProjectionMatrix()
{  _projectionMatrixStack.push_back(projectionMatrix);
}
//-----------------------------------------------------------------------------
/*! Pops back the top matrix from the projection matrix stack & sets it current
*/
void SLGLState::popProjectionMatrix()   
{  
   projectionMatrix = _projectionMatrixStack.back();
   _projectionMatrixStack.pop_back();
}
//-----------------------------------------------------------------------------
/*! Transforms the light position into the view space
*/
void SLGLState::calcLightPosVS(SLint nLights)
{  assert(nLights>=0 && nLights<=SL_MAX_LIGHTS);
   for (SLint i=0; i<nLights; ++i)
   {  lightPosVS[i].set(viewMatrix * lightPosWS[i]);
   }
}
//-----------------------------------------------------------------------------
/*! Transforms the lights spot direction into the view space
*/
void SLGLState::calcLightDirVS(SLint nLights)
{  assert(nLights>=0 && nLights<=SL_MAX_LIGHTS);
   SLMat4f vRot(viewMatrix);
   vRot.translation(0,0,0); // delete translation part, only rotation needed
   
   for (SLint i=0; i<nLights; ++i)
   {  lightDirVS[i].set(vRot * lightDirWS[i]);
   }
}
//-----------------------------------------------------------------------------
/*! Returns the global ambient color as the componentwise product of the global
ambient light instensity and the materials ambient reflection. This is used to
give the scene a minimal ambient lighting.
*/
const SLCol4f* SLGLState::globalAmbient()
{  _globalAmbient.set(globalAmbientLight & matAmbient);
   return &_globalAmbient;
}
//-----------------------------------------------------------------------------
/*! SLGLState::depthTest enables or disables depth testing but only if the 
state really changes. The depth test decides for each pixel in the depthbuffer 
which polygon is the closest to the eye.
*/
void SLGLState::depthTest(SLbool stateNew)
{  static SLbool oldDepthTestState = false;
   if (oldDepthTestState != stateNew)
   {  if (stateNew) glEnable(GL_DEPTH_TEST);
      else glDisable(GL_DEPTH_TEST);
      oldDepthTestState = stateNew;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::depthTest enables or disables depth buffer writing but only if 
the state really changes. Turning on depth masking freezes the depth test but 
keeps all values in the depth buffer.
*/
void SLGLState::depthMask(SLbool stateNew)
{  static SLbool oldDepthMaskState = true;
   if (oldDepthMaskState != stateNew)
   {  glDepthMask(stateNew ? GL_TRUE : GL_FALSE);
      oldDepthMaskState = stateNew;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::cullFace sets the GL_CULL_FACE state but only if the state 
really changes. If face culling is turned on no backfaces are processed. 
*/
void SLGLState::cullFace(SLbool stateNew)
{  static SLbool oldCullFaceState = true;
   if (oldCullFaceState != stateNew)
   {  if (stateNew) glEnable(GL_CULL_FACE);
      else glDisable(GL_CULL_FACE);
      oldCullFaceState = stateNew;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::blend enables or disables alpha blending but only if the state 
really changes.
*/
void SLGLState::blend(SLbool stateNew)
{  if (_blend != stateNew)
   {  if (stateNew) glEnable(GL_BLEND);
      else glDisable(GL_BLEND);
      _blend = stateNew;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::multiSample enables or disables multisampling but only if the 
state really changes. Multisampling turns on fullscreen anti aliasing on the GPU
witch produces smooth polygon edges, lines and points.
*/
void SLGLState::multiSample(SLbool stateNew)
{  
   #ifndef SL_GLES2
   static SLbool oldMultisampleState = false;
   if (oldMultisampleState != stateNew)
   {  if (stateNew) glEnable(GL_MULTISAMPLE);
      else glDisable(GL_MULTISAMPLE);
      oldMultisampleState = stateNew;
   }
   #endif
}
//-----------------------------------------------------------------------------
/*! SLGLState::polygonMode sets the polygonMode to GL_LINE but only if the 
state really changes. OpenGL ES doesn't support glPolygonMode. It has to be 
mimicked with GL_LINE_LOOP drawing.
*/
void SLGLState::polygonLine(SLbool stateNew)
{  
   #ifndef SL_GLES2
   static SLbool oldPolyLineState = false;
   if (oldPolyLineState != stateNew)
   {  if (stateNew) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      oldPolyLineState = stateNew;
   }
   #endif
}
//-----------------------------------------------------------------------------
/*! SLGLState::polygonOffset turns on/off polygon offset (for filled polygons)
and sets the factor and unit for glPolygonOffset but only if the state really 
changes. Polygon offset is used to reduce z-fighting due to parallel planes or
lines.
*/
void SLGLState::polygonOffset(SLbool stateNew, SLfloat factor, SLfloat units)
{  static SLbool oldPolygonOffsetEnabled = false;
   static SLfloat oldFactor = -1.0f;
   static SLfloat oldUnits = -1.0f;
   
   if (oldPolygonOffsetEnabled != stateNew)
   {  if (stateNew) 
      {  glEnable(GL_POLYGON_OFFSET_FILL);   
         if (oldFactor != factor || oldUnits != units)
         {  glPolygonOffset(factor, units);
            oldFactor = factor;
            oldUnits = units;
         }
      }
      else glDisable(GL_POLYGON_OFFSET_FILL);
      oldPolygonOffsetEnabled = stateNew;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::viewport sets the OpenGL viewport position and size 
*/
void SLGLState::viewport(SLint x, SLint y, SLsizei width, SLsizei height)
{  static SLint oldX=-1, oldY=-1, oldW=-1, oldH=-1;
   if (x != oldX || y != oldY || width != oldW || height != oldH)
   {  glViewport(x, y, width, height);
      oldX = x;
      oldY = y;
      oldW = width;
      oldH = height;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::colorMask sets the OpenGL colorMask for framebuffer masking 
*/
void SLGLState::colorMask(SLint r, SLint g, SLint b, SLint a)
{  static SLint oldR=-1, oldG=-1, oldB=-1, oldA=-1;
   if (r != oldR || g != oldG || b != oldB || a != oldA)
   {  glColorMask(r, g, b, a);
      oldR = r;
      oldG = g;
      oldB = b;
      oldA = a;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::useProgram sets the current active shader program
*/
void SLGLState::useProgram(SLuint progID)
{  static SLuint oldProgramID = 0;
   if (oldProgramID != progID)
   {  glUseProgram(progID);
      oldProgramID = progID;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::bindTexture sets the current active texture name
*/
void SLGLState::bindTexture(GLenum target, SLuint textureID)
{  static SLenum oldTextureTarget = -1;
   static SLuint oldTextureID = 0;
   if (target != oldTextureTarget || textureID != oldTextureID)
   {  glBindTexture(target, textureID);
      oldTextureTarget = target;
      oldTextureID = textureID;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::activeTexture sets the current active texture unit
*/
void SLGLState::activeTexture(SLenum textureUnit)
{  static SLenum oldTextureUnit = -1;
   if (textureUnit != oldTextureUnit) 
   {  glActiveTexture(textureUnit);
      oldTextureUnit = textureUnit;
   }
}
//-----------------------------------------------------------------------------
/*! SLGLState::unbindAnythingAndFlush unbinds all shaderprograms and buffers in 
use and calls glFlush. This shoud be the last call to GL before buffer 
swapping.
*/
void SLGLState::unbindAnythingAndFlush()
{  useProgram(0);
   //glBindBuffer(GL_ARRAY_BUFFER, 0);
   //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   //glFlush();
}
//-----------------------------------------------------------------------------
