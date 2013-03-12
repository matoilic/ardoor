//#############################################################################
//  File:      glfwMain.cpp
//  Purpose:   Implementation of the GUI with the GLFW (http://www.glfw.org/)
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include "SLInterface.h"

//-----------------------------------------------------------------------------
// GLobal application variables
SLint    mouseX;             //!< Last mouse position x in pixels
SLint    mouseY;             //!< Last mouse position y in pixels
SLint    touchX2;            //!< Last finger touch 2 position x in pixels
SLint    touchY2;            //!< Last finger touch 2 position y in pixels
SLint    touchDeltaX;        //!< Delta between two fingers in x
SLint    touchDeltaY;        //!< Delta between two fingers in <
SLint    lastWidth;          //!< Last window width in pixels
SLint    lastHeight;         //!< Last window height in pixels
SLint    lastMouseWheelPos;  //!< Last mouse wheel position
SLKey    modifiers=KeyNone;  //!< Current modifier key state
SLfloat  lastMouseDownTime = 0.0f; //!< Last mouse press time

//-----------------------------------------------------------------------------
/*! 
onClose event handler for deallocation of the scene & sceneview. onClose is
called glfwPollEvents, glfwWaitEvents or glfwSwapBuffers.
*/
int onClose()
{
   // Remove callback functions
   glfwSetWindowSizeCallback (NULL);
   glfwSetWindowCloseCallback(NULL);
   glfwSetMouseButtonCallback(NULL);
   glfwSetMousePosCallback   (NULL);
   glfwSetMouseWheelCallback (NULL);
   glfwSetKeyCallback        (NULL);
   slClose();
   return 1;
}

//-----------------------------------------------------------------------------
/*!
onPaint: Paint event handler that passes the event to the slPaint function. 
For accurate frame rate meassurement we have to take the time after the OpenGL 
frame buffer swapping. The FPS calculation is done in slGetWindowTitle.
*/
SLbool onPaint()
{  
   bool updated = slPaint();
   
   // Fast copy the back buffer to the front buffer. This is OS dependent.
   glfwSwapBuffers();
   
   // Show the title generetad by the scene library (FPS etc.)
   glfwSetWindowTitle(slGetWindowTitle().c_str());
   return updated;
}

//-----------------------------------------------------------------------------
//! Maps the GLFW key codes to the SLKey codes
const SLKey mapKeyToSLKey(SLint key)
{  
   switch (key)
   {  case GLFW_KEY_SPACE:        return KeySpace;
      case GLFW_KEY_ESC:          return KeyEsc;
      case GLFW_KEY_F1:           return KeyF1;
      case GLFW_KEY_F2:           return KeyF2;
      case GLFW_KEY_F3:           return KeyF3;
      case GLFW_KEY_F4:           return KeyF4;
      case GLFW_KEY_F5:           return KeyF5;
      case GLFW_KEY_F6:           return KeyF6;
      case GLFW_KEY_F7:           return KeyF7;
      case GLFW_KEY_F8:           return KeyF8;
      case GLFW_KEY_F9:           return KeyF9;
      case GLFW_KEY_F10:          return KeyF10;
      case GLFW_KEY_F11:          return KeyF11;
      case GLFW_KEY_F12:          return KeyF12;      
      case GLFW_KEY_UP:           return KeyUp;
      case GLFW_KEY_DOWN:         return KeyDown;
      case GLFW_KEY_LEFT:         return KeyLeft;
      case GLFW_KEY_RIGHT:        return KeyRight;
      case GLFW_KEY_LSHIFT:       return KeyShift;
      case GLFW_KEY_RSHIFT:       return KeyShift;
      case GLFW_KEY_LCTRL:        return KeyCtrl;
      case GLFW_KEY_RCTRL:        return KeyCtrl;
      case GLFW_KEY_LALT:         return KeyAlt;
      case GLFW_KEY_RALT:         return KeyAlt;
      case GLFW_KEY_TAB:          return KeyTab;
      case GLFW_KEY_ENTER:        return KeyEnter;
      case GLFW_KEY_BACKSPACE:    return KeyBackspace;
      case GLFW_KEY_INSERT:       return KeyInsert;
      case GLFW_KEY_DEL:          return KeyDelete;
      case GLFW_KEY_PAGEUP:       return KeyPageUp;
      case GLFW_KEY_PAGEDOWN:     return KeyPageDown;
      case GLFW_KEY_HOME:         return KeyHome;
      case GLFW_KEY_END:          return KeyEnd;
      case GLFW_KEY_KP_0:         return KeyNP0;
      case GLFW_KEY_KP_1:         return KeyNP1;
      case GLFW_KEY_KP_2:         return KeyNP2;
      case GLFW_KEY_KP_3:         return KeyNP3;
      case GLFW_KEY_KP_4:         return KeyNP4;
      case GLFW_KEY_KP_5:         return KeyNP5;
      case GLFW_KEY_KP_6:         return KeyNP6;
      case GLFW_KEY_KP_7:         return KeyNP7;
      case GLFW_KEY_KP_8:         return KeyNP8;
      case GLFW_KEY_KP_9:         return KeyNP9;
      case GLFW_KEY_KP_DIVIDE:    return KeyNPDivide;
      case GLFW_KEY_KP_MULTIPLY:  return KeyNPMultiply;
      case GLFW_KEY_KP_SUBTRACT:  return KeyNPSubtract;
      case GLFW_KEY_KP_ADD:       return KeyNPAdd;
      case GLFW_KEY_KP_DECIMAL:   return KeyNPDecimal;    
   }
   return (SLKey)key;
}

//-----------------------------------------------------------------------------
/*!
onResize: Event handler called on the resize event of the window. This event
should called once before the onPaint event.
*/
static void GLFWCALL onResize(int width, int height)
{  
   lastWidth = width;
   lastHeight = height;
   slResize(width, height);
}

//-----------------------------------------------------------------------------
/*!
Mouse button eventhandler forwards the events to the slMouseDown or slMouseUp.
Two finger touches of touch devices are simulated with ALT & CTRL modifiers.
*/
static void GLFWCALL onMouseButton(int button, int state)
{
   SLint x = mouseX;
   SLint y = mouseY;
      
   if (state==GLFW_PRESS)
   {  
      // simulate double touch from touch devices
      if (modifiers & KeyAlt) 
      {  
         // Do parallel double finger move
         if (modifiers, KeyShift)
         {  if (slTouch2Down(x, y, x - touchDeltaX, y - touchDeltaY))
               onPaint();
         } else // Do concentric double finger pinch
         {  if (slTouch2Down(x, y, touchX2, touchY2))
               onPaint();
         }
      } 
      else  // Do standard mouse down
      {  
         SLfloat mouseDeltaTime = (SLfloat)glfwGetTime() - lastMouseDownTime;
         lastMouseDownTime = (SLfloat)glfwGetTime();

         // handle double click 
         if (mouseDeltaTime < 0.3f)
         {  
            switch (button)
            {  case GLFW_MOUSE_BUTTON_LEFT:
                  if (slDoubleClick(ButtonLeft, x, y, modifiers)) onPaint();
                  break;
               case GLFW_MOUSE_BUTTON_RIGHT:
                  if (slDoubleClick(ButtonRight, x, y, modifiers)) onPaint();
                  break;
               case GLFW_MOUSE_BUTTON_MIDDLE:
                  if (slDoubleClick(ButtonMiddle, x, y, modifiers)) onPaint();
                  break;
            }
         } 
         else // normal mouse clicks
         {
            switch (button)
            {  case GLFW_MOUSE_BUTTON_LEFT:
                  if (slMouseDown(ButtonLeft, x, y, modifiers)) onPaint();
                  break;
               case GLFW_MOUSE_BUTTON_RIGHT:
                  if (slMouseDown(ButtonRight, x, y, modifiers)) onPaint();
                  break;
               case GLFW_MOUSE_BUTTON_MIDDLE:
                  if (slMouseDown(ButtonMiddle, x, y, modifiers)) onPaint();
                  break;
            }
         }
      }
   }
   else
   {  
      // simulate double touch from touch devices
      if (modifiers & KeyAlt) 
      {  
         // Do parallel double finger move
         if (modifiers & KeyShift)
         {  if (slTouch2Up(x, y, x - (touchX2 - x), y - (touchY2 - y)))
               onPaint();
         } else // Do concentric double finger pinch
         {   if (slTouch2Up(x, y, touchX2, touchY2))
               onPaint(); 
         }   
      } 
      else  // Do standard mouse down
      {  switch (button)
         {  case GLFW_MOUSE_BUTTON_LEFT:
               if (slMouseUp(ButtonLeft, x, y, modifiers)) onPaint(); 
               break;
            case GLFW_MOUSE_BUTTON_RIGHT:
               if (slMouseUp(ButtonRight, x, y, modifiers)) onPaint(); 
               break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
               if (slMouseUp(ButtonMiddle, x, y, modifiers)) onPaint(); 
               break;
         }
      }
   }
}

//-----------------------------------------------------------------------------
/*!
Mouse move eventhandler forwards the events to slMouseMove or slTouch2Move.
*/
static void GLFWCALL onMouseMove(int x, int y)
{     
   mouseX  = x;
   mouseY  = y;
   
   // Offset of 2nd. finger for two finger simulation
   
   // Simulate double finger touches   
   if (modifiers & KeyAlt) 
   {  
      // Do parallel double finger move
      if (modifiers & KeyShift)
      {  slTouch2Move(x, y, x - touchDeltaX, y - touchDeltaY);
      } 
      else // Do concentric double finger pinch
      {  int scrW2 = lastWidth / 2;
         int scrH2 = lastHeight / 2;
         touchX2 = scrW2 - (x - scrW2);
         touchY2 = scrH2 - (y - scrH2);
         touchDeltaX = x - touchX2;
         touchDeltaY = y - touchY2;
         slTouch2Move(x, y, touchX2, touchY2);
      }
   } else // Do normal mouse move
      if (slMouseMove(x, y)) onPaint();
}

//-----------------------------------------------------------------------------
/*!
Mouse wheel eventhandler forwards the events to slMouseWheel
*/
static void GLFWCALL onMouseWheel(int pos)
{     
   if (slMouseWheel(pos, modifiers)) onPaint();
}

//-----------------------------------------------------------------------------
/*!
Key action eventhandler sets the modifier key state & forewards the event to
the slKeyPress function.
*/
static void GLFWCALL onKeyAction(int GLFWKey, int action)
{     
   SLKey key = mapKeyToSLKey(GLFWKey);
    
   if (action==GLFW_PRESS)
   {  switch (key)
      {  case KeyCtrl:  modifiers = (SLKey)(modifiers|KeyCtrl);  return;
         case KeyAlt:   modifiers = (SLKey)(modifiers|KeyAlt);   return;
         case KeyShift: modifiers = (SLKey)(modifiers|KeyShift); return;
      }
   } else 
   if (action==GLFW_RELEASE)
   {  switch (key)
      {  case KeyCtrl:  modifiers = (SLKey)(modifiers&~KeyCtrl);  return;
         case KeyAlt:   modifiers = (SLKey)(modifiers&~KeyAlt);   return;
         case KeyShift: modifiers = (SLKey)(modifiers&~KeyShift); return;
      }
   }
   
   // Special treatment for ESC key
   if (key == KeyEsc && action==GLFW_PRESS) 
   {  if (slKeyPress(key, modifiers)) // ESC during RT stops it and returns false
      {  onClose();
         glfwCloseWindow();
      }
   } else
   {  
      if (action==GLFW_PRESS)
         slKeyPress(key, modifiers);
   }
}

//-----------------------------------------------------------------------------
/*!
The C main procedure running the GLFW GUI application.
*/
int main()
{  
   if (!glfwInit())
   {  fprintf(stderr, "Failed to initialize GLFW\n");
      exit(1);
   }
   
   // Enable fullscreen anti aliasing with 4 samples
   glfwOpenWindowHint( GLFW_FSAA_SAMPLES, 4);

   //Don't request a forward compatible profile
   //We still use backward compatible parts in GLSL
   //glfwOpenWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   //glfwOpenWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3);
   //glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 2);
   //glfwOpenWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

   if (!glfwOpenWindow(640, 480,  0,  0,  0,  0, 32, 0, GLFW_WINDOW))
   {  glfwTerminate();
      fprintf(stderr, "Failed to create GLFW window");
      exit(1);
   }
   
   /* Include OpenGL via GLEW
   The goal of the OpenGL Extension Wrangler Library (GLEW) is to assist C/C++ 
   OpenGL developers with two tedious tasks: initializing and using extensions 
   and writing portable applications. GLEW provides an efficient run-time 
   mechanism to determine whether a certain extension is supported by the 
   driver or not. OpenGL core and extension functionality is exposed via a 
   single header file. Download GLEW at: http://glew.sourceforge.net/
   */
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return -1;
   }
   
   glfwEnable(GLFW_MOUSE_CURSOR);
   glfwDisable(GLFW_KEY_REPEAT);
   glfwSetWindowTitle("SLProject Test Application");
   
   /* This function selects the minimum number of monitor vertical retraces 
   that should occur between two buffer swaps. If the selected swap interval 
   is one, the rate of buffer swaps will never be higher than the vertical 
   refresh rate of the monitor. If the selected swap interval is zero, 
   the rate of buffer swaps is only limited by the speed of the software and 
   the hardware.
   */
   glfwSwapInterval(1);

   // Set your own physical screen dpi
   int dpi = 142;
   
   // Initialize Scene Library
   slInit(640, 480, dpi, (SLCmd)SL_STARTSCENE,
          "../_globals/oglsl/",
          "../_data/models/3DS/",
          "../_data/images/textures/",
          (void*)&onPaint);
 
   // Set GLFW callback functions
   glfwSetWindowSizeCallback (onResize);
   glfwSetWindowCloseCallback(onClose);
   glfwSetMouseButtonCallback(onMouseButton);
   glfwSetMousePosCallback   (onMouseMove);
   glfwSetMouseWheelCallback (onMouseWheel);
   glfwSetKeyCallback        (onKeyAction);
   
   // The never ending event loop
   while (glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
   {
      // if no updated occured wait for the next event (power saving)
      if (!onPaint()) 
         glfwWaitEvents(); 
   }
   
   glfwTerminate();
   exit(0);
}
//-----------------------------------------------------------------------------
