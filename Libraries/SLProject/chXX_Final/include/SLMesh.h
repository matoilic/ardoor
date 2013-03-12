//#############################################################################
//  File:      SLMesh.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLMESH_H
#define SLMESH_H

#include <stdafx.h>
#include "SLShape.h"
#include "SLAABBox.h"
#include "SLGLBuffer.h"

class SLSceneView;
class SLGroup;
class SLRay;
class SLAccelStruct;

//-----------------------------------------------------------------------------
//! SLFace stores the 3 vertex indexes of the triangle
/*! 
The array F in SLMesh holds all triangles indexes for OpenGL rendering. It is 
used as the vertex index array for vertex buffer rendering. The faces in F are
sorted by material.
*/
struct SLFace
{  SLushort iA;      //!< Unsigned short index of the 1st triangle vertex
   SLushort iB;      //!< Unsigned short index of the 2nd triangle vertex
   SLushort iC;      //!< Unsigned short index of the 3rd triangle vertex
};
//-----------------------------------------------------------------------------
//! SLMatFaces stores the faces data per material
struct SLMatFaces
{  SLushort startF;  //!< start index in the face vertex index array F
   SLushort numF;    //!< No. of faces (triangles)
   SLMaterial* mat;  //!< pointer to material in scene material
};
//-----------------------------------------------------------------------------
//!The SLMesh class represents a triangle mesh object w. a face-vertex list.
/*!
The SLMesh class represents a single triangle mesh object. The vertex 
attributes are stored in arrays with equal number (numV) of elements:
\n P (vertex position)
\n N (vertex normals)
\n Tc (vertex texture coordinates) optional
\n T (vertex tangents) optional \n
The max. number of vertices is 65535 as their index is stored as a USHORT.
The triangle vertex indexes are stored in the array F sorted by material.
The array M holds per material a struct of the type SLMatFaces that tells us
witch material is used (mat), how many faces use the material (numF) and
where the triangle vertex index begins (startF) in the array F.\n
For each attribute array vertex buffer object (VBO) is used and encapsulated
in SLGLBuffer.
*/      
class SLMesh: public SLShape 
{  public:                    
                              SLMesh         (SLstring name = "Mesh");
                             ~SLMesh         ();
               
               void           shapeInit      (SLSceneView* sv);
               void           shapeDraw      (SLSceneView* sv);
               SLShape*       shapeCopy      ();
               void           updateStats    (SLGroup* parent);
               SLAABBox&      buildAABB      ();
               SLbool         shapeHit       (SLRay* ray);               
               void           preShade       (SLRay* ray);
               
               void           deleteData     ();
               void           calcNormals    ();
               void           calcTangents   ();
               void           calcMinMax     (SLVec3f &minV, SLVec3f &maxV);
               void           calcCenterRad  (SLVec3f& center, SLfloat& radius);
               SLbool         hitTriangleOS  (SLRay* ray, SLushort iT);
                              
               SLVec3f*       P;       //!< Array of vertex positions
               SLVec3f*       N;       //!< Array of vertex normals
               SLVec2f*       Tc;      //!< Array of vertex tex. coords. (opt.)
               SLVec4f*       T;       //!< Array of vertex tangents (opt.)
               
               SLFace*        F;       //!< Array of face vertex indexes
               SLMatFaces*    M;       //!< Array of materials triangle groups
               
               SLushort       numV;    //!< Number of elements in P, N, T, B & Tc   
               SLuint         numF;    //!< Number of elements in F           
               SLushort       numM;    //!< Number of elements in M
   
   protected:
               SLGLBuffer     _bufP;   //!< Buffer for vertex positions
               SLGLBuffer     _bufN;   //!< Buffer for vertex normals
               SLGLBuffer     _bufTc;  //!< Buffer for vertex texcoords
               SLGLBuffer     _bufT;   //!< Buffer for vertex tangents
               SLGLBuffer     _bufF;   //!< Buffer for face vertex indexes
               
               SLGLBuffer     _bufN2;  //!< Buffer for normal line rendering
               SLGLBuffer     _bufT2;  //!< Buffer for tangent line rendering
               
               /*! flag if mesh is a closed volume. This flag decides
               backfaces are intersected by rays.*/
               SLbool         _isVolume;
               
               SLAccelStruct* _accelStruct;  //!< KD-tree or uniform grid
};
//-----------------------------------------------------------------------------
#endif //SLMESH_H

