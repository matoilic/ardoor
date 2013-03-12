//#############################################################################
//  File:      SLInterface.cpp
//  Purpose:   Implementation of the main Scene Library C-Interface. Only these 
//             functions should called by the OS-dependend GUI applications. 
//             These functions can be called from any C, C++ or ObjectiveC GUI 
//             framework or by a native API such as Java Native Interface (JNI)
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#include <SLInterface.h>
#include "SLScene.h"
#include "SLSceneView.h"
#include "SLCamera.h"
#include "SL3DSMesh.h"
#include "SL3DSMeshFile.h"

//-----------------------------------------------------------------------------
SLScene*     scene = 0; //!< Global pointer to the scene instance
SLSceneView* sv = 0;    //!< Global pointer to the Scene view instance
SLint        scrWidth;  //!< Global integer for window width in pixels
SLint        scrHeight; //!< Global integer for window height in pixels
//-----------------------------------------------------------------------------
/*! Global Initialization function that creates the instances for the scene and 
the sceneview. After the creation it calls the onLoad method for loading 
the predefined scenes.
*/
bool slInit(int screenWidth,
            int screenHeight,
            int dotsPerInch,
            SLCmd initScene, 
            string shaderPath,
            string modelPath,
            string texturePath,
            void* raytracingCallback)
{
   SLGLShaderProg::defaultPath  = shaderPath;
   SL3DSMeshFile::defaultPath   = modelPath;
   SLGLTexture::defaultPath     = texturePath;
   
   //SL_LOG("Models: %s", modelPath.c_str());
   //SL_LOG("Shaders: %s", shaderPath.c_str());
   //SL_LOG("Textures: %s", texturePath.c_str());

   scrWidth  = screenWidth;
   scrHeight = screenHeight;
   scene     = new SLScene("");
   sv        = new SLSceneView("", screenWidth, screenHeight, dotsPerInch);
   scene->activeSV(sv);
   scene->onLoad(initScene);

   #ifdef SL_RT1
   /* The ray tracing callback function is used to refresh the ray tracing image
   during the rendering process. The ray tracing image is drawn by OpenGL as a
   texture on a single quad.*/
   sv->raytracer()->guiRTWndUpdate = (cbRTWndUpdate)raytracingCallback;
   #endif

   return true;
}
//-----------------------------------------------------------------------------
/*! Global closing function that deallocates the sceneview and scene instances.
All the scenegraph deallocation is started from here and has to be done before
the GUI app terminates.
*/
void slClose()
{
   delete sv;
   scene->activeSV(0);
   sv = 0;
   delete scene;
   scene = 0;
}
//-----------------------------------------------------------------------------
/*! Global rendering function that simply calls the sceneview's onPaint method.
This function must be called for each frame. After the frame is generated the
OS must swap the OpenGL's backbuffer to the visible front buffer.
*/
bool slPaint()
{  if (!sv) return false;
   return sv->onPaint();
}
//-----------------------------------------------------------------------------
/*! Global resize function that must be called whenever the OpenGL frame
changes it's size.
*/
void slResize(int width, int height)
{  
   scrWidth = width;
   scrHeight = height;
   sv->onResize(width, height);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse button down events. 
*/
bool slMouseDown(SLMouseButton button, int xpos, int ypos, SLKey modifier) 
{  
   return sv->onMouseDown(button, xpos, ypos, modifier);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse move events.
*/
bool slMouseMove(int x, int y)
{  
   return sv->onMouseMove(x, y);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse button up events.
*/
bool slMouseUp(SLMouseButton button, int xpos, int ypos, SLKey modifier) 
{  
   return sv->onMouseUp(button, xpos, ypos, modifier);
}
//-----------------------------------------------------------------------------
/*! Global event handler for double click events.
*/
bool slDoubleClick(SLMouseButton button, int xpos, int ypos, SLKey modifier) 
{  
   return sv->onDoubleClick(ButtonLeft, xpos, ypos, modifier);
}
//-----------------------------------------------------------------------------
/*! Global event handler for the two finger touch down events of touchscreen 
devices.
*/
bool slTouch2Down(int xpos1, int ypos1, int xpos2, int ypos2) 
{  
   return sv->onTouch2Down(xpos1, ypos1, xpos2, ypos2);
}
//-----------------------------------------------------------------------------
/*! Global event handler for the two finger move events of touchscreen devices. 
*/
bool slTouch2Move(int xpos1, int ypos1, int xpos2, int ypos2) 
{  
   return sv->onTouch2Move(xpos1, ypos1, xpos2, ypos2);
}
//-----------------------------------------------------------------------------
/*! Global event handler for the two finger touch up events of touchscreen 
devices. 
*/
bool slTouch2Up(int xpos1, int ypos1, int xpos2, int ypos2) 
{  
   return sv->onTouch2Up(xpos1, ypos1, xpos2, ypos2);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse wheel events. 
*/
bool slMouseWheel(int pos, SLKey modifier)
{  
   return sv->onMouseWheelPos(pos, modifier);
}
//-----------------------------------------------------------------------------
/*! Global event handler for keyboard key press events. 
*/
bool slKeyPress(SLKey key, SLKey modifier) 
{  
   return sv->onKeyPress(key, modifier);
}
//-----------------------------------------------------------------------------
/*! Global event handler for keyboard key release events. 
*/
bool slKeyRelease(SLKey key, SLKey modifier) 
{  
   return sv->onKeyRelease(key, modifier);
}
//-----------------------------------------------------------------------------
/*! Global event handler for keyboard key release events. 
*/
bool slCommand(SLCmd command) 
{  
   return sv->onCommand(command);
}
//-----------------------------------------------------------------------------
/*! Global function to retrieve a window title text generated by the scene
library. 
*/
string slGetWindowTitle() 
{  if (!sv) return "";
   return sv->windowTitle();
}
//-----------------------------------------------------------------------------
