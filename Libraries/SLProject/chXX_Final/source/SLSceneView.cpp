//#############################################################################
//  File:      SLSceneView.cpp
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

#include "SLLight.h"
#include "SLGroup.h"
#include "SLCamera.h"
#include "SLAABBox.h"
#include "SLGLShaderProg.h"
#include "SLRefShape.h"
#include "SLLightSphere.h"
#include "SLLightRect.h"
#include "SLRay.h"
#include "SLTexFont.h"
#include "SLButton.h"

//-----------------------------------------------------------------------------
//! SLSceneView ctor
SLSceneView::SLSceneView(SLstring name,
                         SLint screenWidth,
                         SLint screenHeight,
                         SLint dotsPerInch) : SLObject(name)
{  _scrW = screenWidth;
   _scrH = screenHeight;
   _scrDPI = dotsPerInch;
   init();
}
//-----------------------------------------------------------------------------
SLSceneView::~SLSceneView()
{  SL_LOG("~SLSceneView\n");
}
//-----------------------------------------------------------------------------
/*!
SLSceneView::init initializes default values for an empty scene
*/
void SLSceneView::init()
{  
   _stateGL = 0;
   
   // enables and modes
   _mouseDownL = false;
   _mouseDownR = false;
   _mouseDownM = false;
   _touchDowns = 0;

   _doDepthTest = true;
   _doMultiSample = true;     // true=OpenGL multisampling is turned on
   _doFrustumCulling = true;  // true=enables view frustum culling

   _drawBits.allOff();
       
   _showStats = false;
   _showInfo = true;
   
   _camera = 0;               // 0=no active camera

   _fps = 0.0f,               // Frames per second
   _frameTime = 0.0f;
   _waitEvents = true;
   _scrWdiv2 = _scrW>>1;
   _scrHdiv2 = _scrH>>1;
   _scrWdivH = (SLfloat)_scrW / (SLfloat)_scrH;
      
   _doRT = false;
 }
//-----------------------------------------------------------------------------
/*!
SLSceneView::onInitialize is called by the window system before the first 
rendering. It applies all scene rendering attributes with the according 
OpenGL function.
*/
void SLSceneView::onInitialize()
{  
   SLScene* s = SLScene::current;
   _stateGL = SLGLState::getInstance();
   _stateGL->onInitialize(s->backColor());

   // init 3D scene with initial depth 1
   if (s->root3D())
   {  
      // traverse scene for node initialization
      clock_t  t = clock();
      _stateGL->modelViewMatrix.identity();
      s->root3D()->init(this, 1);
      SL_LOG("Time for init     : %5.2f sec.\n", 
             (SLfloat)(clock()-t)/(SLfloat)CLOCKS_PER_SEC);
      
      // build axis aligned bounding box hierarchy after init
      t = clock();
      _aabb = s->_root3D->buildAABB(); 
      SL_LOG("Time for grid/tree: %5.2f sec.\n", 
             (SLfloat)(clock()-t)/(SLfloat)CLOCKS_PER_SEC);
      
      s->root3D()->updateStats(0);
   } else SL_LOG("\nNo scene available.\n\n");
   
   build2DMenus();
}
//-----------------------------------------------------------------------------
/*!
SLSceneView::onResize is called by the window system before the first 
rendering and whenever the window changes its size.
*/
void SLSceneView::onResize(const SLint width, const SLint height)
{  
   SLScene* s = SLScene::current;
   
   if (_scrW!=width || _scrH != height)
   {
      _scrW = width;
      _scrH = height;
      _scrWdiv2 = _scrW>>1;  // width / 2
      _scrHdiv2 = _scrH>>1;  // height / 2
      _scrWdivH = (SLfloat)_scrW/(SLfloat)_scrH;
      
      if (_doRT && (_scrW!=width || _scrH!=height))
      {  // stop ray tracing on resize
         _doRT = false;
         _raytracer.continuous(false);
         s->menu2D(s->_menuGL);
         s->menu2D()->hideAndReleaseRec();
         s->menu2D()->drawBits()->off(SL_DB_HIDDEN);
      }
   }
}
//-----------------------------------------------------------------------------
/*!
SLSceneView::onPaint is called by window system whenever the window therefore 
the scene needs to be painted.
*/
SLbool SLSceneView::onPaint()
{  
   SLScene* s = SLScene::current;

   // calculate elapsed time
   static SLfloat lastTimeSec = 0;
   SLfloat timeNow = s->timeSec();
   SLfloat elapsedTimeSec = timeNow - lastTimeSec;

   SLbool updated3D = false;
   
   // Don't render anything without camera and scene
   if (!_camera || !s->_root3D) return false;

   // Render the 3D scenegraph by by raytracing or OpenGL
   if (_doRT)
        updated3D = updateAndRT3D(elapsedTimeSec);
   else updated3D = updateAndDraw3D(elapsedTimeSec);
   
   // Render the 2D GUI (menu etc.)
   SLbool updated2D = updateAndDraw2D(elapsedTimeSec);
   
   _stateGL->unbindAnythingAndFlush();

   // Calculate the frames per second metric
   calcFPS(elapsedTimeSec);
   lastTimeSec = timeNow;

   // Return true if a repaint is needed
   return !_waitEvents || updated3D || updated2D;
}
//-----------------------------------------------------------------------------
// Window event handlers
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onMouseDown gets called whenever a mouse button gets pressed and
dispatches the event to the currently attached eventhandler object.
*/
SLbool SLSceneView::onMouseDown(const SLMouseButton button, 
                                SLint x, SLint y, const SLKey mod)
{  	   
   SLScene* s = SLScene::current;
   
   // Check first if mouse down was on a button    
   if (s->menu2D() && s->menu2D()->onMouseDown(button, x, y, mod))
      return true;

   // if menu is open close it
   if (SLButton::buttonParent && s->menu2D())
      s->menu2D()->closeAll();
   
   _mouseDownL = (button == ButtonLeft);
   _mouseDownR = (button == ButtonRight);
   _mouseDownM = (button == ButtonMiddle);
   _mouseMod = mod;
   
   SLbool result = false;
   for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
   {  if (s->_eventHandlers[i]->onMouseDown(button, x, y, mod))
         result = true;
   }  
   return result;
}  
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onMouseUp gets called whenever a mouse button gets released.
*/
SLbool SLSceneView::onMouseUp(const SLMouseButton button, 
                              SLint x, SLint y, const SLKey mod)
{  
   SLScene* s = SLScene::current;
   _touchDowns = 0;
   
   if (_raytracer.state()==rtMoveGL)
   {  _doRT = true;
      _raytracer.state(rtReady);
   }   
   
   // Check first if mouse up was on a button    
   if (s->menu2D() && s->menu2D()->onMouseUp(button, x, y, mod))
      return true;
           
   if (button == ButtonLeft)
   {  _mouseDownL = false;
      if (_camera && s->_root3D)
      {  SLbool result = false;
         for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
         {  if (s->_eventHandlers[i]->onMouseUp(button, x, y, mod))
               result = true;
         }  
         return result;
      }
   } else
   if (button == ButtonRight)
   {  _mouseDownR = false;
      
   } else
   if (button == ButtonMiddle)
   {  _mouseDownM = false;
   }
   return false;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onMouseMove gets called whenever the mouse is moved.
*/
SLbool SLSceneView::onMouseMove(SLint x, SLint y)
{     
   SLScene* s = SLScene::current;
   _touchDowns = 0;
   SLbool result = false;
      
   if (_mouseDownL || _mouseDownR || _mouseDownM)
   {  SLMouseButton btn = _mouseDownL ? ButtonLeft : 
                          _mouseDownR ? ButtonRight : ButtonMiddle;
      
      // Handle move in RT mode
      if (_doRT && !_raytracer.continuous())
      {  if (_raytracer.state()==rtFinished)
            _raytracer.state(rtMoveGL);
         else
         {  _raytracer.continuous(false);
            s->menu2D(s->_menuGL);
            s->menu2D()->hideAndReleaseRec();
            s->menu2D()->drawBits()->off(SL_DB_HIDDEN);
         }
         _doRT = false;
      }

      for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
      {  if (s->_eventHandlers[i]->onMouseMove(btn, x, y, _mouseMod))
            result = true;
      }
   }  
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onMouseWheel gets called whenever the mouse wheel is turned.
The parameter wheelPos is an increesing or decreesing counter number.
*/
SLbool SLSceneView::onMouseWheelPos(const SLint wheelPos, const SLKey mod)
{  
   static SLint lastMouseWheelPos = 0;
   SLint delta = wheelPos-lastMouseWheelPos;
   lastMouseWheelPos = wheelPos;
   return onMouseWheel(delta, mod);
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onMouseWheel gets called whenever the mouse wheel is turned.
The paramter delta is positive/negative depending on the wheel direction
*/
SLbool SLSceneView::onMouseWheel(const SLint delta, const SLKey mod)
{  
   // Handle mousewheel in RT mode
   if (_doRT && !_raytracer.continuous() && _raytracer.state()==rtFinished)
      _raytracer.state(rtReady);

   SLScene* s = SLScene::current;
   SLbool result = false;
   for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
   {  if (s->_eventHandlers[i]->onMouseWheel(delta, mod))
         result = true;
   }
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onDoubleClick gets called when a mouse double click or finger 
double tab occures.
*/
SLbool SLSceneView::onDoubleClick(const SLMouseButton button, 
                                  SLint x, SLint y, const SLKey mod)
{  
   SLScene* s = SLScene::current;
   
   // Check first if mouse down was on a button    
   if (s->menu2D() && s->menu2D()->onDoubleClick(button, x, y, mod))
      return true;

   SLbool result = false;
   
   // Do object picking with ray cast
   if (button == ButtonLeft)
   {  _mouseDownR = false;
      
      SLRay pickRay;  
      _camera->eyeToPixelRay((SLfloat)x, (SLfloat)y, &pickRay);
      s->_root3D->hit(&pickRay);
      
      if (pickRay.length < SL_FLOAT_MAX)
      {  
         if (s->_selectedShape)
         {  s->_selectedShape->drawBits()->off(SL_DB_SELECTED);
         }
         if (s->_selectedShape == pickRay.hitShape)
         {  s->_selectedShape = 0;
         } else
         {  s->_selectedShape = pickRay.hitShape;
            s->_selectedShape->drawBits()->toggle(SL_DB_SELECTED);
         } 
         result = true;
      }
      
   } else
   {
      for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
      {  if (s->_eventHandlers[i]->onDoubleClick(button, x, y, mod))
            result = true;
      }
   }
   return result;
} 
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onTouch2Down gets called whenever two fingers touch a handheld
screen.
*/
SLbool SLSceneView::onTouch2Down(SLint x1, SLint y1, SLint x2, SLint y2)
{	
   SLScene* s = SLScene::current;
   _touch[0].set(x1, y1);
   _touch[1].set(x2, y2);
   _touchDowns = 2;
   
   SLbool result = false;
   for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
   {  if (s->_eventHandlers[i]->onTouch2Down(x1, y1, x2, y2))
         result = true;
   }  
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onTouch2Move gets called whenever two fingers touch a handheld
screen.
*/
SLbool SLSceneView::onTouch2Move(SLint x1, SLint y1, SLint x2, SLint y2)
{	
   SLScene* s = SLScene::current;
   _touch[0].set(x1, y1);
   _touch[1].set(x2, y2);
   
   SLbool result = false;
   if (_touchDowns==2)
   {  for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
      {  if (s->_eventHandlers[i]->onTouch2Move(x1, y1, x2, y2))
            result = true;
      }
   }   
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onTouch2Up gets called whenever two fingers touch a handheld
screen.
*/
SLbool SLSceneView::onTouch2Up(SLint x1, SLint y1, SLint x2, SLint y2)
{	
   SLScene* s = SLScene::current;
   _touch[0].set(x1, y1);
   _touch[1].set(x2, y2);
   _touchDowns = 0;
   SLbool result = false;
   
   for (SLuint i=0; i < s->_eventHandlers.size(); ++i)
   {  if (s->_eventHandlers[i]->onTouch2Up(x1, y1, x2, y2))
         result = true;
   }  
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onKeyPress gets get called whenever a key is pressed. Before 
passing the command to the eventhandlers the main key commands are handled by
forwarding them to onCommand. 
*/
SLbool SLSceneView::onKeyPress(const SLKey key, const SLKey mod)
{  
   SLScene* s = SLScene::current;

   if (key=='N') return onCommand(cmdNormalsToggle);
   if (key=='P') return onCommand(cmdPolygonToggle);
   if (key=='C') return onCommand(cmdFaceCullToggle);
   if (key=='T') return onCommand(cmdTextureToggle);
   if (key=='M') return onCommand(cmdAAToggle);
   if (key=='F') return onCommand(cmdFrustCullToggle);
   if (key=='B') return onCommand(cmdBBoxToggle);
   if (key=='G') return onCommand(cmdBBoxGroupToggle);

   if (key==KeyEsc)
   {  if(_doRT)
      {  _stopRT = true;
         return false;
      } else return true; // end the program
   }

   SLbool result = false;
   if (key || mod)
   {  for (SLuint i=0; i < s->_eventHandlers.size(); ++i)
      {  if (s->_eventHandlers[i]->onKeyPress(key, mod))
            result = true;
      }
   }
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onKeyRelease get called whenever a key is released.
*/
SLbool SLSceneView::onKeyRelease(const SLKey key, const SLKey mod)
{  
   SLScene* s = SLScene::current;
   SLbool result = false;
   
   if (key || mod)
   {  for (SLuint i=0; i<s->_eventHandlers.size(); ++i)
      {  if (s->_eventHandlers[i]->onKeyRelease(key, mod))
            result = true;
      }
   }
   return result;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::onCommand: Event handler for commands. Most key press or menu 
commands are collected and dispatched here.
*/
SLbool SLSceneView::onCommand(const SLCmd cmd)
{  
   SLScene* s = SLScene::current;

   switch(cmd)
   {     
      case cmdAboutToggle:
         if (s->_menu2D)
         {  if (s->_menu2D == s->_menuGL)
                 s->_menu2D = s->_btnAbout;
            else s->_menu2D = s->_menuGL;
            return true;
         } else return false;
      case cmdHelpToggle:
         if (s->_menu2D)
         {  if (s->_menu2D == s->_menuGL)
                 s->_menu2D = s->_btnHelp;
            else s->_menu2D = s->_menuGL;
            return true;
         } else return false;
      case cmdCreditsToggle:
         if (s->_menu2D)
         {  if (s->_menu2D == s->_menuGL)
                 s->_menu2D = s->_btnCredits;
            else s->_menu2D = s->_menuGL;
            return true;
         } else return false;
            
      case cmdSceneFigure:
      case cmdSceneMesh3DS:
      case cmdSceneRevolver:
      case cmdSceneTextureFilter:
      case cmdSceneFrustumCull:
      case cmdSceneTextureBlend:
      case cmdScenePerVertexBlinn:
      case cmdScenePerPixelBlinn:
      case cmdScenePerVertexWave:
      case cmdSceneWater:
      case cmdSceneBumpNormal:  
      case cmdSceneBumpParallax:
      case cmdSceneEarth: 
      case cmdSceneRTSpheres:
      case cmdSceneMuttenzerBox:
      case cmdSceneRTSoftShadows:
      case cmdSceneRTDoF:        s->onLoad((SLCmd)cmd); return false;

      case cmdStatsToggle:       _showStats = !_showStats; return true;
      case cmdSceneInfoToggle:   _showInfo = !_showInfo; return true;
      case cmdWaitEventsToggle:  _waitEvents = !_waitEvents; return true;
      case cmdAAOn:              _doMultiSample = true; return true;      
      case cmdAAOff:             _doMultiSample = false; return true;
      case cmdAAToggle:
         _doMultiSample = !_doMultiSample;
         _raytracer.aaSamples(_doMultiSample?3:1);
         return true;
      case cmdFrustCullOn:       _doFrustumCulling = true; return true;
      case cmdFrustCullOff:      _doFrustumCulling = false; return true;
      case cmdFrustCullToggle:   _doFrustumCulling = !_doFrustumCulling;; return true;
      
      case cmdProjPersp:         _camera->projection(monoPerspective); return true;
      case cmdProjOrtho:         _camera->projection(monoOrthographic); return true;
      case cmdProjSideBySide:    _camera->projection(stereoSideBySide); return true;
      case cmdProjSideBySideP:   _camera->projection(stereoSideBySideP); return true;
      case cmdProjLineByLine:    _camera->projection(stereoLineByLine); return true;
      case cmdProjColByCol:      _camera->projection(stereoColByCol); return true;
      case cmdProjCheckerBoard:  _camera->projection(stereoCheckerBoard); return true;
      case cmdProjColorRC:       _camera->projection(stereoColorRC); return true;
      case cmdProjColorRG:       _camera->projection(stereoColorRG); return true;
      case cmdProjColorRB:       _camera->projection(stereoColorRB); return true;
      case cmdProjColorYB:       _camera->projection(stereoColorYB); return true;
      
      case cmdCamSpeedLimitInc:  _camera->speedLimit(_camera->speedLimit()*1.2f); return true;
      case cmdCamSpeedLimitDec:  _camera->speedLimit(_camera->speedLimit()*0.8f); return true;
      case cmdCamEyeSepInc:      _camera->onMouseWheel( 1, KeyCtrl); return true;
      case cmdCamEyeSepDec:      _camera->onMouseWheel(-1, KeyCtrl); return true;
      case cmdCamFocalDistInc:   _camera->onMouseWheel( 1, KeyShift); return true;
      case cmdCamFocalDistDec:   _camera->onMouseWheel(-1, KeyShift); return true;
      case cmdCamFOVInc:         _camera->onMouseWheel( 1, KeyAlt); return true;
      case cmdCamFOVDec:         _camera->onMouseWheel(-1, KeyAlt); return true;
      case cmdCamAnimTurnTable:  _camera->camAnim(turntable); return true;
      case cmdCamAnimWalk1stP:   _camera->camAnim(walking1stP); return true;
      case cmdCamReset:          _camera->shapeInit(this); return true;
      
      case cmdFaceCullOn:        _drawBits.on(SL_DB_CULLOFF); return true;
      case cmdFaceCullOff:       _drawBits.off(SL_DB_CULLOFF); return true;
      case cmdFaceCullToggle:    _drawBits.toggle(SL_DB_CULLOFF); return true;
      case cmdNormalsOn:         _drawBits.on(SL_DB_NORMALS); return true;
      case cmdNormalsOff:        _drawBits.off(SL_DB_NORMALS); return true;
      case cmdNormalsToggle:     _drawBits.toggle(SL_DB_NORMALS); return true;
      case cmdPolygonLine:       _drawBits.on(SL_DB_POLYGONLINE); return true;
      case cmdPolygonFill:       _drawBits.off(SL_DB_POLYGONLINE); return true;
      case cmdPolygonToggle:     _drawBits.toggle(SL_DB_POLYGONLINE); return true;
      case cmdBBoxOn:            _drawBits.on(SL_DB_BBOX); return true;
      case cmdBBoxOff:           _drawBits.off(SL_DB_BBOX); return true;
      case cmdBBoxToggle:        _drawBits.toggle(SL_DB_BBOX); return true;
      case cmdBBoxGroupOn:       _drawBits.on(SL_DB_BBOXGROUP); return true;
      case cmdBBoxGroupOff:      _drawBits.off(SL_DB_BBOXGROUP); return true;
      case cmdBBoxGroupToggle:   _drawBits.toggle(SL_DB_BBOXGROUP); return true;
      case cmdVoxelsOn:          _drawBits.on(SL_DB_VOXELS); return true;
      case cmdVoxelsOff:         _drawBits.off(SL_DB_VOXELS); return true;
      case cmdVoxelsToggle:      _drawBits.toggle(SL_DB_VOXELS); return true;
      case cmdTextureOn:         _drawBits.on(SL_DB_TEXOFF); return true;
      case cmdTextureOff:        _drawBits.off(SL_DB_TEXOFF); return true;
      case cmdTextureToggle:     _drawBits.toggle(SL_DB_TEXOFF); return true;
      case cmdNoAnimationOn:     _drawBits.on(SL_DB_NOANIM); return true;
      case cmdNoAnimationOff:    _drawBits.off(SL_DB_NOANIM); return true;
      case cmdNoAnimationToggle: _drawBits.toggle(SL_DB_NOANIM); return true;
      
      case cmdRenderOpenGL:
         _doRT = false;
         s->menu2D(s->_menuGL);
         return true;
      case cmdRTContinuously:   
         _raytracer.continuous(!_raytracer.continuous()); 
         return true;
      case cmdRT1: startRaytracing(1); return true;
      case cmdRT2: startRaytracing(2); return true;
      case cmdRT3: startRaytracing(3); return true;
      case cmdRT4: startRaytracing(4); return true;
      case cmdRT5: startRaytracing(5); return true;
      case cmdRT6: startRaytracing(6); return true;
      case cmdRT7: startRaytracing(7); return true;
      case cmdRT8: startRaytracing(8); return true;
      case cmdRT9: startRaytracing(9); return true;
      case cmdRT0: startRaytracing(0); return true;
      case cmdRTSaveImage: _raytracer.saveImage(); return true;
      default: break;
   }
   return false;
}
//-----------------------------------------------------------------------------
//! Draw the main 3D scene with OpenGL
SLbool SLSceneView::updateAndDraw3D(SLfloat elapsedTimeSec)
{  
   SLScene* s = SLScene::current;
   
   //0: Do animations and update the WM and AABB
   SLbool animated = false;
   if (!_drawBits.get(SL_DB_NOANIM) && s->_root3D->animate(elapsedTimeSec))
   {  _stateGL->modelViewMatrix.identity();
      s->_root3D->updateWM(false);
      s->_root3D->updateAABB();
      animated = true;
   }
   
   //1: Change state (only when changed)
   _stateGL->multiSample(_doMultiSample);
   _stateGL->depthTest(_doDepthTest);
   _stateGL->depthMask(true);
   _stateGL->cullFace(_drawBits.get(SL_DB_CULLOFF));
   _stateGL->blend(false);
   _stateGL->polygonLine(_drawBits.get(SL_DB_POLYGONLINE));

   //2: Set projection and viewport
   if (_camera->projection() > monoOrthographic)   
        _camera->setProjection(leftEye);
   else _camera->setProjection(centerEye);

   //3: Set view center eye or left eye
   if (_camera->projection() > monoOrthographic)   
        _camera->setView(leftEye);
   else _camera->setView(centerEye);
   
   //4: Update camera seperately   
   float camUpdated = _camera->shapeUpdate(this, elapsedTimeSec);
         
   //5: Do frustum culling
   if (_doFrustumCulling) 
   {  _camera->setFrustumPlanes(); 
      _blendShapes.clear();
      s->_root3D->cull(this);      
   } else _camera->numRendered(s->_root3D->numShapes);
    
   //6: Draw opaque objects once for center or left eye
   s->_root3D->draw(this);
   
   //7: Draw transparent object with blending for center or left eye
   draw3DBlended();
   
   //8: For stereo draw for right eye
   if (_camera->projection() > monoOrthographic)   
   {  _camera->setProjection(rightEye);
      _camera->setView(rightEye);
      s->_root3D->draw(this);
      draw3DBlended();
      
      // Enable all color channels again
      _stateGL->colorMask(1, 1, 1, 1); 
   }
   
   GET_GL_ERROR; // Check if any OGL errors occured
   return animated || camUpdated;
}
//-----------------------------------------------------------------------------
//! CompareViewDist C-function declaration to avoid XCode warning
SLbool CompareViewDist(SLShape* a, SLShape* b); 
//-----------------------------------------------------------------------------
/*! 
CompareViewDist C-function serves as the sort comparison function for the 
blend sorting.
*/
SLbool CompareViewDist(SLShape* a, SLShape* b)
{  if (!a) return false;
   if (!b) return true;
   return a->aabb()->sqrViewDist() > b->aabb()->sqrViewDist();
}
//-----------------------------------------------------------------------------
//! SLSceneView::drawBlendedShapes renders the transparent shapes
/*! Blended shapes must be drawn after the opaque (=non transparent) shapes 
from back to the front for correct transparency. During the cull traversal all
shapes with transparent materials are flagged and added the to the array
_blendShapes. In the SLCamera::isInFrustum function the squared distance from
the AABBs center point to the eye is calculate and can be used as the sorting
criteria.
*/
void SLSceneView::draw3DBlended()
{  
   if (_blendShapes.size()==0) return;
   
   std::sort(_blendShapes.begin(), _blendShapes.end(), CompareViewDist);
   
   _stateGL->blend(true);        // turn on blending
   _stateGL->depthMask(false);   // freeze the depth buffer
      
   // draw the blended shapes directly w. their wm transform
   for(SLuint i=0; i<_blendShapes.size(); ++i)
   {  
      // Set the view transform
      _stateGL->modelViewMatrix.setMatrix(_stateGL->viewMatrix);
      
      // Draw first AABB if required (there are no groups)
      if (_drawBits.get(SL_DB_BBOX)) 
      {  if (typeid(*_blendShapes[i])==typeid(SLRefShape))
              _blendShapes[i]->aabb()->drawWS(SLCol3f(0,0,1));
         else _blendShapes[i]->aabb()->drawWS(SLCol3f(1,0,0));
      }
      
      // Draw AABB for selected shapes
      if (_blendShapes[i]->drawBits()->get(SL_DB_SELECTED))
         _blendShapes[i]->aabb()->drawWS(SLCol3f(1,1,0));
      
      // Apply world transform
      _stateGL->modelViewMatrix.multiply(_blendShapes[i]->wm().m());
      
      if (typeid(*_blendShapes[i])==typeid(SLRefShape))
         // Don't call refShapes::shapeDraw because transform is applied x2
         ((SLRefShape*)_blendShapes[i])->refShape()->shapeDraw(this);
      else _blendShapes[i]->shapeDraw(this);
   }
   
   _stateGL->blend(false);          // turn off blending
   _stateGL->depthMask(true);       // enable depth buffer writing
   GET_GL_ERROR;        // Check if any OGL errors occured
}
//-----------------------------------------------------------------------------
/*!
SLSceneView::updateAndDraw2D draws GUI tree in ortho projection. So far no
update is done.
*/
SLbool SLSceneView::updateAndDraw2D(SLfloat elapsedTimeSec)
{     
   SLScene* s = SLScene::current;
   SLfloat w2 = (SLfloat)_scrWdiv2;
   SLfloat h2 = (SLfloat)_scrHdiv2;
   
   _stateGL->depthMask(false);         // Freeze depth buffer for blending
   _stateGL->blend(true);              // Enable blending
   _stateGL->polygonLine(false);       // Only filled polygons
   
   // Set orthographic projection with 0,0,0 in the screen center
   _stateGL->projectionMatrix.ortho(-w2, w2,-h2, h2, 0.0f, 1.0f);
   
   // Set viewport over entire screen
   _stateGL->viewport(0, 0, _scrW, _scrH);
   
   // Go to upper-left screen corner
   _stateGL->modelViewMatrix.identity();
   _stateGL->modelViewMatrix.translate(-w2, h2, 0);
   
   // Draw 2D info text for GL
   if (_showStats && s->_menu2D==s->_menuGL)
   {  build2DInfoGL();
      if (s->_infoGL) s->_infoGL->draw(this);
   }
   
   // Draw 2D info text for RT
   if (_showStats && s->_menu2D==s->_menuRT)
   {  build2DInfoRT();
      if (s->_infoRT) s->_infoRT->draw(this); 
   }

   // Draw scene info text if menuGL or menuRT is closed
   if (_showInfo && s->_info && 
       (s->_menu2D==s->_menuGL || s->_menu2D==s->_menuRT) && 
       SLButton::buttonParent==0)
   {  _stateGL->modelViewMatrix.identity();
      _stateGL->modelViewMatrix.translate(-w2, -h2, 0);
      s->_info->draw(this);
   }
   
   // Draw menu buttons tree
   if (s->_menu2D)
   {  _stateGL->modelViewMatrix.identity();
   
      // Go to lower-left screen corner
      _stateGL->modelViewMatrix.translate(-w2, -h2, 0);
      
      // Draw the 2D GUI scenegraph
      s->_menu2D->draw(this);
   }   
   
   // 2D finger touch points  
   #ifndef SL_GLES2
   if (_touchDowns)
   {  _stateGL->multiSample(true);
      _stateGL->modelViewMatrix.identity();
      
      // Go to lower-left screen corner
      _stateGL->modelViewMatrix.translate(-w2, -h2, 0);
      
      SLCol4f grayAlpha(0.5f, 0.5f, 0.5f, 0.5f);
      
      SLVec3f* touch = new SLVec3f[_touchDowns];
      for (SLuint i=0; i<_touchDowns; ++i)
      {  touch[i].x = (SLfloat)_touch[i].x;
         touch[i].y = (SLfloat)(_scrH - _touch[i].y);
         touch[i].z = 0.0f;
      }
      
      _bufTouch.generate(touch, _touchDowns, 3);
      delete [] touch;
      
      SLCol4f yelloAlpha(1.0f, 1.0f, 0.0f, 0.5f);
      _bufTouch.drawArrayAsConstantColorPoints(yelloAlpha, 20);
   }
   #endif   
   
   _stateGL->blend(false);          // turn off blending
   _stateGL->depthMask(true);       // enable depth buffer writing
   GET_GL_ERROR;                 // Check if any OGL errors occured
   return false;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::build2D builds the GUI menu button tree for the _menuGL and the 
_menuRT group as well as the _infoGL and _infoRT groups with info texts. 
See SLButton and SLText class for more infos. 
*/
void SLSceneView::build2DMenus()
{  
   SLScene* s = SLScene::current;
   
   // Create menus only once
   if (s->_menu2D) return;
   
   // Set font size depending on DPI
   SLTexFont* f;
   if (_scrDPI > 300) f = SLTexFont::font22; else 
   if (_scrDPI > 250) f = SLTexFont::font18; else 
   if (_scrDPI > 200) f = SLTexFont::font14; else 
   if (_scrDPI > 150) f = SLTexFont::font12; else 
   if (_scrDPI > 100) f = SLTexFont::font10; else 
   f = SLTexFont::font09;

   SLButton *mn1, *mn2, *mn3, *mn4, *mn5;    // submenu button pointer
   SLCmd curS = (SLCmd)s->_current;    // current scene number
   
   mn1 = new SLButton(">", f, cmdMenu, false, false, 0, true, 0, 0, SLCol3f::RED, 0.3f, centerCenter);
   mn1->drawBits()->off(SL_DB_HIDDEN);

   mn2 = new SLButton("Load Scene >", f);
   mn1->addNode(mn2);
   
   mn3 = new SLButton("General >", f);
   mn2->addNode(mn3);
   mn3->addNode(new SLButton("Scene Hierarchy", f, cmdSceneFigure, true, curS==cmdSceneFigure, mn2));
   mn3->addNode(new SLButton("3DS Mesh Loader", f, cmdSceneMesh3DS, true, curS==cmdSceneMesh3DS, mn2));
   mn3->addNode(new SLButton("Texture Blending", f, cmdSceneTextureBlend, true, curS==cmdSceneTextureBlend, mn2));
   mn3->addNode(new SLButton("Frustum Culling", f, cmdSceneFrustumCull, true, curS==cmdSceneFrustumCull, mn2));
   mn3->addNode(new SLButton("Texture Filtering", f, cmdSceneTextureFilter, true, curS==cmdSceneTextureFilter, mn2));

   mn3 = new SLButton("Shader >", f);
   mn2->addNode(mn3);
   mn3->addNode(new SLButton("Per Vertex Lighting", f, cmdScenePerVertexBlinn, true, curS==cmdScenePerVertexBlinn, mn2));
   mn3->addNode(new SLButton("Per Pixel Lighting", f, cmdScenePerPixelBlinn, true, curS==cmdScenePerPixelBlinn, mn2));
   mn3->addNode(new SLButton("Per Vertex Wave", f, cmdScenePerVertexWave, true, curS==cmdScenePerVertexWave, mn2));
   mn3->addNode(new SLButton("Water", f, cmdSceneWater, true, curS==cmdSceneWater, mn2));
   mn3->addNode(new SLButton("Bump Mapping", f, cmdSceneBumpNormal, true, curS==cmdSceneBumpNormal, mn2, true));
   mn3->addNode(new SLButton("Parallax Mapping", f, cmdSceneBumpParallax, true, curS==cmdSceneBumpParallax, mn2));
   mn3->addNode(new SLButton("Glass Shader", f, cmdSceneRevolver, true, curS==cmdSceneRevolver, mn2));
   mn3->addNode(new SLButton("Earth Shader", f, cmdSceneEarth, true, curS==cmdSceneEarth, mn2));
   
   mn3 = new SLButton("Ray tracing >", f);
   mn2->addNode(mn3);
   mn3->addNode(new SLButton("Spheres", f, cmdSceneRTSpheres, true, curS==cmdSceneRTSpheres, mn2));
   mn3->addNode(new SLButton("Muttenzer Box", f, cmdSceneMuttenzerBox, true, curS==cmdSceneMuttenzerBox, mn2));
   mn3->addNode(new SLButton("Soft Shadows", f, cmdSceneRTSoftShadows, true, curS==cmdSceneRTSoftShadows, mn2));
   mn3->addNode(new SLButton("Depth of Field", f, cmdSceneRTDoF, true, curS==cmdSceneRTDoF, mn2));

   mn2 = new SLButton("Camera >", f); mn1->addNode(mn2);
   
   mn2->addNode(new SLButton("Reset", f, cmdCamReset));
    
   mn3 = new SLButton("Projection >", f);
   mn2->addNode(mn3);
   for (SLint p=monoPerspective; p<=monoOrthographic; ++p) 
   {  mn3->addNode(new SLButton(SLCamera::projectionToStr((SLProjection)p), f, 
                                (SLCmd)(cmdProjPersp+p), true, _camera->projection()==p, mn3));
   }

   mn4 = new SLButton("Stereo >", f);
   mn3->addNode(mn4);

   mn5 = new SLButton("Eye separation >", f);
   mn4->addNode(mn5);
   mn5->addNode(new SLButton("-10%", f, cmdCamEyeSepDec, false, false, 0, false));
   mn5->addNode(new SLButton("+10%", f, cmdCamEyeSepInc, false, false, 0, false));

   mn5 = new SLButton("Focal dist. >", f);
   mn4->addNode(mn5);
   mn5->addNode(new SLButton("+5%", f, cmdCamFocalDistInc, false, false, 0, false));
   mn5->addNode(new SLButton("-5%", f, cmdCamFocalDistDec, false, false, 0, false));

   #if defined(SL_OS_WIN32)
   for (SLint p=stereoSideBySide; p<=stereoColorYB; ++p)
   #else 
   for (SLint p=stereoColorRC; p<=stereoColorYB; ++p)
   #endif 
   {  mn4->addNode(new SLButton(SLCamera::projectionToStr((SLProjection)p), f, 
                                (SLCmd)(cmdProjPersp+p), true, _camera->projection()==p, mn3));
   }
   
   mn3 = new SLButton("Animation >", f);
   mn2->addNode(mn3);
   mn3->addNode(new SLButton("Walking",   f, cmdCamAnimWalk1stP,  true, _camera->camAnim()==walking1stP, mn3));
   mn3->addNode(new SLButton("Turntable", f, cmdCamAnimTurnTable, true, _camera->camAnim()==turntable, mn3));
      
   mn3 = new SLButton("View Angle >", f);
   mn2->addNode(mn3);
   mn3->addNode(new SLButton("+5 deg.", f, cmdCamFOVInc, false, false, 0, false));
   mn3->addNode(new SLButton("-5 deg.", f, cmdCamFOVDec, false, false, 0, false));
   
   mn3 = new SLButton("Walk Speed >", f);
   mn2->addNode(mn3);
   mn3->addNode(new SLButton("+20%", f, cmdCamSpeedLimitInc, false, false, 0, false));
   mn3->addNode(new SLButton("-20%", f, cmdCamSpeedLimitDec, false, false, 0, false));
   
   mn2 = new SLButton("Render Flags >", f);
   mn1->addNode(mn2);
   #if defined(SL_OS_WIN32)
   mn2->addNode(new SLButton("Antialiasing", f, cmdAAToggle, true, _doMultiSample, 0, false));
   #endif
   mn2->addNode(new SLButton("Slowdown on Idle", f, cmdWaitEventsToggle, true, _waitEvents));
   mn2->addNode(new SLButton("View Culling", f, cmdFrustCullToggle, true, _doFrustumCulling, 0, false));
   mn2->addNode(new SLButton("No Animation", f, cmdNoAnimationToggle, true, _drawBits.get(SL_DB_NOANIM), 0, false));
   mn2->addNode(new SLButton("Textures off", f, cmdTextureToggle, true, _drawBits.get(SL_DB_TEXOFF), 0, false)); 
   mn2->addNode(new SLButton("Back faces", f, cmdFaceCullToggle, true, _drawBits.get(SL_DB_CULLOFF), 0, false));
   mn2->addNode(new SLButton("AABB (Groups)", f, cmdBBoxGroupToggle, true, _drawBits.get(SL_DB_BBOXGROUP), 0, false));
   mn2->addNode(new SLButton("AABB", f, cmdBBoxToggle, true, _drawBits.get(SL_DB_BBOX), 0, false));
   mn2->addNode(new SLButton("Voxels", f, cmdVoxelsToggle, true, _drawBits.get(SL_DB_VOXELS), 0, false));
   mn2->addNode(new SLButton("Normals", f, cmdNormalsToggle, true, _drawBits.get(SL_DB_NORMALS), 0, false));
   mn2->addNode(new SLButton("Wiremesh", f, cmdPolygonToggle, true, _drawBits.get(SL_DB_POLYGONLINE), 0, false));
   
   mn2 = new SLButton("Infos >", f);
   mn1->addNode(mn2);
   mn2->addNode(new SLButton("Statistics", f, cmdStatsToggle, true, _showStats));
   mn2->addNode(new SLButton("Scene Info", f, cmdSceneInfoToggle, true, _showInfo));
   mn2->addNode(new SLButton("About", f, cmdAboutToggle));
   mn2->addNode(new SLButton("Help", f, cmdHelpToggle));
   mn2->addNode(new SLButton("Credits", f, cmdCreditsToggle));

   mn1->addNode(new SLButton("Ray tracing", f, cmdRT5, false, false, 0, true));

   // Init
   _stateGL->modelViewMatrix.identity();
   mn1->setSizeRec();
   mn1->setPosRec(SLButton::minMenuPos.x, SLButton::minMenuPos.y);
   mn1->init(this, 1);
   mn1->buildAABB();
   s->_menuGL = mn1;
   
   // Build ray tracing menu
   SLCol3f green(0.0f,0.5f,0.0f);
   
   mn1 = new SLButton(">", f, cmdMenu, false, false, 0, true,  0, 0, green, 0.3f, centerCenter);
   mn1->drawBits()->off(SL_DB_HIDDEN);
   
   mn1->addNode(new SLButton("OpenGL Rendering", f, cmdRenderOpenGL, false, false, 0, true,  0, 0, green));
   mn1->addNode(new SLButton("Render continuously", f, cmdRTContinuously, true, _raytracer.continuous(), 0, true,  0, 0, green));
   mn1->addNode(new SLButton("Redering Depth 1", f, cmdRT1, false, false, 0, true,  0, 0, green));
   mn1->addNode(new SLButton("Redering Depth 5", f, cmdRT5, false, false, 0, true,  0, 0, green));
   mn1->addNode(new SLButton("Redering Depth max.", f, cmdRT0, false, false, 0, true,  0, 0, green));
   #if defined(SL_OS_WIN32)
   mn1->addNode(new SLButton("Save Image", f, cmdRTSaveImage, false, false, 0, true,  0, 0, green));
   #endif

   // Init
   _stateGL->modelViewMatrix.identity();
   mn1->setSizeRec();
   mn1->setPosRec(SLButton::minMenuPos.x, SLButton::minMenuPos.y);
   mn1->init(this, 1);
   mn1->buildAABB();
   s->_menuRT = mn1;

   build2DMsgBoxes(); 

   s->_menu2D = s->_btnAbout;

}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::build2DInfoGL builds the _infoGL groups with info texts. 
See SLButton and SLText class for more infos. 
*/
void SLSceneView::build2DInfoGL()
{     
   SLScene* s = SLScene::current;
   if (s->_infoGL) delete s->_infoGL;
      
   SLCamera*   cam = camera();
   SLGroup*    g = new SLGroup("statGL"); 
   SLText*     t;             // text object
   SLchar      str[255];      // charcter array
   
   // Set font size depending on DPI
   SLTexFont* f;
   if (_scrDPI > 300) f = SLTexFont::font14; else 
   if (_scrDPI > 250) f = SLTexFont::font12; else 
   if (_scrDPI > 200) f = SLTexFont::font10; else 
   if (_scrDPI > 150) f = SLTexFont::font09; else 
   if (_scrDPI > 100) f = SLTexFont::font08; else 
   f = SLTexFont::font07;

   SLint       ln = 2; // line number
   SLfloat     lh = (SLfloat)f->charsHeight;  // line height
   
   // prepare some statistic infos
   SLfloat vox = (SLfloat)s->root3D()->numVoxels;
   SLfloat voxEmpty = (SLfloat)s->root3D()->numVoxEmpty;
   SLfloat voxelsEmpty  = vox ? voxEmpty / vox*100.0f : 0.0f;
   SLfloat numRTTria = (SLfloat)s->root3D()->numRTTriangles;
   SLfloat avgTriPerVox = vox ? numRTTria / (vox-voxEmpty) : 0.0f;
   
   sprintf(str, "Scene: %s", s->name().c_str());   
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "FPS: %4.1f  (Size: %d x %d)", _fps, _scrW, _scrH); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Frame Time : %6.4f sec.", _frameTime); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Shapes in Frustum: %d", cam->numRendered()); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "--------------------------------------------"); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "OpenGL: %s", _stateGL->glVersion().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Vendor: %s", _stateGL->glVendor().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Renderer: %s", _stateGL->glRenderer().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "GLSL: %s", _stateGL->glGLSLVersion().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "--------------------------------------------"); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Projection: %s", cam->projectionStr().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Animation: %s", cam->animationStr().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Speed Limit: %4.1f/sec.", cam->speedLimit());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   if (camera()->projection() > monoOrthographic)
   {  sprintf(str, "Eye separation: %4.2f (%3.1f%% of f)", 
                 cam->eyeSep(),cam->eyeSep()/cam->focalDist()*100);
      t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   }
   sprintf(str, "fov: %4.2f", cam->fov());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Focal distance: %4.2f", cam->focalDist());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Projection size: %4.2f x %4.2f", cam->focalDistScrW(),cam->focalDistScrH());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "--------------------------------------------"); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);     
   sprintf(str, "Groups: %d", s->root3D()->numGroups);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Shapes: %d", s->root3D()->numShapes);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "RefShapes: %d", s->root3D()->numRefShapes);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Lights: %d", s->root3D()->numLights);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "MB in Meshes: %f", (SLfloat)s->root3D()->numBytes / 1000000.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "MB in Accel.: %f", (SLfloat)s->root3D()->numBytesAccel / 1000000.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "No. of VBOs: %u", SLGLBuffer::totalBufferCount);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "MB in VBOs: %6.5f", (SLfloat)SLGLBuffer::totalBufferSize / 1000000.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);

   sprintf(str, "RTVertices: %d", s->root3D()->numRTVertices);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "RTTriangles: %d", s->root3D()->numRTTriangles);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Voxels: %d", s->root3D()->numVoxels);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Voxels empty: %4.1f%%", voxelsEmpty); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Avg. Tria/Voxel: %4.1f", avgTriPerVox);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Max. Tria/Voxel: %d", s->root3D()->numVoxMaxTria);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   
   s->_infoGL = g;
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::build2DInfoRT builds the _infoRT group with info texts. 
See SLButton and SLText class for more infos. 
*/
void SLSceneView::build2DInfoRT()
{     
   SLScene* s = SLScene::current;
   if (s->_infoRT) 
      delete s->_infoRT;
   s->_infoRT = 0; 
   
   SLCamera*    cam = camera();
   SLGroup*     g = new SLGroup("infoRT"); 
   SLText*      t;             // text object
   SLchar       str[255];      // charcter array

   // Set font size depending on DPI
   SLTexFont* f;
   if (_scrDPI > 300) f = SLTexFont::font14; else 
   if (_scrDPI > 250) f = SLTexFont::font12; else 
   if (_scrDPI > 200) f = SLTexFont::font10; else 
   if (_scrDPI > 150) f = SLTexFont::font09; else 
   if (_scrDPI > 100) f = SLTexFont::font08; else 
   f = SLTexFont::font07;

   SLint       ln = 2; // line number
   SLfloat     lh = (SLfloat)f->charsHeight;  // line height

   SLRaytracer* rt = &_raytracer;
   SLint  primaries = _scrW * _scrH;
   SLuint total = primaries + 
                  SLRay::reflectedRays + 
                  SLRay::subsampledRays + 
                  SLRay::refractedRays + 
                  SLRay::shadowRays;
   
   // prepare some statistic infos
   SLfloat vox = (SLfloat)s->root3D()->numVoxels;
   SLfloat voxEmpty = (SLfloat)s->root3D()->numVoxEmpty;
   SLfloat voxelsEmpty  = vox ? voxEmpty / vox*100.0f : 0.0f;
   SLfloat numRTTria = (SLfloat)s->root3D()->numRTTriangles;
   SLfloat avgTriPerVox = vox ? numRTTria / (vox-voxEmpty) : 0.0f;

   sprintf(str, "Scene: %s", s->name().c_str());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Time per frame: %4.2f sec.  (Size: %d x %d)", rt->renderSec(), _scrW, _scrH);  
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "fov: %4.2f", cam->fov());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Focal dist. (f): %4.2f", cam->focalDist());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "--------------------------------------------"); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);     
   sprintf(str, "Threads: %d", rt->numThreads());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Max. allowed RT depth: %d", SLRay::maxDepth);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Max. reached RT depth: %d", SLRay::maxDepthReached);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Average RT depth: %4.2f", SLRay::avgDepth/primaries);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "AA threshhold: %2.1f", rt->aaThreshold());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "AA samples: %d x %d", rt->aaSamples(), rt->aaSamples());
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "AA pixels: %u, %3.1f%%", SLRay::subsampledPixels, (SLfloat)SLRay::subsampledPixels/primaries*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Primary rays: %u, %3.1f%%", primaries, (SLfloat)primaries/total*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Reflected rays: %u, %3.1f%%", SLRay::reflectedRays, (SLfloat)SLRay::reflectedRays/total*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Refracted rays: %u, %3.1f%%", SLRay::refractedRays, (SLfloat)SLRay::refractedRays/total*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "TIR rays: %u, %3.1f%%", SLRay::tirRays, (SLfloat)SLRay::tirRays/total*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Shadow rays: %u, %3.1f%%", SLRay::shadowRays, (SLfloat)SLRay::shadowRays/total*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "AA rays: %u, %3.1f%%", SLRay::subsampledRays, (SLfloat)SLRay::subsampledRays/total*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Total rays: %u, %3.1f%%", total, 100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Intersection tests: %u", SLRay::intersections);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Intersections: %u, %3.1f%%", SLRay::tests, SLRay::intersections/(SLfloat)SLRay::tests*100.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "--------------------------------------------"); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);     
   sprintf(str, "Groups: %d", s->root3D()->numGroups);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Shapes: %d", s->root3D()->numShapes);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "RefShapes: %d", s->root3D()->numRefShapes);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Lights: %d", s->root3D()->numLights);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "MB in Meshes: %f", (SLfloat)s->root3D()->numBytes / 1000000.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "MB in Accel.: %f", (SLfloat)s->root3D()->numBytesAccel / 1000000.0f);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "RTVertices: %d", s->root3D()->numRTVertices);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "RTTriangles: %d", s->root3D()->numRTTriangles);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Voxels: %d", s->root3D()->numVoxels);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Voxels empty: %4.1f%%", voxelsEmpty); 
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Avg. Tria/Voxel: %4.1f", avgTriPerVox);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   sprintf(str, "Max. Tria/Voxel: %d", s->root3D()->numVoxMaxTria);
   t = new SLText(str, f); t->translate(10.0f, -lh*ln++, 0.0f); g->addNode(t);
   
   s->_infoRT = g;
}
//-----------------------------------------------------------------------------
//! Builds the message buttons. They depend on screen width.
void SLSceneView::build2DMsgBoxes()
{ 
   SLScene* s = SLScene::current;


   
   // Set font size depending on DPI
   SLTexFont* f;
   if (_scrDPI > 300) f = SLTexFont::font18; else 
   if (_scrDPI > 250) f = SLTexFont::font16; else 
   if (_scrDPI > 200) f = SLTexFont::font14; else 
   if (_scrDPI > 150) f = SLTexFont::font12; else 
   if (_scrDPI > 100) f = SLTexFont::font10; else 
   f = SLTexFont::font09;
   
   // Help button
   if (s->_btnHelp) delete s->_btnHelp;
   s->_btnHelp = new SLButton("\
Help for mouse or finger control: \\n\
- Use mouse or your finger to rotate the scene \\n\
- Use mouse-wheel or pinch 2 fingers to go forward/backward \\n\
- Use CTRL-mouse or 2 fingers to move sidewards/up-down \\n\
- Double click or double tap to select object\\n\
- Screenshot: Use a screenshot tool,\\n\
   on iOS: Quick hold down home & power button, \\n\
   on Android: Quick hold down back & home button \\n\
   on desktop: Use a screenshot tool", f,
   cmdAboutToggle, false, false, 0, true, 
   _scrW - 2*SLButton::minMenuPos.x, 0.0f, 
   SLCol3f::RED, 0.8f, centerCenter);

   _stateGL->modelViewMatrix.identity();
   s->_btnHelp->drawBits()->off(SL_DB_HIDDEN);
   s->_btnHelp->setSizeRec();
   s->_btnHelp->setPosRec(SLButton::minMenuPos.x, SLButton::minMenuPos.y);
   s->_btnHelp->init(this, 1);
   s->_btnHelp->buildAABB();
   
   // About button
   if (s->_btnAbout) delete s->_btnAbout;
   s->_btnAbout = new SLButton(
"Welcome to the SLProjet demo app. It is developed at the \
Computer Science Department of the Berne University of Applied Sciences. \
The app shows what you can learn in one semester about 3D computer graphics \
in real time rendering and ray tracing. The framework is developed in \
in C++ with OpenGL ES2 so that it can run also on mobile devices. \
Ray tracing provides in addition highquality transparencies, reflections and soft shadows. \
Click to close and use the menu to choose different scenes and view settings. \
For more information please visit: http://code.google.com/p/slproject/", f,
   cmdAboutToggle, false, false, 0, true,
   _scrW - 2*SLButton::minMenuPos.x, 0.0f, 
   SLCol3f::RED, 0.8f, centerCenter);

   _stateGL->modelViewMatrix.identity();
   s->_btnAbout->drawBits()->off(SL_DB_HIDDEN);
   s->_btnAbout->setSizeRec();
   s->_btnAbout->setPosRec(SLButton::minMenuPos.x, SLButton::minMenuPos.y);
   s->_btnAbout->init(this, 1);
   s->_btnAbout->buildAABB();
   
   // Credits button
   if (s->_btnCredits) delete s->_btnCredits;
   s->_btnCredits = new SLButton(
"Credits for external libraries: \\n\
- glew: glew.sourceforge.net \\n\
- glfw: www.glfw.org \\n\
- jpeg: www.jpeg.org \\n\
- nvwa: sourceforge.net/projects/nvwa \\n\
- png: www.libpng.org \\n\
- randomc: www.agner.org/random \\n\
- zlib: zlib.net", f,
   cmdAboutToggle, false, false, 0, true,
   _scrW - 2*SLButton::minMenuPos.x, 0.0f, 
   SLCol3f::RED, 0.8f, centerCenter);

   _stateGL->modelViewMatrix.identity();
   s->_btnCredits->drawBits()->off(SL_DB_HIDDEN);
   s->_btnCredits->setSizeRec();
   s->_btnCredits->setPosRec(SLButton::minMenuPos.x, SLButton::minMenuPos.y);
   s->_btnCredits->init(this, 1);
   s->_btnCredits->buildAABB();
}
//-----------------------------------------------------------------------------
/*! 
SLSceneView::calcFPS calculates the precise frame per second rate. 
The calculation is done every half second and averaged with the last FPS calc.
*/
SLfloat SLSceneView::calcFPS(SLfloat deltaTime)
{  const  SLint   FILTERSIZE = 20;
   static SLfloat frameTime[FILTERSIZE];
   static SLuint  frameNo = 0;

   frameTime[frameNo % FILTERSIZE] = deltaTime;
   SLfloat sumTime = 0.0f;
   for (SLuint i=0; i<FILTERSIZE; ++i) sumTime += frameTime[i];
   frameNo++;
   _frameTime = sumTime / (SLfloat)FILTERSIZE;
   _fps = 1 / _frameTime;

   return _fps;
}
//-----------------------------------------------------------------------------
//! Returns the window title with name & FPS
SLstring SLSceneView::windowTitle()
{  
   SLScene* s = SLScene::current;
   SLchar title[255];
   if (!_camera || !s->_root3D)
      return SLstring("-");

   if (_doRT)
   {  if (_raytracer.continuous())
      {  sprintf(title, "%s (fps: %4.1f, Threads: %d)", 
                       s->name().c_str(), 
                       _fps, 
                       _raytracer.numThreads());
      } else
      {  sprintf(title, "%s (%d%%, Threads: %d)", 
                        s->name().c_str(), 
                        _raytracer.pcRendered(), 
                        _raytracer.numThreads());
      }
   } else
   {  sprintf(title, "%s (fps: %4.1f, %u shapes rendered)", 
                     s->name().c_str(), 
                     _fps,
                     camera()->numRendered());
   }
   return SLstring(title);
}
//-----------------------------------------------------------------------------
//! SLSceneView::camera sets the active camera
void SLSceneView::camera(SLCamera* camera)    
{  
   SLScene* s = SLScene::current;
   
   // Find & erase active camera from the scenes eventhandler array.
   if (_camera && s->_eventHandlers.size() > 0)
   {  SLVEventHandler::iterator i;
      i = std::find(s->_eventHandlers.begin(), 
                    s->_eventHandlers.end(), _camera);
      
      if (i != s->_eventHandlers.end())
         s->_eventHandlers.erase(i);
   }  
   
   // set new active camera and add it to the event handler array
   _camera = camera;
   
   if (camera) 
      s->_eventHandlers.push_back(camera);
}
//-----------------------------------------------------------------------------
//! Starts the ray tracing & sets the RT menu 
void SLSceneView::startRaytracing(SLint maxDepth)
{  
   SLScene* s = SLScene::current;
   _doRT = true;
   _stopRT = false;
   _raytracer.maxDepth(maxDepth);
   _raytracer.aaSamples(_doMultiSample?3:1);
   s->_menu2D = s->_menuRT;
}
//-----------------------------------------------------------------------------
/*!
SLSceneView::updateAndRT3D starts the raytracing or refreshes the current RT
image during rendering. The function returns true if an animation was done 
prior to the rendering start.
*/
SLbool SLSceneView::updateAndRT3D(SLfloat elapsedTimeSec)
{
   SLScene* s = SLScene::current;
   SLbool updated = false;
   
   // if the raytracer not yet got started
   if (_raytracer.state()==rtReady)
   {   
      if (!_drawBits.get(SL_DB_NOANIM) && s->_root3D->animate(0.05f))
      {  _stateGL->modelViewMatrix.identity();
         s->_root3D->updateWM(false);
         s->_root3D->updateAABB();
         updated = true;
      }  

      // Start raytracing
      _raytracer.render();
   }

   // Refresh the render image during RT
   _raytracer.renderImage();

   // React on the stop flag (e.g. ESC)
   if(_stopRT)
   {  _doRT = false;
      s->menu2D(s->_menuGL);
      s->menu2D()->closeAll();
      updated = true;
   }

   return updated;
}
//-----------------------------------------------------------------------------
