//#############################################################################
//  File:      SLRaytracer.cpp
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
#ifdef SL_OMP
#include <omp.h>              // OpenMP
#endif

#include "SLRay.h"
#include "SLRaytracer.h"
#include "SLCamera.h"
#include "SLSceneView.h"
#include "SLLightSphere.h"
#include "SLLightRect.h"
#include "SLLight.h"
#include "SLGroup.h"
#include "SLMesh.h"
#include "SLGLTexture.h"
#include "SLSamples2D.h"
#include "SLGLShaderProg.h"

//-----------------------------------------------------------------------------
SLRaytracer::SLRaytracer()
{  
   name("myCoolRaytracer");
   
   _state = rtReady;
   _maxDepth = 5;
   _aaThreshold = 0.3f; // = 10% color difference
   _aaSamples = 3;
   
   // set texture properies
   _min_filter   = GL_NEAREST;
   _mag_filter   = GL_NEAREST;
   _wrap_s       = GL_CLAMP_TO_EDGE;
   _wrap_t       = GL_CLAMP_TO_EDGE;
   _resizeToPow2 = false;
   
   // init callback function pointer
   guiRTWndUpdate = 0;
   
   _numThreads = 1;
   _continuous = false;
}
//-----------------------------------------------------------------------------
SLRaytracer::~SLRaytracer()
{  
   //SL_LOG("~SLRaytracer\n");
}
//-----------------------------------------------------------------------------
/*!
This is the main rendering method for ray tracing. It loops over all 
lines and pixels and determines for each pixel a color with a partly global 
illumination calculation.
*/
SLbool SLRaytracer::render()
{  
   SLScene* s = SLScene::current;      // scene shortcut
   SLSceneView* sv = s->activeSV();    // sceneview shortcut
   SLCamera* cam = sv->_camera;        // camera shortcut
   _state = rtBusy;                    // From here we state the RT as busy
   _stateGL = SLGLState::getInstance();// OpenGL state shortcut
   _numThreads = 1;                    // No. of threads
   _pcRendered = 0;                    // % rendered
   _renderSec = 0.0f;                  // reset time
   SLint pc = (_aaSamples>1)?50:100;   // %-factor depending on AA
   _infoText  = SLScene::current->info()->text();  // keep original info string
   _infoColor = SLScene::current->info()->color(); // keep original info color
   
   initStats(_maxDepth);

   ///////////////////////
   //  PRECALCULATIONS  // 
   ///////////////////////
   
   // calculate half window width & height in world coords   
   SLint   resX = sv->scrW();
   SLint   resY = sv->scrH();   
   SLfloat hh = tan(SL_DEG2RAD*cam->fov()*0.5f) * cam->focalDist();
   SLfloat hw = hh * (SLfloat)resX / (SLfloat)resY;
   
   // calculate the size of a pixel in world coords. 
   SLfloat pxSize = hw * 2 / sv->scrW();
   
   // get camera vectors eye, lookAt, lookUp
   SLVec3f EYE, LA, LU, LR;
   cam->vm().lookAt(&EYE, &LA, &LU, &LR);
   
   // calculate a vector to the center (C) of the bottom left (BL) pixel
   SLVec3f C  = LA * cam->focalDist();
   SLVec3f BL = C - hw*LR - hh*LU  +  pxSize/2*LR - pxSize/2*LU;

   createImage(resX, resY);
   
   #ifdef SL_OMP
   SLbool doGUIUpdate = false;
   #endif
   #if defined(SL_OS_ANDROID) || defined(SL_OS_IOS)
   const SLint GUIUPDATE = 8;
   #else
   const SLint GUIUPDATE = 16;
   #endif

   SLbool stop = false;

   clock_t clockstart = clock();
   
   ////////////////////////////////////////////////////////////////////////////
   // single primary ray rendering
   if (cam->lensSamples()->samples() == 1)
   {  
      // Do standard RT with one primary ray per pixel
      #ifdef SL_OMP
      #pragma omp parallel for schedule(dynamic)
      #endif      
      for (SLint x=0; x<resX; ++x)
      {  if (!stop)
         {  for (SLint y=0; y<resY; ++y)
            {  
               // calculate ray from eye to pixel
               SLVec3f primaryDir(BL + pxSize*((SLfloat)x*LR + (SLfloat)y*LU));
               primaryDir.normalize();
               SLRay primaryRay(EYE, primaryDir, x, y);
            
               ///////////////////////////////////
               SLCol4f color = trace(&primaryRay);
               ///////////////////////////////////
            
               _img[0].setPixeliRGB(x, y, color);
            
               SLRay::avgDepth += SLRay::depthReached;
               SLRay::maxDepthReached = SL_max(SLRay::depthReached, SLRay::maxDepthReached);
            }
         
            // Allow the GUI to process events & refresh RT window every 16th line
            #ifdef SL_OMP
            _numThreads = omp_get_num_threads();
            #else
            _numThreads = 1;
            #endif
         
            if (!_continuous)
            {  _pcRendered = (SLint)((SLfloat)x/(SLfloat)resX*pc);
               #ifdef SL_OMP
               if (!doGUIUpdate) doGUIUpdate = (x%GUIUPDATE==0);
               if (doGUIUpdate && omp_get_thread_num()==0) 
                  stop = guiRTWndUpdate();
               #else
               if (x%GUIUPDATE==0) 
            	   stop = guiRTWndUpdate();
               #endif
            }
         }
      }
   }
   else // Do lens sampling with multiple primary rays per pixel -------------
   {  
      // lens sampling constants
      SLVec3f lensRadiusX = LR*(cam->lensDiameter()*0.5f);
      SLVec3f lensRadiusY = LU*(cam->lensDiameter()*0.5f);

      // Loop over pixels and then over lense
      #ifdef SL_OMP
      #pragma omp parallel for schedule(dynamic)
      #endif
      for (SLint x=0; x<resX; ++x) 
      {  if (!stop)
         {  for (SLint y=0; y<resY; ++y)
            {           
               // focal point is single shot primary dir
               SLVec3f primaryDir(BL + pxSize*((SLfloat)x*LR + (SLfloat)y*LU));
               SLVec3f FP = EYE + primaryDir;
               SLCol4f color(SLCol4f::BLACK);
            
               // Loop over radius r and angle phi of lens
               for (SLint iR=cam->lensSamples()->samplesX()-1; iR>=0; --iR)
               {  for (SLint iPhi=cam->lensSamples()->samplesY()-1; iPhi>=0; --iPhi)
                  {   
                     SLVec2f discPos(cam->lensSamples()->point(iR,iPhi));
                  
                     // calculate lensposition out of disc position
                     SLVec3f lensPos(EYE + discPos.x*lensRadiusX + discPos.y*lensRadiusY);
                     SLVec3f lensToFP(FP-lensPos);
                     lensToFP.normalize();
                     SLRay primaryRay(lensPos, lensToFP, x, y);
                  
                     ///////////////////////////
                     color += trace(&primaryRay);
                     ///////////////////////////
                  
                     SLRay::avgDepth += SLRay::depthReached;
                     SLRay::maxDepthReached = SL_max(SLRay::depthReached, SLRay::maxDepthReached);   
                  }
               }
               color /= (SLfloat)cam->lensSamples()->samples();
               _img[0].setPixeliRGB(x, y, color);
            }
         
            #ifdef SL_OMP
            _numThreads = omp_get_num_threads();
            #else
            _numThreads = 1;
            #endif
         
            if (!_continuous)
            {  _pcRendered = (SLint)((SLfloat)x/(SLfloat)resX*pc);
               
               // Allow the GUI to process events & refresh RT window every 32th line
               #ifdef SL_OMP
               if (!doGUIUpdate) doGUIUpdate = (x%GUIUPDATE==0);
               if (doGUIUpdate && omp_get_thread_num()==0) 
               {  stop = guiRTWndUpdate();
                  doGUIUpdate = false;
               }
               #else
               if (x%GUIUPDATE==0) 
            	   stop = guiRTWndUpdate();
               #endif
            }
         }
      }
   }
   
   ////////////////////////////////////////////////////////////////////////////
   // Do anti-aliasing w. contrast compare in a 2nd. pass
   if (!stop && !_continuous && _aaSamples > 1 && 
       cam->lensSamples()->samples() == 1)
   {  SLCol4f  color, colorLeft, colorUp;    // pixel colors to be compared      
      SLbool*  gotSampled = new SLbool[resX];// Flags if above pixel got sampled
      SLbool   isSubsampled = false;         // Flag if pixel got subsampled
      SLVPixel pix;                          // Dynamic vector for AA pixels
            
      // Nothing got sampled at beginning
      for (SLint x=0; x<resX; ++x) gotSampled[x] = false;
      
      // Loop through all pixels & add the pixel that have to be subsampled
      for (SLint y=0; y<resY; ++y)
      {  for (SLint x=0; x<resX; ++x) 
         {  
            color = _img[0].getPixeli(x, y);        
            isSubsampled = false;
            if (x>0)
            {  colorLeft = _img[0].getPixeli(x-1, y);
               if (color.diffRGB(colorLeft) > _aaThreshold)
               {  if (!gotSampled[x-1]) 
                  {  pix.push_back(SLRTAAPixel(x-1,y));
                     gotSampled[x-1] = true;
                  }
                  pix.push_back(SLRTAAPixel(x,y));
                  isSubsampled = true;
               }
            }
            if (y>0)
            {  colorUp = _img[0].getPixeli(x, y-1);
               if(color.diffRGB(colorUp) > _aaThreshold)
               {  if (!gotSampled[x]) pix.push_back(SLRTAAPixel(x,y-1));
                  if (!isSubsampled) 
                  {  pix.push_back(SLRTAAPixel(x,y));
                     isSubsampled = true;
                  }
               }
            }
            gotSampled[x] = isSubsampled;
         }
      } 
      delete[] gotSampled;
      
      // Subsample all pixels in the vector pix
      SLRay::subsampledPixels = pix.size();
      SLint deltaPix = pix.size() / 20;
      
      #ifdef SL_OMP
      #pragma omp parallel for schedule(dynamic)
      #endif
      for (SLint i=0; i<(SLint)pix.size(); ++i)
      {  
         if (!stop)
         {  SLCol4f color = _img[0].getPixeli(pix[i].x, pix[i].y);
            color = subSample(pix[i].x, pix[i].y, color, EYE, BL, LR, LU, pxSize);
            _img[0].setPixeliRGB(pix[i].x, pix[i].y, color);
         
            // Allow the GUI to process events & refresh RT window ~20 times
            if (!_continuous)
            {  _pcRendered = 50+(SLint)((SLfloat)i/pix.size()*50);
               
               #ifdef SL_OMP
               if (!doGUIUpdate) doGUIUpdate = (i%deltaPix==0);
               if (doGUIUpdate && omp_get_thread_num()==0) 
               {  stop = guiRTWndUpdate();
                  doGUIUpdate = false;
               }
               #else
               if (i%deltaPix==0) 
                  stop = guiRTWndUpdate();
               #endif
            }
         }
      }
   }
   ////////////////////////////////////////////////////////////////////////////
   
   _renderSec = (SLfloat)(clock()-clockstart)/(SLfloat)CLOCKS_PER_SEC;
   _pcRendered = 100;
   
   if (_continuous && !stop)
      _state = rtReady;
   else
   {  _state = rtFinished;
      printStats(_renderSec);
   }
   return true;
}
//-----------------------------------------------------------------------------
/*!
This method is the classic recursive ray tracing method that checks the scene
for intersection. If the ray hits an object the local color is calculated and 
if the material is reflective and/or transparent new rays are created and 
passed to this trace method again. If no object got intersected the 
background color is return.
*/
SLCol4f SLRaytracer::trace(SLRay* ray)
{  
   SLScene* s = SLScene::current;
   SLCol4f color(s->backColor());
   
   s->root3D()->hit(ray);
   
   if (ray->length < SL_FLOAT_MAX)
   {  
      color = shade(ray);
      
      if (ray->depth < SLRay::maxDepth && ray->contrib > SLRay::minContrib)
      {  
         if (ray->hitTexCol.a < 1.0f)
         {  SLRay refracted;
            ray->refract(&refracted);
            color += (1.0f-ray->hitTexCol.a) * trace(&refracted);
         } 
         else 
         if (ray->hitMat->kt())
         {  SLRay refracted, reflected;
            ray->refract(&refracted);
            ray->reflect(&reflected);
            SLCol4f refrCol = trace(&refracted);
            SLCol4f reflCol = trace(&reflected);
            
            // Mix refr. & refl. color w. Schlick's Fresnel aproximation
            SLfloat F0 = ray->hitMat->kr();
            SLfloat theta = -(ray->dir * ray->hitNormal);
            SLfloat F_theta = F0 + (1-F0) * pow(1-theta, 5);
            color += refrCol*(1-F_theta) + reflCol*F_theta;
         } else
         {  if (ray->hitMat->kr()) 
            {  SLRay reflected;
               ray->reflect(&reflected);
               color += ray->hitMat->kr() * trace(&reflected);
            }
         }
      }
   }
   
   if (_stateGL->fogIsOn) 
      color = fogBlend(ray->length,color);
   
   color.clampMinMax(0,1);
   return color;
}
//-----------------------------------------------------------------------------
/*!
This method calculates the local illumination at the rays intersection point. 
It uses the OpenGL local light model where the color is calculated as 
follows:
color = material emission + 
        global ambient light scaled by the material's ambient color + 
        ambient, diffuse, and specular contributions from all lights, 
        properly attenuated
*/
SLCol4f SLRaytracer::shade(SLRay* ray)
{  
   SLScene*    s = SLScene::current;
   SLCol4f     localColor = SLCol4f::BLACK;
   SLMaterial* mat = ray->hitMat;
   SLVGLTexture& texture = mat->textures();
   SLVec3f     L,N,H;
   SLfloat     lightDist, LdN, NdH, df, sf, spotEffect, att, lighted = 0.0f;
   SLCol4f     amdi, spec;
   SLCol4f     localSpec(0,0,0,1);
   
   // Don't shade lights. Only take emissive color as material 
   if (typeid(*ray->hitShape)==typeid(SLLightSphere) || 
       typeid(*ray->hitShape)==typeid(SLLightRect))
   {  localColor = mat->emission();
      return localColor;
   } 

   localColor = mat->emission() + (mat->ambient()&s->globalAmbiLight());
      
   ray->hitShape->preShade(ray);
      
   for (SLint i=0; i<s->lights().size(); ++i) 
   {  SLLight* light = s->lights()[i];
   
      if (light && light->on())
      {              
         // calculate light vector L and distance to light
         N.set(ray->hitNormal);
         L.sub(light->positionWS(), ray->hitPoint);
         lightDist = L.length();
         L/=lightDist; 
         LdN = L.dot(N);

         // check shadow ray if hit point is towards the light
         lighted = (LdN>0) ? light->shadowTest(ray, L, lightDist) : 0;
         
         // calculate the ambient part
         amdi = light->ambient() & mat->ambient();
         spec.set(0,0,0);
      
         // calculate spot effect if light is a spotlight
         if (lighted > 0.0f && light->spotCutoff() < 180.0f)
         {  SLfloat LdS = SL_max(-L.dot(light->spotDirWS()), 0.0f);
         
            // check if point is in spot cone
            if (LdS > light->spotCosCut())
            {  spotEffect = pow(LdS, (SLfloat)light->spotExponent());
            } else 
            {  lighted = 0.0f;
               spotEffect = 0.0f;
            }
         } else spotEffect = 1.0f;
         
         // calculate local illumination only if point is not shaded
         if (lighted > 0.0f) 
         {  H.sub(L,ray->dir); // half vector between light & eye
            H.normalize();
            df   = SL_max(LdN     , 0.0f);           // diffuse factor
            NdH  = SL_max(N.dot(H), 0.0f);
            sf = pow(NdH, (SLfloat)mat->shininess()); // specular factor
         
            amdi += lighted * df * light->diffuse() & mat->diffuse();
            spec  = lighted * sf * light->specular()& mat->specular();
         }
      
         // apply attenuation and spot effect
         att = light->attenuation(lightDist) * spotEffect;
         localColor += att * amdi;
         localSpec  += att * spec;
      }
   }

   if (texture.size()) 
   {  localColor &= ray->hitTexCol;    // componentwise multiply
      localColor += localSpec;         // add afterwards the specular component
   } else localColor += localSpec; 
         
   localColor.clampMinMax(0, 1); 
   return localColor;  
}
//-----------------------------------------------------------------------------
/*!
This method is used in SLRaytracer::render for anti aliased ray tracing. 
It loops masksize by masksize times over the pixel at x, y. The returned color 
is averaged and clamped.
*/
SLCol4f SLRaytracer::subSample(SLint x, SLint y,
                               SLCol4f centerColor,
                               SLVec3f EYE, SLVec3f TL, 
                               SLVec3f LR, SLVec3f LU, 
                               SLfloat pxSize)
{  
   assert(_aaSamples%2==1 && "subSample: maskSize must be uneven");
   SLint   centerIndex = _aaSamples>>1;
   SLfloat f = 1.0f/(SLfloat)_aaSamples;
   SLCol4f color(0,0,0);
   SLfloat xpos = x - centerIndex*f;
   SLfloat ypos = y - centerIndex*f;
   SLfloat samples = (SLfloat)_aaSamples*_aaSamples;

   // Loop over float pixel
   for (SLint j=0; j<_aaSamples; ++j)
   {  for (SLint i=0; i<_aaSamples; ++i)
      {  if (i==centerIndex && j==centerIndex) 
            color += centerColor; // don't shoot for center position
         else 
         {  SLVec3f primaryDir(TL + pxSize*((xpos+i*f)*LR + (ypos+j*f)*LU));
            primaryDir.normalize();
            SLRay primaryRay(EYE, primaryDir, x, y);
            color += trace(&primaryRay);
         }
      }
      ypos += f;
   }
   SLRay::subsampledRays += (SLuint)samples;
   color /= samples;
   return color;
}
//-----------------------------------------------------------------------------
/*! 
fogBlend: Blends the a fog color to the passed color according to to OpenGL fog 
calculation. See OpenGL docs for more information on fog properties.
*/
SLCol4f SLRaytracer::fogBlend(SLfloat z, SLCol4f color)
{  
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   SLfloat f=0.0f;
   if (z > sv->_camera->clipFar()) z = sv->_camera->clipFar();
   switch (_stateGL->fogMode)
   {  case 0:  f = (_stateGL->fogDistEnd-z)/
                   (_stateGL->fogDistEnd-_stateGL->fogDistStart); break;
      case 1:  f = exp(-_stateGL->fogDensity*z); break;
      default: f = exp(-_stateGL->fogDensity*z*_stateGL->fogDensity*z); break;
   }
   color = f*color + (1-f)*_stateGL->fogColor;
   color.clampMinMax(0, 1);
   return color;   
}
//-----------------------------------------------------------------------------
/*!
Initialises the statistic variables in SLRay to zero
*/
void SLRaytracer::initStats(SLint depth)
{  
   SLRay::maxDepth = (depth) ? depth : SL_MAXTRACE;
   SLRay::reflectedRays = 0;
   SLRay::refractedRays = 0;
   SLRay::shadowRays = 0;
   SLRay::subsampledRays = 0;
   SLRay::subsampledPixels = 0;
   SLRay::tests = 0;
   SLRay::intersections = 0;
   SLRay::maxDepthReached = 0;
   SLRay::avgDepth = 0.0f;
}
//-----------------------------------------------------------------------------
/*! 
Prints some statistics after the rendering
*/
void SLRaytracer::printStats(SLfloat sec)
{  
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   SLint  primarys = sv->scrW()*sv->scrH();
   SLuint total = primarys + 
                  SLRay::reflectedRays + 
                  SLRay::subsampledRays + 
                  SLRay::refractedRays + 
                  SLRay::shadowRays;
   SL_LOG("\nRendering time    : %10.2f sec.", sec);
   SL_LOG("\nImage size        : %10d x %d",sv->scrW(), sv->scrH());
   SL_LOG("\nNum. Threads      : %10d", _numThreads);
   SL_LOG("\nAllowed depth     : %10d", SLRay::maxDepth);
   SL_LOG("\nMaximum depth     : %10d", SLRay::maxDepthReached);
   SL_LOG("\nAverage depth     : %10.6f", SLRay::avgDepth/primarys);
   SL_LOG("\nAA threshold      : %10.1f", _aaThreshold);
   SL_LOG("\nAA subsampling    : %8dx%d\n", _aaSamples, _aaSamples);
   SL_LOG("\nSubsampled pixels : %10u, %4.1f%% of total", SLRay::subsampledPixels,  
          (SLfloat)SLRay::subsampledPixels/primarys*100.0f);   
   SL_LOG("\nPrimary rays      : %10u, %4.1f%% of total", primarys,               
          (SLfloat)primarys/total*100.0f);
   SL_LOG("\nReflected rays    : %10u, %4.1f%% of total", SLRay::reflectedRays,   
          (SLfloat)SLRay::reflectedRays/total*100.0f);
   SL_LOG("\nTransmitted rays  : %10u, %4.1f%% of total", SLRay::refractedRays, 
          (SLfloat)SLRay::refractedRays/total*100.0f);
   SL_LOG("\nTIR rays          : %10u, %4.1f%% of total", SLRay::tirRays,         
          (SLfloat)SLRay::tirRays/total*100.0f);
   SL_LOG("\nShadow rays       : %10u, %4.1f%% of total", SLRay::shadowRays,      
          (SLfloat)SLRay::shadowRays/total*100.0f);
   SL_LOG("\nAA subsampled rays: %10u, %4.1f%% of total", SLRay::subsampledRays,  
          (SLfloat)SLRay::subsampledRays/total*100.0f);
   SL_LOG("\nTotal rays        : %10u,100.0%%\n", total);
   
   SL_LOG("\nRays per second   : %10u", (SLuint)(total / sec));
   SL_LOG("\nIntersection tests: %10u", SLRay::tests);
   SL_LOG("\nIntersections     : %10u, %4.1f%%", SLRay::intersections, 
          SLRay::intersections/(SLfloat)SLRay::tests*100.0f);
   SL_LOG("\n\n");
}

//-----------------------------------------------------------------------------
/*!
Creates the inherited image in the texture class. The RT is drawn into
a texture map that is displayed with OpenGL in 2D-orthographic projection.
If
*/
void SLRaytracer::createImage(SLint width, SLint height)
{
   // Allocate image of the inherited texture class 
   if (width != _img[0].width() || height != _img[0].height())
   {  
      // Delete the OpenGL Texture if it allready exists
      if (_texName) 
      {  glDeleteTextures(1, &_texName);
         //SL_LOG("glDeleteTextures id: %u   ", _texName);
         _texName = 0;
      }

      // Dispose VBO is they allready exist
      if (_bufP.id()) _bufP.dispose();
      if (_bufT.id()) _bufT.dispose();
      if (_bufI.id()) _bufI.dispose();

      _img[0].allocate(width, height, GL_RGB);
   }
   
   // Fill image black for single RT
   if (!_continuous) _img[0].fill();
}
//-----------------------------------------------------------------------------
/*! 
Draw the RT-Image as a textured quad in 2D-Orthographic projection
*/
void SLRaytracer::renderImage()
{
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();
   SLfloat w = (SLfloat)sv->scrW();
   SLfloat h = (SLfloat)sv->scrH();
   if (w != _img[0].width()) return;
   if (h != _img[0].height()) return;
      
   // Set orthographic projection with the size of the window
   _stateGL->projectionMatrix.ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);
   
   glClear(GL_COLOR_BUFFER_BIT);
   _stateGL->depthTest(false);
   _stateGL->multiSample(false);
   _stateGL->polygonLine(false);
   
   // build buffer object once
   if (!_bufP.id() && !_bufT.id() && !_bufI.id())
   {
      // Vertex X & Y of wnd. corners
      SLfloat P[8] = {0.0f,h, 0.0f,0.0f, w,h, w,0.0f};
      
      // Texture coords of wnd. corners
      SLfloat T[8] = {0.0f,1.0f, 0.0f,0.0f, 1.0f,1.0f, 1.0f,0.0f};
      
      // Indexes for a triangle strip
      SLushort I[4] = {0,1,2,3};
    
      _bufP.generate(P, 4, 2);
      _bufT.generate(T, 4, 2);
      _bufI.generate(I, 4, 1, SL_UNSIGNED_SHORT, SL_ELEMENT_ARRAY_BUFFER);
   }
   
   SLGLShaderProg* sp = SLScene::current->shaderProgs(TextureOnly);
   
   SLGLTexture::bindActive(0); // Enable & build texture with the ray tracing image
   SLGLTexture::fullUpdate();  // Update the OpenGL texture on each draw
   
   // Draw the character triangles                       
   sp->useProgram();
   sp->uniformMatrix4fv("u_mvpMatrix", 1,
                        (SLfloat*)&_stateGL->projectionMatrix);
   sp->uniform1i("u_texture0", 0);
   
   // bind buffers and draw 
   _bufP.bindAndEnableAttrib(sp->getAttribLocation("a_position"));
   _bufT.bindAndEnableAttrib(sp->getAttribLocation("a_texCoord"));
   
   _bufI.bindAndDrawElementsAs(SL_TRIANGLE_STRIP);
   
   _bufP.disableAttribArray();
   _bufT.disableAttribArray();
   
   // Write progress into info text
   if (_pcRendered < 100)
   {  SLchar str[255];  
      sprintf(str,"%s Ray tracing Depth: %d, Threads: %d, Progess: %d%%", 
              _infoText.c_str(), _maxDepth, _numThreads, _pcRendered);
      s->info(str, _infoColor);
   } else s->info(_infoText.c_str(), _infoColor);

   GET_GL_ERROR;
}
//-----------------------------------------------------------------------------
//! Saves the current RT image as PNG image
void SLRaytracer::saveImage()
{  static SLint no = 0;
   SLchar filename[255];  
   sprintf(filename,"Raytrace_%d_%d.png", no++, _maxDepth);
   _img[0].savePNG(filename);
}
//-----------------------------------------------------------------------------