//#############################################################################
//  File:      SLLightRect.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include "SLLightRect.h"
#include "SLScene.h"
#include "SLSceneView.h"
#include "SLMaterial.h"
#include "SLGroup.h"
#include "SLRay.h"
#include "SLPhotonMapper.h"

//-----------------------------------------------------------------------------
SLLightRect::SLLightRect(SLfloat w, SLfloat h) : SLPolygon(w, h, "LightRect")
{  
   width(w);
   height(h);
   
   _samples.set(1,1);
   
   // make sample number even
   if (_samples.x%2==0) _samples.x++;
   if (_samples.y%2==0) _samples.y++;
   
   spotCutoff(90.0f);
   spotExponent(1.0);
}
//-----------------------------------------------------------------------------
//! shapeInit sets the light id, the light states & creates an emissive mat.
void SLLightRect::shapeInit(SLSceneView* sv)
{  
   // Check if OpenGL lights are available
   if (SLScene::current->lights().size() >= SL_MAX_LIGHTS) 
      SL_EXIT_MSG("Max. NO. of lights is exceeded!");

   // Add the light to the lights array of the scene
   if (_id==-1) 
   {  _id = SLScene::current->lights().size();
      SLScene::current->lights().push_back(this);
   }
   
   // Set the OpenGL light states
   SLLightRect::setState(stateGL);
   stateGL->numLightsUsed = SLScene::current->lights().size();
   
   // Automatically create emissive light material (lights do not reflect)
   if (M && M->mat==0)
   {  M->mat = new SLMaterial(_name.c_str(), SLCol4f::BLACK, SLCol4f::BLACK);
      M->mat->emission(_on ? diffuse() : SLCol4f::BLACK);
      SLScene::current->materials().push_back(M->mat);   
   }
   
   SLMesh::shapeInit(sv);
}
//-----------------------------------------------------------------------------
//! SLLightRect::shapeCopy returns a full copy of itself
SLShape* SLLightRect::shapeCopy()
{  
   // Check if OpenGL lights are available
   if (SLScene::current->lights().size() >= SL_MAX_LIGHTS) 
      SL_EXIT_MSG("Max. NO. of lights is exceeded!");
     
   SLLightRect* copy = new SLLightRect(_width, _height);
   copy->samples(_samples);
   copy->on(_on);
   copy->spotCutoff(_spotCutoff);
   copy->spotExponent(_spotExponent);
   copy->kc(_kc);
   copy->kl(_kl);
   copy->kq(_kq);
   copy->ambient(_ambient);
   copy->diffuse(_diffuse);
   copy->specular(_specular);
   return copy;
}
//-----------------------------------------------------------------------------
/*!
SLLightRect::shapeDraw draws a light rectangle if the light is a pointlight. 
The light transformation (position and orientation) is done as for other shapes 
in the inherited SLShape class.
*/
void SLLightRect::shapeDraw(SLSceneView* sv)
{  
   if (_id!=-1) 
   {  
      SLLightRect::setState(stateGL);
      stateGL->numLightsUsed = SLScene::current->lights().size();
   
      // now draw the inherited object
      SLMesh::shapeDraw(sv);
   }
}
//-----------------------------------------------------------------------------
/*!
SLLightRect::intersect calls the mesh intersection code.
*/
SLbool SLLightRect::shapeHit(SLRay* ray)
{            
   // do not intersect shadow rays
   if (ray->type==SHADOW) return false;
   
   // only allow intersection with primary rays (no lights in reflections)
   //if (ray->type!=PRIMARY) return false;
   
   // call the intersection routine of the mesh   
   return SLMesh::shapeHit(ray);
}
//-----------------------------------------------------------------------------
/*!
SLLightRect::shadowTest returns 0.0 if the hit point is completely shaded and 
1.0 if it is 100% lighted. A return value inbetween is calculate by the ratio 
of the shadow rays not blocked to the total number of casted shadow rays.
*/
SLfloat SLLightRect::shadowTest(SLRay* ray, // ray of hit point
                               const SLVec3f& L, // vector from hit point to light
                               const SLfloat lightDist) // distance to light
{  
   if (_samples.x==1 && _samples.y==1)
   {  
      // define shadow ray
      SLRay shadowRay(lightDist, L, ray);
            
      SLScene::current->root3D()->hit(&shadowRay);

      return (shadowRay.length < lightDist) ? 0.0f : 1.0f;
   } 
   else // do light sampling for soft shadows
   {  SLfloat dw = (SLfloat)_width/(SLfloat)_samples.x; // width of a sample cell
      SLfloat dl = (SLfloat)_height/(SLfloat)_samples.y;// length of a sample cell
      SLint   x, y, hx=_samples.x/2, hy=_samples.y/2;
      SLint   samples = _samples.x*_samples.y;
      SLbool* isSampled = new SLbool[samples];
      SLbool  importantPointsAreLighting = true;
      SLfloat lighted = 0.0f; // return value
      SLfloat invSamples = 1.0f/(SLfloat)(samples);
      SLVec3f SP; // vector hitpoint to samplepoint in world coords

      for (y=0; y<_samples.y; ++y)
      {  for (x=0; x<_samples.x; ++x)
         {  SLint iSP = y*_samples.x + x;
            isSampled[iSP]=false;
         }
      }

      /*
      Important sample points (X) on a 7 by 5 rectangular light.
      If all of them are lighting the hitpoint the sample points
      in between (O) are not tested anymore.

        0   1   2   3   4   5   6         
      +---+---+---+---+---+---+---+
   0  | X | . | . | X | . | . | X |
      +---+---+---+---+---+---+---+
   1  | . | . | . | . | . | . | . |
      +---+---+---+---+---+---+---+
   2  | X | . | . | X | . | . | X |
      +---+---+---+---+---+---+---+
   3  | . | . | . | . | . | . | . |
      +---+---+---+---+---+---+---+
   4  | X | . | . | X | . | . | X |
      +---+---+---+---+---+---+---+
      
      */

      // Double loop for the important sample points
      for (y=-hy; y<=hy; y+=hy)
      {  for (x=-hx; x<=hx; x+=hx)
         {  SLint iSP = (y+hy)*_samples.x + x+hx;
            isSampled[iSP]=true;
            
            SP.set(_wm*SLVec3f(x*dw, y*dl, 0) - ray->hitPoint);
            SLfloat SPDist = SP.length();
            SP.normalize();
            SLRay shadowRay(SPDist, SP, ray);

            SLScene::current->root3D()->hit(&shadowRay);
            
            if (shadowRay.length >= SPDist-SL_EPSILON) 
               lighted += invSamples; // sum up the light
            else 
               importantPointsAreLighting = false;
         }
      }

      if (importantPointsAreLighting)
      { lighted = 1.0f;
      } else
      {  // Double loop for the samplepoints inbetween
         for (y=-hy; y<=hy; ++y)
         {  for (x=-hx; x<=hx; ++x)
            {  SLint iSP = (y+hy)*_samples.x + x+hx;
               if (!isSampled[iSP])
               {  SP.set(_wm*SLVec3f(x*dw, y*dl, 0) - ray->hitPoint);
                  SLfloat SPDist = SP.length();
                  SP.normalize();
                  SLRay shadowRay(SPDist, SP, ray);

                  SLScene::current->root3D()->hit(&shadowRay);
                  
                  // sum up the light
                  if (shadowRay.length >= SPDist-SL_EPSILON) 
                     lighted += invSamples;
               }
            }
         }
      }
      if (isSampled) delete [] isSampled;
      return lighted;
   }
}
//-----------------------------------------------------------------------------
/*! SLLightRect::setState sets the global rendering state
*/
void SLLightRect::setState(SLGLState* state)
{  
   if (_id!=-1) 
   {  state->lightIsOn[_id]       = _on;
      state->lightPosWS[_id]      = positionWS();           
      state->lightDirWS[_id]      = spotDirWS();           
      state->lightAmbient[_id]    = _ambient;              
      state->lightDiffuse[_id]    = _diffuse;              
      state->lightSpecular[_id]   = _specular;    
      state->lightSpotCutoff[_id] = _spotCutoff;           
      state->lightSpotCosCut[_id] = _spotCosCut;           
      state->lightSpotExp[_id]    = _spotExponent;         
      state->lightAtt[_id].x      = _kc;  
      state->lightAtt[_id].y      = _kl;    
      state->lightAtt[_id].z      = _kq; 
      state->lightDoAtt[_id]      = isAttenuated();
   }
}
//-----------------------------------------------------------------------------
void SLLightRect::samples(const SLVec2i samples)
{  assert(samples.x%2 == 1 && samples.y%2 == 1);
   _samples = samples;
}
//-----------------------------------------------------------------------------
void SLLightRect::samplesXY(const SLint x, const SLint y)
{  assert(x%2 == 1 && y%2 == 1);
   _samples.set(x,y);
}
//-----------------------------------------------------------------------------

void SLLightRect::photonEmission()
{
   SLPhotonMapper* pm = SLScene::current->activeSV()->photonMapper();
   
   SLVec3f C,N,u,v,randVec,x;
   SLVec3f power;
   SLfloat eta1,eta2,eta1sqrt;

   //max Flux of light source is calculated as Radiance * surface area
   SLfloat A = _width * _height;
   power.set(_diffuse.r*A, _diffuse.g*A, _diffuse.b*A);
   
   //get pointers to photonmap
   SLPhotonMap* mapCaustic = pm->mapCaustic();
   SLPhotonMap* mapGlobal  = pm->mapGlobal();
   
   SLMat4f rotMat = _wm;
   rotMat.translation(0.0,0.0,0.0);       // remove translation
   C.set(_corner[0].x,_corner[0].y,0.0);  // corner 0 of square
   
   // directional vector to corners
   u = _corner[1] - _corner[0];
   v = _corner[3] - _corner[0];
   //u.set(_corner[1].x-_corner[0]. x,_corner[1].y-_corner[0].y, 0.0);
   //v.set(_corner[3].x-_corner[0]. x,_corner[3].y-_corner[0].y, 0.0);

   // transform to global coord system
   C.set(_wm*C);     // translate and rotate
   u.set(rotMat*u);  // rotate only
   v.set(rotMat*v);
   
   // progress
   SLlong maxPhotons = mapCaustic->maxStoredPhotons()+mapGlobal->maxStoredPhotons();
   SLlong curPhotons = mapCaustic->storedPhotons()+mapGlobal->storedPhotons();;

   SLlong emitted=0;
   // shoot the photons as long as maps are not full
   while(emitted<_photons && !(mapCaustic->isFull()&&(mapGlobal->isFull())))
   {
      // progress
      if(emitted%1000==0)
      {  curPhotons=mapCaustic->storedPhotons()+mapGlobal->storedPhotons();
         printf("\b\b\b\b%3.0f%%",(SLfloat)curPhotons/(SLfloat)maxPhotons*100.0f);
         //qApp->processEvents();
      }

      // create random point within square light in global coordinates
      eta1 = pm->random();
      eta2 = pm->random();
      x = C + eta1*u + eta2*v;

      // create random direction around z-axis (cosine distribution)
      eta1 = pm->random();
      eta2 = SL_2PI * pm->random();
      eta1sqrt = sqrt(1.0f-eta1);
      randVec.set(eta1sqrt*cos(eta2), eta1sqrt*sin(eta2), -sqrt(eta1));   

      // create and emit photon (direction transformed)
      SLRay scattered(x, rotMat*randVec, PRIMARY, this, SL_FLOAT_MAX, 1);
      
      // init power with max Flux of light source (will be scaled by number of emitted after shooting)
      pm->photonScatter(&scattered,power, LIGHT);

      emitted++;
      // scaling of stored photons is necessary if one of the maps was filled by this photon
      // (because emission of photons continues in order to fill the other map)
      if(mapCaustic->isFull() && !pm->mapCausticGotFull())
      {  pm->mapCausticGotFull(true);
         mapCaustic->scalePhotonPower(1.0f/SLfloat(emitted));
      }
      if(mapGlobal->isFull() && !pm->mapGlobalGotFull())
      {  pm->mapGlobalGotFull(true);
         mapGlobal->scalePhotonPower(1.0f/SLfloat(emitted));
      }

   }

   // scale all stored photons of this light source
   if(emitted)
   {  SLRay::emittedPhotons+=emitted;
      mapCaustic->scalePhotonPower(1.0f/SLfloat(emitted));
      mapGlobal->scalePhotonPower(1.0f/SLfloat(emitted));
   }
}
//-----------------------------------------------------------------------------
