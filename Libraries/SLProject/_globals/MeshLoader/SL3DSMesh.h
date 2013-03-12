//#############################################################################
//  File:      SL3DSMesh.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLMesh3DS_H
#define SLMesh3DS_H

#include <stdafx.h>
#include "SLGroup.h"

//-----------------------------------------------------------------------------
//!Holds the list of faces with the same material used for 3DS file loading.
/*!The SLMatFace3DS instance holds an array of face indexes and the face vertex
indexes of faces with the same material that is identified with an index into
the material array of the scene. The triangle primitives of
the mesh object are rendered with vertex arrays of the SLMesh object and the 
index array (I) of this SLMatFace instance. In other words the triangles are
rendered per material to reduce the expensive material changes for the OpenGL
pipeline.
*/ 
class SL3DSMatFaces 
{  public:
                              SL3DSMatFaces(SLuint iMaterial =-1)
                              {  iMat = iMaterial;
                                 F = I = 0;
                                 numF = 0;
                              } 
                             ~SL3DSMatFaces();
               
               /*! index to material array in meshgroup. If this index is -1
               then the currently active material in the scene graph is used.*/
               SLshort        iMat; //!< index to material array
               SLushort*      F;    //!< Array of face indexes
               SLushort*      I;    //!< Array of indexes of face vertices
               SLushort       numF; //!< No. of elements in F    
};
typedef std::vector<SL3DSMatFaces*>  SLVMatFaces3DS;
//-----------------------------------------------------------------------------
//!Holds the list of faces from a smooth group used for 3DS file loading.
/*!The SLSmoothFace3DS instance holds during the initialisation phase (loading,
consolidation & normal calculation the data of all faces that belong to this
smooth group. See also the SLMesh class for more information. After 
initialisation only the originial smoothgroup number from the file is kept.
*/
class SL3DSSmoothFaces 
{  public:
                              SL3DSSmoothFaces(SLulong sgID=0)
                              {  ID = sgID;
                                 F = I = 0;
                                 numF = 0;
                              } 
                             ~SL3DSSmoothFaces();
                              
               SLulong        ID;   //!< Smooth group number from 3DS file
               SLushort*      F;    //!< Temp. array of face indexes
               SLushort*      I;    //!< Temp. array of face vertex indexes
               SLushort       numF; //!< Temp. no. of elements in F
};
typedef std::vector<SL3DSSmoothFaces*>  SLVSmoFaces3DS;
//-----------------------------------------------------------------------------
//! Holds a smooth face group and material group for a triangle used for 3DS file loading.
/*! A simple struct type per face to assign its smooth face group and material
face group. This type is only used when a mesh has more than one material or
more than one smooth group.
*/
struct SL3DSFace
{  SLuint   iSF;  //!< Smooth group index
   SLuint   iMF;  //!< MatFace index
};//-----------------------------------------------------------------------------
//! Represents a triangle mesh object within a group node used for 3DS file loading.
/*!
The SL3DSMesh class represents a triangle 3DS-mesh object within a group node.
- The vertex data is stored in arrays with equal number (numV) of elements:
  - P (position)
  - N (normals)
  - Tc (texture coordinates) optional
- The data of triangles (faces) that use the specific material is stored in the
  in an instance of the class SLMatFaces. A mesh holds for each material an
  SLMatFaces instance in the SLAMatFaces STL vector. The vertex indexes of each
  triangle is stored in the unsigned integer array I.
- The vertex normals are often not stored in file formats and have to be 
  calculated for every vertex. For smooth shading vertex normals are averaged
  from neighbouring triangle normals. But sometimes there are sharp edges,
  e.g. the edges of a cube, where we do not want to average the normals at the
  corners. Therefore many file formats (3DS, OBJ) know so called smooth groups
  that hold the faces for witch the normals must be averaged. Because we only
  can assign one normal to one vertex, neighbouring smootgroups have on the 
  edge duplicated vertices with different normals (one per smoothgroup). 
  A corner of a cube must be created therefore with 3 vertices at the same 
  point but with 3 different normals, one for each face. Unfortunately this 
  and other rules are not always fullfilled by certain file formats and must 
  be corrected. See the method fixVertexRedundancy for more details.
*/      
class SL3DSMesh 
{  public:                     
                              SL3DSMesh(SLGroup* parent=0, 
                                        SLstring name = "Mesh");
                             ~SL3DSMesh();
			      
               void           fixVertexRedundancy(SLint depth=0,
                                                  SLint dumpLevel=0);
               void           calcNormals();
               void           applyInverseLocalMatrix();
               
               SLstring       name;    //!< name of the mesh
               SLGroup*       g;       //!< parent group
               SLMat4f        om;      //!< mesh object matrix
               SLVec3f*       P;       //!< Array of vertex position
               SLVec3f*       N;       //!< Array of vertex normals
               SLVec2f*       Tc;      //!< Array of vertex texture coords. (optional)
               SLushort       numV;    //!< Number of elements in P, N & Tc
               SL3DSFace*     SMF;     //!< Array of faces w. matF & smoF index
               SLushort       numSMF;  //!< Number of elements in SMF
               
               SLVMatFaces3DS matF;    //!< vector of material faces               
               SLVSmoFaces3DS smoF;    //!< vector of smooth faces
};
typedef std::vector<SL3DSMesh*>  SLVMesh3DS;
//-----------------------------------------------------------------------------
#endif //SLMesh3DS_H
