//#############################################################################
//  File:      SLShape.cpp 
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include <SLShape.h>
#include <SLGroup.h>
#include <SLCamera.h>
#include <SLSceneView.h>
#include <SLLightSphere.h>
#include <SLLightRect.h>
#include <SLRefGroup.h>
#include <SLRefShape.h>
#include <SLRay.h>
#include <SLButton.h>
#include <SLAnimation.h>

//-----------------------------------------------------------------------------
//! SLShape ctor inits all matrices to identity
SLShape::SLShape(SLstring name) : SLNode(name)
{  
   identity();
   _wm.identity();
   _wmI.identity();
   _wmN.identity();
   _drawBits.allOff();
   _animation = 0;
}
//-----------------------------------------------------------------------------
//! Deletes the optional animator instance
SLShape::~SLShape()
{
   if (_animation) 
      delete _animation;
}
//-----------------------------------------------------------------------------
/*!
SLShape::init sets the current depth of a node within the entire scene tree and 
calls the individual shapeInit methods. This can only be done with with a full 
dop-down scene traversal. The depth of every node is needed for testing that 
the scene tree is directed and acyclic. In other words this guarantees that 
there are no endless loops in the traversal. The check is done in SLGroup::
shapeInit. As in shapeDraw the modelview matrix is cummulated for retrieving
the entire world transform of the shape that is then stored in the matrix _wm.
Compared to the shapeDraw the modelView matrix does NOT contain the view
transform.
*/
void SLShape::init(SLSceneView* sv, SLint currentDepth)  
{  
   depth(currentDepth);

   // apply local transform m to global model transfrom (no view transform)
   stateGL->pushModelViewMatrix();
   stateGL->modelViewMatrix.multiply(m());
   stateGL->buildInverseAndNormalMatrix();
   
   // store world-, inverse- & normal transformation
   _wm.setMatrix(stateGL->modelViewMatrix);
   _wmI.setMatrix(stateGL->invModelViewMatrix());
   _wmN.setMatrix(stateGL->normalMatrix());

   //////////////
   shapeInit(sv);
   //////////////
   
   stateGL->popModelViewMatrix();   
}
//-----------------------------------------------------------------------------
/*!
SLShape::animate applies an animation transform to the local matrix. If an
animation was done here or in one of the children node the function returns 
true.
*/
SLbool SLShape::animate(SLfloat time) 
{  
   SLbool gotAnimated = false;

   if (!_drawBits.get(SL_DB_NOANIM))
   {  
      if (_animation && !_animation->isFinished()) 
      {  _animation->animate(this, time);
         gotAnimated = true;
      }

      // animate children nodes
      if (typeid(*this)==typeid(SLGroup) || typeid(*this)==typeid(SLRefGroup))
      {  SLNode* current = ((SLGroup*)this)->first();
         while (current)
         {  if (current->animate(time)) gotAnimated = true;
            current = current->next();
         }
      }
   }
   return gotAnimated;
}
//-----------------------------------------------------------------------------
/*!
SLShape::update returns true if the shape got updated. If the shape needs to 
update its world model matrix it multiplies its local matrix m to the parents
world matrix and stores the result in the matrix wm. The wm does NOT contain
the view transform.
*/
void SLShape::updateWM(SLbool ancestorGotUpdated)
{  
   SLbool needsUpdate = ancestorGotUpdated || 
                        (_animation && !_animation->isFinished());
   
   stateGL->pushModelViewMatrix();
   
   if (needsUpdate) 
   {  
      if (!ancestorGotUpdated && parent())
         stateGL->modelViewMatrix.setMatrix(((SLShape*)parent())->_wm);

      // cummulate world transform
      stateGL->modelViewMatrix.multiply(m());
      stateGL->buildInverseAndNormalMatrix();
   
      // store world-, inverse- & normal transformation
      _wm.setMatrix(stateGL->modelViewMatrix);
      _wmI.setMatrix(stateGL->invModelViewMatrix());
      _wmN.setMatrix(stateGL->normalMatrix());
   }

   // update children nodes
   if (typeid(*this)==typeid(SLGroup) || typeid(*this)==typeid(SLRefGroup))
   {  SLNode* current = ((SLGroup*)this)->first();
      while (current)
      {  current->updateWM(needsUpdate);
         current = current->next();
      }
   } 
   
   stateGL->popModelViewMatrix();
}
//-----------------------------------------------------------------------------
/*!
SLShape::updateAABB updates the axis aligned bounding box by updating the AABB
in OS with the current world matrix. This is gives not a tight AABB but it's 
fast.
*/
SLAABBox& SLShape::updateAABB()
{
   if (typeid(*this)==typeid(SLGroup) || typeid(*this)==typeid(SLRefGroup))
   {  
      // empty the groups AABB    
      _aabb.minWS(SLVec3f( SL_FLOAT_MAX, SL_FLOAT_MAX, SL_FLOAT_MAX));
      _aabb.maxWS(SLVec3f(-SL_FLOAT_MAX,-SL_FLOAT_MAX,-SL_FLOAT_MAX));

      // merge the children AABB
      SLNode* current = ((SLGroup*)this)->first();
      while (current)
      {  _aabb.merge(current->updateAABB());
         current = current->next();
      }
      _aabb.fromWStoOS(_aabb.minWS(), _aabb.maxWS(), _wmI);
   } 
   else
      _aabb.fromOStoWS(_aabb.minOS(), _aabb.maxOS(), _wm);

   return _aabb;
}
//-----------------------------------------------------------------------------
/*!
SLShape::draw implements the virtual function of the node class and calls the
shapeDraw method of derived shape node. After pushing the modelview matrix to
the stack the local transformation matrix m is applied to it. The normal matrix
is built from the modelview matrix for a correct transform of the normal in the
shader. After the polymorph call of the derived shapeDraw method the previous 
modelview matrix is restored from the stack. This is important for hierarchical
transform in the scenegraph.
*/
void SLShape::draw(SLSceneView* sv)  
{    
   // Do frustum culling for all shapes except cameras & lights
   if (sv->doFrustumCulling() && !_aabb.isVisible()) return; 
   
   stateGL->pushModelViewMatrix();
   stateGL->modelViewMatrix.multiply(m());
   stateGL->buildInverseAndNormalMatrix();

   //////////////
   shapeDraw(sv);
   //////////////
      
   stateGL->popModelViewMatrix();
   
   // Don't draw AABB for buttons
   if (typeid(*this)==typeid(SLButton)) return;

   // Draw axis aligned bounding box
   if (sv->drawBits()->get(SL_DB_BBOXGROUP)  || 
       sv->drawBits()->get(SL_DB_BBOX)  || 
       _drawBits.get(SL_DB_SELECTED))
   {  
      stateGL->pushModelViewMatrix();
      stateGL->modelViewMatrix.setMatrix(sv->camera()->vm().m());
      
      // Draw AABB of group shapes only
      if (sv->drawBits()->get(SL_DB_BBOXGROUP))
      {  if (typeid(*this)==typeid(SLRefGroup)) 
            _aabb.drawWS(SLCol3f(0,1,1));
         else
            if (typeid(*this)==typeid(SLGroup)) 
               _aabb.drawWS(SLCol3f(1,0,1));
      }
      
      // Draw AABB of all other shapes only
      if (sv->drawBits()->get(SL_DB_BBOX))
      {  if (!_drawBits.get(SL_DB_SELECTED))
         {  if (typeid(*this)==typeid(SLRefShape)) 
               _aabb.drawWS(SLCol3f(0,0,1));
            else
               if (!(typeid(*this)==typeid(SLGroup)) && 
                   !(typeid(*this)==typeid(SLRefGroup)))
                  _aabb.drawWS(SLCol3f(1,0,0));
         }
      }
      
      // Draw AABB if shapes is selected
      if (_drawBits.get(SL_DB_SELECTED))
         _aabb.drawWS(SLCol3f(1,1,0));

      // Draw the animation curve
      if (_animation)
         _animation->drawWS();
         
      stateGL->popModelViewMatrix(); 
   }
}
//-----------------------------------------------------------------------------

/*!
Simplified Draw Call, used to render into the ShadowMap used for ShadowMapping
*/
void SLShape::drawShadow(SLSceneView* sv)
{
   stateGL->pushModelViewMatrix();
   stateGL->modelViewMatrix.multiply(m());

   //////////////
   shapeDraw(sv);          // call the shapes drawing method
   //////////////

   stateGL->popModelViewMatrix();
}
//-----------------------------------------------------------------------------
/*!
SLShape::copy copies the shape part of the returned SLNode
*/
SLNode* SLShape::copy()
{  SLShape* copy = shapeCopy();
   copy->setMatrix(m());
   copy->wm(_wm);
   copy->wmI(_wmI);
   copy->wmN(_wmN);
   copy->_drawBits = _drawBits;
   return copy;
}
//-----------------------------------------------------------------------------
/*!
SLShape::cull does the view frustum culling by checking whether the AABB is 
inside the view frustum.
*/
void SLShape::cull(SLSceneView* sv)  
{     
   // Do frustum culling for all shapes except cameras & lights
   if (sv->doFrustumCulling() &&
       typeid(*this)!=typeid(SLCamera) &&
       typeid(*this)!=typeid(SLLightSphere) &&
       typeid(*this)!=typeid(SLLightRect))
      sv->camera()->isInFrustum(&_aabb);
   else _aabb.isVisible(true);
   
   // Cull the groups recursively
   if (_aabb.isVisible())
   {  if (typeid(*this)==typeid(SLGroup) ||     
          typeid(*this)==typeid(SLRefGroup))
      {  SLNode* current = ((SLGroup*)this)->first();
         while (current)
         {  current->cull(sv);
            current = current->next();
         }
      } else
      {  // Add visible & transparent non-groups to the blendShapes array
         if (_aabb.hasAlpha())
            sv->blendShapes()->push_back(this);
      }
   }
}
//-----------------------------------------------------------------------------
/*!
SLShape::hit does the ray shape test by transforming the ray into the object
space. This is slower than doing it in world space but allows referencing and 
moving shapes without recalculating the acceleration structures.
*/
SLbool SLShape::hit(SLRay* ray)  
{  
   if (_drawBits.get(SL_DB_HIDDEN)) return false;
   
   // Check first AABB for intersection
   if (!_aabb.isHitInWS(ray)) return false;
     
   // Transform ray to object space for non-groups
   if (typeid(*this)!=typeid(SLGroup) && typeid(*this)!=typeid(SLRefGroup))    
   {  
      // transform origin position to object space
      ray->originOS.set(_wmI * ray->origin);
         
      // transform the direction only with the linear sub matrix
      ray->setDirOS(_wmI.mat3() * ray->dir);
   }

   /////////////////////
   return shapeHit(ray);
   /////////////////////
}
//-----------------------------------------------------------------------------
/*! SLShape::scaleToCenter scales and translates the shape so that its largest
dimension is maxDim and the center is in [0,0,0].
*/
void SLShape::scaleToCenter(SLfloat maxDim)
{  
   _aabb = buildAABB();
   SLVec3f size(_aabb.maxWS()-_aabb.minWS());
   SLVec3f center((_aabb.maxWS()+_aabb.minWS()) * 0.5f);
   SLfloat scaleFactor = maxDim / size.maxXYZ();
   if (fabs(scaleFactor) > SL_EPSILON)
      scale(scaleFactor);  // scale method of the matrix _m
   else cout << "Shape can't be scaled: " << name().c_str() << endl;
   translate(-center);  // translate method of the matrix _m
}
//-----------------------------------------------------------------------------
