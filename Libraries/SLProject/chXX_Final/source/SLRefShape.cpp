//#############################################################################
//  File:      SLRefShape.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#include "SLGroup.h"
#include "SLRefShape.h"
#include "SLRay.h"

//-----------------------------------------------------------------------------
/*!
The SLRefShape constructor stores the refShape & increments the ref. counter.
*/
SLRefShape::SLRefShape(SLShape* refShape, SLstring name) : SLShape(name)
{  
   assert(refShape != 0 && "No node passed as reference");
   assert(refShape != this && "A reference node can not reference itself");
   ////////assert(typeid(*refShape)!=typeid(SLGroup) && "Only non-group can be referenced");
   _refShape = refShape;
   _refShape->incRefs();
}
//-----------------------------------------------------------------------------
/*!
The SLRefShape destructor decrements the refShape counter and deletes the 
refShape if it is not anymore linked to.
*/
SLRefShape::~SLRefShape() 
{  _refShape->decRefs();
   
   // delete only if not referenced and not in a list anymore
   if (!_refShape->refs() && 
       !_refShape->next() && 
       !_refShape->prev()) 
       delete _refShape;
}
//-----------------------------------------------------------------------------
/*!
The SLRefShape::shapeInit checks the validity of the referenced node. To avoid 
endless loops a refShape node is not allowed to refShape its ancestors. An 
ancestor of a refShape node is group node followed along the previous pointers 
with lower depth than the depth of the refShape node.
Do not initialize the referenced shape twice.
*/
void SLRefShape::shapeInit(SLSceneView* sv)
{  (void)sv; // avoid unused parameter warning

   // cummulate my wm with referenced object transform (m)
   SLShape* ref = (SLShape*)_refShape;
   _wm *= ref->m();
   _wmI.setMatrix(_wm.inverse());
   _wmN.setMatrix(_wmI.mat3());
   _wmN.transpose();
   
   // set transparency flag
   _aabb.hasAlpha(((SLShape*)_refShape)->aabb()->hasAlpha());
   
   // check circular references
   SLNode* parent = this->parent();
   while (parent)
   {  if (parent==_refShape)
         SL_EXIT_MSG("Reference node produces a never ending loop.");
      parent = parent->parent();
   }
}
//-----------------------------------------------------------------------------
/*!
SLRefShape::buildAABB returns the AABB of the shape reference node
*/
SLAABBox& SLRefShape::buildAABB()
{  
   // recalc my AABBs with MinMax of the referenced shape with my wm
   SLShape* ref = (SLShape*)_refShape;
   assert(ref->aabb()->radiusWS()!=0 && "Referenced node has no AABB");
   _aabb.fromOStoWS(ref->aabb()->minOS(), ref->aabb()->maxOS(), _wm);
   return _aabb;
}
//-----------------------------------------------------------------------------
/*!
SLRefShape::shapeDraw draws the reference
*/
void SLRefShape::shapeDraw(SLSceneView* sv) 
{  
   // Don't draw the referenced node. This wouldn't draw the references AABBs
   //_refShape->draw(sv);
   
   SLShape* ref = (SLShape*)_refShape; 
     
   stateGL->pushModelViewMatrix();
   stateGL->modelViewMatrix.multiply(ref->m());
   
   ref->shapeDraw(sv);        // other call now referneced shapeDraw not draw
   
   stateGL->popModelViewMatrix();   
}
//-----------------------------------------------------------------------------
/*!
SLRefShape::shapeHit calls the shapeHit method of the referenced shape. The ray
is in the OS of the reference node (not the referenced!)-
*/
SLbool SLRefShape::shapeHit(SLRay* ray)
{  if (((SLShape*)_refShape)->shapeHit(ray))
   {  
      // In case the referenced shape was hit set the hitShape now to this
      if (ray->hitShape == (SLShape*)_refShape)
      {  ray->hitShape = (SLShape*)this;
         return true;
      }
   }
   return false;
}
//-----------------------------------------------------------------------------
/*!
SLRefShape::updateStats updates the statistics
*/
void SLRefShape::updateStats(SLGroup* parent)
{  assert(parent != 0);   
   parent->numRefShapes++;
   parent->numShapes++;
}
//-----------------------------------------------------------------------------
/*!
SLRefShape::preShade
*/
void SLRefShape::preShade(SLRay* ray)
{  
   _refShape->preShade(ray);
}
//-----------------------------------------------------------------------------
