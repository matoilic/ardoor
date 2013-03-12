//#############################################################################
//  File:      SLGroup.cpp
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
#include "SLMaterial.h"
#include "SLSceneView.h"
#include "SLRay.h"
#include "SLNode.h"
#include "SLCamera.h"

//-----------------------------------------------------------------------------
SLGroup::SLGroup(SLstring name) : SLShape(name) 
{  
   _first = _last = 0;
   numNodes = 0;
}
//-----------------------------------------------------------------------------
SLGroup::~SLGroup()
{  deleteAll();
}
//-----------------------------------------------------------------------------
/*!
SLGroup::addNode adds a node at the end of the linked list.
*/
void SLGroup::addNode(SLNode* toAdd)
{  assert(toAdd != 0);
   assert(toAdd != this && "A group can not a add itself as a children");
   assert(toAdd->prev() == 0 && "The node to be added is already in a group");

   // add node at the end 
   if (_last == 0)
   {  _first = _last = toAdd;
      toAdd->prev(this);
   } else 
   {  toAdd->prev(_last);
      toAdd->next(0);
      _last->next(toAdd);
      _last = toAdd;
   }
   toAdd->parent(this);
   numNodes++;
}
//-----------------------------------------------------------------------------
/*!
SLGroup::insertNode inserts a node after a specified node in the linked list.
*/
void SLGroup::insertNode(SLNode* toInsert, SLNode* after)
{  assert(after != 0);
   assert(toInsert != 0);
   assert(toInsert != this && "A group can not a insert itself as a children");
   assert(toInsert->prev() == 0 && "toInsert is already in a group");

   // insert node
   if (after == _last)
      _last = toInsert;
   else
      after->next()->prev(toInsert);
   
   toInsert->next(after->next()); 
   toInsert->prev(after);
   toInsert->parent(this);
   after->next(toInsert);
   numNodes++;
}
//-----------------------------------------------------------------------------
/*!
SLGroup::deleteNode deletes a specified node in the group. If this node is
referenced by a refShape node it is not deleted but only unlinked from the 
groups linked list.
*/
void SLGroup::deleteNode(SLNode* toDelete)  
{  assert(toDelete != 0);
   assert(toDelete->prev() != 0 && "toDelete is not in a group");
   assert(_first != 0 && "Nothing to delete");
  
   // unlink toDelete
   if (toDelete == _first)                         // unlink first
   {  if (_first->next())
      {  _first = _first->next();
         _first->prev(this);
      } else
      {  _first = _last = 0;
      } 
   } else                                          // unlink last
   if (toDelete == _last)
   {  _last = toDelete->prev();
      _last->next(0);
   } else                                          // unlink inbetween
   {  toDelete->prev()->next(toDelete->next());
      toDelete->next()->prev(toDelete->prev());
   }

   // check references before delete
   if (toDelete->refs() == 0)       // delete node if not referenced anymore
   {  delete toDelete;
   } else                           // else decrement refShape counter
   {  toDelete->prev(0);
      toDelete->next(0);
      toDelete->parent(0);
   }
   numNodes--;                          
}
//-----------------------------------------------------------------------------
/*!
SLGroup::deleteAll() deletes all child nodes.
*/
void SLGroup::deleteAll()
{  while (_first) deleteNode(_first);
}
//-----------------------------------------------------------------------------
/*!
SLGroup::printStats() prints the statistic info of this group.
*/
void SLGroup::printStats()
{  
   SLfloat voxelsEmpty  = numVoxels ? (SLfloat)numVoxEmpty / 
                                      (SLfloat)numVoxels*100.0f : 0;
   SLfloat avgTriPerVox = numVoxels ? (SLfloat)numRTTriangles / 
                                      (SLfloat)(numVoxels-numVoxEmpty) : 0;
   SL_LOG("RTVertices        : %d\n", numRTVertices);
   SL_LOG("RTTriangles       : %d\n", numRTTriangles);
   SL_LOG("Voxels            : %d\n", numVoxels);
   SL_LOG("Voxels empty      : %4.1f%%\n", voxelsEmpty); 
   SL_LOG("Avg. Tria/Voxel   : %4.1f\n", avgTriPerVox);
   SL_LOG("Max. Tria/Voxel   : %d\n", numVoxMaxTria);
   SL_LOG("MBytes in Meshes  : %f\n", (SLfloat)numBytes / 1000000.0f);
   SL_LOG("Groups            : %d\n", numGroups);
   SL_LOG("Shapes            : %d\n", numShapes);
   SL_LOG("RefShapes         : %d\n", numRefShapes);
   SL_LOG("Lights            : %d\n", numLights);
   SL_LOG("\n");
}
//-----------------------------------------------------------------------------
/*!
SLGroup::shapeInit loops over all child nodes and calls their init method with
an incremented depth. While looping it must be checked that all child nodes
have a depth equal the groups depth + 1.
*/
void SLGroup::shapeInit(SLSceneView* sv)
{  SLNode* current = _first;
   while (current)
   {  if (current->depth() && current->depth() != depth()+1) 
      {  SL_EXIT_MSG("Scenegraph is not directed acyclic. There is a loop.");
      }
      current->init(sv, depth()+1);
      
      // Set transparent flags of the group
      if (!_aabb.hasAlpha() && ((SLShape*)current)->aabb()->hasAlpha())
         _aabb.hasAlpha(true); 
         
      current = current->next();
   }
}
//-----------------------------------------------------------------------------
/*!
SLGroup::shapeDraw loops over all child nodes and calls the their draw method.
Groups are only traversed for opaque objects. Transparent objects are rendere
extra afterwards.
*/
void SLGroup::shapeDraw(SLSceneView* sv)
{  SLNode* current = _first;
   
   while (current)
   {  current->draw(sv);   
      current = current->next();
   }
}
//-----------------------------------------------------------------------------
/*!
SLGroup::shapeCopy loops over all child nodes and calls their copy method.
A new group with all copied child node is returned.
*/
SLShape* SLGroup::shapeCopy()
{  SLGroup* copy = new SLGroup(name());
   SLNode* current = _first;
   
   while (current)
   {  copy->addNode(current->copy());
      current = current->next();
   }
   return copy;
}
//-----------------------------------------------------------------------------
/*!
SLGroup::updateStats updates the statistic counters of a group. 
Be aware that when you add, insert or delete nodes in group only the 
numChildren counter is updated automatically. For the correct counters of the 
root3D group you have to call its updateStats method first. 
Achtung: the light node is counted in numShapes and numLights.
*/
void SLGroup::updateStats(SLGroup* parent)
{  SLNode* current = _first;
   numBytes      = 0;
   numBytesAccel = 0;
   numGroups     = 1;
   numShapes     = 0;
   numRefShapes  = 0;
   numLights     = 0;
   numTriangles  = 0;
   numRTTriangles= 0;
   numRTVertices = 0;
   numVoxels     = 0;
   numVoxEmpty   = 0;
   numVoxMaxTria = 0;
   

   while (current)
   {  current->updateStats(this);
      current = current->next();
   }
   
   if (parent)
   {  ((SLGroup*)parent)->numBytes      += numBytes;
      ((SLGroup*)parent)->numBytesAccel += numBytesAccel;
      ((SLGroup*)parent)->numGroups     += numGroups;
      ((SLGroup*)parent)->numShapes     += numShapes;
      ((SLGroup*)parent)->numRefShapes  += numRefShapes;
      ((SLGroup*)parent)->numTriangles  += numTriangles;
      ((SLGroup*)parent)->numRTTriangles+= numRTTriangles;
      ((SLGroup*)parent)->numRTVertices += numRTVertices;
      ((SLGroup*)parent)->numVoxels     += numVoxels;
      ((SLGroup*)parent)->numVoxEmpty   += numVoxEmpty;
      if (numVoxMaxTria > ((SLGroup*)parent)->numVoxMaxTria)
         ((SLGroup*)parent)->numVoxMaxTria = numVoxMaxTria;
   }
}
//-----------------------------------------------------------------------------
/*!
SLGroup::getNode finds a node by a name
*/
SLNode* SLGroup::getNode(SLstring name) 
{  SLNode* current = _first;
   
   while (current)
   {  if(current->name() == name) return current;
      if (typeid(*current)==typeid(SLGroup))
      {  SLNode* found = ((SLGroup*)current)->getNode(name);
         if (found) return found;
      }
      current = current->next();
   }
   return 0;
}
//-----------------------------------------------------------------------------
/*!
SLGroup::intersect loops over all child nodes of the group and calls their
intersect method.  
*/
SLbool SLGroup::shapeHit(SLRay* ray)
{  assert(ray != 0);

   SLNode* current = _first;
   SLbool wasHit = false;
   while (current)
   {  
      // do not test origin node for shadow rays 
      if (!(current==ray->originShape && ray->type==SHADOW))
      {  if (current->hit(ray) && !wasHit) wasHit = true;
      }
      
      if (ray->isShaded()) return true;
      current = current->next();
   }
   return wasHit;
}
//-----------------------------------------------------------------------------
/*!
SLGroup::buildAABB() loops over all child nodes and merges their AABB
to the axis aligned bounding box of the group.
*/
SLAABBox& SLGroup::buildAABB()
{  SLNode* current = _first;
   
   // empty the groups AABB    
   _aabb.minWS(SLVec3f( SL_FLOAT_MAX, SL_FLOAT_MAX, SL_FLOAT_MAX));
   _aabb.maxWS(SLVec3f(-SL_FLOAT_MAX,-SL_FLOAT_MAX,-SL_FLOAT_MAX));

   while (current)
   {  _aabb.merge(current->buildAABB());
      current = current->next();
   }
   
   _aabb.fromWStoOS(_aabb.minWS(), _aabb.maxWS(), _wmI);
   return _aabb;
}
//-----------------------------------------------------------------------------
