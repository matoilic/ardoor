//#############################################################################
//  File:      SLScene.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLSCENE_H
#define SLSCENE_H

#include <stdafx.h>
#include <SLMaterial.h>
#include <SLEventHandler.h>
#include <SLLight.h>
#include <SLText.h>

class SLGroup;
class SLShape;
class SLNode;
class SLSceneView;
class SLButton;

//-----------------------------------------------------------------------------
//! Scene class representing the top level instance holding the scene structure
/*!      
The SLScene class holds everything that is common for all scene views such as 
the root pointer (_root3D) to the scene, the background color, an array of
lights as well as the global resources (_materials, _textures, _shaderProgs).
A scene could have multiple scene views. The active one is _activeSV. The
onLoad method builds a scene from source code.
*/
class SLScene: public SLObject    
{  
   friend class SLGroup;
   friend class SLSceneView;
   
   public:           
                              SLScene           (SLstring name="");
                             ~SLScene           ();

            // Setters
            void              root3D            (SLGroup* root3D){_root3D = root3D;}
            void              activeSV          (SLSceneView* sv) {_activeSV = sv;}
            void              menu2D            (SLButton* menu2D){_menu2D = menu2D;}
            void              backColor         (SLCol4f backColor){_backColor=backColor;}
            void              globalAmbiLight   (SLCol4f gloAmbi){_globalAmbiLight=gloAmbi;}
            void              info              (SLstring infoText, 
                                                 SLCol4f color=SLCol4f::WHITE);
                           
            // Getters     
     inline SLGroup*          root3D            () {return _root3D;}
     inline SLSceneView*      activeSV          () {return _activeSV;}
            SLfloat           timeSec           () {return _timer.getElapsedTimeInSec();}
            SLButton*         menu2D            () {return _menu2D;}
            SLButton*         menuGL            () {return _menuGL;}
            SLCol4f           globalAmbiLight   () {return _globalAmbiLight;}
            SLCol4f           backColor         () {return _backColor;}
            SLCol4f*          backColorV        () {return &_backColor;}
            SLVLight&         lights            () {return _lights;}
            SLVEventHandler&  eventHandlers     () {return _eventHandlers;}
            SLVMaterial&      materials         () {return _materials;}
            SLVGLTexture&     textures          () {return _textures;}
            SLVShaderProgGL&  shaderProgs       () {return _shaderProgs;}
            SLGLShaderProg*   shaderProgs       (SLStdShaderProg i) {return _shaderProgs[i];}
            SLText*           info              ();
            
            // Misc.
            void              onLoad            (SLCmd sceneName);
            void              init              ();
            void              unInit            ();
            
     // global static scene pointer
     static SLScene*          current;

   protected:
            SLGroup*          _root3D;          //!< Root group node for 3D scene
            SLSceneView*      _activeSV;        //!< Pointer to the active scene view
            SLTimer           _timer;           //!< high precision timer
            SLCol4f           _backColor;       //!< Background color 
            SLCol4f           _globalAmbiLight; //!< global ambient light intensity
            SLint             _current;         //!< Identifier of current scene
            SLbool            _rootInitialized; //!< Flag if scene is intialized
            SLShape*          _selectedShape;   //!< Pointer to the selected shape
            
            SLText*           _info;            //!< Text node for scene info
            SLGroup*          _infoGL;          //!< Root group node for 2D GL stats infos
            SLGroup*          _infoRT;          //!< Root group node for 2D GL stats infos
            SLButton*         _menu2D;          //!< Root group node for 2D GUI
            SLButton*         _menuGL;          //!< Root group node for OpenGL menu   
            SLButton*         _menuRT;          //!< Root group node for RT menu  
            SLButton*         _btnAbout;        //!< About button
            SLButton*         _btnHelp;         //!< Help button
            SLButton*         _btnCredits;      //!< Credits button                  
            
            SLVLight          _lights;          //!< vector of all lights
            SLVEventHandler   _eventHandlers;   //!< vector of all event handlers
            SLVMaterial       _materials;       //!< Vector of all materials pointers
            SLVGLTexture      _textures;        //!< Vector of all texture pointers
            SLVShaderProgGL   _shaderProgs;     //!< Vector of all shaderProg pointers
            SLint             _numProgsPreload; //!< No. of preloaded shaderProgs

            SLGLShaderProg*   _shadowShader;    //!< Pointer to the Shadowmapping Shader
};
//-----------------------------------------------------------------------------
#endif
