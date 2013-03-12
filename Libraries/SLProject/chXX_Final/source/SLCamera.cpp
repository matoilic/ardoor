//#############################################################################
//  File:      SLCamera.cpp
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

#include "SLSceneView.h"
#include "SLCamera.h"
#include "SLGroup.h"
#include "SLRay.h"
#include "SLAABBox.h"
#include "SLMesh.h"

//-----------------------------------------------------------------------------
// Static global default parameters for new cameras
SLCamAnim    SLCamera::currentAnimation    = turntable;
SLProjection SLCamera::currentProjection   = monoPerspective;
SLfloat      SLCamera::currentFOV          = 45.0f;
SLint        SLCamera::currentDevRotation  = 0;
//-----------------------------------------------------------------------------
SLCamera::SLCamera() : SLShape("Camera")
{  
   _fovInit     = 0;
   _clipNear    = 0.1f;
   _clipFar     = 300.0f;
   _fov         = currentFOV;
   _projection  = currentProjection;
   _camAnim   = currentAnimation;

   // depth of field parameters
   _lensDiameter = 0.3f;
   _lensSamples.samples(1,1); // e.g. 10,10 > 10x10=100 lenssamples
   _focalDist = 5;
   
   _eyeSep = _focalDist / 30.0f;
   _speedLimit = 2.0f;
}
//-----------------------------------------------------------------------------
//! Destructor: Be sure to delete the OpenGL display list.
SLCamera::~SLCamera() 
{  
}
//-----------------------------------------------------------------------------
/*! 
Shape initialization: Everything must be drawn with OpenGL into the shapes
display list.
*/
void SLCamera::shapeInit(SLSceneView* sv)
{  
   // Set transparency flag
   _aabb.hasAlpha(false);
   
   // For reset keep the first vm as initial vm
   if (_fovInit == 0)
   {  // Set view matrix as the inverse of the world transformation matrix
      _vmInit = _wm.inverse();
      _vm = _vmInit;
      _fovInit = _fov;
   } else
   {  _vm = _vmInit;
      _fov = _fovInit;
   }
   setWMandState();
   
   camAnim(_camAnim);
   _maxSpeed.set(0.0f, 0.0f, 0.0f);
   _curSpeed.set(0.0f, 0.0f, 0.0f);
}
//-----------------------------------------------------------------------------
//! shapeUpdate updates the smooth transition for the walk animation
SLbool SLCamera::shapeUpdate(SLSceneView* sv, SLfloat elapsedTimeSec)
{  
   // ToDo: The recursive update traversal is not yet implemented
   if (_maxSpeed != SLVec3f::ZERO || _curSpeed != SLVec3f::ZERO)
   {  
      SLVec3f pos = _vm.translation();
      
      // delta speed during acceleration/slow down
      SLfloat ds = _speedLimit / 10.0f; 
      
      // Accelerate
      if (_maxSpeed.x>0 && _curSpeed.x<_maxSpeed.x) _curSpeed.x += ds; else
      if (_maxSpeed.x<0 && _curSpeed.x>_maxSpeed.x) _curSpeed.x -= ds;
      if (_maxSpeed.y>0 && _curSpeed.y<_maxSpeed.y) _curSpeed.y += ds; else
      if (_maxSpeed.y<0 && _curSpeed.y>_maxSpeed.y) _curSpeed.y -= ds;      
      if (_maxSpeed.z>0 && _curSpeed.z<_maxSpeed.z) _curSpeed.z += ds; else
      if (_maxSpeed.z<0 && _curSpeed.z>_maxSpeed.z) _curSpeed.z -= ds;
      
      // Slow down
      if (_maxSpeed.z == 0)
      {  if (_curSpeed.z > 0) 
         {  _curSpeed.z -= ds;
            if (_curSpeed.z < 0) _curSpeed.z = 0.0f;
         } else
         if (_curSpeed.z < 0) 
         {  _curSpeed.z += ds;
            if (_curSpeed.z > 0) _curSpeed.z = 0.0f;
         }
      }
      if (_maxSpeed.x == 0)
      {  if (_curSpeed.x < 0) 
         {  _curSpeed.x += ds;
            if (_curSpeed.x > 0) _curSpeed.x = 0.0f;
         } else 
         if (_curSpeed.x > 0) 
         {  _curSpeed.x -= ds;
            if (_curSpeed.x < 0) _curSpeed.x = 0.0f;
         }
      }
      if (_maxSpeed.y == 0)
      {  if (_curSpeed.y < 0) 
         {  _curSpeed.y += ds;
            if (_curSpeed.y > 0) _curSpeed.y = 0.0f;
         } else 
         if (_curSpeed.y > 0) 
         {  _curSpeed.y -= ds;
            if (_curSpeed.y < 0) _curSpeed.y = 0.0f;
         }
      }
      
      //SL_LOG("cs: %3.1f, max: %3.1f, ds: %3.1f\n", _curSpeed.z, _maxSpeed.z, ds);
      
      SLVec3f delta(_curSpeed * elapsedTimeSec);
      
      _vm.translation(pos+delta); 
      setWMandState();
      return true;
   }
   return false;
}
//-----------------------------------------------------------------------------
//! SLCamera::shapeDraw draws the cameras frustum lines
/*!
Only draws the frustum lines without lighting when the camera is not the 
active one. This means that it can be seen from the active view point.
*/
void SLCamera::shapeDraw(SLSceneView* sv) 
{  
   if (sv->camera()!=this && _projection==monoPerspective)
   {  
      if (!_bufP.id())
      {
         SLVec3f P[12*2]; 
         SLuint  i=0;    
         SLfloat aspect = (float)sv->scrW() / (float)sv->scrH();
         SLfloat tanFov = tan(_fov*SL_DEG2RAD*0.5f);
         SLfloat tF = tanFov * _clipFar;  //top far
         SLfloat rF = tF * aspect;        //right far
         SLfloat tN = tanFov * _clipNear; //top near
         SLfloat rN = tN * aspect;        //right near
         
         // frustum pyramid lines
         P[i++].set(0,0,0); P[i++].set( rF, tF,-_clipFar);
         P[i++].set(0,0,0); P[i++].set(-rF, tF,-_clipFar);
         P[i++].set(0,0,0); P[i++].set(-rF,-tF,-_clipFar);
         P[i++].set(0,0,0); P[i++].set( rF,-tF,-_clipFar);
         
         // around far clipping plane
         P[i++].set( rF, tF,-_clipFar); P[i++].set( rF,-tF,-_clipFar);
         P[i++].set( rF,-tF,-_clipFar); P[i++].set(-rF,-tF,-_clipFar);
         P[i++].set(-rF,-tF,-_clipFar); P[i++].set(-rF, tF,-_clipFar);
         P[i++].set(-rF, tF,-_clipFar); P[i++].set( rF, tF,-_clipFar);
         
         // around near clipping plane
         P[i++].set( rN, tN,-_clipNear); P[i++].set( rN,-tN,-_clipNear);
         P[i++].set( rN,-tN,-_clipNear); P[i++].set(-rN,-tN,-_clipNear);
         P[i++].set(-rN,-tN,-_clipNear); P[i++].set(-rN, tN,-_clipNear);
         P[i++].set(-rN, tN,-_clipNear); P[i++].set( rN, tN,-_clipNear);
         
         _bufP.generate(P, i, 3);
      }
      
      _bufP.drawArrayAsConstantColorLines(SLCol3f::WHITE*0.7f);
   }
}
//-----------------------------------------------------------------------------
//! Updates the parent groups statistic parameters
void SLCamera::updateStats(SLGroup* parent)
{  assert(parent != 0);
   parent->numShapes++;
   parent->numTriangles += 12;
}
//-----------------------------------------------------------------------------
//! Returns a copy of this shape
SLShape* SLCamera::shapeCopy()
{  SLCamera* cam = new SLCamera();
   // ToDo: copy all private members
   return cam;
}
//-----------------------------------------------------------------------------
//! Returns the projection type as string
SLstring SLCamera::projectionToStr(SLProjection p)
{  switch (p)
   {  case monoPerspective:      return "Perspective";
      case monoOrthographic:     return "Orthographic";
      case stereoSideBySide:     return "Side by Side";
      case stereoSideBySideP:    return "Side by Side prop.";
      case stereoLineByLine:     return "Line by Line";
      case stereoColByCol:       return "Column by Column";
      case stereoCheckerBoard:   return "Checkerboard";
      case stereoColorRC:        return "Red-Cyan";
      case stereoColorRG:        return "Red-Green";
      case stereoColorRB:        return "Red-Blue";
      case stereoColorYB:        return "Yellow-Blue";
      default:                   return "Unknown";
   }
}
//-----------------------------------------------------------------------------
/*! 
Returns the height of the screen at focal distance. In stereo rendering this
shoud correspond to the height of the projection plane.
*/
SLfloat SLCamera::focalDistScrH()
{  
   return tan(_fov*SL_DEG2RAD/2.0f) * _focalDist * 2.0f;
}
//-----------------------------------------------------------------------------
/*! 
Returns the width of the screen at focal distance. In stereo rendering this
shoud correspond to the width of the projection plane.
*/
SLfloat SLCamera::focalDistScrW()
{  
   SLSceneView* sv = SLScene::current->activeSV();
   return focalDistScrH() * sv->scrWdivH();
}
//-----------------------------------------------------------------------------
/*!
Sets the projection transformation matrix, the viewport transformation and the
drawing buffer. In case of a stereographic projection it additionally sets the
stereo splitting parameters such as the color masks and the color filter matrix
for stereo color anaglyphs. 
*/
void SLCamera::setProjection(const SLEye eye)
{  
   SLSceneView* sv = SLScene::current->activeSV();
   stateGL->stereoEye  = eye;
   stateGL->projection = _projection;
   
   SLVec3f pos(_vm.translation());
   SLfloat top, bottom, left, right, d;   // frustum paramters
   SLfloat aspect = sv->scrWdivH();
   SLfloat w = (SLfloat)sv->scrW();
   SLfloat h = (SLfloat)sv->scrH();;
   
   switch (_projection) 
   {  case monoPerspective:
         stateGL->projectionMatrix.perspective(_fov, aspect, _clipNear, _clipFar);
         break; 
      case monoOrthographic:
         top    = tan(SL_DEG2RAD*_fov/2) * pos.length();
         bottom = -top;
         left   = -aspect*top;
         right  = -left; 
         stateGL->projectionMatrix.ortho(left,right,bottom,top,_clipNear,_clipFar);
         break;
         
      // stereo projection
      default: 
         // frustum shift d
         d = (SLfloat)eye * 0.5f * _eyeSep * _clipNear / _focalDist; 
         top    = tan(SL_DEG2RAD*_fov/2) * _clipNear;
         bottom = -top;
         left   = -aspect*top - d;
         right  =  aspect*top - d;
         stateGL->projectionMatrix.frustum(left,right,bottom,top,_clipNear,_clipFar);
   }
   
   // Set ViewPort
   SLint w2 = sv->scrWdiv2();
   SLint h2 = sv->scrHdiv2();
   SLint h4 = (SLint)((SLfloat)sv->scrHdiv2() * 0.5f);
   
   switch (_projection) 
   {  case stereoSideBySide: 
         if (eye==leftEye) 
              stateGL->viewport(0, 0, w2, (SLint)h);
         else stateGL->viewport(w2, 0, w2, (SLint)h);
         break;
      case stereoSideBySideP: 
         if (eye==leftEye) 
              stateGL->viewport( 0, h4, w2, h2);
         else stateGL->viewport(w2, h4, w2, h2);
         break;
      default: stateGL->viewport(0, 0, (SLint)(w), (SLint)(h));
   }
   
   
   // Clear Buffers
   if (eye==rightEye) 
      // Do not clear color on right eye because it contains the color of the
      // left eye. The right eye must be drawn after the left into the same buffer
      stateGL->clearDepthBuffer();
   else 
      stateGL->clearColorDepthBuffer();
   
   
   //  Set Color Mask and Filter
   if (_projection >= stereoColorRC)
   {  if (eye==leftEye)
      {  switch (_projection) 
         {  case stereoColorRC: stateGL->colorMask(1, 0, 0, 1); break;
            case stereoColorRB: stateGL->colorMask(1, 0, 0, 1); break;
            case stereoColorRG: stateGL->colorMask(1, 0, 0, 1); break;
            case stereoColorYB: stateGL->colorMask(1, 1, 0, 1); break;
            default: break;
         }
      } else
      {  switch (_projection) 
         {  case stereoColorRC: stateGL->colorMask(0, 1, 1, 1); break;
            case stereoColorRB: stateGL->colorMask(0, 0, 1, 1); break;
            case stereoColorRG: stateGL->colorMask(0, 1, 0, 1); break;
            case stereoColorYB: stateGL->colorMask(0, 0, 1, 1); break;
            default: break;
         }
      }
      
      // Set color filter matrix for red-cyan and yello-blue (ColorCode3D)
      switch (_projection) 
      {  case stereoColorRC:
            stateGL->stereoColorFilter.setMatrix(0.29f, 0.59f, 0.12f,
                                                 0.00f, 1.00f, 0.00f,
                                                 0.00f, 0.00f, 1.00f); break;
         case stereoColorYB:
            stateGL->stereoColorFilter.setMatrix(1.00f, 0.00f, 0.00f,
                                                 0.00f, 1.00f, 0.00f,
                                                 0.15f, 0.15f, 0.70f); break;
         default: break;
      }
   }
   GET_GL_ERROR;
}
//-----------------------------------------------------------------------------
/*!
Applies the view transform to the modelview matrix.
*/
void SLCamera::setView(const SLEye eye)
{  
   if (eye == centerEye)
   {  stateGL->modelViewMatrix.identity();
      stateGL->modelViewMatrix.multiply(_vm); 
   } 
   else // stereo viewing
   {
      // Get central camera vectors eye, lookAt, lookUp out of the view matrix vm
      SLVec3f EYE, LA, LU, LR;
      _vm.lookAt(&EYE, &LA, &LU, &LR);
   
      // Shorten LR to half of the eye dist (eye=-1 for left, eye=1 for right)
      LR *= _eyeSep * 0.5f * (SLfloat)eye; 
      
      // Set the OpenGL view matrix for the left eye
      SLMat4f vmEye;
      vmEye.lookAt(EYE+LR, EYE + _focalDist*LA+LR, LU);
      
      stateGL->modelViewMatrix.identity();
      stateGL->modelViewMatrix.multiply(vmEye);
   } 
}
//-----------------------------------------------------------------------------
//! SLCamera::animationStr()
SLstring SLCamera::animationStr()
{  
   switch (_camAnim)
   {  case turntable: return "Turntable";
      case walking1stP:  return "Walking 1st. Pers.";
      default: return "unknown";
   }
}
//-----------------------------------------------------------------------------
/* 
Event Handlers: Because the SLShape class also inherits the SLEventHandler
class a shape can also act as a event handler. The camera class uses this to
implement the camera animation.
*/
//-----------------------------------------------------------------------------
//! Gets called whenever a mouse button gets pressed.
SLbool SLCamera::onMouseDown(const SLMouseButton button, 
                             const SLint x, const SLint y, const SLKey mod)
{  
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   
   // Determine the lookAt point by ray cast
   eyeToPixelRay((SLfloat)sv->scrWdiv2(), 
                 (SLfloat)sv->scrHdiv2(), &_lookAtRay);
   s->root3D()->hit(&_lookAtRay);
   
   // Init both position in case that the second finger came with delay
   _oldTouchPos1.set((SLfloat)x, (SLfloat)y);
   _oldTouchPos2.set((SLfloat)x, (SLfloat)y);
   
   return false;
}  
//-----------------------------------------------------------------------------
//! Gets called whenever the mouse is moved.
SLbool SLCamera::onMouseMove(const SLMouseButton button, 
                             const SLint x, const SLint y, const SLKey mod)
{  
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   
   if (button == ButtonLeft) //================================================
   {         
      // Get camera vectors: eye pos., lookAt, lookUp, lookRight
      SLVec3f eye, LA, LU, LR;
      _vm.lookAt(&eye, &LA, &LU, &LR);
      
      // The lookAt and lookUp point in VS
      SLVec3f laP = eye + _focalDist * LA;
         
      // Determine rotation point as the center of the AABB of the hitShape
      SLVec3f rtP;
      if (_lookAtRay.length < SL_FLOAT_MAX && _lookAtRay.hitShape)
           rtP = _lookAtRay.hitShape->aabb()->centerWS();
      else rtP = laP;
      
      if (_camAnim==turntable) //........................................
      {  
         // Determine rot angles around x- & y-axis
         SLVec2f angle(_oldTouchPos1.y-y,_oldTouchPos1.x-x);
         angle *= _rotFactor;
         
         // Apply rotation around the lookAt point
         SLMat4f rot;
         rot.translate(rtP);
         rot.rotate(-angle.x, LR);
         rot.rotate(-angle.y, SLVec3f(0,1,0));
         rot.translate(-rtP);

         _vm *= rot;
      } 
      else if (_camAnim==walking1stP) //....................................
      {  SLVec2f angle(_oldTouchPos1.y-y,_oldTouchPos1.x-x);
         angle *= _rotFactor;
         angle *= 0.5f;
         
         // Apply rotation around the lookRight and the Y-axis
         SLMat4f rot;
         rot.rotate(-angle.x, LR);
         rot.rotate(-angle.y, SLVec3f(0,1,0));
         
         // rotate eye position
         LA.set(rot*LA);
         
         _vm.lookAt(eye, eye+LA*_focalDist, SLVec3f(0,1,0));
      }
            
      setWMandState();  
      _oldTouchPos1.set((SLfloat)x,(SLfloat)y);
      return true;
   } 
   else
   if (button == ButtonMiddle) //==============================================
   {  if (_camAnim==turntable)
      {  
         // Calculate the fraction delta of the mouse movement
         SLVec2f dMouse(x-_oldTouchPos1.x, _oldTouchPos1.y-y);
         dMouse.x /= (SLfloat)sv->scrW();
         dMouse.y /= (SLfloat)sv->scrH();
         
         // Scale the mouse delta by the lookAt distance
         SLfloat lookAtDist;
         if (_lookAtRay.length < SL_FLOAT_MAX)
              lookAtDist = _lookAtRay.length;
         else lookAtDist = _focalDist;

         // scale factor depending on the space sice at focal dist
         SLfloat spaceH = tan(SL_DEG2RAD*_fov/2) * lookAtDist * 2.0f;
         SLfloat spaceW = spaceH * sv->scrWdivH();

         dMouse.x *= spaceW;
         dMouse.y *= spaceH;
         
         if (mod==KeyCtrl)
         {  _vm.translation(_vm.m(12) + dMouse.x,
                            _vm.m(13),
                            _vm.m(14) + dMouse.y);
         } else
         {  _vm.translation(_vm.m(12) + dMouse.x,
                            _vm.m(13) + dMouse.y,
                            _vm.m(14));
         }
         setWMandState();
         _oldTouchPos1.set((SLfloat)x,(SLfloat)y); 
         return true;
      }
   } //========================================================================
   return false;
}
//-----------------------------------------------------------------------------
//! Gets called whenever the mouse button is released
SLbool SLCamera::onMouseUp(const SLMouseButton button, 
                           const SLint x, const SLint y, const SLKey mod)
{  // Stop any motion
   _maxSpeed.set(0.0f, 0.0f, 0.0f);
   
   //SL_LOG("onMouseUp\n");
   if (button == ButtonLeft) //================================================
   {  if (_camAnim==turntable) //..............................................
      {  return true;
      } 
      else if (_camAnim==walking1stP) //.......................................
      {  return true;
      }
   } else if (button == ButtonMiddle) //=======================================
   {
   } //========================================================================
   return false;
}
//-----------------------------------------------------------------------------
/*! 
SLCamera::onMouseWheel event handler moves camera forwards or backwards
*/
SLbool SLCamera::onMouseWheel(const SLint delta, const SLKey mod)
{  
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   SLfloat sign = (SLfloat)SL_sign(delta);
   
   if (_camAnim==turntable) //...........................................
   {  if (mod==KeyNone)
      {  
         // Determine the lookAt point by ray cast
         eyeToPixelRay((SLfloat)sv->scrWdiv2(), 
                       (SLfloat)sv->scrHdiv2(), &_lookAtRay);
         s->root3D()->hit(&_lookAtRay);
         if (_lookAtRay.length < SL_FLOAT_MAX) 
            _lookAtRay.hitPoint = _lookAtRay.origin + 
                                  _lookAtRay.dir*_lookAtRay.length;
         
         // Scale the mouse delta by the lookAt distance
         SLfloat lookAtDist;
         if (_lookAtRay.length < SL_FLOAT_MAX && _lookAtRay.hitShape)
         {  lookAtDist = _lookAtRay.length;
         }
         else lookAtDist = _focalDist;
                  
         _vm.translation(_vm.m(12),_vm.m(13),_vm.m(14) + sign*lookAtDist*_dPos);
         _lookAtRay.length = SL_FLOAT_MAX;
         setWMandState();
      }
      if (mod==KeyCtrl)
      {  _eyeSep *= (1.0f + sign*0.1f);
      }
      if (mod==KeyAlt)
      {  _fov += sign*5.0f;
         currentFOV = _fov;
      }
      if (mod==KeyShift)
      {  _focalDist *= (1.0f + sign*0.05f);
         
      }
      return true;
   }
   else if (_camAnim==walking1stP) //.......................................
   {  
      _speedLimit *= (1.0f + sign*0.1f);
   }
   return false;
}
//-----------------------------------------------------------------------------
/*! 
SLCamera::onDoubleTouch gets called whenever two fingers touch a handheld
screen.
*/
SLbool SLCamera::onTouch2Down(const SLint x1, const SLint y1,
                              const SLint x2, const SLint y2)
{	
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   
   // Determine the lookAt point by ray cast
   eyeToPixelRay((SLfloat)sv->scrWdiv2(), 
                 (SLfloat)sv->scrHdiv2(), &_lookAtRay);
   s->root3D()->hit(&_lookAtRay);
   
   _oldTouchPos1.set((SLfloat)x1, (SLfloat)y1);
   _oldTouchPos2.set((SLfloat)x2, (SLfloat)y2);
   return true;
}
//-----------------------------------------------------------------------------
/*! 
SLCamera::onTouch2Move gets called whenever two fingers move on a handheld 
screen.
*/
SLbool SLCamera::onTouch2Move(const SLint x1, const SLint y1,
                              const SLint x2, const SLint y2)
{	
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   SLVec2f now1((SLfloat)x1, (SLfloat)y1);
   SLVec2f now2((SLfloat)x2, (SLfloat)y2);
   SLVec2f delta1(now1-_oldTouchPos1);
   SLVec2f delta2(now2-_oldTouchPos2);
   
   // Average out the deltas over the last 4 events for correct 1 pixel moves
   static SLuint  cnt=0;
   static SLVec2f d1[4];
   static SLVec2f d2[4];
   d1[cnt%4] = delta1;
   d2[cnt%4] = delta2;
   SLVec2f avgDelta1(d1[0].x+d1[1].x+d1[2].x+d1[3].x, d1[0].y+d1[1].y+d1[2].y+d1[3].y);
   SLVec2f avgDelta2(d2[0].x+d2[1].x+d2[2].x+d2[3].x, d2[0].y+d2[1].y+d2[2].y+d2[3].y);
   avgDelta1 /= 4.0f;
   avgDelta2 /= 4.0f;
   cnt++;
      
   SLfloat r1, phi1, r2, phi2;
   avgDelta1.toPolar(r1, phi1);
   avgDelta2.toPolar(r2, phi2);
    
   // Scale the mouse delta by the lookAt distance
   SLfloat lookAtDist;
   if (_lookAtRay.length < SL_FLOAT_MAX)
        lookAtDist = _lookAtRay.length;
   else lookAtDist = _focalDist;
         
   // scale factor depending on the space sice at focal dist
   SLfloat spaceH = tan(SL_DEG2RAD*_fov/2) * lookAtDist * 2.0f;
   SLfloat spaceW = spaceH * sv->scrWdivH();
   
   //SL_LOG("avgDelta1: (%05.2f,%05.2f), dPhi=%05.2f\n", avgDelta1.x, avgDelta1.y, SL_abs(phi1-phi2));
   
   // if fingers move parallel slide camera vertically or horizontally
   if (SL_abs(phi1-phi2) < 0.2f)
   {  
      // Calculate center between finger points
      SLVec2f nowCenter((now1+now2)*0.5f);
      SLVec2f oldCenter((_oldTouchPos1+_oldTouchPos2)*0.5f);
      
      // For first move set oldCenter = nowCenter
      if (oldCenter == SLVec2f::ZERO) oldCenter = nowCenter;
      
      SLVec2f delta(nowCenter - oldCenter);

      // scale to 0-1
      delta.x /= (SLfloat)sv->scrW();
      delta.y /= (SLfloat)sv->scrH();

      // scale to space size
      delta.x *= spaceW;
      delta.y *= spaceH;
      
      if (_camAnim==turntable)
      {           
         // apply delta to x- and y-position
         _vm.translation(_vm.m(12) + delta.x,
                         _vm.m(13) - delta.y,
                         _vm.m(14));

         setWMandState();
      } 
      else if (_camAnim == walking1stP)
      {
      	_maxSpeed.x = delta.x * 100.0f,
         _maxSpeed.z = delta.y * 100.0f;
      }

   } else // Two finger pinch
   {  
      // Calculate vector between fingers
      SLVec2f nowDist(now2 - now1);
      SLVec2f oldDist(_oldTouchPos2-_oldTouchPos1);
      
      // For first move set oldDist = nowDist
      if (oldDist == SLVec2f::ZERO) oldDist = nowDist;
      
      SLfloat delta = oldDist.length() - nowDist.length();

      if (_camAnim==turntable)
      {  // scale to 0-1
         delta /= (SLfloat)sv->scrH();

         // scale to space height
         delta *= spaceH*2;
         
         // apply delta to the z-position
         _vm.translation(_vm.m(12),
                         _vm.m(13),
                         _vm.m(14) - delta);

         setWMandState();
      } 
      else if (_camAnim == walking1stP)
      {  
         // change field of view
         _fov += SL_sign(delta) * 0.5f;
         currentFOV = _fov;
      }
   }

   _oldTouchPos1.set((SLfloat)x1, (SLfloat)y1);
   _oldTouchPos2.set((SLfloat)x2, (SLfloat)y2);
   return true;
}
//-----------------------------------------------------------------------------
/*! 
SLCamera::onDoubleTouch gets called whenever two fingers touch a handheld
screen.
*/
SLbool SLCamera::onTouch2Up(const SLint x1, const SLint y1,
                            const SLint x2, const SLint y2)
{	
   _maxSpeed.set(0.0f, 0.0f, 0.0f);
   return true;
}
//-----------------------------------------------------------------------------
/*!
SLCamera::onKeyPress applies the keyboard view navigation to the view matrix. 
The key code constants are defined in SL.h
*/
SLbool SLCamera::onKeyPress(const SLKey key, const SLKey mod)  
{  
   switch ((SLchar)key)
   {  case 'W': _maxSpeed.z = _speedLimit; return true;
      case 'S': _maxSpeed.z =-_speedLimit; return true;
      case 'A': _maxSpeed.x = _speedLimit; return true;
      case 'D': _maxSpeed.x =-_speedLimit; return true;
      case 'Q': _maxSpeed.y = _speedLimit; return true;
      case 'E': _maxSpeed.y =-_speedLimit; return true;
      case (SLchar)KeyDown: return onMouseWheel( 1, mod);
      case (SLchar)KeyUp:   return onMouseWheel(-1, mod);
      default:  return false;
   }
}
//-----------------------------------------------------------------------------
/*!
SLCamera::onKeyRelease gets called when a key is released
*/
SLbool SLCamera::onKeyRelease(const SLKey key, const SLKey mod)
{  
   switch ((SLchar)key)
   {  case 'W':
      case 'S': _maxSpeed.z = 0.0f; return true;
      case 'A':
      case 'D': _maxSpeed.x = 0.0f; return true;
      case 'Q':
      case 'E': _maxSpeed.y = 0.0f; return true;
      default:  return false;
   }
}
//-----------------------------------------------------------------------------
//! SLCamera::setFrustumPlanes set the 6 plane from the view frustum.
/*! SLCamera::setFrustumPlanes set the 6 frustum planes by extracting the plane 
coefficients from the combined view and projection matrix.
See the paper from Gribb and Hartmann:
http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf
*/
void SLCamera::setFrustumPlanes()
{
   // build combined view projection matrix
	SLMat4f A(stateGL->projectionMatrix*_vm);
	
	// set the A,B,C & D coeffitient for each plane
	_plane[T].setCoefficients(-A.m( 1) + A.m( 3),-A.m( 5) + A.m( 7),
				                 -A.m( 9) + A.m(11),-A.m(13) + A.m(15));
	_plane[B].setCoefficients( A.m( 1) + A.m( 3), A.m( 5) + A.m( 7),
				                  A.m( 9) + A.m(11), A.m(13) + A.m(15));
	_plane[L].setCoefficients( A.m( 0) + A.m( 3), A.m( 4) + A.m( 7),
				                  A.m( 8) + A.m(11), A.m(12) + A.m(15));
	_plane[R].setCoefficients(-A.m( 0) + A.m( 3),-A.m( 4) + A.m( 7),
				                 -A.m( 8) + A.m(11),-A.m(12) + A.m(15));
	_plane[N].setCoefficients( A.m( 2) + A.m( 3), A.m( 6) + A.m( 7),
				                  A.m(10) + A.m(11), A.m(14) + A.m(15));
	_plane[F].setCoefficients(-A.m( 2) + A.m( 3),-A.m( 6) + A.m( 7),
				                 -A.m(10) + A.m(11),-A.m(14) + A.m(15));
   _numRendered = 0;
}
//-----------------------------------------------------------------------------
//! Whenever the vm changes the wm and the global state must be adapted
void SLCamera::setWMandState()
{  _wm = _vm.inverse();
   stateGL->viewMatrix = _vm;
}
//-----------------------------------------------------------------------------
//! eyeToPixelRay returns the a ray from the eye to the center of a pixel.
/*! This method is used for object picking. The calculation is the same as for
primary rays in Ray Tracing.
*/
void SLCamera::eyeToPixelRay(SLfloat x, SLfloat y, SLRay* ray)
{  
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   SLfloat  hw, hh, pixel; 
   SLVec3f  dir, EYE, LA, LU, LR, C, TL;
      
   // calculate half window width & height in world coords     
   hh = tan(SL_DEG2RAD*_fov/2); 
   hw = hh * sv->scrWdivH();

   // calculate the size of a pixel in world coords. 
   // height & width must be equal because perspective is undistorted.
   pixel = hw * 2 / sv->scrW();

   // get camera vectors
   _vm.lookAt(&EYE, &LA, &LU, &LR);

   // calculate a vector to the center (C) of the top left (TL) pixel
   C = LA;
   TL = C - hw*LR + hh*LU  +  pixel/2*LR - pixel/2*LU;

   // Calculate direction of ray
   dir = TL + pixel*x*LR - pixel*y*LU;
   
   // Fill in ray parameters
   dir.normalize();
   ray->origin.set(EYE);
   ray->setDir(dir);
   ray->length = SL_FLOAT_MAX;
   ray->depth = 1;
   ray->contrib = 1.0f; 
   ray->type = PRIMARY;
   ray->x = x;
   ray->y = y;  
   ray->hitTriangle = 0;
   ray->hitMat = 0;
   ray->hitNormal.set(SLVec3f::ZERO);
   ray->hitPoint.set(SLVec3f::ZERO); 
   ray->originMat = &SLMaterial::AIR;
   ray->originTria = 0;
}
//-----------------------------------------------------------------------------
//! SLCamera::isInFrustum does a simple and fast frustum culling test for AABBs
/*! SLCamera::isInFrustum checks if the bounding sphere of an AABB is within 
the view frustum defined by its 6 planes by simply testing the distance of the
AABBs center minus its radius. This is faster than the AABB in frustum test but
not as precise. Please refer to the nice tutorial on frustum culling on:
http://www.lighthouse3d.com/opengl/viewfrustum/
*/
SLbool SLCamera::isInFrustum(SLAABBox* aabb) 
{	
   // check the 6 planes of the frustum
	for(SLint i=0; i < 6; ++i) 
	{	SLfloat distance = _plane[i].distToPoint(aabb->centerWS());
		if (distance < -aabb->radiusWS()) 
		{  aabb->isVisible(false);
		   return false;
		}
	}
	aabb->isVisible(true);
	_numRendered++;
	
	// Calculate squared dist. from AABB's center to viewer for blend sorting.
   // SLSceneView::drawBlendedShapes for more infos.
	SLVec3f viewToCenter(_wm.translation()-aabb->centerWS());
	aabb->sqrViewDist(viewToCenter.lengthSqr());    	   
	return true;
}
//-----------------------------------------------------------------------------

