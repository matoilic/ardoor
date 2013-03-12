//#############################################################################
//  File:      SLAABBox.cpp
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

#include "SLAABBox.h"
#include "SLRay.h"
#include "SLGLShaderProg.h"
#include "SLMesh.h"
#include "SLScene.h"

//-----------------------------------------------------------------------------
SLAABBox::SLAABBox()
{  _minWS    = SLVec3f::ZERO;
   _maxWS    = SLVec3f::ZERO;
   _centerWS = SLVec3f::ZERO;
   _radiusWS = 0;
   
   _minOS    = SLVec3f::ZERO;
   _maxOS    = SLVec3f::ZERO;
   _centerOS = SLVec3f::ZERO;
   _radiusOS = 0;
   
   // Transparent flags are set during shapeInit
   _hasTransp = false;
   _isVisible = true;
}
//-----------------------------------------------------------------------------
//! Recalculate min and max after transformation in world coords
void SLAABBox::fromOStoWS(const SLVec3f minOS,     
                          const SLVec3f maxOS, 
                          SLMat4f &wm)
{  
   // Do not transform empty AABB (such as from the camera)
   if (minOS==SLVec3f::ZERO && maxOS==SLVec3f::ZERO)
      return;

   _minOS.set(minOS);
   _maxOS.set(maxOS);
   _minWS.set(minOS);
   _maxWS.set(maxOS);

   // we need to transform all 8 corners for a non-optimal bounding box
   SLVec3f vCorner[8];
   SLint i;

   vCorner[0].set(_minWS);
   vCorner[1].set(_maxWS.x, _minWS.y, _minWS.z); 
   vCorner[2].set(_maxWS.x, _minWS.y, _maxWS.z);  
   vCorner[3].set(_minWS.x, _minWS.y, _maxWS.z);  
   vCorner[4].set(_maxWS.x, _maxWS.y, _minWS.z);  
   vCorner[5].set(_minWS.x, _maxWS.y, _minWS.z);
   vCorner[6].set(_minWS.x, _maxWS.y, _maxWS.z);  
   vCorner[7].set(_maxWS);

   // apply world transform
   for (i=0; i<8; ++i) vCorner[i] = wm * vCorner[i];

   //sets the minimum and maximum of the vertex components of the 8 corners
   _minWS.set(vCorner[0]);
   _maxWS.set(vCorner[0]);
   for (i=1; i<8; ++i)
   {  _minWS.setMin(vCorner[i]);
      _maxWS.setMax(vCorner[i]);
   }

   // Update OpenGL vertex buffer
   if (_bufP.id())
   {  
      SLVec3f P[24];  // vertex positions
      P[ 0].set(_minWS.x, _minWS.y, _minWS.z); // lower rect
      P[ 1].set(_maxWS.x, _minWS.y, _minWS.z);
      P[ 2].set(_maxWS.x, _minWS.y, _minWS.z);
      P[ 3].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[ 4].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[ 5].set(_minWS.x, _minWS.y, _maxWS.z);
      P[ 6].set(_minWS.x, _minWS.y, _maxWS.z);
      P[ 7].set(_minWS.x, _minWS.y, _minWS.z);
      P[ 8].set(_minWS.x, _maxWS.y, _minWS.z); // upper rect
      P[ 9].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[10].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[11].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[12].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[13].set(_minWS.x, _maxWS.y, _maxWS.z);
      P[14].set(_minWS.x, _maxWS.y, _maxWS.z);
      P[15].set(_minWS.x, _maxWS.y, _minWS.z); // vertical lines
      P[16].set(_minWS.x, _minWS.y, _minWS.z);
      P[17].set(_minWS.x, _maxWS.y, _minWS.z);
      P[18].set(_maxWS.x, _minWS.y, _minWS.z);
      P[19].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[20].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[21].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[22].set(_minWS.x, _minWS.y, _maxWS.z);
      P[23].set(_minWS.x, _maxWS.y, _maxWS.z);
      _bufP.generate(P, 24, 3);
   }
   
   setCenterAndRadius();
}
//-----------------------------------------------------------------------------
//! Recalculate min and max before transformation in object coords
void SLAABBox::fromWStoOS(const SLVec3f minWS, 
                          const SLVec3f maxWS, 
                          SLMat4f &wmI)
{  
   _minOS.set(minWS);
   _maxOS.set(maxWS);
   _minWS.set(minWS);
   _maxWS.set(maxWS);

   // we need to transform all 8 corners for a non-optimal bounding box
   SLVec3f vCorner[8];
   SLint i;

   vCorner[0].set(_minOS);
   vCorner[1].set(_maxOS.x, _minOS.y, _minOS.z); 
   vCorner[2].set(_maxOS.x, _minOS.y, _maxOS.z);  
   vCorner[3].set(_minOS.x, _minOS.y, _maxOS.z);  
   vCorner[4].set(_maxOS.x, _maxOS.y, _minOS.z);  
   vCorner[5].set(_minOS.x, _maxOS.y, _minOS.z);
   vCorner[6].set(_minOS.x, _maxOS.y, _maxOS.z);  
   vCorner[7].set(_maxOS);

   // apply world transform
   for (i=0; i<8; ++i) vCorner[i] = wmI * vCorner[i];

   //sets the minimum and maximum of the vertex components of the 8 corners
   _minOS.set(vCorner[0]);
   _maxOS.set(vCorner[0]);
   for (i=1; i<8; ++i)
   {  _minOS.setMin(vCorner[i]);
      _maxOS.setMax(vCorner[i]);
   }

   // Update OpenGL vertex buffer
   if (_bufP.id())
   {  
      SLVec3f P[24];  // vertex positions
      P[ 0].set(_minWS.x, _minWS.y, _minWS.z); // lower rect
      P[ 1].set(_maxWS.x, _minWS.y, _minWS.z);
      P[ 2].set(_maxWS.x, _minWS.y, _minWS.z);
      P[ 3].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[ 4].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[ 5].set(_minWS.x, _minWS.y, _maxWS.z);
      P[ 6].set(_minWS.x, _minWS.y, _maxWS.z);
      P[ 7].set(_minWS.x, _minWS.y, _minWS.z);
      P[ 8].set(_minWS.x, _maxWS.y, _minWS.z); // upper rect
      P[ 9].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[10].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[11].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[12].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[13].set(_minWS.x, _maxWS.y, _maxWS.z);
      P[14].set(_minWS.x, _maxWS.y, _maxWS.z);
      P[15].set(_minWS.x, _maxWS.y, _minWS.z); // vertical lines
      P[16].set(_minWS.x, _minWS.y, _minWS.z);
      P[17].set(_minWS.x, _maxWS.y, _minWS.z);
      P[18].set(_maxWS.x, _minWS.y, _minWS.z);
      P[19].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[20].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[21].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[22].set(_minWS.x, _minWS.y, _maxWS.z);
      P[23].set(_minWS.x, _maxWS.y, _maxWS.z);
      _bufP.generate(P, 24, 3);
   }
   
   setCenterAndRadius();
}
//-----------------------------------------------------------------------------
//! Calculates center & radius of the bounding sphere around the AABB
void SLAABBox::setCenterAndRadius()
{  
   _centerWS =  _minWS;
   _centerWS += _maxWS;
   _centerWS *= 0.5f;
   SLVec3f ext(_maxWS-_centerWS);
   _radiusWS = ext.length();
   
   _centerOS =  _minOS;
   _centerOS += _maxOS;
   _centerOS *= 0.5f;
   ext.set(_maxOS-_centerOS);
   _radiusOS = ext.length();
}
//-----------------------------------------------------------------------------
//! Draws the AABB in world space with lines in a color
void SLAABBox::drawWS(const SLCol3f color)
{  
   // Create buffer object once
   if (!_bufP.id())
   {  
      SLVec3f P[24];  // vertex positions
      P[ 0].set(_minWS.x, _minWS.y, _minWS.z); // lower rect
      P[ 1].set(_maxWS.x, _minWS.y, _minWS.z);
      P[ 2].set(_maxWS.x, _minWS.y, _minWS.z);
      P[ 3].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[ 4].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[ 5].set(_minWS.x, _minWS.y, _maxWS.z);
      P[ 6].set(_minWS.x, _minWS.y, _maxWS.z);
      P[ 7].set(_minWS.x, _minWS.y, _minWS.z);
      P[ 8].set(_minWS.x, _maxWS.y, _minWS.z); // upper rect
      P[ 9].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[10].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[11].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[12].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[13].set(_minWS.x, _maxWS.y, _maxWS.z);
      P[14].set(_minWS.x, _maxWS.y, _maxWS.z);
      P[15].set(_minWS.x, _maxWS.y, _minWS.z); // vertical lines
      P[16].set(_minWS.x, _minWS.y, _minWS.z);
      P[17].set(_minWS.x, _maxWS.y, _minWS.z);
      P[18].set(_maxWS.x, _minWS.y, _minWS.z);
      P[19].set(_maxWS.x, _maxWS.y, _minWS.z);
      P[20].set(_maxWS.x, _minWS.y, _maxWS.z);
      P[21].set(_maxWS.x, _maxWS.y, _maxWS.z);
      P[22].set(_minWS.x, _minWS.y, _maxWS.z);
      P[23].set(_minWS.x, _maxWS.y, _maxWS.z);
      _bufP.generate(P, 24, 3);
   }
   
   if (!_bufP.id()) return;
   
   _bufP.drawArrayAsConstantColorLines(color);
}
//-----------------------------------------------------------------------------
//! SLAABBox::isHitInWS: Ray - AABB Intersection Test in object space
#define SL_RAY_AABB_FYFFE
SLbool SLAABBox::isHitInOS(SLRay* ray)
{     
   //See: "An Efficient and Robust Ray Box Intersection Algorithm"
   //by Amy L. Williams, Steve Barrus, R. Keith Morley, Peter Shirley
   //This test is about 10% faster than the test from Woo
   //It need the pre computed values invDir and sign in SLRay

   SLVec3f  params[2] = {_minOS, _maxOS};
   SLfloat  tymin, tymax, tzmin, tzmax;

   ray->tmin = (params[  ray->signOS[0]].x - ray->originOS.x) * ray->invDirOS.x;
   ray->tmax = (params[1-ray->signOS[0]].x - ray->originOS.x) * ray->invDirOS.x;
   tymin     = (params[  ray->signOS[1]].y - ray->originOS.y) * ray->invDirOS.y;
   tymax     = (params[1-ray->signOS[1]].y - ray->originOS.y) * ray->invDirOS.y;

   if ((ray->tmin > tymax) || (tymin > ray->tmax)) return false;
   if (tymin > ray->tmin) ray->tmin = tymin;
   if (tymax < ray->tmax) ray->tmax = tymax;

   tzmin = (params[  ray->signOS[2]].z - ray->originOS.z) * ray->invDirOS.z;
   tzmax = (params[1-ray->signOS[2]].z - ray->originOS.z) * ray->invDirOS.z;

   if ((ray->tmin > tzmax) || (tzmin > ray->tmax)) return false;
   if (tzmin > ray->tmin) ray->tmin = tzmin;
   if (tzmax < ray->tmax) ray->tmax = tzmax;
   
   return ((ray->tmin < ray->length) && (ray->tmax > 0));
 }

//-----------------------------------------------------------------------------
//! SLAABBox::isHitInWS: Ray - AABB Intersection Test in world space
SLbool SLAABBox::isHitInWS(SLRay* ray)
{     
   //See: "An Efficient and Robust Ray Box Intersection Algorithm"
   //by Amy L. Williams, Steve Barrus, R. Keith Morley, Peter Shirley
   //This test is about 10% faster than the test from Woo
   //It need the pre computed values invDir and sign in SLRay
   SLVec3f  params[2] = {_minWS, _maxWS};
   SLfloat  tymin, tymax, tzmin, tzmax;

   ray->tmin = (params[  ray->sign[0]].x - ray->origin.x) * ray->invDir.x;
   ray->tmax = (params[1-ray->sign[0]].x - ray->origin.x) * ray->invDir.x;
   tymin     = (params[  ray->sign[1]].y - ray->origin.y) * ray->invDir.y;
   tymax     = (params[1-ray->sign[1]].y - ray->origin.y) * ray->invDir.y;

   if ((ray->tmin > tymax) || (tymin > ray->tmax)) return false;
   if (tymin > ray->tmin) ray->tmin = tymin;
   if (tymax < ray->tmax) ray->tmax = tymax;

   tzmin = (params[  ray->sign[2]].z - ray->origin.z) * ray->invDir.z;
   tzmax = (params[1-ray->sign[2]].z - ray->origin.z) * ray->invDir.z;

   if ((ray->tmin > tzmax) || (tzmin > ray->tmax)) return false;
   if (tzmin > ray->tmin) ray->tmin = tzmin;
   if (tzmax < ray->tmax) ray->tmax = tzmax;
   
   return ((ray->tmin < ray->length) && (ray->tmax > 0));
}

//-----------------------------------------------------------------------------
//! Merges the bounding box bb to this one by extending this one axis aligned
void SLAABBox::merge(SLAABBox &bb)
{  
   if (bb.minWS()!=SLVec3f::ZERO && bb.maxWS()!=SLVec3f::ZERO)
   {  
      _minWS.setMin(bb.minWS());
      _maxWS.setMax(bb.maxWS());
   }
}
//-----------------------------------------------------------------------------
