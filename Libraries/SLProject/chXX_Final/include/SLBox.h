//#############################################################################
//  File:      SLBox.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLBOX_H
#define SLBOX_H

#include <stdafx.h>
#include "SLMesh.h"

//-----------------------------------------------------------------------------
//! Axis aligned box mesh
/*!      
The SLBox node draws an axis aligned box from a minimal corner to a maximal 
corner.
*/
class SLBox: public SLMesh
{  public:
                           SLBox       (SLfloat minx=0, 
                                        SLfloat miny=0, 
                                        SLfloat minz=0,
                                        SLfloat maxx=1, 
                                        SLfloat maxy=1, 
                                        SLfloat maxz=1,
                                        SLstring name = "Box",
                                        SLMaterial* mat = 0);
                           SLBox       (SLVec3f min, 
                                        SLVec3f max,
                                        SLstring name = "Box",
                                        SLMaterial* mat = 0);
               
               void        buildMesh(SLMaterial* mat);
                             
               SLShape*    shapeCopy();
               bool        onKeyPress  (const SLKey modKey);
   
   private:    
               SLVec3f     _min;       //!< minimal corner
               SLVec3f     _max;       //!< maximum corner
};
//-----------------------------------------------------------------------------
#endif
