//#############################################################################
//  File:      SLRefGroup.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLREFGROUP_H
#define SLREFGROUP_H

#include <stdafx.h>
#include "SLGroup.h"

class SLNode;

//-----------------------------------------------------------------------------
//! Reference node for groups holding only SLRefShape node as childrens.
/*!      
The SLRefGroup node class is for reusing groups in the same scene.  Mostly
only the according methods of the referenced group are used. See also 
SLRefShape for referencing of non-group shape nodes.
The big advantage of referenced nodes is saving in memory. The geometry data as 
well as the acceleration structure data is used only once. 
Because Ray Tracing is done in object space the ray is transformed in the
SLShape::hit method after the ray hit the AABB in the world space. Thats why 
SLRefShape and SLRefGroup inherit themself from SLShape through witch they get
their own transform matrix and their own worldmatrix _wm.
*/
class SLRefGroup: public SLGroup
{  public:
                           SLRefGroup  (SLGroup* refGroup,
                                        SLstring name = "RefGrp");
                          ~SLRefGroup  ();

               void        shapeInit   (SLSceneView* sv);
               void        shapeDraw   (SLSceneView* sv);
               SLShape*    shapeCopy   (){return (SLShape*)_refGroup->copy();}
               void        updateStats (SLGroup* parent);
               SLAABBox&   buildAABB   ();
               SLbool      shapeHit    (SLRay* ray);
               void        preShade    (SLRay* ray);

               // Getters
               SLGroup*    refGroup()  {return _refGroup;}

   private:    
               SLGroup*    _refGroup;  //!< pointer to the referenced group
};
//-----------------------------------------------------------------------------
#endif
