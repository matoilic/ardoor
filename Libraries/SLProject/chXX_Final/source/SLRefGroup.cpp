//#############################################################################
//  File:      SLRefGroup.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#include "SLRefGroup.h"
#include "SLRefShape.h"
#include "SLRay.h"

//-----------------------------------------------------------------------------
/*!
The SLRefGroup constructor stores the refShape & increments the ref. counter.
*/
SLRefGroup::SLRefGroup(SLGroup* refGroup, SLstring name) : SLGroup(name)
{  assert(refGroup != 0 && "No node passed as reference");
   assert(refGroup != this && "A reference node can not reference itself");
   assert(typeid(*refGroup)==typeid(SLGroup) && "Only groups can be referenced");
   _refGroup = refGroup;
   _refGroup->incRefs();
}
//-----------------------------------------------------------------------------
/*!
The SLRefGroup destructor decrements the refShape counter and deletes the 
refShape if it is not anymore linked to.
*/
SLRefGroup::~SLRefGroup() 
{  _refGroup->decRefs();
   
   // delete only if not referenced and not in a list anymore
   if (!_refGroup->refs() && 
       !_refGroup->next() && 
       !_refGroup->prev()) 
       delete _refGroup;
}
//-----------------------------------------------------------------------------
/*!
The SLRefGroup::shapeInit checks the validity of the referenced node. To avoid 
endless loops a refShape node is not allowed to refShape its ancestors. An 
ancestor of a refShape node is group node followed along the previous pointers 
with lower depth than the depth of the refShape node.
*/
void SLRefGroup::shapeInit(SLSceneView* sv)
{  
   // cummulate wm with referenced wm
   SLShape* ref = (SLShape*)_refGroup; 
   _wm *= ref->m();
   _wmI.setMatrix(_wm.inverse());
   _wmN.setMatrix(_wmI.mat3());
   _wmN.transpose();
   
   // check circular references
   SLNode* parent = this->parent();
   while (parent)
   {  if (parent==_refGroup)
         SL_EXIT_MSG("Reference node produces a never ending loop.");
      parent = parent->parent();
   }
   
   // set transparency flag
   _aabb.hasAlpha(((SLShape*)_refGroup)->aabb()->hasAlpha());
   
   // delete all child references
   if (_first) deleteAll();
      
   // loop through the referenced group and add a SLRefShape or SLRefGroup
   SLNode* current = ((SLGroup*)_refGroup)->first();
   while (current)
   {  if (typeid(*current)==typeid(SLGroup))
           addNode(new SLRefGroup((SLGroup*)current, name()+"_"+current->name()));
      else addNode(new SLRefShape((SLShape*)current, name()+"_"+current->name()));
      ((SLShape*)_last)->wm(_wm);
      ((SLShape*)_last)->depth(depth()+1);
      ((SLShape*)_last)->shapeInit(sv);
      current = current->next();
   }
}
//-----------------------------------------------------------------------------
/*!
SLRefGroup::buildAABB returns the AABB of the reference
*/
SLAABBox& SLRefGroup::buildAABB()
{  
   _aabb = SLGroup::buildAABB();
   return _aabb;
}
//-----------------------------------------------------------------------------
/*!
SLRefGroup::shapeDraw draws the reference
*/
void SLRefGroup::shapeDraw(SLSceneView* sv) 
{  
   // Don't draw the referenced node. This wouldn't draw the references AABBs
   //_refGroup->draw(sv);
   
   SLShape* ref = (SLShape*)_refGroup;    
   
   stateGL->pushModelViewMatrix();
   stateGL->modelViewMatrix.multiply(ref->m());
   
   SLGroup::shapeDraw(sv);    // in case of a group call the groups drawing loop
   
   stateGL->popModelViewMatrix();   
}
//-----------------------------------------------------------------------------
/*!
SLRefGroup::shapeHit calls the shapeHit method of the referenced group.
*/
SLbool SLRefGroup::shapeHit(SLRay* ray)
{  
   // loop through child references an hit each
   SLNode* current = _first;
   SLbool wasHit = false;
   while (current)
   {  if (((SLShape*)current)->hit(ray) && !wasHit) wasHit = true;
      current = current->next();
   }
   return wasHit;
}
//-----------------------------------------------------------------------------
/*!
SLRefGroup::updateStats updates the statistics
*/
void SLRefGroup::updateStats(SLGroup* parent)
{  
   SLGroup::updateStats(parent);
}
//-----------------------------------------------------------------------------
/*!
SLRefGroup::preShade
*/
void SLRefGroup::preShade(SLRay* ray)
{  
   _refGroup->preShade(ray);
}
//-----------------------------------------------------------------------------
