//#############################################################################
//  File:      SLMaterial.cpp
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

#include "SLMaterial.h"
#include "SLSceneView.h"
#include "SLShape.h"

//-----------------------------------------------------------------------------
SLMaterial SLMaterial::AIR = SLMaterial((SLchar*)"Air", 
                                        SLCol4f(0,0,0), SLCol4f(0,0,0));
//-----------------------------------------------------------------------------
SLfloat SLMaterial::PERFECT = 1000.0f;
//-----------------------------------------------------------------------------
SLMaterial* SLMaterial::current = 0;
//-----------------------------------------------------------------------------
// Default ctor
SLMaterial::SLMaterial(const SLchar* name,
                       SLCol4f amdi, 
                       SLCol4f spec,
                       SLfloat shininess, 
                       SLfloat kr, 
                       SLfloat kt, 
                       SLfloat kn) : SLObject(name)
{  _ambient = _diffuse = amdi;
   _specular = spec;
   _emission.set(0,0,0,0);
   _shininess = shininess;
   _shaderProg = 0;
   
   _kr = kr;
   _kt = kt;
   _kn = kn;
   
   // sync the transparency coeffitient with the alpha value or vice versa
   if (_kt!=0) _diffuse.w = 1.0f - _kt;
   if (_diffuse.w!=1) _kt = 1.0f - _diffuse.w;
}
//-----------------------------------------------------------------------------
// Ctor for textures
SLMaterial::SLMaterial(const SLchar*   name,
                       SLGLTexture*    texture1,
                       SLGLTexture*    texture2,
                       SLGLTexture*    texture3,
                       SLGLTexture*    texture4,
                       SLGLShaderProg* shaderProg) : SLObject(name)
{  _ambient.set(1,1,1);
   _diffuse.set(1,1,1);
   _specular.set(1,1,1);
   _emission.set(0,0,0,0);
   _shininess = 125;
   
   if (texture1) _textures.push_back(texture1);
   if (texture2) _textures.push_back(texture2);
   if (texture3) _textures.push_back(texture3);
   if (texture4) _textures.push_back(texture4);
   
   _shaderProg = shaderProg;
   
   _kr = 0.0f;
   _kt = 0.0f;
   _kn = 1.0f;
   _diffuse.w = 1.0f - _kt;
}
//-----------------------------------------------------------------------------
/*! 
The destructor doesn't delete attached the textures or shader program because
Such shared resources get deleted in the arrays of SLScene.
*/
SLMaterial::~SLMaterial()                        
{
}
//-----------------------------------------------------------------------------
/*!
SLMaterial::activate applies the material parameter to the global render state
and activates the attached shader
*/
void SLMaterial::activate(SLGLState* state, SLShape* shape)
{      
   SLScene* s = SLScene::current;
   SLSceneView* sv = s->activeSV();

   // Deactivate shader program of the current active material
   if (current && current->shaderProg()) 
      current->shaderProg()->endShader();

   // Set this material as the current material
   current = this;

   // If no shader program is attached add the default shader program
   if (!_shaderProg)
   {  if (_textures.size()>0)
           shaderProg(s->shaderProgs(PerVrtBlinnTex));
      else shaderProg(s->shaderProgs(PerVrtBlinn));
   }

   // Check if shader had compile error and the error texture should be shown
   if (_shaderProg && _shaderProg->name().find("ErrorTex")!=string::npos)
   {  _textures.clear();
      _textures.push_back(new SLGLTexture("CompileError.png"));
   }
   
   // Set material in the state
   state->matAmbient    = _ambient;
   state->matDiffuse    = _diffuse;
   state->matSpecular   = _specular;
   state->matEmissive   = _emission;
   state->matShininess  = _shininess;
   
   // Determine use of shaders & textures
   SLbool useTexture = !(sv->drawBits()->get(SL_DB_TEXOFF) || 
                         shape->drawBits()->get(SL_DB_TEXOFF));
                            
   // Enable or disable texturing
   if (useTexture && _textures.size()>0)
   {  for (SLuint i=0; i<_textures.size(); ++i)
         _textures[i]->bindActive(i);
   }
    
   // Activate the shader program now
   shaderProg()->beginUse(this);
}
//-----------------------------------------------------------------------------
