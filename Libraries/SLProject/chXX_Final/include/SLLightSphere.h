//#############################################################################
//  File:      SLLightSphere.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLLIGHTSPHERE_H
#define SLLIGHTSPHERE_H

#include <stdafx.h>
#include "SLSphere.h"
#include "SLLight.h"
#include "SLSamples2D.h"

class SLGroup;
class SLSceneView;
class SLRay;

//-----------------------------------------------------------------------------
//! LightSphere class for a spherical light source
/*!      
SLLightSphere is a SLSphere shape node that renders a sphere and applies the 
OpenGL light settings through the SLLight class.
*/
class SLLightSphere: public SLSphere, public SLLight
{  public:
                           SLLightSphere  (SLfloat radius = 0.3f);
                           SLLightSphere  (SLfloat posx, SLfloat posy, SLfloat posz,
                                           SLfloat radius = 0.3f,
                                           SLfloat ambiPower = 1.0f,
                                           SLfloat diffPower = 10.0f,
                                           SLfloat specPower = 10.0f);
                          ~SLLightSphere  () {;}

               void        shapeInit      (SLSceneView* s);
               void        shapeDraw      (SLSceneView* s);
               SLShape*    shapeCopy      ();
               SLbool      shapeHit       (SLRay* ray);
            
               void        setState       (SLGLState* state);
               SLfloat     shadowTest     (SLRay* ray,   
                                           const SLVec3f& L, 
                                           SLfloat lightDist); 
               void        photonEmission (); // PM
               
               // Setters
               void        samples        (SLint x, SLint y)
                                          {_samples.samples(x, y, false);}
               
               // Getters
               SLint       samples        () {return _samples.samples();}
               SLVec3f     positionWS     () {return wm().translation();}
               SLVec3f     spotDirWS      () {return SLVec3f(_wm.m(8),
                                                             _wm.m(9),
                                                             _wm.m(10))*-1.0;}

   private:
               SLSamples2D _samples;      //!< 2D samplepoints for soft shadows
};
//-----------------------------------------------------------------------------
#endif
