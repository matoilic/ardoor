//#############################################################################
//  File:      SLCamera.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCAMERA_H
#define SLCAMERA_H

#include <stdafx.h>
#include <SLGLState.h>
#include <SLShape.h>
#include <SLGLBuffer.h>
#include <SLSamples2D.h>
#include <SLRay.h>

//-----------------------------------------------------------------------------
//! Enumeration for possible camera animation types
typedef enum
{  turntable,
   walking1stP
} SLCamAnim;

//-----------------------------------------------------------------------------
//! Enumeration for differen projections
typedef enum
{  monoPerspective,     //! standard mono pinhole perspective projection
   monoOrthographic,    //! standard mono orthgraphic projection
   stereoSideBySide,    //! side-by-side
   stereoSideBySideP,   //! side-by-side proportional for mirror stereoscopes
   stereoLineByLine,    //! line-by-line
   stereoColByCol,      //! column-by-column
   stereoCheckerBoard,  //! checkerboard pattern (DLP3D)
   stereoColorRC,       //! color masking for red-cyan anaglyphs
   stereoColorRG,       //! color masking for red-green anaglyphs
   stereoColorRB,       //! color masking for red-blue anaglyphs
   stereoColorYB        //! color masking for yellow-blue anaglyphs (ColorCode 3D)
} SLProjection;
//-----------------------------------------------------------------------------
//! Enumeration for stereo eye
typedef enum
{  leftEye   =-1,
   centerEye = 0,
   rightEye  = 1
} SLEye;

//-----------------------------------------------------------------------------
class SLSceneView;

//-----------------------------------------------------------------------------
//! Active or visible camera shape class
/*! An instance of this SLShape derived class serves as an active camera with
all its view and projection parameters or if inactive as a visible scene graph
shape node with camera body and its view frustum. The position and orientation
of the active camera is set in the setView method by loading the viewmatrix _vm
into the OpenGL modelview matrix. The view matrix _vm is simply the inverse of
the shapes world matrix _wm. Every SLSceneView instance has a pointer _camera
to its active camera.
Because the the SLShape class is inherited from the abstract SLEventHandler
class a camera can handle mouse & keyboard event. All camera animations are
handled in these eventhandlers. 
*/ 
class SLCamera: public SLShape
{  public:
                           SLCamera       ();
                          ~SLCamera       ();

               void        shapeInit      (SLSceneView* sv);
               SLbool      shapeUpdate    (SLSceneView* sv, SLfloat time);
               void        shapeDraw      (SLSceneView* sv);
               SLShape*    shapeCopy      ();
               void        updateStats    (SLGroup* parent);
               SLAABBox&   buildAABB      () {return _aabb;}
               SLbool      shapeHit       (SLRay* ray){(void)ray; return false;}
               SLbool      hit            (SLRay* ray){(void)ray; return false;}
               void        preShade       (SLRay* ray){(void)ray;}

               // Event handlers for camera animation
               SLbool      onMouseDown    (const SLMouseButton button, 
                                           const SLint x, const SLint y, 
                                           const SLKey mod); 
               SLbool      onMouseMove    (const SLMouseButton button, 
                                           const SLint x, const SLint y,
                                           const SLKey mod);
               SLbool      onMouseUp      (const SLMouseButton button, 
                                           const SLint x, const SLint y, 
                                           const SLKey mod);
               SLbool      onMouseWheel   (const SLint delta, const SLKey mod);
               SLbool      onTouch2Down   (const SLint x1, const SLint y1,
                                           const SLint x2, const SLint y2);
               SLbool      onTouch2Move   (const SLint x1, const SLint y1,
                                           const SLint x2, const SLint y2);
               SLbool      onTouch2Up     (const SLint x1, const SLint y1,
                                           const SLint x2, const SLint y2);
               SLbool      onKeyPress     (const SLKey key, const SLKey mod);
               SLbool      onKeyRelease   (const SLKey key, const SLKey mod);
               
               void        eyeToPixelRay  (SLfloat x, SLfloat y, SLRay* ray);  
               SLbool      isInFrustum    (SLAABBox* aabb);

               // Apply projection, viewport and view transformations
               void        setProjection  (const SLEye eye);
               void        setView        (const SLEye eye);
               void        setFrustumPlanes();
               void        setWMandState  ();

               // Setters
               void        projection     (SLProjection p)     {_projection = p;
                                                                currentProjection = p;}
               void        fov            (const SLfloat fov)  {_fov = fov;
                                                                currentFOV = fov;}
               void        camAnim        (SLCamAnim ca)       {_camAnim = ca;
                                                                currentAnimation = ca;}
               void        clipNear       (const SLfloat cNear){_clipNear = cNear;}
               void        clipFar        (const SLfloat cFar) {_clipFar = cFar;}
               void        numRendered    (const SLuint numR)  {_numRendered = numR;}
               void        speedLimit     (const SLfloat sl)   {_speedLimit = sl;}
               void        focalDist      (const SLfloat f)    {_focalDist = f;}
               void        lensDiameter   (const SLfloat d)    {_lensDiameter = d;}
               void        lensSamples    (SLint x, SLint y)   {_lensSamples.samples(x, y);}
               void        eyeSep         (const SLfloat es)   {_eyeSep = es;}
               
               // Getters
               SLMat4f     vm             () {return _vm;}
               SLProjection projection    () {return _projection;}
               SLstring    projectionStr  () {return projectionToStr(_projection);}  
                 
               SLfloat     fov            () {return _fov;}
               SLfloat     clipNear       () {return _clipNear;}
               SLfloat     clipFar        () {return _clipFar;}
               SLCamAnim   camAnim        () {return _camAnim;}
               SLstring    animationStr   ();
               SLuint      numRendered    () {return _numRendered;}
               SLfloat     focalDist      () {return _focalDist;} 
               SLfloat     lensDiameter   () {return _lensDiameter;}
               SLSamples2D* lensSamples   () {return &_lensSamples;} 
               SLfloat     eyeSep         () {return _eyeSep;}
               SLfloat     focalDistScrW  ();
               SLfloat     focalDistScrH  ();
               SLRay*      lookAtRay      () {return &_lookAtRay;}
               SLfloat     speedLimit     () {return _speedLimit;}
   
   // Static global default parameters for new cameras
   static      SLCamAnim      currentAnimation;
   static      SLProjection   currentProjection;
   static      SLfloat        currentFOV;
   static      SLint          currentDevRotation;
   static      SLstring       projectionToStr(SLProjection p);

   private:
               SLMat4f     _vm;           //!< Current view matrix
               SLMat4f     _vmInit;       //!< Initial view matrix for reset
               
               // projection parameters
               SLProjection _projection;  //!< projection type
               SLfloat     _fov;          //!< Current field of view (view angle)
               SLfloat     _fovInit;      //!< Initial field of view (view angle)
               SLfloat     _clipNear;     //!< Dist. to the near clipping plane
               SLfloat     _clipFar;      //!< Dist. to the far clipping plane
               SLPlane     _plane[6];     //!< 6 frustum planes (t, b, l, r, n, f)
               SLuint      _numRendered;  //!< num. of shapes in frustum
               enum {T=0,B,L,R,N,F};      //!< enum for planes
               
               SLGLBuffer  _bufP;         //!< Buffer object for visualization
               
               // animation parameters
               SLCamAnim   _camAnim;      //!< Type of camera animation
               SLVec2f     _oldTouchPos1; //!< Old mouse/thouch position in pixels
               SLVec2f     _oldTouchPos2; //!< Old 2nd finger touch position in pixels
               SLVec3f     _maxSpeed;     //!< max. speed per axis
               SLVec3f     _curSpeed;     //!< current speed per axis
               SLfloat     _speedLimit;   //!< speed limit per sec. for all axis
               
               // ray tracing parameters
               SLRay       _lookAtRay;    //!< Ray through the center of screen
               SLfloat     _focalDist;    //!< distance of focal plane from lens
               SLfloat     _lensDiameter; //!< Lens diameter
               SLSamples2D _lensSamples;  //!< samplepoints for lens sampling (dof)

               // Stereo rendering
               SLfloat     _eyeSep;       //!< eye separation for stereo mode
};
//-----------------------------------------------------------------------------
#endif
