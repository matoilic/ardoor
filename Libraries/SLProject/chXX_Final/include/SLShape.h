//#############################################################################
//  File:      SLShape.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLSHAPE_H
#define SLSHAPE_H

#include <stdafx.h>
#include "SLDrawBits.h"
#include "SLNode.h"
#include "SLMaterial.h"
#include "SLEventHandler.h"
#include "SLAABBox.h"
#include "SLRaytracer.h"
//#include "SLAnimator.h"

class SLSceneView;
class SLRay;
class SLAnimation;

//-----------------------------------------------------------------------------
//! Base class for all transformable scenegraph nodes
/*!      
The SLShape class is derived from SLMat4, SLNode and SLEventHandler. A shape 
can therefore be placed into the scenegraph as a node. You can apply any matrix 
transform methods to a shape object as a local transform and you overwrite one
of the eventhandlers to react on mouse, touch or keyboard event.
The global (world) transformation of the shape will be stored in the _wm. It is
built during the scene traversal in the init method. It doesn't contain the 
view transform.
*/
class SLShape : public SLNode, public SLMat4f, public SLEventHandler
{  public:
                            SLShape    (SLstring name);
      virtual              ~SLShape    ();
      
               void         init       (SLSceneView* sv, SLint currentDepth);
               SLbool       animate    (SLfloat time);
               void         updateWM   (SLbool ancestorGotUpdated);
               SLAABBox&    updateAABB ();
               void         cull       (SLSceneView* sv);
               void         draw       (SLSceneView* sv);
               void         drawShadow (SLSceneView* sv);
               SLNode*      copy       ();
               SLbool       hit        (SLRay* ray);

      virtual  void         shapeInit  (SLSceneView* sv) = 0;
      virtual  void         shapeDraw  (SLSceneView* sv) = 0;
      virtual  SLShape*     shapeCopy  ()                = 0;
      virtual  SLbool       shapeHit   (SLRay* ray)      = 0;

               void         scaleToCenter(SLfloat maxDim=1.0);
      
               // Setters
               void         wm         (SLMat4f wm) {_wm  = wm;}
               void         wmI        (SLMat4f wmI){_wmI = wmI;}
               void         wmN        (SLMat3f wmN){_wmN = wmN;}
               void         animation  (SLAnimation* a){_animation = a;}
               
               // Getters
               SLMat4f      wm         () {return _wm;}
               SLDrawBits*  drawBits   () {return &_drawBits;}
               SLAABBox*    aabb       () {return &_aabb;}
               SLMat4f      wmI        () {return _wmI;}
               SLMat3f      wmN        () {return _wmN;}
               SLAnimation* animation  () {return _animation;}
   protected:
               SLMat4f      _wm;       //!< Matrix for world transformation
               SLDrawBits   _drawBits; //!< Shape level drawing flags
               SLAABBox     _aabb;     //!< Axis aligned bounding box
               SLMat4f      _wmI;      //!< Matrix for inverse world transform
               SLMat3f      _wmN;      //!< Matrix for normal world transform
               SLAnimation* _animation;//!< Animation of the shape
};
//-----------------------------------------------------------------------------
typedef std::vector<SLShape*>  SLVShape;
//-----------------------------------------------------------------------------
#endif
