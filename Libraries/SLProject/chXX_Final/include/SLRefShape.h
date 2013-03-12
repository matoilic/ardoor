//#############################################################################
//  File:      SLRefGroup.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLREFSHAPE_H
#define SLREFSHAPE_H

#include <stdafx.h>
#include "SLShape.h"

class SLNode;

//-----------------------------------------------------------------------------
//! Shape reference node
/*!      
The SLRefShape node class is for reusing other nodes in the same scene. Mostly
only the according methods of the referenced shape are used. See also 
SLRefGroup for reusing groups of nodes. 
The big advantage of referenced nodes is saving in memory. The geometry data as 
well as the acceleration structure data is used only once. 
Because Ray Tracing is done in object space the ray is transformed in the
SLShape::hit method after the ray hit the AABB in the world space. Thats why 
SLRefShape and SLRefGroup inherit themself from SLShape through witch they get
their own transform matrix and their own worldmatrix _wm.
*/
class SLRefShape: public SLShape
{  public:
                           SLRefShape  (SLShape* refShape,
                                        SLstring name = "RefShp");
                          ~SLRefShape  ();

               void        shapeInit   (SLSceneView* sv);
               void        shapeDraw   (SLSceneView* sv);
               SLShape*    shapeCopy   (){return (SLShape*)_refShape->copy();}
               void        updateStats (SLGroup* parent);
               SLAABBox&   buildAABB   ();
               void        preShade    (SLRay* ray);
               SLbool      shapeHit    (SLRay* ray);

               // Getters
               SLShape*    refShape    () {return _refShape;}

   private:    
               SLShape*   _refShape;  //!< pointer to the referenced shape
};
//-----------------------------------------------------------------------------
#endif
