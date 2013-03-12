//#############################################################################
//  File:      SLBox.cpp
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

#include "SLBox.h"
#include "SLGroup.h"
#include "SLMaterial.h"
#include "SLRay.h"

//-----------------------------------------------------------------------------
//! SLBox::SLBox ctor with min. & max. coords. of the axis aligned box
SLBox::SLBox(SLfloat minx, SLfloat miny, SLfloat minz,
             SLfloat maxx, SLfloat maxy, SLfloat maxz,
             SLstring name, SLMaterial* mat) : SLMesh(name)
{  
   _min.set(minx, miny, minz);
   _max.set(maxx, maxy, maxz);
   buildMesh(mat);
}
//-----------------------------------------------------------------------------
//! SLBox::SLBox ctor with min. & max. vectors of the axis aligned box
SLBox::SLBox(SLVec3f min, SLVec3f max, 
             SLstring name, SLMaterial* mat) : SLMesh(name)
{  
   _min.set(min);
   _max.set(max);
   buildMesh(mat);
}
//-----------------------------------------------------------------------------
//! SLBox::shapeCopy returns a deep copy of the SLBox object. 
SLShape* SLBox::shapeCopy()
{  return new SLBox(_min, _max, _name);
}
//-----------------------------------------------------------------------------
//! SLBox::buildMesh fills in the underlying arrays from the SLMesh object
void SLBox::buildMesh(SLMaterial* mat)
{  
   deleteData();
   
   // allocate new arrays of SLMesh
   numV = 24; // 6 sides with 4 vertices
   numF = 12; // 6 sides with 2 triangles
   numM = 1;  // 1 default material
   P = new SLVec3f[numV];
   N = new SLVec3f[numV];
   Tc = 0; // no texcoords
   F = new SLFace[numF];
    
   //Set one default material index
   M = new SLMatFaces[numM];
   M->numF = numF;
   M->startF = 0;
   M->mat = mat;
   
   SLint i=0, f=0;
   
   // predifined normals
   SLVec3f NX = SLVec3f( 1, 0, 0);
   SLVec3f NY = SLVec3f( 0, 1, 0);
   SLVec3f NZ = SLVec3f( 0, 0, 1);
   
   // towards +x
   P[i].x=_max.x; P[i].y=_max.y; P[i].z=_max.z; N[i]= NX; i++;  //  0
   P[i].x=_max.x; P[i].y=_min.y; P[i].z=_max.z; N[i]= NX; i++;  //  1
   P[i].x=_max.x; P[i].y=_min.y; P[i].z=_min.z; N[i]= NX; i++;  //  2
   P[i].x=_max.x; P[i].y=_max.y; P[i].z=_min.z; N[i]= NX; i++;  //  3
   F[f].iA = 0; F[f].iB = 1; F[f].iC = 2; f++;
   F[f].iA = 0; F[f].iB = 2; F[f].iC = 3; f++;
   
   // towards -z
   P[i].x=_max.x; P[i].y=_max.y; P[i].z=_min.z; N[i]=-NZ; i++;  //  4
   P[i].x=_max.x; P[i].y=_min.y; P[i].z=_min.z; N[i]=-NZ; i++;  //  5
   P[i].x=_min.x; P[i].y=_min.y; P[i].z=_min.z; N[i]=-NZ; i++;  //  6
   P[i].x=_min.x; P[i].y=_max.y; P[i].z=_min.z; N[i]=-NZ; i++;  //  7
   F[f].iA = 4; F[f].iB = 5; F[f].iC = 6; f++;
   F[f].iA = 4; F[f].iB = 6; F[f].iC = 7; f++;
   
   // towards -x
   P[i].x=_min.x; P[i].y=_max.y; P[i].z=_max.z; N[i]=-NX; i++;  //  8
   P[i].x=_min.x; P[i].y=_max.y; P[i].z=_min.z; N[i]=-NX; i++;  //  9
   P[i].x=_min.x; P[i].y=_min.y; P[i].z=_min.z; N[i]=-NX; i++;  // 10
   P[i].x=_min.x; P[i].y=_min.y; P[i].z=_max.z; N[i]=-NX; i++;  // 11
   F[f].iA = 8; F[f].iB = 9;  F[f].iC = 10; f++;
   F[f].iA = 8; F[f].iB = 10; F[f].iC = 11; f++;
   
   // towards +z
   P[i].x=_max.x; P[i].y=_max.y; P[i].z=_max.z; N[i]= NZ; i++;  // 12
   P[i].x=_min.x; P[i].y=_max.y; P[i].z=_max.z; N[i]= NZ; i++;  // 13
   P[i].x=_min.x; P[i].y=_min.y; P[i].z=_max.z; N[i]= NZ; i++;  // 14
   P[i].x=_max.x; P[i].y=_min.y; P[i].z=_max.z; N[i]= NZ; i++;  // 15
   F[f].iA = 12; F[f].iB = 13; F[f].iC = 14; f++;
   F[f].iA = 12; F[f].iB = 14; F[f].iC = 15; f++;
   
   // towards +y
   P[i].x=_max.x; P[i].y=_max.y; P[i].z=_max.z; N[i]= NY; i++;  // 16
   P[i].x=_max.x; P[i].y=_max.y; P[i].z=_min.z; N[i]= NY; i++;  // 17
   P[i].x=_min.x; P[i].y=_max.y; P[i].z=_min.z; N[i]= NY; i++;  // 18
   P[i].x=_min.x; P[i].y=_max.y; P[i].z=_max.z; N[i]= NY; i++;  // 19
   F[f].iA = 16; F[f].iB = 17; F[f].iC = 18; f++;
   F[f].iA = 16; F[f].iB = 18; F[f].iC = 19; f++;
   
   // towards -y
   P[i].x=_min.x; P[i].y=_min.y; P[i].z=_max.z; N[i]=-NY; i++;  // 20
   P[i].x=_min.x; P[i].y=_min.y; P[i].z=_min.z; N[i]=-NY; i++;  // 21
   P[i].x=_max.x; P[i].y=_min.y; P[i].z=_min.z; N[i]=-NY; i++;  // 22
   P[i].x=_max.x; P[i].y=_min.y; P[i].z=_max.z; N[i]=-NY; i++;  // 23   
   F[f].iA = 20; F[f].iB = 21; F[f].iC = 22; f++;
   F[f].iA = 20; F[f].iB = 22; F[f].iC = 23; f++;
}
//-----------------------------------------------------------------------------
bool SLBox::onKeyPress(const SLKey modKey)
{  
   switch ((SLuint)modKey)
   {  case KeyTab:         
         _min *= 1.1f;
         _max *= 1.1f;
         buildMesh(M->mat);
         shapeInit(0);
         buildAABB();
         return true;
      case (SLuint)KeyCtrl|(SLuint)KeyTab:
         _min *= 0.9f;
         _max *= 0.9f;
         buildMesh(M->mat);
         shapeInit(0);
         buildAABB();
         return true;
      default:                      
         return false;
   }
   return false;
}
//-----------------------------------------------------------------------------
