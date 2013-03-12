//#############################################################################
//  File:      SLMesh.cpp
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

#include "SLGroup.h"
#include "SLMesh.h"
#include "SLRay.h"
#include "SLRaytracer.h"
#include "SLSceneView.h"
#include "SLCamera.h"
#include "SLUniformGrid.h"
#include "SLLightSphere.h"
#include "SLLightRect.h"
#include "SLGLShaderProg.h"
#include "TriangleBoxIntersect.h"

//-----------------------------------------------------------------------------
/*! 
The ctor sets the parent scene and initialises everything to 0.
*/
SLMesh::SLMesh(SLstring name) : SLShape(name)
{  
   P  = 0;
   N  = 0;
   T  = 0;
   Tc = 0;
   M  = 0;
   F  = 0;
   numV = 0;
   numF = 0;
   numM = 0;
   
   _isVolume = true; // is used for RT to decide inside/outside
   
   _accelStruct = new SLUniformGrid(this);
}
//-----------------------------------------------------------------------------
//! The destructor deletes all VBO's and C-arrays
SLMesh::~SLMesh()
{  
   deleteData();   
   if (_accelStruct) delete _accelStruct; 
}
//-----------------------------------------------------------------------------
//! SLMesh::deleteData deletes all mesh data and vbo's
void SLMesh::deleteData()
{
   // delete existing arrays
   delete[] P;  P=0;
   delete[] N;  N=0;
   delete[] T;  T=0;
   delete[] Tc; Tc=0;
   delete[] M;  M=0;
   delete[] F;  F=0;
}
//-----------------------------------------------------------------------------
//! SLMesh::shapeInit sets the transparency flag of the AABB
void SLMesh::shapeInit(SLSceneView* sv)
{  
   (void)sv; // avoid unused parameter warning
   
   if (P && N)
   {  // loop through material faces and check for opaqueness & transparency
      for (SLuint m = 0; m < numM; ++m)
      {  
         // check if the mesh has it own material
         if (M[m].mat) SLMaterial::current = M[m].mat;
         else M[m].mat = SLMaterial::current;
         
         // be sure that we have a material at this point
         assert(SLMaterial::current && "No current material set!");
                  
         // set transparent flag of the mesh
         if (!_aabb.hasAlpha() && M[m].mat->hasAlpha()) 
            _aabb.hasAlpha(true);
         
         // build tangents for bump mapping
         if (M[m].mat->needsTangents() && Tc && T==0)
            calcTangents();
      }
   }
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::shapeDraw does the OpenGL rendering of the mesh. The triangle 
primitives are rendered per material (SLMatFaces) with the vertex array P, 
the normal array N, the array Tc and the face vertex index array F. 
Optionally you can draw the normals and/or the uniform grid voxels.
*/
void SLMesh::shapeDraw(SLSceneView* sv)
{  
   if (P && N)
   {     
      ////////////////////////
      // 1: Check drawing bits
      ////////////////////////
            
      // Return if hidden
      if (sv->drawBits()->get(SL_DB_HIDDEN) || _drawBits.get(SL_DB_HIDDEN)) 
         return;
              
      // Set polygon mode
      SLPrimitive primitiveType = SL_TRIANGLES;
      if (sv->drawBits()->get(SL_DB_POLYGONLINE) || 
         _drawBits.get(SL_DB_POLYGONLINE))
      	primitiveType = SL_LINE_LOOP;
         
      // Set face culling      
      stateGL->cullFace(!(sv->drawBits()->get(SL_DB_CULLOFF) || 
                         _drawBits.get(SL_DB_CULLOFF)));
      
      // check if texture exists
      SLbool useTexture = Tc && !(sv->drawBits()->get(SL_DB_TEXOFF) || 
                                     _drawBits.get(SL_DB_TEXOFF));
                                     
      // enable polygonoffset if voxels are drawn to avoid stitching
      if (sv->drawBits()->get(SL_DB_VOXELS) || _drawBits.get(SL_DB_VOXELS))
      {  stateGL->polygonOffset(true, 1.0f, 1.0f);
      }


      //////////////////////
      // 2: Build VBO's once
      //////////////////////

      if (!_bufP.id()) _bufP.generate(P, numV, 3);
      if (!_bufN.id()) _bufN.generate(N, numV, 3);
      if (!_bufTc.id() && Tc) _bufTc.generate(Tc, numV, 2);
      if (!_bufT.id() && T) _bufT.generate(T, numV, 4);
      if (!_bufF.id() && F) _bufF.generate(F, numF, 3, SL_UNSIGNED_SHORT, SL_ELEMENT_ARRAY_BUFFER);    
       
      
      ////////////////////////////////
      // 3: Draw elements per material
      ////////////////////////////////

      for (SLuint m = 0; m < numM; ++m)
      {  
         // Opaque materials only in 1st non-blended pass
         // Transparent materials only in 2nd blended pass
         if ((!stateGL->blend() && !M[m].mat->hasAlpha()) || 
             ( stateGL->blend() &&  M[m].mat->hasAlpha()))
         {
            // 3.a: Apply mesh material if exists & differs from current
             if (M[m].mat != SLMaterial::current || SLMaterial::current->shaderProg()==0)
               M[m].mat->activate(stateGL, this);
            
            // 3.b: Pass the matrices to the shader program
            SLGLShaderProg* sp = SLMaterial::current->shaderProg();
            sp->uniformMatrix4fv("u_mvMatrix",    1, (SLfloat*)&stateGL->modelViewMatrix);
            sp->uniformMatrix4fv("u_mvpMatrix",   1, (SLfloat*)stateGL->mvpMatrix());
            sp->uniformMatrix3fv("u_nMatrix",     1, (SLfloat*)stateGL->normalMatrix());
            sp->uniformMatrix4fv("u_invMvMatrix", 1, (SLfloat*)stateGL->invModelViewMatrix());
                              
            // 3.c: Enable attribute pointers
            _bufP.bindAndEnableAttrib(sp->getAttribLocation("a_position"));
            _bufN.bindAndEnableAttrib(sp->getAttribLocation("a_normal"));
            if (_bufTc.id() && useTexture) 
               _bufTc.bindAndEnableAttrib(sp->getAttribLocation("a_texCoord"));
            if (_bufT.id())  
               _bufT.bindAndEnableAttrib(sp->getAttribLocation("a_tangent"));
   
            // 3.d: Finally draw elements
            _bufF.bindAndDrawElementsAs(primitiveType, M[m].numF*3, 
                                        M[m].startF*3*sizeof(SLushort));

            // 3.e: Disable attribute pointers
            _bufP.disableAttribArray();
            _bufN.disableAttribArray();
            if (_bufTc.id()) _bufTc.disableAttribArray();
            if (_bufT.id())  _bufT.disableAttribArray();
         }
      }
      
      //////////////////////////////////////
      // 4: Draw optional normals & tangents
      //////////////////////////////////////

      if (sv->drawBits()->get(SL_DB_NORMALS) || _drawBits.get(SL_DB_NORMALS))
      {  
         // scale the normals to 5% of the surounding sphere
         float r = _aabb.radiusOS() * 0.05f;
         SLVec3f* V2;
         
         if (!_bufN2.id())
         {  // scalefactor r from scaled radius for normals & tangents
            // build array between vertex and normal target point
            V2 = new SLVec3f[numV*2]; 
            for (SLuint i=0; i < numV; ++i)
            {  V2[i<<1] = P[i];
               V2[(i<<1)+1].set(P[i] + N[i]*r);
            }
            
            // Create buffer object for normals
            _bufN2.generate(V2, numV*2, 3);
            
            if (T)
            {  if (!_bufT2.id())
               {  for (SLuint i=0; i < numV; ++i)
                  {  V2[(i<<1)+1].set(P[i].x+T[i].x*r, 
                                      P[i].y+T[i].y*r, 
                                      P[i].z+T[i].z*r);
                  }
               
                  // Create buffer object for tangents
                  _bufT2.generate(V2, numV*2, 3);
               }
            } 
            delete[] V2;

         }
         _bufN2.drawArrayAsConstantColorLines(SLCol3f::BLUE);
         if (T) _bufT2.drawArrayAsConstantColorLines(SLCol3f::RED);
      } 
      else
      {  
         // release buffer objects for normal & tangent rendering
         if (_bufN2.id()) _bufN2.dispose();
         if (_bufT2.id()) _bufT2.dispose();
      }

      //////////////////////////////////////////
      // 5: Draw optional acceleration structure
      //////////////////////////////////////////

      if (_accelStruct) 
      {  if (sv->drawBits()->get(SL_DB_VOXELS) || _drawBits.get(SL_DB_VOXELS))
         {  _accelStruct->draw(sv);
            stateGL->polygonOffset(false);
         } else
         {  // Delete the visualization VBO if not rendered anymore
            _accelStruct->disposeBuffers();
         }
      }
            
      GET_GL_ERROR;
   } else
   {  SL_EXIT_MSG("SLMesh::shapeDraw: Arrays P or/and N empty");
   }
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::shapeCopy returns a deep copy of the mesh object. 
*/
SLShape* SLMesh::shapeCopy()
{  
   SLMesh* copy = new SLMesh(name());
   copy->numV   = numV;
   copy->numF   = numF;
   copy->numM   = numM;
   
   copy->P = new SLVec3f[numV];    
   memcpy(copy->P, P, numV*sizeof(SLVec3f));
   
   copy->N = new SLVec3f[numV];    
   memcpy(copy->N, N, numV*sizeof(SLVec3f));
   
   if (Tc)
   {  copy->Tc = new SLVec2f[numV];    
      memcpy(copy->Tc, Tc, numV*sizeof(SLVec2f));
   } else copy->Tc = 0;
   
   if (T)
   {  copy->T = new SLVec4f[numV];    
      memcpy(copy->T, T, numV*sizeof(SLVec4f));
   } else copy->T = 0;
   
   copy->F = new SLFace[numF];     
   memcpy(copy->F, F, numF*sizeof(SLFace));
   
   copy->M = new SLMatFaces[numM]; 
   memcpy(copy->M, M, numM*sizeof(SLMatFaces));
   return copy;
}
//-----------------------------------------------------------------------------
/*!
SLMesh::intersect does the ray-mesh intersection test. If no acceleration 
structure is defined all triangles are tested in a brute force manner.
*/
SLbool SLMesh::shapeHit(SLRay* ray)
{  
   // Avoid intersection of another mesh before the ray left this one
   if (!ray->isOutside && ray->originShape != this) return false;

   if (_accelStruct)
      return _accelStruct->intersect(ray);
   else
   {  // intersect against all faces
      SLbool wasHit = false;
      for (SLuint t=0; t<numF; ++t)
      {  if(hitTriangleOS(ray, t) && !wasHit) wasHit = true;
      }
      return wasHit;
   }
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::updateStats updates the parent groups statistics.
*/
void SLMesh::updateStats(SLGroup* parent)
{  assert(parent != 0);
   parent->numBytes += sizeof(SLMesh);                // myself
   parent->numBytes += numV*sizeof(SLVec3f)*2;        // P & N
   if (T) parent->numBytes += numV*sizeof(SLVec3f);   // T
   if (Tc) parent->numBytes += numV*sizeof(SLVec2f);  // Tc
   parent->numBytes += numF*sizeof(SLFace);           // F
   parent->numBytes += numM*sizeof(SLMatFaces);       // M
   
   parent->numShapes++;
   parent->numTriangles += numF;
   parent->numRTTriangles += numF;
   parent->numRTVertices += numV;
   
   if (typeid(*this)==typeid(SLLightSphere)) parent->numLights++;
   if (typeid(*this)==typeid(SLLightRect)) parent->numLights++;

   if (_accelStruct) 
      _accelStruct->updateStats(parent);
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::calcMinMax calculates the axis alligned minimum and maximum point
*/
void SLMesh::calcMinMax(SLVec3f &minV, SLVec3f &maxV)
{  // init min & max points
   minV.set( SL_FLOAT_MAX,  SL_FLOAT_MAX,  SL_FLOAT_MAX);
   maxV.set(-SL_FLOAT_MAX, -SL_FLOAT_MAX, -SL_FLOAT_MAX);

   // calc min and max point of all vertices
   for (SLuint i=0; i<numV; ++i)
   {  if (P[i].x < minV.x) minV.x = P[i].x; else // x component
      if (P[i].x > maxV.x) maxV.x = P[i].x; 
      if (P[i].y < minV.y) minV.y = P[i].y; else // y component
      if (P[i].y > maxV.y) maxV.y = P[i].y; 
      if (P[i].z < minV.z) minV.z = P[i].z; else // z component
      if (P[i].z > maxV.z) maxV.z = P[i].z;
   } 
}
//-----------------------------------------------------------------------------
/*!
SLMesh::calcCenterRad calculates the center and the radius of an almost minimal
bounding sphere. Code by Jack Ritter from Graphic Gems.
*/
void SLMesh::calcCenterRad(SLVec3f& center, SLfloat& radius)
{
   SLint    i;
   SLfloat  dx, dy, dz;
   SLfloat  radius2, xspan, yspan, zspan, maxspan;
   SLfloat  old_to_p, old_to_p_sq, old_to_new;
   SLVec3f  xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2;

   // FIRST PASS: find 6 minima/maxima points
   xmin.x = ymin.y = zmin.z=  SL_FLOAT_MAX;
   xmax.x = ymax.y = zmax.z= -SL_FLOAT_MAX;
   
   for (i=0; i<numV; ++i)
   {
      if (P[i].x < xmin.x) xmin = P[i]; else
      if (P[i].x > xmax.x) xmax = P[i];
      if (P[i].y < ymin.y) ymin = P[i]; else
      if (P[i].y > ymax.y) ymax = P[i];
      if (P[i].z < zmin.z) zmin = P[i]; else
      if (P[i].z > zmax.z) zmax = P[i];
   }
   
   // Set xspan = distance between the 2 points xmin & xmax (squared)
   dx = xmax.x - xmin.x;
   dy = xmax.y - xmin.y;
   dz = xmax.z - xmin.z;
   xspan = dx*dx + dy*dy + dz*dz;

   // Same for y & z spans
   dx = ymax.x - ymin.x;
   dy = ymax.y - ymin.y;
   dz = ymax.z - ymin.z;
   yspan = dx*dx + dy*dy + dz*dz;

   dx = zmax.x - zmin.x;
   dy = zmax.y - zmin.y;
   dz = zmax.z - zmin.z;
   zspan = dx*dx + dy*dy + dz*dz;

   // Set points dia1 & dia2 to the maximally separated pair
   dia1 = xmin; dia2 = xmax; // assume xspan biggest
   maxspan = xspan;
   if (yspan > maxspan)
   {  maxspan = yspan;
      dia1 = ymin; dia2 = ymax;
   }
   if (zspan > maxspan)
   {  dia1 = zmin; dia2 = zmax;
   }

   // dia1,dia2 is a diameter of initial sphere
   // calc initial center
   center.x = (dia1.x + dia2.x)*0.5f;
   center.y = (dia1.y + dia2.y)*0.5f;
   center.z = (dia1.z + dia2.z)*0.5f;
   
   // calculate initial radius*radius and radius
   dx = dia2.x - center.x; // x component of radius vector
   dy = dia2.y - center.y; // y component of radius vector
   dz = dia2.z - center.z; // z component of radius vector
   radius2 = dx*dx + dy*dy + dz*dz;
   radius  = sqrt(radius2);

   // SECOND PASS: increment current sphere
   for (i=0; i<numV; ++i)
   {
      dx = P[i].x - center.x;
      dy = P[i].y - center.y;
      dz = P[i].z - center.z;
      old_to_p_sq = dx*dx + dy*dy + dz*dz;
      
      if (old_to_p_sq > radius2) 	// do r**2 test first
      { 	
         // this point is outside of current sphere
         old_to_p = sqrt(old_to_p_sq);
         
         // calc radius of new sphere
         radius  = (radius + old_to_p) * 0.5f;
         radius2 = radius*radius; 	// for next r**2 compare
         old_to_new = old_to_p - radius;
         
         // calc center of new sphere
         center.x = (radius*center.x + old_to_new*P[i].x) / old_to_p;
         center.y = (radius*center.y + old_to_new*P[i].y) / old_to_p;
         center.z = (radius*center.z + old_to_new*P[i].z) / old_to_p;
         
         // Suppress if desired
         SL_LOG("\n New sphere: center,radius = %f %f %f   %f",
                center.x,center.y,center.z, radius);
      }
   }
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::buildAABB builds and returns the axis-aligned bounding box.
*/
SLAABBox& SLMesh::buildAABB()
{     
   // calculate min & max in object space
   SLVec3f minOS, maxOS;
   calcMinMax(minOS, maxOS);
   
   // enlarge aabb for avoiding rounding errors 
   minOS -= 0.01f;
   maxOS += 0.01f;
   
   // apply world matrix
   _aabb.fromOStoWS(minOS, maxOS, _wm);
   
   // build accelerations structure
   if (_accelStruct && numF > 5) 
      _accelStruct->build(minOS, maxOS);
      
   return _aabb;
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::calcNormals recalculates the normals only from the vertices.
This algorithms doesn't know anything about smoothgroups. It just loops over
the triangle of the material faces and sums up the normal for each of its
vertices. Note that the face normals are not normalized. The cross product of
2 vectors is proportional to the area of the triangle. Like this the normal of
big triangles are more weighted than small triangles and we get a better normal
quality. At the end all vertex normals are normalized.
*/
void SLMesh::calcNormals()
{
   // Create array for the normals & Zero out the normals array
   delete[] N;
   N = new SLVec3f[numV];
   for (SLuint i = 0; i < numV; ++i) N[i] = SLVec3f::ZERO;
     
   // Loop over all triangles
   for (SLuint f = 0; f < numF; ++f)
   {  
      // Calculate the face's normal
      SLVec3f e1, e2, n;
      
      // Calculate edges of triangle
      e1.sub(P[F[f].iB],P[F[f].iC]);   // e1 = B - C
      e2.sub(P[F[f].iB],P[F[f].iA]);   // e2 = B - A
      
      // Build normal with cross product but do NOT normalize it.
      n.cross(e1,e2);                  // n = e1 x e2

      // Add this normal to its vertices normals
      N[F[f].iA] += n;
      N[F[f].iB] += n;
      N[F[f].iC] += n;
   }
   
   // normalize vertex normals
   for (SLuint i=0; i < numV; ++i) N[i].normalize();
}
//-----------------------------------------------------------------------------
/*! 
SLMesh::calcTangents computes the tangent and bi-tangent per vertex used for 
GLSL normal map bumb mapping. The code and mathematical derivation is in detail 
explained in: http://www.terathon.com/code/tangent.html
*/
void SLMesh::calcTangents()
{
   if (P && N && Tc)
   {
      // allocat tangents
      delete[] T;
      T = new SLVec4f[numV];
      
      // allocate temp arrays for tangents
      SLVec3f* T1 = new SLVec3f[numV * 2];
      SLVec3f* T2 = T1 + numV;
      memset(T1, 0, numV * sizeof(SLVec3f) * 2);

      for (SLuint m = 0; m < numM; ++m)
      {  for (SLuint f = 0; f < M[m].numF; ++f)
         {
            // Get the 3 vertex indexes
            SLushort iVA = F[M[m].startF + f].iA;
            SLushort iVB = F[M[m].startF + f].iB;
            SLushort iVC = F[M[m].startF + f].iC;

            float x1 = P[iVB].x - P[iVA].x;
            float x2 = P[iVC].x - P[iVA].x;
            float y1 = P[iVB].y - P[iVA].y;
            float y2 = P[iVC].y - P[iVA].y;
            float z1 = P[iVB].z - P[iVA].z;
            float z2 = P[iVC].z - P[iVA].z;

            float s1 = Tc[iVB].x - Tc[iVA].x;
            float s2 = Tc[iVC].x - Tc[iVA].x;
            float t1 = Tc[iVB].y - Tc[iVA].y;
            float t2 = Tc[iVC].y - Tc[iVA].y;

            float r = 1.0F / (s1*t2 - s2*t1);
            SLVec3f sdir((t2*x1 - t1*x2) * r, (t2*y1 - t1*y2) * r, (t2*z1 - t1*z2) * r);
            SLVec3f tdir((s1*x2 - s2*x1) * r, (s1*y2 - s2*y1) * r, (s1*z2 - s2*z1) * r);

            T1[iVA] += sdir;
            T1[iVB] += sdir;
            T1[iVC] += sdir;

            T2[iVA] += tdir;
            T2[iVB] += tdir;
            T2[iVC] += tdir;
         }
      }
       
       for (SLuint i=0; i < numV; ++i)
       {
           // Gram-Schmidt orthogonalize
           T[i] = T1[i] - N[i] * N[i].dot(T1[i]);
           T[i].normalize();
           
           // Calculate temp. bitangent and store its handedness in T.w
           SLVec3f bitangent;
           bitangent.cross(N[i], T1[i]);
           T[i].w = (bitangent.dot(T2[i]) < 0.0f) ? -1.0f : 1.0f;
       }
       
       delete[] T1;
    }
}
//-----------------------------------------------------------------------------
/*!
SLMesh::hitTriangle is the fast and minimum storage ray-triangle intersection 
test by Tomas Möller and Ben Trumbore (Journal of graphics tools 2, 1997)
*/
SLbool SLMesh::hitTriangleOS(SLRay* ray, SLushort iT)
{  ++SLRay::tests;

   // prevent self-intersection of triangle
   if(ray->originTria == &F[iT]) return false;
      
   SLVec3f e1, e2;     // edge 1 and 2
   SLVec3f AO, K, Q;
         
   // find vectors for two edges sharing the triangle vertex A
   e1.sub(P[F[iT].iB], P[F[iT].iA]);
   e2.sub(P[F[iT].iC], P[F[iT].iA]);
         
   // begin calculating determinant - also used to calculate U parameter
   K.cross(ray->dirOS, e2);

   // if determinant is near zero, ray lies in plane of triangle
   const SLfloat det = e1.dot(K);
   
   SLfloat inv_det, t, u, v;
   
   // if ray is outside do test with face culling
   if (ray->isOutside && _isVolume)
   {  // check only front side triangles           
      if (det < SL_EPSILON) return false;

      // calculate distance from A to ray origin
      AO.sub(ray->originOS, P[F[iT].iA]);
            
      // Calculate barycentric coordinates: u>0 && v>0 && u+v<=1
      u = AO.dot(K);
      if (u < 0.0f || u > det) return false;

      // prepare to test v parameter
      Q.cross(AO, e1);

      // calculate v parameter and test bounds
      v = Q.dot(ray->dirOS);
      if (v < 0.0f || u+v > det) return false;
      
      // calculate intersection distance t
      inv_det = 1.0f / det;
      t = e2.dot(Q) * inv_det;
   
      // if intersection is closer replace ray intersection parameters
      if (t > ray->length || t < 0.0f) return false;
      
      ++SLRay::intersections;
      ray->length = t;
      
      // scale down u & v so that u+v<=1
      ray->hitU = u * inv_det;
      ray->hitV = v * inv_det;
   }
   else 
   {  // check front & backside triangles
      if (det < SL_EPSILON && det > -SL_EPSILON) return false;
      
      inv_det = 1.0f / det;
      
      // calculate distance from A to ray origin
      AO.sub(ray->originOS, P[F[iT].iA]);
      
      // Calculate barycentric coordinates: u>0 && v>0 && u+v<=1
      u = AO.dot(K) * inv_det;
      if (u < 0.0f || u > 1.0) return false;
      
      // prepare to test v parameter
      Q.cross(AO, e1);
      
      // calculate v parameter and test bounds
      v = Q.dot(ray->dirOS) * inv_det;
      if (v < 0.0f || u+v > 1.0f) return false;
      
      // calculate t, ray intersects triangle
      t = e2.dot(Q) * inv_det;
         
      // if intersection is closer replace ray intersection parameters
      if (t > ray->length || t < 0.0f) return false;
      
      ++SLRay::intersections;      
      ray->length = t;
      
      ray->hitU = u;
      ray->hitV = v;
   }

   ray->hitTriangle = &F[iT];
   ray->hitShape = (SLShape*)this;
   ray->hitMat = M[0].mat;
   
   // rare case: search for other materials if mesh has multiple materials
   for (SLuint m = 1; m < numM; ++m)
   {  if (iT < M[m].startF) break;
      ray->hitMat = M[m].mat; 
   } 
   
   return true;
}
//-----------------------------------------------------------------------------
/*!
SLMesh::preShade calculates the rest of the intersection information 
after the final hit point is determined. Should be called just before the 
shading when the final intersection point of the closest triangle was found.
*/
void SLMesh::preShade(SLRay* ray)
{
   SLFace* hit = ray->hitTriangle;
   
   // calculate the hit point in world space
   ray->hitPoint.set(ray->origin + ray->length * ray->dir);
      
   // calculate the interpolated normal with vertex normals in object space
   ray->hitNormal.set(N[hit->iA] * (1-(ray->hitU+ray->hitV)) + 
                      N[hit->iB] * ray->hitU + 
                      N[hit->iC] * ray->hitV);
                      
   // transform normal back to world space
   ray->hitNormal.set(ray->hitShape->wmN() * ray->hitNormal);
                 
   // invert normal if the ray is inside a shape
   if (!ray->isOutside) ray->hitNormal *= -1;
   
   // for shading the normal is expected to be unit length
   ray->hitNormal.normalize();
   
   // calculate interpolated texture coordinates
   SLVGLTexture& textures = ray->hitMat->textures();
   if (textures.size() > 0)
   {  SLVec2f Tu(Tc[hit->iB] - Tc[hit->iA]);
      SLVec2f Tv(Tc[hit->iC] - Tc[hit->iA]);
      SLVec2f tc(Tc[hit->iA] + ray->hitU*Tu + ray->hitV*Tv);
      ray->hitTexCol.set(textures[0]->getTexelf(tc.x,tc.y));
      
      // bumpmapping
      if (textures.size() > 1)
      {  if (T)
         {  
            // calculate the interpolated tangent with vertex tangent in object space
            SLVec4f hitT(T[hit->iA] * (1-(ray->hitU+ray->hitV)) + 
                         T[hit->iB] * ray->hitU + 
                         T[hit->iC] * ray->hitV);
                         
            SLVec3f T3(hitT.x,hitT.y,hitT.z);         // tangent with 3 components
            T3.set(ray->hitShape->wmN() * T3);        // transform tangent back to world space
            SLVec2f d = textures[1]->dsdt(tc.x,tc.y);  // slope of bumpmap at tc
            SLVec3f N = ray->hitNormal;               // unperturbated normal
            SLVec3f B(N^T3);                          // binormal tangent B
            B *= T[hit->iA].w;                        // correct handedness
            SLVec3f D(d.x*T3 + d.y*B);                // perturbation vector D
            N+=D;
            N.normalize();
            ray->hitNormal.set(N);
         }
      }
   }
}


//-----------------------------------------------------------------------------
