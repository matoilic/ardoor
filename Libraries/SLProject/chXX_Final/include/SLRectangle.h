//#############################################################################
//  File:      SLRectangle.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLRECTANGLE_H
#define SLRECTANGLE_H

#include <stdafx.h>
#include "SLMesh.h"

//-----------------------------------------------------------------------------
//! SLRectangle creates a rectangular mesh with a certain resolution
/*! 
The SLRectangle node draws a rectangle with a minimal and a maximal corner in 
the x-y-plane. The normale is [0,0,1].
*/
class SLRectangle: public SLMesh
{  public:                 
                           //! ctor for rectangle w. min & max corner
                           SLRectangle(SLVec2f min, SLVec2f max,
                                       SLint resX, SLint resY,
                                       SLstring name = "Rectangle",
                                       SLMaterial* mat=0);

                           //! ctor for rectangle w. min & max corner & texCoord
                           SLRectangle(SLVec2f min, SLVec2f max,
                                       SLVec2f tmin, SLVec2f tmax,
                                       SLint resX, SLint resY,
                                       SLstring name = "Rectangle",
                                       SLMaterial* mat=0);
               
               void        buildMesh(SLMaterial* mat);
                             
               SLShape*    shapeCopy();
               
   protected:
               SLVec3f    _min;     //!< min corner
               SLVec3f    _max;     //!< max corner
               SLVec2f    _tmin;    //!< min corner texCoord
               SLVec2f    _tmax;    //!< max corner texCoord
               SLint      _resX;    //!< resolution in x direction
               SLint      _resY;    //!< resolution in y direction
};
//-----------------------------------------------------------------------------
#endif
