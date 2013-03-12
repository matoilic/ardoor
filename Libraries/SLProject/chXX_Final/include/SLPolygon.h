//#############################################################################
//  File:      SLPolygon.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLPOLYGON_H
#define SLPOLYGON_H

#include <stdafx.h>
#include "SLMesh.h"

//-----------------------------------------------------------------------------
//! SLPolygon creates a convex polyon mesh
/*! 
The SLPolygon node draws a convex polygon with. The normale vector is 
calculated from the first 3 vertices.
*/
class SLPolygon: public SLMesh
{  public:                 
                           //! ctor for generic convex polygon
                           SLPolygon(SLVVec3f corner,
                                     SLstring name = "Polygon",
                                     SLMaterial* mat=0);
                           
                           //! ctor for generic convex polygon with texCoords          
                           SLPolygon(SLVVec3f corner, 
                                     SLVVec2f texcoord,
                                     SLstring name = "Polygon",
                                     SLMaterial* mat=0);
                                     
                           //! ctor for centered rectangle in x-y-plane (N=-z)
                           SLPolygon(SLfloat  width, 
                                     SLfloat  height,
                                     SLstring name,
                                     SLMaterial* mat=0);
               
               void        buildMesh(SLMaterial* mat);
                             
               SLShape*    shapeCopy();
               
   protected:
               SLVVec3f    _corner;       //!< corners in ccw order
               SLVVec2f    _texCoord;     //!< texture coords for corners
};
//-----------------------------------------------------------------------------
#endif
