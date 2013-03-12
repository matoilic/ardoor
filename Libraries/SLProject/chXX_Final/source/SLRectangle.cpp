//#############################################################################
//  File:      SLRectangle.cpp
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

#include "SLRectangle.h"

//-----------------------------------------------------------------------------
//! SLRectangle ctor with min & max corners and its resolutions 
SLRectangle::SLRectangle(SLVec2f min, SLVec2f max,
                         SLint resX, SLint resY, 
                         SLstring name,
                         SLMaterial* mat) : SLMesh(name) 
{  assert(min!=max);
   assert(resX>0);
   assert(resY>0);
   assert(name!="");
   _min = min;
   _max = max;
   _tmin.set(0,0);
   _tmax.set(1,1);
   _resX = resX;
   _resY = resY;
   _isVolume = true;
   buildMesh(mat);
}
//-----------------------------------------------------------------------------
//! SLRectangle ctor with min & max corners and its resolutions 
SLRectangle::SLRectangle(SLVec2f min, SLVec2f max,
                         SLVec2f tmin, SLVec2f tmax,
                         SLint resX, SLint resY, 
                         SLstring name,
                         SLMaterial* mat) : SLMesh(name) 
{  assert(min!=max);
   assert(tmin!=tmax);
   assert(resX>0);
   assert(resY>0);
   assert(name!="");
   _min = min;
   _max = max;
   _tmin = tmin;
   _tmax = tmax;
   _resX = resX;
   _resY = resY;
   buildMesh(mat);
}
//-----------------------------------------------------------------------------
//! SLRectangle::shapeCopy returns a deep copy of the SLRectangle object. 
SLShape* SLRectangle::shapeCopy()
{  return new SLRectangle(SLVec2f(_min.x, _min.y), 
                          SLVec2f(_max.x, _max.y), 
                          _tmin, _tmax, _resX, _resY, _name, M->mat);
}
//-----------------------------------------------------------------------------
//! SLRectangle::buildMesh fills in the underlying arrays from the SLMesh object
void SLRectangle::buildMesh(SLMaterial* mat)
{  
   deleteData();
   
   // Check max. allowed no. of verts
   SLuint uIntNumV = (_resX+1) * (_resY+1);
   if (uIntNumV >= 65535) 
      SL_EXIT_MSG("SLRectangle::buildMesh: NO. of vertices exceeds the maximum (65535) allowed.");

   // allocate new arrays of SLMesh
   numV = (_resX+1) * (_resY+1);
   numF = _resX * _resY * 2;
   numM = 1;
   P = new SLVec3f[numV];
   N = new SLVec3f[numV];
   Tc = new SLVec2f[numV];
   F = new SLFace[numF];
   
   // Calculate normal from the first 3 corners
   SLVec3f maxmin(_max.x, _min.y, 0);
   SLVec3f minmax(_min.x, _max.y, 0);
   SLVec3f e1(maxmin - _min);
   SLVec3f e2(minmax - _min);
   SLVec3f curN(e1^e2);
   curN.normalize();
   
   //Set one default material index
   M = new SLMatFaces[numM];
   M->numF = numF;
   M->startF = 0;
   M->mat = mat;
   
   // define delta vectors dX & dY and deltas for texCoord dS,dT
   SLVec3f dX = e1 / (SLfloat)_resX;
   SLVec3f dY = e2 / (SLfloat)_resY;
   SLfloat dS = (_tmax.x - _tmin.x) / (SLfloat)_resX;
   SLfloat dT = (_tmax.y - _tmin.y) / (SLfloat)_resY;

   // Build vertex data
   SLuint i = 0;
   for (SLint y=0; y<=_resY; ++y)
   {  
      SLVec3f curV = _min;
      SLVec2f curT = _tmin;
      curV   += (SLfloat)y*dY;
      curT.y += (SLfloat)y*dT;

      for (SLint x=0; x<=_resX; ++x, ++i)
      {  
         P[i]  = curV;
         Tc[i] = curT;
         N[i]  = curN;
         curV   += dX;
         curT.x += dS;
      }      
   }
   
   // Build face vertex indexes
   SLuint v = 0, f = 0; //index for vertices and faces
   for (SLint y=0; y<_resY; ++y)
   {  
      for (SLint x=0; x<_resX; ++x, ++v)
      {  // triangle 1
         F[f].iA = v;
         F[f].iB = v+_resX+2;
         F[f].iC = v+_resX+1;
         f++;
         // triangle 2
         F[f].iA = v;
         F[f].iB = v+1;
         F[f].iC = v+_resX+2;
         f++;
      }      
      v++;
   }
}
//-----------------------------------------------------------------------------
