//#############################################################################
//  File:      SLSphere.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLSPHERE_H
#define SLSPHERE_H

#include <stdafx.h>
#include "SLRevolver.h"
#include "SLGroup.h"

class SLRay;
class SLMaterial;

//-----------------------------------------------------------------------------
//! SLSphere creates a sphere mesh based on SLRevolver     
class SLSphere: public SLRevolver 
{  public:                     
                              SLSphere(SLfloat radius,
                                       SLint   stacks = 32,
                                       SLint   slices = 32,
                                       SLstring name = "Sphere",
                                       SLMaterial* mat = 0);
                                           
                             ~SLSphere(){;}
                             
               SLShape*       shapeCopy()
                              {  return new SLSphere(_radius, 
                                                     _stacks, 
                                                     _slices,
                                                     name());
                              }
                              
               // Getters
               SLfloat        radius() {return _radius;}
               SLint          stacks() {return _stacks;}
               
   protected:    
               SLfloat        _radius; //!< radius of the sphere
               SLint          _stacks; //!< No. of stacks of the sphere
};
//-----------------------------------------------------------------------------
#endif //SLSPHERE_H

