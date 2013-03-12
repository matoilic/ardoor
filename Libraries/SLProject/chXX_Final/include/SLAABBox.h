//#############################################################################
//  File:      SLAABBox.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLAABBox_H
#define SLAABBox_H

#include <stdafx.h>
#include <SLGLBuffer.h>

class SLRay;
class SLScene;

//-----------------------------------------------------------------------------
//! Defines an axis aligned bounding box
/*!
The SLAABBox class defines an axis aligned bounding box with a minimal and 
maximal point. Each shape (SLShape) will have an AABB the will be calculated
in buildAABB. A mesh (SLMesh) will implement buildAABB and calculate the
minimal and maximal coordiantes in object space (stored in _minOS and _maxOS).
For a fast ray-AABB intersection in world space we transform _minOS and _maxOS
into world space (with the shapes world matrix) and store it in _minWS and
_maxWS.
For an even faster intersection test with the plane of the view frustum we
calculate in addition the bounding sphere around the AABB. The radius and the
center point are stored in _radiusOS/_centerOS and _radiusWS/_centerWS.
*/
class SLAABBox
{  public:
                           SLAABBox  ();
                          ~SLAABBox  (){;}

               // Setters
               void        minWS       (SLVec3f minC)  {_minWS = minC;}
               void        maxWS       (SLVec3f maxC)  {_maxWS = maxC;}
               void        minOS       (SLVec3f minC)  {_minOS = minC;}
               void        maxOS       (SLVec3f maxC)  {_maxOS = maxC;}
               
               void        isVisible   (SLbool visible){_isVisible = visible;}
               void        hasAlpha   (SLbool transp) {_hasTransp = transp;}   
               void        sqrViewDist (SLfloat sqrVD) {_sqrViewDist = sqrVD;}    

               // Getters 
               SLVec3f     minWS       () {return _minWS;}
               SLVec3f     maxWS       () {return _maxWS;}
               SLVec3f     centerWS    () {return _centerWS;}
               SLfloat     radiusWS    () {return _radiusWS;}
               SLVec3f     minOS       () {return _minOS;}
               SLVec3f     maxOS       () {return _maxOS;}
               SLVec3f     centerOS    () {return _centerOS;}
               SLfloat     radiusOS    () {return _radiusOS;}
               SLbool      isVisible   () {return _isVisible;}
               SLbool      hasAlpha   () {return _hasTransp;}
               SLfloat     sqrViewDist () {return _sqrViewDist;}
               
               // Misc.
               void        fromOStoWS  (const SLVec3f minOS, 
                                        const SLVec3f maxOS, 
                                        SLMat4f &wm);
               void        fromWStoOS  (const SLVec3f minWS, 
                                        const SLVec3f maxWS, 
                                        SLMat4f &wmI);
               void        merge       (SLAABBox &bb);
               void        drawWS      (const SLCol3f color);
               void        setCenterAndRadius();    
               SLbool      isHitInOS   (SLRay* ray);
               SLbool      isHitInWS   (SLRay* ray);
               
   private:
               SLVec3f     _minWS;     //!< Min. corner in world space
               SLVec3f     _maxWS;     //!< Max. corner in world space
               SLVec3f     _centerWS;  //!< Center of AABB in world space
               SLfloat     _radiusWS;  //!< Radius of sphere around AABB in WS
               SLfloat     _sqrViewDist; //!< Sqr. dist. from center to viewpoint
               
               SLVec3f     _minOS;     //!< Min. corner in object space
               SLVec3f     _maxOS;     //!< Max. corner in object space
               SLVec3f     _centerOS;  //!< Center of AABB in object space
               SLfloat     _radiusOS;  //!< Radius of sphere around AABB in OS
               
               SLbool      _isVisible; //!< Flag if AABB is in the view frustum
               SLbool      _hasTransp; //!< Flag if AABB has transparent shapes
               
               SLGLBuffer  _bufP;      //!< Buffer object for vertex positions
};
//-----------------------------------------------------------------------------

#endif
