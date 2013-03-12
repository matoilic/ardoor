//#############################################################################
//  File:      SL3DSMesh.cpp
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
#include "SL3DSMesh.h"

//-----------------------------------------------------------------------------
SL3DSMatFaces::~SL3DSMatFaces()
{  
   delete[] F;
   delete[] I;
}
//-----------------------------------------------------------------------------
SL3DSSmoothFaces::~SL3DSSmoothFaces()
{  
   delete[] F;
   delete[] I;
}
//-----------------------------------------------------------------------------
/*! The ctor sets the parent mesh group and initialises everything to 0.
*/
SL3DSMesh::SL3DSMesh(SLGroup* parent, SLstring meshName)
{  
   name = meshName;
   g = parent;
   P = 0;
   N = 0;
   Tc = 0;
   SMF = 0;
   numV = 0;
   numSMF = 0;
}
//-----------------------------------------------------------------------------
/*! The destructor deletes all arrays and clears the STL vectors
*/
SL3DSMesh::~SL3DSMesh()
{  delete[] P;
   delete[] N;
   delete[] Tc;
   delete[] SMF;

   for (SLuint i=0; i<matF.size(); ++i) 
   {  delete matF[i];
   }
   matF.clear();

   for (SLuint i=0; i<smoF.size(); ++i) 
   {  delete smoF[i];
   }
   smoF.clear();
}

//-----------------------------------------------------------------------------
/*! If no normals are passed from a file format such as 3DS the normals are 
generated here grouped by the smooth face group.
*/
void SL3DSMesh::calcNormals()
{
   //create array for the normals & Zero out the normals array
   delete[] N;
   N = new SLVec3f[numV];
   for (SLuint i = 0; i < numV; ++i) N[i] = SLVec3f::ZERO;   
   
   SLVec3f triSize = SLVec3f::ZERO;
  
   if (smoF.size() > 0)
   {
      // Loop through the smoothgroups & its vertex indexes
      for (SLuint iSF = 0; iSF < smoF.size(); ++iSF)
      {  SLuint numiFV = smoF[iSF]->numF*3;
         for (SLuint iFV = 0; iFV < numiFV; ++iFV)
         {  // Get the 3 vertex indexes
            SLushort iVA = smoF[iSF]->I[iFV++];
            SLushort iVB = smoF[iSF]->I[iFV++];
            SLushort iVC = smoF[iSF]->I[iFV  ];

            // Calculate the face's normal
            SLVec3f e1, e2, n;

            e1.sub(P[iVB],P[iVC]);  // e1 = B - C
            e2.sub(P[iVB],P[iVA]);  // e2 = B - A
            n.cross(e1,e2);         // n = e1 x e2

            // Add this normal to its verts' normals
            N[iVA] += n;
            N[iVB] += n;
            N[iVC] += n;
         }
      }
   } 
   else // if there are no smooth groups all faces belong to the same 
   {  
      // Loop through the material faces & its vertex indexes
      for (SLuint iMF = 0; iMF < matF.size(); ++iMF)
      {  for (SLuint iFV = 0; iFV < (SLuint)matF[iMF]->numF*3; ++iFV)
         {  // Get the 3 vertex indexes
            SLushort iVA = matF[iMF]->I[iFV++];
            SLushort iVB = matF[iMF]->I[iFV++];
            SLushort iVC = matF[iMF]->I[iFV  ];

            // Calculate the face's normal
            SLVec3f e1, e2, n;

            e1.sub(P[iVB],P[iVC]);  // e1 = B - C
            e2.sub(P[iVB],P[iVA]);  // e2 = B - A
            n.cross(e1,e2);         // n = e1 x e2

            // Add this normal to its vertices normals
            N[iVA] += n;
            N[iVB] += n;
            N[iVC] += n;
         }
      }
      
   }
   
   // normalize vertex normals
   for (SLuint i=0; i < numV; ++i) N[i].normalize();
   
}
//-----------------------------------------------------------------------------
/*! If a file format such as 3DS stores a local object matrix. It can be
applied to its vertices in world coordinates.
*/
void SL3DSMesh::applyInverseLocalMatrix()
{  
   SLMat4f invLM = om.inverse();
   
   // Transform vertices back to local coordinates
   for (SLuint i=0; i<numV; ++i)
   {  P[i] = invLM * P[i];
   }
}

//-----------------------------------------------------------------------------
/*! fixVertexRedundancy: Consolidation of vertex list among and within the
different smoothgroups. For correct normal calculation 2 conditions must be
fullfilled:
1) A vertex is only allowed to be used in one smoothgroup. For points that are
   on the border of two smoothgroups two vertices one in each smoothgroup must
   be defined. Only this way two normals are generated for each vertex so that
   a sharp edge is rendered.
2) A vertex must be unique within a smoothgroup. For correct smooth shading a 
   point must be only define by one single vertex.
Unfortunately 3DStudioMax doesn't fullfill both rules when exporting its meshes
to a 3DS file.  
*/
void SL3DSMesh::fixVertexRedundancy(SLint depth, SLint dumpLevel)
{  
   //tab string
   SLstring tab;
   for (SLint d=0; d<depth; ++d) tab+="   ";
   
   //Create an array per vertex for its smoothgroup
   SLint* vertexSG = new SLint[numV];
   
   ////////////////
   // Fix Rule 1 //
   ////////////////
   
   //SL_LOG("\n%siFV: ", tab.c_str());
   //for (SLuint i=0; i<numiFV; ++i) SL_LOG("%u,",iFV[i]);
   //SL_LOG("\n");
      
   if (smoF.size() > 0)
   {  // Initialize to an impossible smoothgroup
      for (SLuint iV=0; iV<numV; ++iV) vertexSG[iV] = -1;
   
      //STL vector for new vertices
      std::vector<SLuint> errSG, errSGiFV, errSGiF;
      
      // Loop thourgh all vertex indexes and determine its smooth group
      for (SLuint iSF=0; iSF<smoF.size(); ++iSF)
      {  for (SLuint iFV=0; iFV < (SLuint)smoF[iSF]->numF*3; ++iFV)
         {  SLuint sgiFV = smoF[iSF]->I[iFV];
            SLuint sgiF  = smoF[iSF]->F[iFV/3];
            
            if (vertexSG[sgiFV]==-1) vertexSG[sgiFV] = iSF;
            else if ((SLuint)vertexSG[sgiFV]!=iSF)
            {  errSG.push_back(iSF);
               errSGiFV.push_back(sgiFV);
               errSGiF.push_back(sgiF);
            }
         }
      } 
   
      //Fix false vertices by adding new ones
      //Allocate new memory & copy the existing ones into it
      SLuint newNumV = numV + errSG.size();
      SLVec3f* tmpV = new SLVec3f[newNumV];
      memcpy(tmpV, P, numV*sizeof(SLVec3f));
      SLVec2f* tmpT = 0;
      if (Tc)
      {  tmpT = new SLVec2f[newNumV];
         memcpy(tmpT, Tc, numV*sizeof(SLVec2f));
      }
      
      //resize temporary vertex smoothgroup array
      SLint* tmpVertexSG = new SLint[newNumV];
      memcpy(tmpVertexSG, vertexSG, numV*sizeof(SLint));    
      for (SLuint iV=numV; iV<newNumV; ++iV) tmpVertexSG[iV] = -1;
      delete[] vertexSG;
      vertexSG = tmpVertexSG; 
      
      if (dumpLevel==3) SL_LOG("\n");
      
      // copy the vertex into the appended ones
      for (SLuint e=0; e<errSG.size(); ++e)
      {  if (dumpLevel==3)
         {  SL_LOG("%s", tab.c_str());
            SL_LOG("Error in SG[%u]: wrong I:%u in face:%u\n", errSG[e], errSGiFV[e], errSGiF[e]);
         }
         SLuint newiV = numV + e;
         
         //copy vertex & texcoords
         tmpV[newiV] = P[errSGiFV[e]];
         if (Tc) tmpT[newiV] = Tc[errSGiFV[e]];
         
         //set the smoothgroup of the new vertex
         vertexSG[newiV] = errSG[e];
         
         //correct smoF I
         for (SLuint i=0; i < (SLuint)smoF[errSG[e]]->numF*3; ++i)
         {  if (smoF[errSG[e]]->I[i]==errSGiFV[e]) 
            {  smoF[errSG[e]]->I[i] = newiV;
               break;
            }
         }
         
         //correct matF I
         SLuint iFe = errSGiF[e];
         SLuint iMF = SMF[iFe].iMF;
         for (SLuint i=0; i<matF[iMF]->numF; ++i)
         {  if (matF[iMF]->F[i] == errSGiF[e])
            {  SLuint iiFV = i*3;
               if (matF[iMF]->I[iiFV  ]==errSGiFV[e]) matF[iMF]->I[iiFV  ] = newiV;
               if (matF[iMF]->I[iiFV+1]==errSGiFV[e]) matF[iMF]->I[iiFV+1] = newiV;
               if (matF[iMF]->I[iiFV+2]==errSGiFV[e]) matF[iMF]->I[iiFV+2] = newiV;
            }
         }
      }
   
      //delete the old vertex array and reassign the pointer to the new one
      delete[] P;
      P = tmpV;
      delete[] Tc;
      Tc = tmpT;
      
      numV = newNumV;
      
      if (dumpLevel==2)
      {  SL_LOG("%s", tab.c_str());
         SL_LOG("Non-redundant vertices across smoothgroups: %u\n", (SLuint)errSG.size());
      }
      if (dumpLevel==3)
      {  SL_LOG("\n%s", tab.c_str());
         SL_LOG("Smoothgroups after fix 1:\n");
         for (SLuint iSF=0; iSF<smoF.size(); ++iSF)
         {  SL_LOG("%s", tab.c_str());
            SL_LOG("SmoothGroup[%u]:I=", iSF);
            for (SLuint iFV=0; iFV < (SLuint)smoF[iSF]->numF*3; ++iFV)
            {  SL_LOG("%u,", smoF[iSF]->I[iFV]);
            }
            SL_LOG("  F=%u", iSF);
            for (SLuint iF=0; iF < (SLuint)smoF[iSF]->numF*3; ++iF)
            {  SL_LOG("%u,", smoF[iSF]->F[iF]);
            }
            SL_LOG("\n");
         }
      }
   }
   else // no smoothgroup chunk > assign them to the dummy smoothgroup 0
   {  for (SLuint iV=0; iV<numV; ++iV) vertexSG[iV] = -1;
   }
   
   /* dump vertices
   for (SLuint i=0; i < numV; ++i)
   {  SL_LOG("P[%d].xyz=(%7.3f,%7.3f,%7.3f)\n", i/3, P[i].x, P[i].y, P[i].z);
   }
   */
   
   ////////////////
   // Fix Rule 2 //
   ////////////////
   
   // allocate temporary array per vertex for marking the redundant vertices.
   SLuint redundantVertices = 0;
   SLint* vertexRedundant   = new SLint[numV];
   for (SLuint iV=0; iV<numV; ++iV) vertexRedundant[iV] = -1;
   
   //SL_LOG("\n%siFV: ", tab.c_str());
   //for (SLuint i=0; i<numiFV; ++i) SL_LOG("%u,",iFV[i]);
   //SL_LOG("\n");
   
   if (dumpLevel>2)
   {  SL_LOG("\n%s", tab.c_str());
      SL_LOG("Redundant vertices:\n");
   }   
   
   // Loop through all vertices and compare them in a inner loop
   // with all following vertices to check for equality.
   for (SLuint iV1=0; iV1 < numV; ++iV1)
   {  for (SLuint iV2=iV1+1; iV2 < numV; ++iV2)
      {  if (vertexSG[iV1]!=-1 && vertexSG[iV1]==vertexSG[iV2] && vertexRedundant[iV1]==-1)
         {  if(P[iV1] == P[iV2])
            {  redundantVertices++;
               vertexRedundant[iV2] = iV1;
               
               if (dumpLevel>2)
               {  SL_LOG("%s", tab.c_str());
                  SL_LOG("SG[%u]: P[%u] copy of P[%u]\n", vertexSG[iV2], iV2, iV1); 
               }
               
               // correct the smoothgroups I list
               if (smoF.size() > 0)
               {  for (SLuint i=0; i < (SLuint)smoF[vertexSG[iV2]]->numF*3; ++i)
                  {  if (smoF[vertexSG[iV2]]->I[i] == iV2) 
                        smoF[vertexSG[iV2]]->I[i] = iV1;
                  }
               }
               // correct the matF I list
               for (SLuint iMF=0; iMF<matF.size(); ++iMF)
               {  for (SLuint i=0; i < (SLuint)matF[iMF]->numF*3; ++i)
                  {  if (matF[iMF]->I[i] == iV2) 
                        matF[iMF]->I[i] = iV1;
                  }
               }          
            }
         }
      }
   }
   
   //SL_LOG("\n%siFV: ", tab.c_str());
   //for (SLuint i=0; i<numiFV; ++i) SL_LOG("%u,",iFV[i]);
   //SL_LOG("\n");
    
   if (dumpLevel==2)
   {  SL_LOG("%s", tab.c_str());
      SL_LOG("Redundant vertices within smoothgroups: %u\n", redundantVertices);
   }
   if (dumpLevel==3)
   {  SL_LOG("\n%s", tab.c_str());
      SL_LOG("Smoothgroups after fix 2:\n");
      for (SLuint iSF=0; iSF<smoF.size(); ++iSF)
      {  SL_LOG("%s", tab.c_str());
         SL_LOG("SmoothGroup[%u]:I=", iSF);
         for (SLuint i=0; i < (SLuint)smoF[iSF]->numF*3; ++i)
         {  SL_LOG("%u,", smoF[iSF]->I[i]);
         }
         SL_LOG("  F=%u", iSF);
         for (SLuint i=0; i<smoF[iSF]->numF; ++i)
         {  SL_LOG("%u,", smoF[iSF]->F[i]);
         }
         SL_LOG("\n");
      }
   }
   
   ///////////////////////////////////
   //   delete redundant vertices   //
   ///////////////////////////////////
   
   if (redundantVertices)
   {
      SLuint newNumV = numV - redundantVertices;
      SLVec3f* tmpV = new SLVec3f[newNumV];
      SLVec2f* tmpT = 0;
      if (Tc) tmpT = new SLVec2f[newNumV];
      SLint* iNewV = new SLint[numV];
      SLuint iVnew=0;
      
      // Loop over all old vertices and copy the ones that are in use
      if (Tc)
      {  for (SLuint iV=0; iV<numV; ++iV) 
         {  if (vertexRedundant[iV] == -1)
            {  iNewV[iV] = iVnew;      
               tmpV[iVnew] = P[iV];
               tmpT[iVnew] = Tc[iV];
               iVnew++;
            } else 
            {  iNewV[iV] = -1;
            }
         }
      } else
      {  for (SLuint iV=0; iV<numV; ++iV) 
         {  if (vertexRedundant[iV] == -1)
            {  iNewV[iV] = iVnew;      
               tmpV[iVnew] = P[iV];
               iVnew++;
            } else 
            {  iNewV[iV] = -1;
            }
         }
      }
      
      // reindex the smoothgroups I
      if (smoF.size() > 0)
      {  for (SLuint iSF=0; iSF<smoF.size(); ++iSF)
         {  for (SLuint i=0; i < (SLuint)smoF[iSF]->numF*3; ++i)
            {  SLuint ix = smoF[iSF]->I[i];
               smoF[iSF]->I[i] = iNewV[ix];
            }
         }
      }
      
      // reindex the matF I
      if (matF.size() > 0)
      {  for (SLuint iMF=0; iMF<matF.size(); ++iMF)
         {  for (SLuint i=0; i < (SLuint)matF[iMF]->numF*3; ++i)
            {  SLuint ix = matF[iMF]->I[i];
               matF[iMF]->I[i] = iNewV[ix];
            }
         }
      }
      
      // delete old vertex array & reassign new
      delete[] P;
      P = tmpV;
      numV = newNumV;
      delete[] iNewV;
      delete[] Tc;
      Tc = tmpT;
   }

   
   // delete temporary allocations
   delete[] vertexSG;
   delete[] vertexRedundant;
}
