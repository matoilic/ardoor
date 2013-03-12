//#############################################################################
//  File:      SLScene.cpp
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

#include "SLScene.h"
#include "SLSceneView.h"
#include "SLCamera.h"
#include "SLLight.h"
#include "SLGroup.h"
#include "SLTexFont.h"
#include "SLButton.h"

//-----------------------------------------------------------------------------
//! Global static scene pointer
SLScene* SLScene::current = 0;
//-----------------------------------------------------------------------------
/*! The constructor of the scene does all one time initialization such as 
loading the standard shader programs from which the pointers are stored in
the dynamic array _shaderProgs. Custom shader programs that are loaded in a
scene must be deleted when the scene changes.
*/
SLScene::SLScene(SLstring name) : SLObject(name)
{  _root3D     = 0;
   _menu2D     = 0;
   _menuGL     = 0;
   _menuRT     = 0;
   _info       = 0;
   _infoGL     = 0;
   _infoRT     = 0;
   _btnHelp    = 0;
   _btnAbout   = 0;
   _btnCredits = 0;
     
   // Load std. shader programs in order as defined in SLStdShaderProgs enum
   _shaderProgs.push_back(new SLGLShaderProgGeneric("ColorAttribute.vert",
                                                    "Color.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("ColorUniform.vert",
                                                    "Color.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("PerVrtBlinn.vert",
                                                    "PerVrtBlinn.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("PerVrtBlinnTex.vert",
                                                    "PerVrtBlinnTex.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("TextureOnly.vert",
                                                    "TextureOnly.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("PerPixBlinn.vert",
                                                    "PerPixBlinn.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("PerPixBlinnTex.vert",
                                                    "PerPixBlinnTex.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("BumpNormal.vert",
                                                    "BumpNormal.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("BumpNormal.vert",
                                                    "BumpNormalParallax.frag"));
   _shaderProgs.push_back(new SLGLShaderProgGeneric("FontTex.vert",
                                                    "FontTex.frag"));
   _numProgsPreload = _shaderProgs.size();
   _shadowShader = 0;
   
   // Generate std. fonts   
   SLTexFont::generateFonts();
}
//-----------------------------------------------------------------------------
/*! The destructor does the final total deallocation of all global resources.
*/
SLScene::~SLScene()
{  
   SL_LOG("~SLScene\n");
   unInit();
   
   // delete global SLGLState instance
   SLGLState::deleteInstance();

   // clear light pointers
   _lights.clear();
   
   // delete materials 
   for (SLuint i=0; i<_materials.size(); ++i) delete _materials[i];
   _materials.clear();
   
   // delete textures
   for (SLuint i=0; i<_textures.size(); ++i) delete _textures[i];
   _textures.clear();
   
   // delete shader programs
   for (SLuint i=0; i<_shaderProgs.size(); ++i) delete _shaderProgs[i];
   _shaderProgs.clear();
   
   // delete fonts   
   SLTexFont::deleteFonts();
   
   // delete menus & statistic texts
   delete _menuGL;      _menuGL     = 0;
   delete _menuRT;      _menuRT     = 0;
   delete _info;        _info       = 0;
   delete _infoGL;      _infoGL     = 0;
   delete _infoRT;      _infoRT     = 0;
   delete _btnAbout;    _btnAbout   = 0;
   delete _btnHelp;     _btnHelp    = 0;
   delete _btnCredits;  _btnCredits = 0;
   
   current = 0;   
}
//-----------------------------------------------------------------------------
/*! The scene init is called whenever the scene is new loaded.
*/
void SLScene::init()
{  
   current = this;
   
   unInit();
   
   _backColor.set(0.1f,0.4f,0.8f,1.0f);
   _globalAmbiLight.set(0.2f,0.2f,0.2f,0.0f);
   _selectedShape = 0;

   _timer.start();
}
//-----------------------------------------------------------------------------
/*! The scene uninitializing clears the scenegraph (_root3D) and all global
global resources such as materials, textures & custom shaders loaded with the 
scene. The standard shaders, the fonts and the 2D-GUI elements remain.
*/
void SLScene::unInit()
{  
   if (_activeSV)
      _activeSV->camera(0);  

   // delete entire scene graph
   delete _root3D;
   _root3D = 0;

   // clear light pointers
   _lights.clear();
   
   // delete materials 
   for (SLuint i=0; i<_materials.size(); ++i) delete _materials[i];
   _materials.clear();
   
   SLMaterial::current = 0;
   
   // delete textures
   for (SLuint i=0; i<_textures.size(); ++i) delete _textures[i];
   _textures.clear();
   
   // delete custom shader programs but not default shaders
   while (_shaderProgs.size() > _numProgsPreload) 
   {  SLGLShaderProg* sp = _shaderProgs.back();
      delete sp;
      _shaderProgs.pop_back();
   }
   
   // clear eventHandlers
   _eventHandlers.clear();
}
//-----------------------------------------------------------------------------
/*!
SLScene::info deletes previous info text and sets new one with a max. width 
*/
void SLScene::info(SLstring infoText, SLCol4f color)
{  
   delete _info;
   SLfloat minX = 80;
   
   // Set font size depending on DPI
   SLTexFont* f;
   if (_activeSV->scrDPI() > 300) f = SLTexFont::font18; else 
   if (_activeSV->scrDPI() > 250) f = SLTexFont::font16; else 
   if (_activeSV->scrDPI() > 200) f = SLTexFont::font14; else 
   if (_activeSV->scrDPI() > 150) f = SLTexFont::font12; else 
   if (_activeSV->scrDPI() > 100) f = SLTexFont::font10; else 
   f = SLTexFont::font10;

   _info = new SLText(infoText, f, color, 
                      _activeSV->scrW()-minX-5.0f,
                      1.2f);

   _info->translate(minX, SLButton::minMenuPos.y, 0);
}
//-----------------------------------------------------------------------------
/*! 
SLScene::info returns the info text. If null it creates an empty one
*/
SLText* SLScene::info()
{
   if (_info == 0) info("", SLCol4f::WHITE);
   return _info;
}
//-----------------------------------------------------------------------------
