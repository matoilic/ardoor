//#############################################################################
//  File:      SLScene_onLoad.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include "SLScene.h"
#include "SLSceneView.h"
#include "SLGroup.h"
#include "SLCamera.h"
#include "SLLightSphere.h"
#include "SLLightRect.h"
#include "SLMesh.h"
#include "SL3DSMeshFile.h"
#include "SLPolygon.h"
#include "SLBox.h"
#include "SLCone.h"
#include "SLCylinder.h"
#include "SLSphere.h"
#include "SLRectangle.h"
#include "SLRefGroup.h"
#include "SLRefShape.h"
#include "SLKeyframe.h"
#include "SLAnimation.h"

SLNode* SphereGroup(SLint, SLfloat, SLfloat, SLfloat, SLfloat, SLint, SLMaterial*, SLMaterial*);
//-----------------------------------------------------------------------------
//! Creates a recursive sphere group used for the ray tracing scenes
SLNode* SphereGroup(SLint depth,                      // depth of recursion 
                    SLfloat x, SLfloat y, SLfloat z,  // position of group
                    SLfloat scale,                    // scale factor
                    SLint  resolution,       // resolution of spheres
                    SLMaterial* matGlass,    // material for center sphere
                    SLMaterial* matRed)      // material for orbiting spheres
{  if (depth==0)
   {  SLSphere* s = new SLSphere(0.5f*scale,resolution,resolution,"RedSphere", matRed); 
      s->translate(x,y,z);
      return s;
   } else
   {  depth--;
      SLGroup* sGroup = new SLGroup;
      sGroup->translate(x,y,z);
      SLint newRes = max(resolution-8,8);
      sGroup->addNode(new SLSphere(0.5f*scale,resolution,resolution,"RedSphere", matGlass));
      sGroup->addNode(SphereGroup(depth, 0.643951f*scale, 0,               0.172546f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth, 0.172546f*scale, 0,               0.643951f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth,-0.471405f*scale, 0,               0.471405f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth,-0.643951f*scale, 0,              -0.172546f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth,-0.172546f*scale, 0,              -0.643951f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth, 0.471405f*scale, 0,              -0.471405f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth, 0.272166f*scale, 0.544331f*scale, 0.272166f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth,-0.371785f*scale, 0.544331f*scale, 0.099619f*scale, scale/3, newRes, matRed, matRed));
      sGroup->addNode(SphereGroup(depth, 0.099619f*scale, 0.544331f*scale,-0.371785f*scale, scale/3, newRes, matRed, matRed));
      return sGroup;
   }
}
//-----------------------------------------------------------------------------
//! Build a hierarchical figurine with arms and legs
SLGroup* BuildFigureGroup(SLMaterial* material); // avoid warning on XCode
SLGroup* BuildFigureGroup(SLMaterial* material)
{
   SLCylinder* cyl;
   SLSphere* sph;

   // Assemble low arm
   SLGroup* armlow = new SLGroup("armLow group");
   sph = new SLSphere(0.2f, 16, 16, "ellbow");
   armlow->addNode(sph);                        
   cyl = new SLCylinder(0.15f, 1.0f, 1, 16, true, false, "arm");           
   cyl->translate(0.0f, 0.0f, 0.14f);           
   armlow->addNode(cyl);
   
   // Feet
   SLGroup* feet = new SLGroup("feet group");
   sph = new SLSphere(0.2f, 16, 16, "ankle");
   feet->addNode(sph);
   SLBox* feetbox = new SLBox(-0.2f,-0.1f, 0.0f, 0.2f, 0.1f, 0.8f, "foot");
   feetbox->translate(0.0f,-0.25f,-0.15f);
   feet->addNode(feetbox);
   feet->translate(0.0f,0.0f,1.6f);
   feet->rotate(-90.0f, 1.0f, 0.0f, 0.0f);
   
   // Assemble low leg
   SLGroup* leglow = new SLGroup("leglow group");
   leglow->addNode(new SLSphere(0.3f, 16, 16, "knee")); 
   cyl = new SLCylinder(0.2f, 1.4f, 1, 16, false, false, "shin");            
   cyl->translate(0.0f, 0.0f, 0.2f);            
   leglow->addNode(cyl);
   leglow->addNode(feet);

   // Assemble arm
   SLGroup* arm = new SLGroup("arm group");
   sph = new SLSphere(0.3f, 16, 16, "shoulder");
   arm->addNode(sph);                           
   cyl = new SLCylinder(0.2f, 1.0f, 1, 16, false, false, "upper arm");            
   cyl->translate(0.0f, 0.0f, 0.2f);            
   arm->addNode(cyl);
   armlow->translate(0.0f, 0.0f, 1.2f);         
   armlow->rotate(45, -1.0f, 0.0f, 0.0f);       
   arm->addNode(armlow);
   
   // Assemble leg
   SLGroup* leg = new SLGroup("leg group");
   leg->addNode (new SLSphere(0.4f, 16, 16, "hip joint"));     
   cyl = new SLCylinder(0.3f, 1.0f, 1, 16, false, false, "thigh");            
   cyl->translate(0.0f, 0.0f, 0.27f);           
   leg->addNode(cyl);
   leglow->translate(0.0f, 0.0f, 1.27f);        
   leglow->rotate(5, 1.0f, 0.0f, 0.0f);         
   leg->addNode(leglow);

   // Assemble left & right arm
   SLGroup* armLeft = new SLGroup("left arm group");
   armLeft->translate(-1.1f, 0.0f, 0.3f);       
   armLeft->rotate(10, -1,0,0);
   armLeft->addNode(arm);
   SLGroup* armRight= new SLGroup("right arm group");
   armRight->translate(1.1f, 0.0f, 0.3f);       
   armRight->rotate(-60, -1,0,0);
   armRight->addNode(arm->copy());

   // Assemble left & right leg
   SLGroup* legLeft = new SLGroup("left leg group");
   legLeft->translate(-0.4f, 0.0f, 2.2f);       
   legLeft->rotate(-10, -1,0,0);
   legLeft->addNode(leg);               
   SLGroup* legRight= new SLGroup("right leg group");           
   legRight->translate(0.4f, 0.0f, 2.2f);       
   legRight->rotate(70, -1,0,0);
   legRight->addNode(leg->copy());  

   // Assemble head & neck
   SLSphere* head = new SLSphere(0.5f, 16, 16, "Head");
   head->translate(0.0f, 0.0f,-0.7f);
   SLCylinder* neck = new SLCylinder(0.25f, 0.3f, 1, 16, false, false, "neck");
   neck->translate(0.0f, 0.0f,-0.3f);
      
   // Assemble figure Left
   SLGroup* figure = new SLGroup("figure");
   figure->addNode(new SLBox(-0.8f,-0.4f, 0.0f, 0.8f, 0.4f, 2.0f, "Box", material));
   figure->addNode(head);
   figure->addNode(neck);
   figure->addNode(armLeft);
   figure->addNode(armRight);
   figure->addNode(legLeft);
   figure->addNode(legRight);
   figure->rotate(90, 1,0,0);

   return figure;
}
//-----------------------------------------------------------------------------
//! SLScene::onLoad(int sceneName) builds a scene from source code.
/*! SLScene::onLoad builds a scene from source code.
\param sceneName is the scene to choose and corresponds to enumeration 
SLCommand value for the different scenes. The first scene is cmdSceneFigure.
*/
void SLScene::onLoad(SLCmd sceneName)
{  
   // Initialize all preloaded stuff from SLScene
   cout << "------------------------------------" << endl;
   init();
   _current = sceneName;

   if (sceneName == cmdSceneSmallTest)
   {
      name("Little Test Scene");
  
      // Create texture and material
      SLMaterial* matRed = new SLMaterial("matRed",SLCol4f::RED, SLCol4f::BLACK);

      // Add pointers to the scenes resource vectors
      _materials.push_back(matRed);

      // Define a light
      SLLightSphere* light1 = new SLLightSphere(150, 120, 200, 20);
      light1->ambient (SLCol4f(0.2f,0.2f,0.2f));
      light1->diffuse (SLCol4f(1.0f,1.0f,1.0f));
      light1->specular(SLCol4f(1.0f,1.0f,1.0f));
      light1->attenuation(1,0,0);
   
      // Define camera
      SLCamera* cam1 = new SLCamera;
      cam1->fov(62);
      cam1->posAtUp(0, 50, 150);
      cam1->clipNear(1);
      cam1->clipFar(10000);
      cam1->focalDist(150);
      cam1->eyeSep(cam1->focalDist()/30);
      cam1->projection(monoPerspective);

      SLBox* box = new SLBox(SLVec3f(-50,-50,-50), SLVec3f(50,50,50), "Box", matRed);

      // Assemble scene
      SLGroup* scene = new SLGroup;
      scene->addNode(light1);
      scene->addNode(box);
      scene->addNode(cam1);

      // Set backround color, active camera & the root pointer
      _backColor.set(SLCol4f::BLACK);
      _activeSV->camera(cam1);     
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneFigure)
   {  
      name("Hierarchical Figure Scene"); 
      info("Hierarchical scene structure and animation test. Turn on the bounding boxes to see the animation curves");

      // Create textures and materials
      SLGLTexture* tex1 = new SLGLTexture("Checkerboard0512_C.png");
      SLMaterial* m1 = new SLMaterial("m1", tex1); m1->kr(0.5f);
      SLMaterial* m2 = new SLMaterial("m2", SLCol4f::WHITE*0.5, SLCol4f::WHITE,128, 0.5f, 0.0f, 1.0f);
      _textures.push_back(tex1);
      _materials.push_back(m1);
      _materials.push_back(m2);

      // Define a light
      SLLightSphere* light1 = new SLLightSphere(0, 2, 0, 0.5f);
      light1->ambient (SLCol4f(0.2f,0.2f,0.2f));
      light1->diffuse (SLCol4f(0.9f,0.9f,0.9f));
      light1->specular(SLCol4f(0.9f,0.9f,0.9f));
      light1->attenuation(1,0,0);
      SLLightSphere* light2 = new SLLightSphere(0, 0, 0, 0.2f);
      light2->ambient (SLCol4f(0.2f,0.0f,0.0f));
      light2->diffuse (SLCol4f(0.9f,0.0f,0.0f));
      light2->specular(SLCol4f(0.9f,0.9f,0.9f));
      light2->attenuation(1,0,0);

      // Define 3 cameras
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp( 0, 0, 22);
      cam1->focalDist(22);

      // Floor rectangle
      SLRectangle* rect = new SLRectangle(SLVec2f(-5,-5), SLVec2f(5,5), 20, 20, "Floor", m1);
      rect->rotate(90, -1,0,0);
      rect->translate(0,0,-5.5f);

      // Bouncing balls
      SLSphere* ball1 = new SLSphere(0.3f, 16, 16, "Ball1", m2);
      ball1->translate( 0.0f,0,4);
      ball1->animation(new SLAnimation(1, SLVec3f(0,-5.2f,0), pingPongLoop, linear));
      SLSphere* ball2 = new SLSphere(0.3f, 16, 16, "Ball2", m2);
      ball2->translate(-1.5f,0,4);
      ball2->animation(new SLAnimation(1, SLVec3f(0,-5.2f,0), pingPongLoop, inQuad));
      SLSphere* ball3 = new SLSphere(0.3f, 16, 16, "Ball3", m2);
      ball3->translate(-2.5f,0,4);
      ball3->animation(new SLAnimation(1, SLVec3f(0,-5.2f,0), pingPongLoop, outQuad));
      SLSphere* ball4 = new SLSphere(0.3f, 16, 16, "Ball4", m2);
      ball4->translate( 1.5f,0,4);
      ball4->animation(new SLAnimation(1, SLVec3f(0,-5.2f,0), pingPongLoop, inOutQuad));
      SLSphere* ball5 = new SLSphere(0.3f, 16, 16, "Ball5", m2);
      ball5->translate( 2.5f,0,4);
      ball5->animation(new SLAnimation(1, SLVec3f(0,-5.2f,0), pingPongLoop, outInQuad));

      ///////////////////////////////////////
      SLGroup* figure = BuildFigureGroup(m2);
      ///////////////////////////////////////

      // Add animations for left leg
      SLGroup* legLeft = (SLGroup*)figure->getNode("left leg group");
      legLeft->rotate(-45, 1,0,0);
      legLeft->animation(new SLAnimation(2, 60, SLVec3f(1,0,0), pingPongLoop));
      SLGroup* legLowLeft = (SLGroup*)legLeft->getNode("leglow group");
      legLowLeft->animation(new SLAnimation(2, 40, SLVec3f(1,0,0), pingPongLoop));
      SLGroup* feetLeft = (SLGroup*)legLeft->getNode("feet group");
      feetLeft->animation(new SLAnimation(2, 40, SLVec3f(1,0,0), pingPongLoop));
      SLGroup* legRight = (SLGroup*)figure->getNode("right leg group");
      legRight->rotate(70, 1,0,0);

      // Add animation for light 1
      light1->animation(new SLAnimation(4, 6, ZAxis, 6, XAxis, loop));

      // Add animation for light 2
      SLVKeyframe light2Curve;
      light2Curve.push_back(SLKeyframe(0, SLVec3f(-8,-4, 0)));
      light2Curve.push_back(SLKeyframe(1, SLVec3f( 0, 4, 0)));
      light2Curve.push_back(SLKeyframe(1, SLVec3f( 8,-4, 0)));

      light2->animation(new SLAnimation(light2Curve, 0, pingPongLoop));

      // Assemble scene
      SLGroup* scene = new SLGroup("scene group");
      scene->addNode(light1);
      scene->addNode(light2);
      scene->addNode(rect);
      scene->addNode(figure);
      scene->addNode(ball1);
      scene->addNode(ball2);
      scene->addNode(ball3);
      scene->addNode(ball4);
      scene->addNode(ball5);
      scene->addNode(cam1);

      // Set backround color, active camera & the root pointer
      _backColor.set(SLCol4f(0.1f,0.4f,0.8f));
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneMesh3DS) //........................................
   {
      name("Mesh 3D Loader Test");
      info("3DStudio File (3ds) import Test.");
      
      SLMaterial* matBlu = new SLMaterial("Blue",  SLCol4f(0,0,0.2f),       SLCol4f(1,1,1), 100, 0.8f, 0);
      SLMaterial* matRed = new SLMaterial("Red",   SLCol4f(0.2f,0,0),       SLCol4f(1,1,1), 100, 0.8f, 0);
      SLMaterial* matGre = new SLMaterial("Green", SLCol4f(0,0.2f,0),       SLCol4f(1,1,1), 100, 0.8f, 0);
      SLMaterial* matGra = new SLMaterial("Gray",  SLCol4f(0.3f,0.3f,0.3f), SLCol4f(1,1,1), 100, 0,    0);
      
      // Add shared resources to the arrays in SLScene
      _materials.push_back(matBlu);
      _materials.push_back(matRed);
      _materials.push_back(matGre);
      _materials.push_back(matGra);
      
      SLCamera* cam1 = new SLCamera(); 
      cam1->name("cam1");
      cam1->clipNear(10);
      cam1->clipFar(2000);
      cam1->posAtUp(0,0,450);
      cam1->speedLimit(40);
      cam1->focalDist(450);  
      cam1->eyeSep(cam1->focalDist()/30.0f);
      
      SLLightSphere* light1 = new SLLightSphere(80, 80, 80, 10);
      light1->ambient(SLCol4f(0.1f, 0.1f, 0.1f));
      light1->diffuse(SLCol4f(1.0f, 1.0f, 1.0f));
      light1->specular(SLCol4f(1.0f, 1.0f, 1.0f));
      light1->attenuation(1,0,0);
      light1->animation(new SLAnimation(2,SLVec3f(0,0,-160), pingPongLoop, inOutCubic));
      //light1->samples(8,8);
      
      SLLightSphere* light2 = new SLLightSphere(-80, -80, 80, 10);
      light2->ambient(SLCol4f(0.1f, 0.1f, 0.1f));
      light2->diffuse(SLCol4f(1.0f, 1.0f, 1.0f));
      light2->specular(SLCol4f(1.0f, 1.0f, 1.0f));
      light2->attenuation(1,0,0);
      light2->animation(new SLAnimation(2,SLVec3f(0,160,0), pingPongLoop, inOutCubic));
      
      SLGroup* gm;
      //gm = SL3DSMeshFile::load("enterprise.3ds", 0); cam1->posAtUp(0,0,500); light1->posAtUp(300,200,300);
      //gm = SL3DSMeshFile::load("feto_col.3ds", 0); cam1->posAtUp(3,0,35, 3,0,0);   
      //gm = SL3DSMeshFile::load("liberty.3ds", 0, false); gm->scaleToCenter(150);
      //gm = SL3DSMeshFile::load("Skeleton2.3DS", 0, false); gm->scaleToCenter(150);
      //gm = SL3DSMeshFile::load("Teapot.3DS", 0, true); gm->scaleToCenter(150);
      //gm = SL3DSMeshFile::load("SkeletonLow.3DS", 0, false); gm->scaleToCenter(150);
      //gm = SL3DSMeshFile::load("SkeletonSmall.3DS", 0, false); gm->scaleToCenter(150);
      //gm = SL3DSMeshFile::load("stanfordbunny.3ds", 0, true); cam1->posAtUp(0,0,35);  
      //gm = SL3DSMeshFile::load("Halloween/Jackolan.3DS", 0, true); gm->scaleToCenter(90); 
      //gm = SL3DSMeshFile::load("F5/f5e.3DS", 0, true); gm->scaleToCenter(150);
      #if defined(SL_OS_IOS) || defined(SL_OS_ANDROID)
      gm = SL3DSMeshFile::load("jackolan.3ds", 0, true); gm->scaleToCenter(150);
      #else
      gm = SL3DSMeshFile::load("Halloween/Jackolan.3DS", 0, true); gm->scaleToCenter(150);
      #endif
      //gm = SL3DSMeshFile::load("Iveco/Iveco.3ds", 0, false, true); gm->scaleToCenter(150);
      //gm = SL3DSMeshFile::load("SphereBox.3DS", 0); gm->scaleToCenter(100);
      //gm = SL3DSMeshFile::load("plane.3ds", 0); gm->scaleToCenter(50);
      //gm = SL3DSMeshFile::load("box.3ds", 2); gm->rotate(45, 1,1,1); cam1->posAtUp(0,0,30);  
      //gm = SL3DSMeshFile::load("Mauer.3ds", 3, false, true); gm->translate(-0.5f,-0.5f,0); cam1->posAtUp(0,0,2);    
      //gm = SL3DSMeshFile::load("Sutz/SutzAC10.3ds", 0, false, true); cam1->posAtUp(0,0,30);   
      
      // define rectangles for the surrounding box
      SLfloat b=100; // edge size of rectangles
      SLint   s=10;  // subdivision of rectangles
      SLRectangle *rb, *rl, *rr, *rf, *rt;
      rb = new SLRectangle(SLVec2f(-b,-b), SLVec2f(b,b),s,s,"rectB", matBlu);                         rb->translate(0,0,-b);
      rl = new SLRectangle(SLVec2f(-b,-b), SLVec2f(b,b),s,s,"rectL", matRed); rl->rotate( 90, 0,1,0); rl->translate(0,0,-b);
      rr = new SLRectangle(SLVec2f(-b,-b), SLVec2f(b,b),s,s,"rectR", matGre); rr->rotate(-90, 0,1,0); rr->translate(0,0,-b);
      rf = new SLRectangle(SLVec2f(-b,-b), SLVec2f(b,b),s,s,"rectF", matGra); rf->rotate(-90, 1,0,0); rf->translate(0,0,-b);
      rt = new SLRectangle(SLVec2f(-b,-b), SLVec2f(b,b),s,s,"rectT", matGra); rt->rotate( 90, 1,0,0); rt->translate(0,0,-b);
      
      SLGroup* scene = new SLGroup("Scene");
      scene->addNode(light1);
      scene->addNode(light2);
      scene->addNode(rb);
      scene->addNode(rl);
      scene->addNode(rr);
      scene->addNode(rf);
      scene->addNode(rt);
      scene->addNode(gm);
      scene->addNode(cam1);
      
      _backColor.set(0.5f,0.5f,0.5f);
      _activeSV->camera(cam1);      
      _root3D = scene;
   } 
   else
   if (sceneName == cmdSceneTextureBlend) //...................................
   {
      name("Blending: Texture Transparency with sorting");
      info("Texture map blending with depth sorting. Trees in view frustum are rendered back to front.");                                                 
        
      SLGLTexture* t1 = new SLGLTexture("tree1_1024_C.png",
                                    GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,
                                    ColorMap, 
                                    GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      SLGLTexture* t2 = new SLGLTexture("grass0512_C.jpg", 
                                    GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

      SLMaterial* m1 = new SLMaterial("m1", SLCol4f(1,1,1), SLCol4f(0,0,0), 100);
      SLMaterial* m2 = new SLMaterial("m2", SLCol4f(1,1,1), SLCol4f(0,0,0), 100);
      m1->shaderProg(_shaderProgs[TextureOnly]);
      m1->textures().push_back(t1);
      m2->textures().push_back(t2);
      
      // Add shared resources to the arrays in SLScene
      _textures.push_back(t1);
      _textures.push_back(t2);
      _materials.push_back(m1);
      _materials.push_back(m2);

      SLCamera* cam1 = new SLCamera(); 
      cam1->name("cam1");
      cam1->posAtUp(0,3,25, 0,0,10);
      cam1->focalDist(25);
      
      SLLightSphere* light = new SLLightSphere(0.1f);
      light->lightAt(5,5,5);
      light->attenuation(1,0,0);
      
      // Build arrays for polygon vertices and texcords for tree
      SLVVec3f pNW, pSE;
      SLVVec2f tNW, tSE;
      pNW.push_back(SLVec3f( 0, 0,0)); tNW.push_back(SLVec2f(0.5f,0.0f));
      pNW.push_back(SLVec3f( 1, 0,0)); tNW.push_back(SLVec2f(1.0f,0.0f));
      pNW.push_back(SLVec3f( 1, 2,0)); tNW.push_back(SLVec2f(1.0f,1.0f));
      pNW.push_back(SLVec3f( 0, 2,0)); tNW.push_back(SLVec2f(0.5f,1.0f));
      pSE.push_back(SLVec3f(-1, 0,0)); tSE.push_back(SLVec2f(0.0f,0.0f));
      pSE.push_back(SLVec3f( 0, 0,0)); tSE.push_back(SLVec2f(0.5f,0.0f));
      pSE.push_back(SLVec3f( 0, 2,0)); tSE.push_back(SLVec2f(0.5f,1.0f));
      pSE.push_back(SLVec3f(-1, 2,0)); tSE.push_back(SLVec2f(0.0f,1.0f));
      
      // Build tree out of 4 polygons
      SLPolygon* p1 = new SLPolygon(pNW, tNW, "Tree+X", m1);
      SLPolygon* p2 = new SLPolygon(pNW, tNW, "Tree-Z", m1);  p2->rotate(90, 0,1,0);
      SLPolygon* p3 = new SLPolygon(pSE, tSE, "Tree-X", m1); 
      SLPolygon* p4 = new SLPolygon(pSE, tSE, "Tree+Z", m1);  p4->rotate(90, 0,1,0);
      
      // Turn face culling off so that we see both sides
      p1->drawBits()->on(SL_DB_CULLOFF);
      p2->drawBits()->on(SL_DB_CULLOFF);
      p3->drawBits()->on(SL_DB_CULLOFF);
      p4->drawBits()->on(SL_DB_CULLOFF);
      
      // Build tree group
      SLGroup* tree = new SLGroup("grTree");
      tree->addNode(p1);
      tree->addNode(p2);
      tree->addNode(p3);
      tree->addNode(p4);
      
      // Build arrays for polygon vertices and texcords for ground
      SLVVec3f pG;
      SLVVec2f tG;
      pG.push_back(SLVec3f(-22, 0, 22)); tG.push_back(SLVec2f( 0, 0));
      pG.push_back(SLVec3f( 22, 0, 22)); tG.push_back(SLVec2f(30, 0));
      pG.push_back(SLVec3f( 22, 0,-22)); tG.push_back(SLVec2f(30,30));
      pG.push_back(SLVec3f(-22, 0,-22)); tG.push_back(SLVec2f( 0,30));
      
      SLGroup* scene = new SLGroup("grScene");
      scene->addNode(light);
      scene->addNode(tree);
      scene->addNode(new SLPolygon(pG, tG, "Ground", m2));
      
      //create 21*21*21-1 references around the center tree
      SLint size = 10;
      for (SLint iZ=-size; iZ<=size; ++iZ)
      {  for (SLint iX=-size; iX<=size; ++iX)
         {  if (iX!=0 || iZ!=0)
            {  SLRefGroup* rg = new SLRefGroup(tree);     
               rg->translate(float(iX)*2+SL_random(0.7f,1.4f), 
                             0, 
                             float(iZ)*2+SL_random(0.7f,1.4f));
               rg->rotate(SL_random(0, 90), 0,1,0);
               rg->scale(SL_random(0.5f,1.0f));
               scene->addNode(rg);
            }
         }
      }

      scene->addNode(cam1);
      
      _backColor.set(0.6f,0.6f,1);
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneRevolver) //.......................................
   {
      name("Revolving Mesh Test w. glass shader");
      info("Examples of revolving mesh objects constructed by rotating a 2D curve. The glass shader reflects and refracts the environment map. Try ray tracing."); 
     
      // Testmap material
      SLGLTexture* tex1 = new SLGLTexture("Testmap_0512_C.png"); 
      SLMaterial* mat1 = new SLMaterial("mat1", tex1);
      _materials.push_back(mat1);
      _textures.push_back(tex1);

      // floor material
      SLGLTexture* tex2 = new SLGLTexture("wood0_0512_C.jpg");
      SLMaterial* mat2 = new SLMaterial("mat2", tex2); 
      mat2->specular(SLCol4f::BLACK);
      _materials.push_back(mat2);
      _textures.push_back(tex2);
      
      // Back wall material
      SLGLTexture* tex3 = new SLGLTexture("bricks1_0512_C.jpg");
      SLMaterial* mat3 = new SLMaterial("mat3", tex3); 
      mat3->specular(SLCol4f::BLACK);
      _materials.push_back(mat3);
      _textures.push_back(tex3);
      
      // Left wall material
      SLGLTexture* tex4 = new SLGLTexture("wood2_0512_C.jpg");
      SLMaterial* mat4 = new SLMaterial("mat4", tex4, 0, 0, 0, _shaderProgs[PerPixBlinnTex]); 
      mat4->specular(SLCol4f::BLACK);
      _materials.push_back(mat4);
      _textures.push_back(tex4);
      
      // Glass material
      SLGLTexture* tex5 = new SLGLTexture("wood2_0256_C.jpg", "wood2_0256_C.jpg"
                                         ,"gray_0256_C.jpg", "wood0_0256_C.jpg"
                                         ,"gray_0256_C.jpg", "bricks1_0256_C.jpg");
      SLMaterial* mat5 = new SLMaterial("glass", SLCol4f::BLACK, SLCol4f::WHITE,
                                        100, 0.2f, 0.8f, 1.5f);
      mat5->textures().push_back(tex5);
      SLGLShaderProg* sp1 = new SLGLShaderProgGeneric("RefractReflect.vert", 
                                                      "RefractReflect.frag");
      mat5->shaderProg(sp1);
      _materials.push_back(mat5);
      _textures.push_back(tex5);
      _shaderProgs.push_back(sp1);

      // camera
      SLCamera* cam1 = new SLCamera(); 
      cam1->name("cam1");
      cam1->posAtUp(0,0,17);
      cam1->focalDist(17);
      
      // light
      SLLightSphere* light1 = new SLLightSphere(0, 4, 0, 0.3f);
      light1->diffuse(SLCol4f(1, 1, 1));
      light1->ambient(SLCol4f(0.2f, 0.2f, 0.2f));
      light1->specular(SLCol4f(1, 1, 1));
      light1->attenuation(1,0,0);
      light1->animation(new SLAnimation(4, 6, ZAxis, 6, XAxis, loop));
      
      // wine glass
      SLVVec3f revP; 
      revP.push_back(SLVec3f(0.00f, 0.00f));
      revP.push_back(SLVec3f(2.00f, 0.00f));
      revP.push_back(SLVec3f(2.00f, 0.00f));
      revP.push_back(SLVec3f(2.00f, 0.10f));
      revP.push_back(SLVec3f(1.95f, 0.15f));
      
      revP.push_back(SLVec3f(0.40f, 0.50f));
      revP.push_back(SLVec3f(0.25f, 0.60f));
      revP.push_back(SLVec3f(0.20f, 0.70f));
      revP.push_back(SLVec3f(0.30f, 3.00f));

      revP.push_back(SLVec3f(0.30f, 3.00f));
      revP.push_back(SLVec3f(0.20f, 3.10f));
      revP.push_back(SLVec3f(0.20f, 3.10f));
      
      revP.push_back(SLVec3f(1.20f, 3.90f));
      revP.push_back(SLVec3f(1.60f, 4.30f));
      revP.push_back(SLVec3f(1.95f, 4.80f));
      revP.push_back(SLVec3f(2.15f, 5.40f));
      revP.push_back(SLVec3f(2.20f, 6.20f));
      revP.push_back(SLVec3f(2.10f, 7.10f));
      revP.push_back(SLVec3f(2.05f, 7.15f));

      revP.push_back(SLVec3f(2.00f, 7.10f));
      revP.push_back(SLVec3f(2.05f, 6.00f));
      revP.push_back(SLVec3f(1.95f, 5.40f));
      revP.push_back(SLVec3f(1.70f, 4.80f));
      revP.push_back(SLVec3f(1.30f, 4.30f));
      revP.push_back(SLVec3f(0.80f, 4.00f));
      revP.push_back(SLVec3f(0.20f, 3.80f));
      revP.push_back(SLVec3f(0.00f, 3.82f));
      SLRevolver* glass = new SLRevolver(revP, SLVec3f(0,1,0), 32, false, false, "Revolver", mat5);
      glass->translate(0.0f,-3.5f, 0.0f);
      
      SLSphere* sphere = new SLSphere(1,16,16, "mySphere", mat1);         
      sphere->translate(3,0,0);
      
      SLCylinder* cylinder = new SLCylinder(1, 2, 3, 16, true, true, "myCylinder", mat1);   
      cylinder->translate(-3,0,-1);
      
      SLCone* cone = new SLCone(1, 3, 3, 16, true, "myCone", mat1);
      cone->rotate(90, -1,0,0);
      cone->translate(0,0,2.5f);

      // Cube dimensions
      SLfloat pL = -9.0f, pR = 9.0f; // left/right
      SLfloat pB = -3.5f, pT =14.5f; // bottom/top
      SLfloat pN =  9.0f, pF =-9.0f; // near/far
      
      // bottom rectangle
      SLRectangle* b = new SLRectangle(SLVec2f(pL,-pN), SLVec2f(pR,-pF), 6, 6, "PolygonFloor", mat2); 
      b->rotate(90, -1,0,0); b->translate(0,0,pB);
   
      // top rectangle
      SLRectangle* t = new SLRectangle(SLVec2f(pL,pF), SLVec2f(pR,pN), 6, 6, "top", mat2); 
      t->rotate(90, 1,0,0); t->translate(0,0,-pT);
   
      // far rectangle
      SLRectangle* f = new SLRectangle(SLVec2f(pL,pB), SLVec2f(pR,pT), 6, 6, "far", mat3); 
      f->translate(0,0,pF);
   
      // left rectangle
      SLRectangle* l = new SLRectangle(SLVec2f(-pN,pB), SLVec2f(-pF,pT), 6, 6, "left", mat4); 
      l->rotate(90, 0,1,0); l->translate(0,0,pL);
   
      // right rectangle
      SLRectangle* r = new SLRectangle(SLVec2f(pF,pB), SLVec2f(pN,pT), 6, 6, "right", mat4); 
      r->rotate(90, 0,-1,0); r->translate(0,0,-pR);
      
      SLGroup* scene = new SLGroup;
      scene->addNode(light1);
      scene->addNode(glass);
      scene->addNode(sphere);
      scene->addNode(cylinder);
      scene->addNode(cone);
      scene->addNode(b);
      scene->addNode(f);
      scene->addNode(t);
      scene->addNode(l);
      scene->addNode(r);
      scene->addNode(cam1);   
      
      _backColor.set(0.5f,0.5f,0.5f);
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneFrustumCull) //....................................
   {
      name("Frustum Culling Test");
      info("View frustum culling: Only objects in view frustum are rendered. You can turn view culling off in the render flags.");
      
      // create texture
      SLGLTexture* tex = new SLGLTexture("earth1024_C.jpg");
      SLMaterial* mat1 = new SLMaterial("mat1", tex);
      _materials.push_back(mat1);
      _textures.push_back(tex);
      
      SLCamera* cam1 = new SLCamera(); 
      cam1->name("cam1");
      cam1->clipNear(0.1f);
      cam1->clipFar(100);
      cam1->posAtUp(0,0,5); 
      cam1->focalDist(5);
      
      SLLightSphere* light1 = new SLLightSphere(10, 10, 10, 0.3f);
      light1->ambient(SLCol4f(0.2f, 0.2f, 0.2f));
      light1->diffuse(SLCol4f(0.8f, 0.8f, 0.8f));
      light1->specular(SLCol4f(1, 1, 1));
      light1->attenuation(1,0,0);

      SLGroup* scene = new SLGroup;
      scene->addNode(cam1);
      scene->addNode(light1);
      
      // add one single sphere in the center
      SLint resolution = 16;
      SLSphere* sphere = new SLSphere(0.15f, resolution, resolution, "mySphere", mat1);
      scene->addNode(sphere);
      
      // create spheres around the center sphere
      SLint size = 10;
      for (SLint iZ=-size; iZ<=size; ++iZ)
      {  for (SLint iY=-size; iY<=size; ++iY)
         {  for (SLint iX=-size; iX<=size; ++iX)
            {  if (iX!=0 || iY!=0 || iZ !=0)
               {  
                  SLRefShape* s = new SLRefShape(sphere);
                  s->translate(float(iX), float(iY), float(iZ)); 
                  scene->addNode(s);
               }
            }
         }
      }
      
      SLint num = size + size + 1;
      SL_LOG("Triangles in scene: %d\n", resolution*resolution*2*num*num*num);
      
      _backColor.set(0.1f,0.1f,0.1f);
      _activeSV->camera(cam1);
      _root3D = scene;  
   }
   else
   if (sceneName == cmdSceneTextureFilter) //..................................
   {
      name("Texturing: Filter Compare");
      info("Texture filter comparison: Bottom: nearest neighbour, left: linear, top: linear mipmap, right: anisotropic");
   
      // Create 4 textures with different filter modes
      SLGLTexture* texB = new SLGLTexture("brick0512_C.png"
                                      ,GL_NEAREST
                                      ,GL_NEAREST);
      SLGLTexture* texL = new SLGLTexture("brick0512_C.png"
                                      ,GL_LINEAR
                                      ,GL_LINEAR);
      SLGLTexture* texT = new SLGLTexture("brick0512_C.png"
                                      ,GL_LINEAR_MIPMAP_LINEAR
                                      ,GL_LINEAR);
      SLGLTexture* texR = new SLGLTexture("brick0512_C.png"
                                      ,SL_ANISOTROPY_MAX
                                      ,GL_LINEAR);
                                   
      // define materials with textureOnly shader, no light needed
      SLMaterial* matB = new SLMaterial("matB", texB,0,0,0, _shaderProgs[TextureOnly]);
      SLMaterial* matL = new SLMaterial("matL", texL,0,0,0, _shaderProgs[TextureOnly]);
      SLMaterial* matT = new SLMaterial("matT", texT,0,0,0, _shaderProgs[TextureOnly]);
      SLMaterial* matR = new SLMaterial("matR", texR,0,0,0, _shaderProgs[TextureOnly]);
   
      // Add shared resources to the arrays in SLScene
      _textures.push_back(texB);
      _textures.push_back(texL);
      _textures.push_back(texT);
      _textures.push_back(texR);
      _materials.push_back(matB);
      _materials.push_back(matL);
      _materials.push_back(matT);
      _materials.push_back(matR);
   
      // build polygons for bottom, left, top & right side
      SLVVec3f VB; 
      VB.push_back(SLVec3f(-0.5f,-0.5f, 1.0f));
      VB.push_back(SLVec3f( 0.5f,-0.5f, 1.0f));
      VB.push_back(SLVec3f( 0.5f,-0.5f,-2.0f));
      VB.push_back(SLVec3f(-0.5f,-0.5f,-2.0f));
      SLVVec2f T; 
      T.push_back(SLVec2f( 0.0f, 2.0f));
      T.push_back(SLVec2f( 0.0f, 0.0f));
      T.push_back(SLVec2f( 6.0f, 0.0f));
      T.push_back(SLVec2f( 6.0f, 2.0f));
      SLPolygon* polyB = new SLPolygon(VB, T, "PolygonB", matB);
   
      SLVVec3f VL; 
      VL.push_back(SLVec3f(-0.5f, 0.5f, 1.0f));
      VL.push_back(SLVec3f(-0.5f,-0.5f, 1.0f));
      VL.push_back(SLVec3f(-0.5f,-0.5f,-2.0f));
      VL.push_back(SLVec3f(-0.5f, 0.5f,-2.0f));
      SLPolygon* polyL = new SLPolygon(VL, T, "PolygonL", matL);
   
      SLVVec3f VT; 
      VT.push_back(SLVec3f( 0.5f, 0.5f, 1.0f));
      VT.push_back(SLVec3f(-0.5f, 0.5f, 1.0f));
      VT.push_back(SLVec3f(-0.5f, 0.5f,-2.0f));
      VT.push_back(SLVec3f( 0.5f, 0.5f,-2.0f));
      SLPolygon* polyT = new SLPolygon(VT, T, "PolygonT", matT);
   
      SLVVec3f VR; 
      VR.push_back(SLVec3f( 0.5f,-0.5f, 1.0f));
      VR.push_back(SLVec3f( 0.5f, 0.5f, 1.0f));
      VR.push_back(SLVec3f( 0.5f, 0.5f,-2.0f));
      VR.push_back(SLVec3f( 0.5f,-0.5f,-2.0f));
      SLPolygon* polyR = new SLPolygon(VR, T, "PolygonR", matR);
   
      SLSphere* sphere = new SLSphere(0.2f,16,16,"Sphere", matR);
      sphere->rotate(90, 1,0,0);
      
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0,0,2.2f);
      cam1->focalDist(2.2f);

      SLGroup* scene = new SLGroup();
      scene->addNode(polyB);
      scene->addNode(polyL);
      scene->addNode(polyT);
      scene->addNode(polyR);
      scene->addNode(sphere);
      scene->addNode(cam1);

      _backColor.set(SLCol4f(0.2f,0.2f,0.2f));
      _activeSV->camera(cam1);
      _root3D = scene; 
   }
   else
   if (sceneName == cmdScenePerVertexBlinn) //.................................
   {
      name("Blinn-Phong per vertex lighting");
      info("Per-vertex lighting with Blinn-Phong lightmodel. The reflection of 4 light sources is calculated per vertex and is then interpolated over the triangles.");
                                          
      // create material
      SLMaterial* m1 = new SLMaterial("m1", 0,0,0,0, _shaderProgs[PerVrtBlinn]);
      m1->shininess(500);

      // Add shared resources to the arrays in SLScene
      _materials.push_back(m1);
      
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0,1,8, 0,1,0);
      cam1->focalDist(8);
      
      // define 4 light sources
      SLLightRect* light0 = new SLLightRect(2.0f,1.0f);
      light0->ambient(SLCol4f(0,0,0));
      light0->diffuse(SLCol4f(1,1,1));
      light0->lightAt(0,3,0, 0,0,0, 0,0,-1);
      light0->attenuation(0,0,1);

      SLLightSphere* light1 = new SLLightSphere(0.1f);
      light1->ambient(SLCol4f(0,0,0));
      light1->diffuse(SLCol4f(1,0,0));
      light1->specular(SLCol4f(1,0,0));
      light1->translate(0,0,2);
      light1->attenuation(0,0,1);

      SLLightSphere* light2 = new SLLightSphere(0.1f);
      light2->ambient(SLCol4f(0,0,0));
      light2->diffuse(SLCol4f(0,1,0));
      light2->specular(SLCol4f(0,1,0));
      light2->lightAt(1.5, 1.5, 1.5);
      light2->spotCutoff(20);
      light2->attenuation(0,0,1);
      
      SLLightSphere* light3 = new SLLightSphere(0.1f);
      light3->ambient(SLCol4f(0,0,0));
      light3->diffuse(SLCol4f(0,0,1));
      light3->specular(SLCol4f(0,0,1));
      light3->lightAt(-1.5, 1.5, 1.5);
      light3->spotCutoff(20);
      light3->attenuation(0,0,1);
      
      // Assemble scene graph
      SLGroup* scene = new SLGroup;
      scene->addNode(cam1);
      scene->addNode(light0);
      scene->addNode(light1);
      scene->addNode(light2);
      scene->addNode(light3);
      scene->addNode(new SLSphere(1.0f, 20, 20, "Sphere", m1));
      scene->addNode(new SLBox(1,-1,-1, 2,1,1, "Box", m1)); 

      _backColor.set(SLCol4f(0.1f,0.1f,0.1f));
      _activeSV->camera(cam1);
      _root3D = scene;   
   }
   else
   if (sceneName == cmdScenePerPixelBlinn) //..................................
   {
      name("Blinn-Phong per pixel lighting");
      info("Per-pixel lighting with Blinn-Phong lightmodel. The reflection of 4 light sources is calculated per pixel.");
                                          
      // create material
      SLMaterial* m1 = new SLMaterial("m1", 0,0,0,0, _shaderProgs[PerPixBlinn]);
      
      // Add shared resources to the arrays in SLScene
      _materials.push_back(m1);
      m1->shininess(500);
      
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0,1,8, 0,1,0);
      cam1->focalDist(8);
      
      // define 4 light sources
      SLLightRect* light0 = new SLLightRect(2.0f,1.0f);
      light0->ambient(SLCol4f(0,0,0));
      light0->diffuse(SLCol4f(1,1,1));
      light0->lightAt(0,3,0, 0,0,0, 0,0,-1);
      light0->attenuation(0,0,1);

      SLLightSphere* light1 = new SLLightSphere(0.1f);
      light1->ambient(SLCol4f(0,0,0));
      light1->diffuse(SLCol4f(1,0,0));
      light1->specular(SLCol4f(1,0,0));
      light1->translate(0,0,2);
      light1->attenuation(0,0,1);

      SLLightSphere* light2 = new SLLightSphere(0.1f);
      light2->ambient(SLCol4f(0,0,0));
      light2->diffuse(SLCol4f(0,1,0));
      light2->specular(SLCol4f(0,1,0));
      light2->lightAt(1.5, 1.5, 1.5);
      light2->spotCutoff(20);
      light2->attenuation(0,0,1);
      
      SLLightSphere* light3 = new SLLightSphere(0.1f);
      light3->ambient(SLCol4f(0,0,0));
      light3->diffuse(SLCol4f(0,0,1));
      light3->specular(SLCol4f(0,0,1));
      light3->lightAt(-1.5, 1.5, 1.5);
      light3->spotCutoff(20);
      light3->attenuation(0,0,1);
      
      // Assemble scene graph
      SLGroup* scene = new SLGroup;
      scene->addNode(cam1);
      scene->addNode(light0);
      scene->addNode(light1);
      scene->addNode(light2);
      scene->addNode(light3);
      scene->addNode(new SLSphere(1.0f, 20, 20, "Sphere", m1));
      scene->addNode(new SLBox(1,-1,-1, 2,1,1, "Box", m1)); 

      _backColor.set(SLCol4f(0.1f,0.1f,0.1f));
      _activeSV->camera(cam1);
      _root3D = scene;   
   }
   else
   if (sceneName == cmdScenePerVertexWave) //..................................
   {
      name("Wave Shader");      
      info("Vertex Shader with wave displacment.");
      cout << "Use H-Key to increment (decrement w. shift) the wave height.\n\n";
  
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0,3,8, 0,0,0);
      cam1->focalDist(8);

      // Create generic shader program with 4 custom uniforms
      SLGLShaderProg* sp = new SLGLShaderProgGeneric("Wave.vert", "Wave.frag");
      SLGLShaderUniform1f* u_h = new SLGLShaderUniform1f(UF1Const, "u_h", 0.1f, 0.05f, 0.0f, 0.5f, (SLKey)'H');
      _eventHandlers.push_back(u_h);                              
      sp->addUniform1f(u_h);                                                           
      sp->addUniform1f(new SLGLShaderUniform1f(UF1Inc,    "u_t", 0.0f, 0.06f));
      sp->addUniform1f(new SLGLShaderUniform1f(UF1Const,  "u_a", 2.5f));
      sp->addUniform1f(new SLGLShaderUniform1f(UF1IncDec, "u_b", 2.2f, 0.01f, 2.0f, 2.5f));
   
      // Create materials
      SLMaterial* matWater = new SLMaterial("matWater", SLCol4f(0.45f,0.65f,0.70f), 
                                                        SLCol4f::WHITE, 300);
      matWater->shaderProg(sp);
      SLMaterial* matRed  = new SLMaterial("matRed", SLCol4f(1.00f,0.00f,0.00f));
   
      // Add shared resources to the arrays in SLScene
      _shaderProgs.push_back(sp);
      _materials.push_back(matRed);
      _materials.push_back(matWater);
   
      // water rectangle in the y=0 plane
      SLRectangle* wave = new SLRectangle(SLVec2f(-SL_PI,-SL_PI), SLVec2f( SL_PI, SL_PI), 
                                          40, 40, "WaterRect", matWater);
      wave->rotate(90, -1,0,0);
   
      SLLightSphere* light0 = new SLLightSphere();
      light0->ambient(SLCol4f(0,0,0));
      light0->diffuse(SLCol4f(1,1,1));
      light0->translate(0,4,-4);
      light0->attenuation(1,0,0);
   
      SLGroup* scene = new SLGroup;
      scene->addNode(light0);
      scene->addNode(wave);
      scene->addNode(new SLSphere(1, 32, 32, "Red Sphere", matRed));
      scene->addNode(cam1);

      _backColor.set(SLCol4f(0.1f,0.4f,0.8f));
      _activeSV->camera(cam1);
      _root3D = scene;
      _activeSV->waitEvents(false); 
   }
   else
   if (sceneName == cmdSceneWater) //..........................................
   {
      name("Water Shader");
      info("Water Shader with reflection & refraction mapping.");
      cout << "Use H-Key to increment (decrement w. shift) the wave height.\n\n";
   
      _backColor.set(.5f,.5f,1);
  
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0,3,8);
      cam1->focalDist(8);
   
      // create texture
      SLGLTexture* tex1 = new SLGLTexture("Pool+X0512_C.png","Pool-X0512_C.png"
                                         ,"Pool+Y0512_C.png","Pool-Y0512_C.png"
                                         ,"Pool+Z0512_C.png","Pool-Z0512_C.png");
      SLGLTexture* tex2 = new SLGLTexture("tile1_0256_C.jpg");

      // Create generic shader program with 4 custom uniforms
      SLGLShaderProg* sp = new SLGLShaderProgGeneric("WaveRefractReflect.vert",
                                                     "RefractReflect.frag");
      SLGLShaderUniform1f* u_h = new SLGLShaderUniform1f(UF1Const, "u_h", 0.1f, 0.05f, 0.0f, 0.5f, (SLKey)'H');
      _eventHandlers.push_back(u_h);                              
      sp->addUniform1f(u_h);                                                 
      sp->addUniform1f(new SLGLShaderUniform1f(UF1Inc,    "u_t", 0.0f, 0.06f));
      sp->addUniform1f(new SLGLShaderUniform1f(UF1Const,  "u_a", 2.5f));
      sp->addUniform1f(new SLGLShaderUniform1f(UF1IncDec, "u_b", 2.2f, 0.01f, 2.0f, 2.5f));
   
      // Create materials
      SLMaterial* matWater = new SLMaterial("matWater", SLCol4f(0.45f,0.65f,0.70f), 
                                                        SLCol4f::WHITE, 100, 0.1f, 0.9f, 1.5f);
      matWater->shaderProg(sp);
      matWater->textures().push_back(tex1);
      SLMaterial* matRed  = new SLMaterial("matRed", SLCol4f(1.00f,0.00f,0.00f));
      SLMaterial* matTile = new SLMaterial("matTile");
      matTile->textures().push_back(tex2);
   
      // Add shared resources to the arrays in SLScene
      _shaderProgs.push_back(sp);
      _textures.push_back(tex1);
      _textures.push_back(tex2);
      _materials.push_back(matRed);
      _materials.push_back(matWater);
      _materials.push_back(matTile);
   
      // water rectangle in the y=0 plane
      SLRectangle* rect = new SLRectangle(SLVec2f(-SL_PI,-SL_PI), SLVec2f( SL_PI, SL_PI), 
                                          40, 40, "WaterRect", matWater);
      rect->rotate(90, -1,0,0);
   
      // Pool rectangles
      SLRectangle* rectF = new SLRectangle(SLVec2f(-SL_PI,-SL_PI/6), SLVec2f( SL_PI, SL_PI/6), 
                                           SLVec2f(0,0), SLVec2f(10, 2.5f), 10, 10, "rectF", matTile);
      SLRectangle* rectN = new SLRectangle(SLVec2f(-SL_PI,-SL_PI/6), SLVec2f( SL_PI, SL_PI/6), 
                                           SLVec2f(0,0), SLVec2f(10, 2.5f), 10, 10, "rectN", matTile);
      SLRectangle* rectL = new SLRectangle(SLVec2f(-SL_PI,-SL_PI/6), SLVec2f( SL_PI, SL_PI/6), 
                                           SLVec2f(0,0), SLVec2f(10, 2.5f), 10, 10, "rectL", matTile);
      SLRectangle* rectR = new SLRectangle(SLVec2f(-SL_PI,-SL_PI/6), SLVec2f( SL_PI, SL_PI/6), 
                                           SLVec2f(0,0), SLVec2f(10, 2.5f), 10, 10, "rectR", matTile);
      SLRectangle* rectB = new SLRectangle(SLVec2f(-SL_PI,-SL_PI  ), SLVec2f( SL_PI, SL_PI  ), 
                                           SLVec2f(0,0), SLVec2f(10, 10  ), 10, 10, "rectB", matTile);
      rectF->translate(0,0,-SL_PI);
      rectL->rotate( 90, 0,1,0); rectL->translate(0,0,-SL_PI);
      rectN->rotate(180, 0,1,0); rectN->translate(0,0,-SL_PI);
      rectR->rotate(270, 0,1,0); rectR->translate(0,0,-SL_PI);
      rectB->rotate( 90,-1,0,0); rectB->translate(0,0,-SL_PI/6);
   
      SLLightSphere* light0 = new SLLightSphere();
      light0->ambient(SLCol4f(0,0,0));
      light0->diffuse(SLCol4f(1,1,1));
      light0->translate(0,4,-4);
      light0->attenuation(1,0,0);
   
      SLGroup* scene = new SLGroup;
      scene->addNode(light0);
      scene->addNode(rectF);
      scene->addNode(rectL);
      scene->addNode(rectN);
      scene->addNode(rectR);
      scene->addNode(rectB);
      scene->addNode(rect);
      scene->addNode(new SLSphere(1, 32, 32, "Red Sphere", matRed));
      scene->addNode(cam1);

      _backColor.set(SLCol4f(0.1f,0.4f,0.8f));
      _activeSV->camera(cam1);
      _root3D = scene;
      _activeSV->waitEvents(false); 
   }
   else
   if (sceneName == cmdSceneBumpNormal) //.....................................
   {
      name("Normal Map Bump Mapping");
      info("Normal map bump mapping combined with a per pixel spot lighting.");
               
      // Create textures
      SLGLTexture* texC = new SLGLTexture("brickwall0512_C.jpg"); 
      SLGLTexture* texN = new SLGLTexture("brickwall0512_N.jpg"); 
      
      // Create materials
      SLMaterial* m1 = new SLMaterial("m1", texC, texN, 0, 0, _shaderProgs[BumpNormal]);
      
      // Add shared resources to the arrays in SLScene
      _textures.push_back(texC);
      _textures.push_back(texN);
      _materials.push_back(m1);
      
      SLCamera* cam1 = new SLCamera(); 
      cam1->name("cam1");
      cam1->posAtUp(0,0,20);
      cam1->focalDist(20);
      
      SLLightSphere* light1 = new SLLightSphere(0.3f);
      light1->ambient(SLCol4f(0.1f, 0.1f, 0.1f));
      light1->diffuse(SLCol4f(1, 1, 1));
      light1->specular(SLCol4f(1, 1, 1));
      light1->attenuation(1,0,0);
      light1->lightAt(0,0,5);
      light1->spotCutoff(50);
      light1->animation(new SLAnimation(2, 2, XAxis, 2, YAxis, loop));
      
      SLGroup* scene = new SLGroup;
      scene->addNode(light1);
      scene->addNode(new SLRectangle(SLVec2f(-5,-5),SLVec2f(5,5),1,1,"Rect", m1));
      scene->addNode(cam1);
      
      _backColor.set(0.5f,0.5f,0.5f);
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneBumpParallax) //...................................
   {
      name("Parallax Bump Mapping");
      cout << "Demo application for parallax bump mapping.\n";
      cout << "Use S-Key to increment (decrement w. shift) parallax scale.\n";
      cout << "Use O-Key to increment (decrement w. shift) parallax offset.\n\n";
      info("Normal map parallax mapping.");
      
      // Create shader program with 4 uniforms
      SLGLShaderProg* sp = new SLGLShaderProgGeneric("BumpNormal.vert", "BumpNormalParallax.frag"); 
      SLGLShaderUniform1f* scale = new SLGLShaderUniform1f(UF1Const, "u_scale", 0.04f, 0.002f, 0, 1, (SLKey)'X');
      SLGLShaderUniform1f* offset = new SLGLShaderUniform1f(UF1Const, "u_offset", -0.03f, 0.002f,-1, 1, (SLKey)'O');
      _eventHandlers.push_back(scale);
      _eventHandlers.push_back(offset);
      sp->addUniform1f(scale);
      sp->addUniform1f(offset); 
                                       
      // Create textures
      SLGLTexture* texC = new SLGLTexture("brickwall0512_C.jpg"); 
      SLGLTexture* texN = new SLGLTexture("brickwall0512_N.jpg"); 
      SLGLTexture* texH = new SLGLTexture("brickwall0512_H.jpg");
      
      // Create materials
      SLMaterial* mat1 = new SLMaterial("mat1", texC, texN, texH, 0, sp);
      
      // Add shared resources to the arrays in SLScene
      _shaderProgs.push_back(sp);
      _textures.push_back(texC);
      _textures.push_back(texN);
      _textures.push_back(texH);
      _materials.push_back(mat1);
      
      SLCamera* cam1 = new SLCamera(); 
      cam1->name("cam1");
      cam1->posAtUp(0,0,20);
      cam1->focalDist(20);
      
      SLLightSphere* light1 = new SLLightSphere(0.3f);
      light1->ambient(SLCol4f(0.1f, 0.1f, 0.1f));
      light1->diffuse(SLCol4f(1, 1, 1));
      light1->specular(SLCol4f(1, 1, 1));
      light1->attenuation(1,0,0);
      light1->lightAt(0,0,5);
      light1->spotCutoff(50);
      light1->animation(new SLAnimation(2, 2, XAxis, 2, YAxis, loop));
      
      SLGroup* scene = new SLGroup;
      scene->addNode(light1);
      scene->addNode(new SLRectangle(SLVec2f(-5,-5),SLVec2f(5,5),1,1,"Rect", mat1));
      scene->addNode(cam1);
      
      _backColor.set(0.5f,0.5f,0.5f);
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneEarth) //..........................................
   {
      name("Earth Shader from Markus Knecht");
      cout << "Earth Shader from Markus Knecht\n";
      cout << "Use (SHIFT) & key Y to change scale of the parallax mapping\n";
      cout << "Use (SHIFT) & key X to change bias of the parallax mapping\n";
      cout << "Use (SHIFT) & key C to change cloud height\n";
      info("Complex earth shader with 7 textures: daycolor, nightcolor, normal, height & gloss map of earth, color & alphamap of clouds");
      
      // Create shader program with 4 uniforms
      SLGLShaderProg* sp = new SLGLShaderProgGeneric("BumpNormal.vert", "BumpNormalEarth.frag"); 
      SLGLShaderUniform1f* scale = new SLGLShaderUniform1f(UF1Const, "u_scale", 0.02f, 0.002f, 0, 1, (SLKey)'X');
      SLGLShaderUniform1f* offset = new SLGLShaderUniform1f(UF1Const, "u_offset", -0.02f, 0.002f,-1, 1, (SLKey)'O');
      _eventHandlers.push_back(scale);
      _eventHandlers.push_back(offset);
      sp->addUniform1f(scale);
      sp->addUniform1f(offset); 
                                       
      // Create textures
      #ifndef SL_GLES2
      SLGLTexture* texC   = new SLGLTexture("earth2048_C.jpg"); // color map
      SLGLTexture* texN   = new SLGLTexture("earth2048_N.jpg"); // normal map
      SLGLTexture* texH   = new SLGLTexture("earth2048_H.jpg"); // height map
      SLGLTexture* texG   = new SLGLTexture("earth2048_G.jpg"); // gloss map
      SLGLTexture* texNC  = new SLGLTexture("earthNight2048_C.jpg"); // night color  map
      #else
      SLGLTexture* texC   = new SLGLTexture("earth1024_C.jpg"); // color map
      SLGLTexture* texN   = new SLGLTexture("earth1024_N.jpg"); // normal map
      SLGLTexture* texH   = new SLGLTexture("earth1024_H.jpg"); // height map
      SLGLTexture* texG   = new SLGLTexture("earth1024_G.jpg"); // gloss map
      SLGLTexture* texNC  = new SLGLTexture("earthNight1024_C.jpg"); // night color  map
      #endif
      SLGLTexture* texClC = new SLGLTexture("earthCloud1024_C.jpg"); // cloud color map
      SLGLTexture* texClA = new SLGLTexture("earthCloud1024_A.jpg"); // cloud alpha map

      // Create materials
      SLMaterial* matEarth = new SLMaterial("matEarth", texC, texN, texH, texG, sp);
      matEarth->textures().push_back(texClC);
      matEarth->textures().push_back(texClA);
      matEarth->textures().push_back(texNC);
      matEarth->shininess(4000);
      matEarth->shaderProg(sp);
      
      // Add shared resources to the arrays in SLScene
      _shaderProgs.push_back(sp);
      _textures.push_back(texC);
      _textures.push_back(texN);
      _textures.push_back(texH);
      _textures.push_back(texG);
      _textures.push_back(texClC);
      _textures.push_back(texClA);
      _textures.push_back(texNC);
      _materials.push_back(matEarth);

      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0,0,4);
      cam1->focalDist(4);
      
      SLLightSphere* sun = new SLLightSphere();
      sun->ambient(SLCol4f(0,0,0));
      sun->diffuse(SLCol4f(1,1,1));
      sun->specular(SLCol4f(0.2f,0.2f,0.2f));
      sun->attenuation(1,0,0);
      sun->animation(new SLAnimation(24, 50, XAxis, 50, ZAxis, loop));
      
      SLSphere* earth = new SLSphere(1, 36, 36, "Earth", matEarth);
      earth->rotate(90,-1,0,0);
      
      SLGroup* scene = new SLGroup;
      scene->addNode(sun);
      scene->addNode(earth);
      scene->addNode(cam1);

      _backColor.set(SLCol4f(0,0,0));
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneMuttenzerBox) //...................................
   {
      name("Muttenzer Box");
      info("Muttenzer Box with environment mapped reflective sphere and transparenz refractive glass sphere. Try ray tracing for real reflections and soft shadows.",
           SLCol4f::GRAY);
      
      // Create reflection & glass shaders
      SLGLShaderProg* sp1 = new SLGLShaderProgGeneric("Reflect.vert", "Reflect.frag");
      SLGLShaderProg* sp2 = new SLGLShaderProgGeneric("RefractReflect.vert", "RefractReflect.frag");
   
      // Create cube mapping texture
      SLGLTexture* tex1 = new SLGLTexture("MuttenzerBox+X0512_C.png", "MuttenzerBox-X0512_C.png"
                                         ,"MuttenzerBox+Y0512_C.png", "MuttenzerBox-Y0512_C.png"
                                         ,"MuttenzerBox+Z0512_C.png", "MuttenzerBox-Z0512_C.png");
      
      SLCol4f  lightEmisRGB(7.0f,7.0f,7.0f);
      SLCol4f  grayRGB  (0.70f, 0.70f, 0.70f);
      SLCol4f  redRGB   (0.70f, 0.00f, 0.30f);
      SLCol4f  blueRGB  (0.10f, 0.10f, 0.80f);
      SLCol4f  blackRGB (0.00f, 0.00f, 0.00f);

      // create materials
      SLMaterial* cream = new SLMaterial("cream", grayRGB, SLCol4f::BLACK, 0);
      SLMaterial* red   = new SLMaterial("red",   redRGB , SLCol4f::BLACK, 0);
      SLMaterial* blue  = new SLMaterial("blue",  blueRGB, SLCol4f::BLACK, 0);

      // Material for mirror sphere
      SLMaterial* refl=new SLMaterial("refl", blackRGB, blackRGB, 128, 1.0f);
      refl->textures().push_back(tex1);
      refl->shaderProg(sp1);

      // Material for glass sphere
      SLCol4f refrDiff (0.0f, 0.0f, 0.0f, 0.01f);
      SLCol4f refrSpec (0.05f, 0.05f, 0.05f);
      SLMaterial* refr=new SLMaterial("refr", refrDiff, refrSpec, 128, 0.05f, 0.95f, 1.5f);
      refr->textures().push_back(tex1);
      refr->shaderProg(sp2);
   
      // Add shared resources to the arrays in SLScene
      _shaderProgs.push_back(sp1);
      _shaderProgs.push_back(sp2);
      _textures.push_back(tex1);
      _materials.push_back(cream);
      _materials.push_back(red);
      _materials.push_back(blue);
      _materials.push_back(refl);
      _materials.push_back(refr);
   
      SLSphere* sphere1=new SLSphere(0.5f, 32, 32, "Sphere1", refl);
      sphere1->translate(-0.65f, -0.75f, -0.55f);

      SLSphere* sphere2=new SLSphere(0.45f, 32, 32, "Sphere2", refr);
      sphere2->translate( 0.73f, -0.8f, 0.10f);

      SLGroup* balls = new SLGroup;
      balls->addNode(sphere1);
      balls->addNode(sphere2);

      // Rectangular light 
      SLLightRect* lightRect = new SLLightRect(1, 0.65f);
      lightRect->rotate(90, -1.0f, 0.0f, 0.0f);
      lightRect->translate(0.0f, -0.25f, 1.18f);
      lightRect->spotCutoff(90);
      lightRect->spotExponent(1.0);
      lightRect->diffuse(lightEmisRGB);
      lightRect->attenuation(0,0,1);
      lightRect->samplesXY(11, 7);
   
      _globalAmbiLight.set(lightEmisRGB*0.05f);

      // create label text
      SLText* label = new SLText("Muttenzer Box", SLTexFont::font12);
      label->scale(0.01f, 0.01f, 1.0f);
      label->translate(-55.0f, 0.0f, 0.0f);

      // create camera
      SLCamera* cam1 = new SLCamera();
      cam1->posAtUp(0.0f, 0.40f, 6.35f, 0.0f,-0.05f, 0.0f, 0,1,0);
      cam1->fov(27);
      cam1->focalDist(6.35f);
      
      // assemble scene
      SLGroup* scene = new SLGroup;
      scene->addNode(cam1);
      scene->addNode(lightRect);
      //scene->addNode(label);
      
      // create wall polygons    
      SLfloat pL = -1.48f, pR = 1.48f; // left/right
      SLfloat pB = -1.25f, pT = 1.19f; // bottom/top
      SLfloat pN =  1.79f, pF =-1.55f; // near/far
      
      // bottom plane
      SLRectangle* b = new SLRectangle(SLVec2f(pL,-pN), SLVec2f(pR,-pF), 6, 6, "bottom", cream); 
      b->rotate(90, -1,0,0); b->translate(0,0,pB); scene->addNode(b);
   
      // top plane
      SLRectangle* t = new SLRectangle(SLVec2f(pL,pF), SLVec2f(pR,pN), 6, 6, "top", cream); 
      t->rotate(90, 1,0,0); t->translate(0,0,-pT); scene->addNode(t);
   
      // far plane
      SLRectangle* f = new SLRectangle(SLVec2f(pL,pB), SLVec2f(pR,pT), 6, 6, "far", cream); 
      f->translate(0,0,pF); scene->addNode(f);
   
      // left plane
      SLRectangle* l = new SLRectangle(SLVec2f(-pN,pB), SLVec2f(-pF,pT), 6, 6, "left", red); 
      l->rotate(90, 0,1,0); l->translate(0,0,pL); scene->addNode(l);
   
      // right plane
      SLRectangle* r = new SLRectangle(SLVec2f(pF,pB), SLVec2f(pN,pT), 6, 6, "right", blue); 
      r->rotate(90, 0,-1,0); r->translate(0,0,-pR); scene->addNode(r);
      
      scene->addNode(balls);

      _backColor.set(SLCol4f(0.0f,0.0f,0.0f));
      _activeSV->camera(cam1);
      _root3D = scene;
   }
   else
   if (sceneName == cmdSceneRTSpheres) //......................................
   {
      name("Ray tracing Spheres");
      info("Classic ray tracing scene with transparent and reflective spheres. Be patient on mobile devices.");

      // define materials
      SLMaterial* matGla = new SLMaterial("Glass", SLCol4f(0.0f, 0.0f, 0.0f), 
                                                   SLCol4f(0.5f, 0.5f, 0.5f), 
                                                   100, 0.4f, 0.6f, 1.5f);
      SLMaterial* matRed = new SLMaterial("Red",   SLCol4f(0.5f, 0.0f, 0.0f), 
                                                   SLCol4f(0.5f, 0.5f, 0.5f), 
                                                   100, 0.5f, 0.0f, 1.0f);
      SLMaterial* matYel = new SLMaterial("Floor", SLCol4f(0.8f, 0.6f, 0.2f), 
                                                   SLCol4f(0.8f, 0.8f, 0.8f), 
                                                   100, 0.0f, 0.0f, 1.0f);
      _materials.push_back(matGla);
      _materials.push_back(matRed);
      _materials.push_back(matYel);

      SLCamera* cam1 = new SLCamera();
      cam1->posAtUp(0, 0, 4);
      cam1->focalDist(4);
      
      SLRectangle *rect = new SLRectangle(SLVec2f(-3,-3), SLVec2f(5,4), 20, 20, "Floor", matYel);
      rect->rotate(90, -1,0,0);
      rect->translate(0, -1, -0.5f);

      SLLightSphere* light1 = new SLLightSphere(2, 2, 2, 0.1f);
      light1->ambient(SLCol4f(1, 1, 1));
      light1->diffuse(SLCol4f(7, 7, 7));
      light1->specular(SLCol4f(7, 7, 7));
      light1->attenuation(0,0,1);
      
      SLLightSphere* light2 = new SLLightSphere(2, 2, -2, 0.1f);
      light2->ambient(SLCol4f(1, 1, 1));
      light2->diffuse(SLCol4f(7, 7, 7));
      light2->specular(SLCol4f(7, 7, 7));
      light2->attenuation(0,0,1);

      SLGroup* scene  = new SLGroup;
      scene->addNode(light1);
      scene->addNode(light2);
      scene->addNode(SphereGroup(1, 0,0,0, 1, 32, matGla, matRed));
      scene->addNode(rect);
      scene->addNode(cam1);  

      _backColor.set(SLCol4f(0.1f,0.4f,0.8f));
      _root3D = scene;   
      _activeSV->camera(cam1);
   }
   else
   if (sceneName == cmdSceneRTSoftShadows) //..................................
   {
      name("Ray tracing Softshadows");
      info("Ray tracing with soft shadow light sampling. Each light source is sampled 64x per pixel. Be patient on mobile devices.");
      
      // define materials
      SLCol4f spec(0.8f, 0.8f, 0.8f);
      SLMaterial* matBlk = new SLMaterial("Glass", SLCol4f(0.0f, 0.0f, 0.0f), SLCol4f(0.5f, 0.5f, 0.5f), 100, 0.5f, 0.5f, 1.5f);
      SLMaterial* matRed = new SLMaterial("Red",   SLCol4f(0.5f, 0.0f, 0.0f), SLCol4f(0.5f, 0.5f, 0.5f), 100, 0.5f, 0.0f, 1.0f);
      SLMaterial* matYel = new SLMaterial("Floor", SLCol4f(0.8f, 0.6f, 0.2f), SLCol4f(0.8f, 0.8f, 0.8f), 100, 0.5f, 0.0f, 1.0f);
      _materials.push_back(matBlk);
      _materials.push_back(matRed);
      _materials.push_back(matYel);
      
      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0, 0.1f, 6);
      cam1->focalDist(6);
      
      SLRectangle *rect = new SLRectangle(SLVec2f(-3,-3), SLVec2f(5,4), 32, 32, "Rect", matYel);
      rect->rotate(90, -1,0,0);
      rect->translate(0, -1, -0.5f);

      SLLightSphere* light1 = new SLLightSphere(3, 3, 3, 0.3f);
      #ifndef SL_GLES2
      SLint numSamples = 6;
      #else
      SLint numSamples = 8;
      #endif
      light1->samples(numSamples, numSamples);
      light1->attenuation(0,0,1);
      //light1->lightAt(2,2,2, 0,0,0);
      //light1->spotCutoff(15);

      SLLightSphere* light2 = new SLLightSphere(0, 1.5, -1.5, 0.3f);
      light2->samples(8,8);
      light2->attenuation(0,0,1);

      SLGroup* scene  = new SLGroup;
      scene->addNode(light1);
      scene->addNode(light2);
      scene->addNode(SphereGroup(1, 0,0,0, 1, 32, matBlk, matRed));
      scene->addNode(rect);
      scene->addNode(cam1);
      
      _backColor.set(SLCol4f(0.1f,0.4f,0.8f));
      _root3D = scene;   
      _activeSV->camera(cam1);   
   }
   else
   if (sceneName == cmdSceneRTDoF) //..........................................
   {
      name("Ray tracing: Depth of Field"); 
      info("Ray tracing with depth of field blur. Each pixel is sampled 100x from a lens. Be patient on mobile devices.");
      
      // Create textures and materials
      SLGLTexture* texC = new SLGLTexture("Checkerboard0512_C.png");
      SLMaterial* mT = new SLMaterial("mT", texC, 0, 0, 0); mT->kr(0.5f);
      SLMaterial* mW = new SLMaterial("mW", SLCol4f::WHITE);
      SLMaterial* mB = new SLMaterial("mB", SLCol4f::GRAY);
      SLMaterial* mY = new SLMaterial("mY", SLCol4f::YELLOW);
      SLMaterial* mR = new SLMaterial("mR", SLCol4f::RED);
      SLMaterial* mG = new SLMaterial("mG", SLCol4f::GREEN);
      SLMaterial* mM = new SLMaterial("mM", SLCol4f::MAGENTA);
      
      // Add shared resources to the arrays in SLScene
      _textures.push_back(texC);
      _materials.push_back(mT);
      _materials.push_back(mW);
      _materials.push_back(mB);
      _materials.push_back(mY);
      _materials.push_back(mR);
      _materials.push_back(mG);
      _materials.push_back(mM);

      #ifndef SL_GLES2
      SLint numSamples = 6;
      #else
      SLint numSamples = 10;
      #endif

      SLCamera* cam1 = new SLCamera;
      cam1->posAtUp(0, 2, 7);
      cam1->focalDist(7);
      cam1->lensDiameter(0.4f);
      cam1->lensSamples()->samples(numSamples, numSamples);

      SLRectangle* rect = new SLRectangle(SLVec2f(-5,-5), SLVec2f(5,5), 20, 20, "Rect", mT);
      rect->rotate(90, -1,0,0);
      rect->translate(0,0,-0.5f);

      SLLightSphere* light1 = new SLLightSphere(2,2,0, 0.1f);
      light1->attenuation(0,0,1);

      SLGroup* balls = new SLGroup;
      SLSphere* s;
      s = new SLSphere(0.5f,32,32,"S1",mW); s->translate( 2.0,0,-4);  balls->addNode(s);
      s = new SLSphere(0.5f,32,32,"S2",mB); s->translate( 1.5,0,-3);  balls->addNode(s);
      s = new SLSphere(0.5f,32,32,"S3",mY); s->translate( 1.0,0,-2);  balls->addNode(s);
      s = new SLSphere(0.5f,32,32,"S4",mR); s->translate( 0.5,0,-1);  balls->addNode(s);
      s = new SLSphere(0.5f,32,32,"S5",mG); s->translate( 0.0,0, 0);  balls->addNode(s);
      s = new SLSphere(0.5f,32,32,"S6",mM); s->translate(-0.5,0, 1);  balls->addNode(s);
      s = new SLSphere(0.5f,32,32,"S7",mW); s->translate(-1.0,0, 2);  balls->addNode(s);

      SLGroup* scene  = new SLGroup;
      scene->addNode(light1);
      scene->addNode(balls);
      scene->addNode(rect);
      scene->addNode(cam1);

      _backColor.set(SLCol4f(0.1f,0.4f,0.8f));
      _root3D = scene;   
      _activeSV->camera(cam1);
   }
   
   _activeSV->onInitialize();
}
//-----------------------------------------------------------------------------
