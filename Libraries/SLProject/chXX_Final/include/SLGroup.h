//#############################################################################
//  File:      SLGroup.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGROUP_H
#define SLGROUP_H

#include <stdafx.h>
#include "SLShape.h"
#include "SLAABBox.h"

class SLSceneView;
class SLNode;
class SLRay;

//-----------------------------------------------------------------------------
//! Group node representing a group a children nodes in the scenegraph.
/*!      
The SLGroup represents a group of children nodes in the scenegraph. Because it
is a node it has a draw method that just calls all its children's draw methods.
Because it is derived from SLShape it also has it's own local transformation.
*/
//-----------------------------------------------------------------------------
class SLGroup : public SLShape
{  public:
                           SLGroup     (SLstring name="Group");
   virtual                ~SLGroup     ();
               
               void        shapeInit   (SLSceneView* sv);
               void        shapeDraw   (SLSceneView* sv);
               SLShape*    shapeCopy   ();
               void        updateStats (SLGroup* parent);               
               SLAABBox&   buildAABB   ();
               SLbool      shapeHit    (SLRay* ray);
               void        preShade    (SLRay* ray){(void)ray;}

   virtual     void        addNode     (SLNode* toAdd);
               void        deleteNode  (SLNode* toDelete);
               void        unlinkNode  (SLNode* toUnlink);
               void        insertNode  (SLNode* toInsert, SLNode* after);
               void        deleteAll   ();
               void        printStats  ();

               // Getters
               SLNode*     first       (){return _first;}
               SLNode*     last        (){return _last;}
               SLNode*     getNode     (SLstring name);

               // public counters that can be updated by childrens
               SLuint      numNodes;      //!< NO. of children nodes
               SLuint      numBytes;      //!< NO. of bytes allocated
               SLuint      numBytesAccel; //!< NO. of bytes in accel. structs
               SLuint      numGroups;     //!< NO. of groups in group
               SLuint      numShapes;     //!< NO. of visible shapes in group
               SLuint      numRefShapes;  //!< NO. of referenced shapes
               SLuint      numLights;     //!< NO. of lights in group
               SLuint      numTriangles;  //!< NO. of triangles in group
               SLuint      numRTTriangles;//!< NO. of ray tracing triangles
               SLuint      numRTVertices; //!< NO. of ray tracing vertices
               SLuint      numVoxels;     //!< NO. of voxels
               SLfloat     numVoxEmpty;   //!< NO. of empty voxels
               SLuint      numVoxMaxTria; //!< Max. no. of trng per voxel
   protected:    
               SLNode*     _first;        //!< Pointer to the first child node
               SLNode*     _last;         //!< Pointer to the last child node
};
//-----------------------------------------------------------------------------
#endif
