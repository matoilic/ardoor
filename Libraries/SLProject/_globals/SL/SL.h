//#############################################################################
//  File:      Globals/SL.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SL_H
#define SL_H

//-----------------------------------------------------------------------------
// Include standard C++ libraries
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <typeinfo>
#include <string>
#include <algorithm>
#include <map>

//-----------------------------------------------------------------------------
// Include standard C libraries
#include <stdio.h>               // for the old ANSI C IO functions
#include <float.h>               // for defines like FLT_MAX & DBL_MAX
#include <assert.h>              // for debug asserts
#include <time.h>                // for clock()  
#include <sys/stat.h>            // for file info used in SLUtils
#include <math.h>                // for math functions
#include <string.h>              // for string functions

//-----------------------------------------------------------------------------
#if defined(TARGET_OS_IOS)
   #include <sys/time.h>
   #import <OpenGLES/ES2/gl.h>
   #import <OpenGLES/ES2/glext.h>
   #include <zlib.h>   
#elif defined(ANDROID_NDK)
   #include <sys/time.h>
   #include <jni.h>
   #include <android/log.h>
   #include <GLES2/gl2.h>
   #include <GLES2/gl2ext.h>
#elif defined(_WIN32)
   /* Include OpenGL via GLEW
   The goal of the OpenGL Extension Wrangler Library (GLEW) is to assist C/C++ 
   OpenGL developers with two tedious tasks: initializing and using extensions 
   and writing portable applications. GLEW provides an efficient run-time 
   mechanism to determine whether a certain extension is supported by the 
   driver or not. OpenGL core and extension functionality is exposed via a 
   single header file. Download GLEW at: http://glew.sourceforge.net/
   */
   #include <windows.h>
   #include <GL/glew.h>
   #include <GL/glfw.h>
#elif defined(__APPLE__)
   #include <sys/time.h>
   #if defined(SL_GUI_QT)
      #include <QGLWidget>
   #else
      #include <GL/glew.h>
      #include <GL/glfw.h>
   #endif
#elif defined(linux) || defined(__linux) || defined(__linux__)
   #include <sys/time.h>
   #include <GL/glew.h>
   #include <GL/glfw.h>
#endif

//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
// Determine operating system
#ifdef __APPLE__
   #ifdef TARGET_OS_IOS
      #define SL_OS_IOS
      #define SL_GLES2
      //#define SL_OMP
   #else
      #define SL_OS_MACOSX
      #if defined(_DEBUG)
         #define SL_MEMLEAKDETECT
      #else
         #define SL_OMP
      #endif
   #endif
#elif defined(ANDROID_NDK)
	#define SL_OS_ANDROID 
   #define SL_GLES2 
#elif defined(_WIN32)
   #define SL_OS_WIN32
   #define SL_USE_DISCARD_STEREOMODES
   #ifdef _DEBUG
      #define SL_MEMLEAKDETECT
   #else
      #define SL_OMP
   #endif
   #define STDCALL __stdcall
#elif defined(linux) || defined(__linux) || defined(__linux__)
   #define SL_OS_LINUX
   #define SL_USE_DISCARD_STEREOMODES
   #ifdef _DEBUG
      #define SL_MEMLEAKDETECT
   #else
      #define SL_OMP
   #endif
#else 
   #error "SL has not been ported to this OS"
#endif

//-----------------------------------------------------------------------------
// Determine compiler
#if defined(_MSC_VER)
   #define SL_COMP_MSVC
   #define SL_STDCALL __stdcall
   #define _CRT_SECURE_NO_DEPRECATE // visual 8 secure crt warning
#elif defined(__BORLANDC__)
   #define SL_COMP_BORLANDC
   #define SL_STDCALL Stdcall
#elif defined(__INTEL_COMPILER)
   #define SL_COMP_INTEL
   #define SL_STDCALL Stdcall
#elif defined(__GNUC__)
   #define SL_COMP_GNUC
   #define SL_STDCALL
#else 
   #error "SL has not been ported to this compiler"
#endif

//-----------------------------------------------------------------------------
// Redefinition of standard types for platform independency
typedef std::string     SLstring;
#ifndef SL_OS_ANDROID
typedef std::wstring    SLwstring;
#endif
typedef GLchar          SLchar;  // char is signed [-128 ... 127]!
typedef unsigned char   SLuchar;
typedef signed long     SLlong;
typedef unsigned long   SLulong;
typedef GLbyte          SLbyte;
typedef GLubyte         SLubyte;
typedef GLshort         SLshort;
typedef GLushort        SLushort; 
typedef GLint           SLint;
typedef GLuint          SLuint;
typedef GLsizei         SLsizei;
typedef GLfloat         SLfloat;
#ifdef SL_HAS_DOUBLE
typedef GLdouble        SLdouble;
typedef GLfloat         SLreal;
#else
typedef GLfloat         SLreal;
#endif
typedef bool            SLbool; 
typedef GLenum          SLenum;
typedef GLbitfield      SLbitfield;
typedef GLfloat         SLfloat;

typedef std::vector<SLbyte>   SLVbyte;
typedef std::vector<SLint>    SLVint;
typedef std::vector<SLuint>   SLVuint;
typedef std::vector<SLshort>  SLVshort;
typedef std::vector<SLushort> SLVushort;
typedef std::vector<SLfloat>  SLVfloat;
typedef std::vector<SLstring> SLVstring;

// OS specific defines & types
#ifdef SL_COMP_MSVC
typedef __int64               SLint64;
typedef unsigned __int64      SLuint64;
#define SL_INLINE __forceinline
#elif defined(SL_COMP_GNUC)
typedef long long int         SLint64;
typedef unsigned long long    SLuint64;
#define SL_INLINE inline
#endif

//-----------------------------------------------------------------------------
// Bit manipulation makros for ones that forget it always
#define SL_GETBIT(VAR, BITVAL) VAR&BITVAL
#define SL_SETBIT(VAR, BITVAL) VAR|=BITVAL
#define SL_DELBIT(VAR, BITVAL) VAR&=~BITVAL
#define SL_TOGBIT(VAR, BITVAL) if (VAR&BITVAL) VAR &=~BITVAL; else VAR|=BITVAL
//-----------------------------------------------------------------------------
//! Keyboard key codes
typedef enum 
{  KeyNone=0, 
   KeySpace=32,
   KeyTab=256,KeyEnter,KeyEsc,KeyBackspace,KeyDelete,
   KeyUp,KeyDown,KeyRight,KeyLeft, 
   KeyHome, KeyEnd, KeyInsert, KeyPageUp, KeyPageDown,
   KeyNP0,KeyNP1,KeyNP2,KeyNP3,KeyNP4,KeyNP5,KeyNP6,KeyNP7,KeyNP8,KeyNP9,
   KeyNPDivide,KeyNPMultiply,KeyNPAdd,KeyNPSubtract,KeyNPEnter,KeyNPDecimal,
   KeyF1,KeyF2,KeyF3,KeyF4,KeyF5,KeyF6,KeyF7,KeyF8,KeyF9,KeyF10,KeyF11,KeyF12, 
   KeyShift=0x00200000,KeyCtrl=0x00400000,KeyAlt=0x00800000
} SLKey;
//-----------------------------------------------------------------------------
//! Coordinate axis enumeration
typedef enum 
{  XAxis=0, 
   YAxis=1,
   ZAxis=2
} SLAxis;
//-----------------------------------------------------------------------------
//! SLCmd enumerates all possible menu and keyboard commands
typedef enum
{  cmdMenu,
   cmdAboutToggle,
   cmdHelpToggle,
   cmdCreditsToggle,
   cmdSceneInfoToggle,
   
   cmdSceneSmallTest,   // Loads the different scenes
   cmdSceneFigure,      
   cmdSceneMesh3DS,     
   cmdSceneRevolver,
   cmdSceneTextureFilter,
   cmdSceneTextureBlend,
   cmdSceneFrustumCull,
   cmdScenePerVertexBlinn,
   cmdScenePerPixelBlinn,
   cmdScenePerVertexWave,
   cmdSceneWater,
   cmdSceneBumpNormal,
   cmdSceneBumpParallax,
   cmdSceneEarth,
   cmdSceneTerrain,
   cmdSceneMuttenzerBox,
   cmdSceneRTSpheres,
   cmdSceneRTSoftShadows,
   cmdSceneRTDoF,
   
   cmdBBoxOn,
   cmdBBoxOff,
   cmdBBoxToggle,       // Toggles bounding box drawing bit
   cmdFaceCullOn,
   cmdFaceCullOff,
   cmdFaceCullToggle,   // Toggles face culling
   cmdFrustCullOn,
   cmdFrustCullOff,
   cmdFrustCullToggle,  // Toggles frustum culling
   cmdBBoxGroupOn,
   cmdBBoxGroupOff,
   cmdBBoxGroupToggle,  // Toggles group bbox drawing bit
   cmdAAOn,
   cmdAAOff,
   cmdAAToggle,         // Toggles anti aliasing
   cmdPolygonFill,
   cmdPolygonLine,
   cmdPolygonToggle,    // Toggles wireframe drawing bit
   cmdNormalsOn,
   cmdNormalsOff,
   cmdNormalsToggle,    // Toggles normale drawing bit
   
   cmdProjPersp,        // Perspective projection
   cmdProjOrtho,        // Orthographic projection
   cmdProjSideBySide,   // side-by-side
   cmdProjSideBySideP,  // side-by-side proportional
   cmdProjLineByLine,   // line-by-line
   cmdProjColByCol,     // column-by-column
   cmdProjCheckerBoard, // checkerboard pattern (DLP3D)
   cmdProjColorRC,      // color masking for red-cyan anaglyphs
   cmdProjColorRG,      // color masking for red-green anaglyphs
   cmdProjColorRB,      // color masking for red-blue anaglyphs
   cmdProjColorYB,      // color masking for yellow-blue anaglyphs (ColorCode 3D)
   
   cmdCamReset,         // Resets to the initial camera view
   cmdCamEyeSepInc,     // Cameras eye separation distance increase
   cmdCamEyeSepDec,     // Cameras eye separation distance decrease
   cmdCamFocalDistInc,  // Cameras focal distance increase
   cmdCamFocalDistDec,  // Cameras focal distance decrease
   cmdCamFOVInc,        // Cameras field of view increase
   cmdCamFOVDec,        // Cameras field of view decrease
   cmdCamAnimTurnTable, // Sets turntable camera animation
   cmdCamAnimWalk1stP,  // Sets 1st person walking camera animation
   cmdCamAnimFly1stP,   // Sets 1st person flying camera animation
   cmdCamSpeedLimitInc, // Increments the speed limit by 10%
   cmdCamSpeedLimitDec, // Decrements the speed limit by 10%
   
   cmdNoAnimationOn,    // No animation bit on
   cmdNoAnimationOff,   // No animation bit off
   cmdNoAnimationToggle,// No animation bit toggle
   cmdTextureOn,        // Texture drawing bit on
   cmdTextureOff,       // Texture drawing bit off
   cmdTextureToggle,    // Texture drawing bit toggle
   cmdVoxelsOn,
   cmdVoxelsOff,
   cmdVoxelsToggle,     // Toggles the voxel drawing bit
   cmdStatsToggle,      // Toggles statistics on/off
   cmdWaitEventsToggle, // Toggles the wait event flag
   cmdRenderOpenGL,     // Render with GL
   cmdRTContinuously,   // Do ray tracing continuously
   cmdRTStop,           // Stop ray tracing
   cmdRT1,              //1: Do ray tracing with max. depth 1
   cmdRT2,              //2: Do ray tracing with max. depth 2
   cmdRT3,              //3: Do ray tracing with max. depth 3
   cmdRT4,              //4: Do ray tracing with max. depth 4
   cmdRT5,              //5: Do ray tracing with max. depth 5
   cmdRT6,              //6: Do ray tracing with max. depth 6
   cmdRT7,              //7: Do ray tracing with max. depth 7
   cmdRT8,              //8: Do ray tracing with max. depth 8
   cmdRT9,              //9: Do ray tracing with max. depth 9
   cmdRT0,              //0: Do ray tracing with max. depth
   cmdRTSaveImage       // Save the ray tracing image
} SLCmd;
//-----------------------------------------------------------------------------
//! Mouse button codes
typedef enum 
{  ButtonNone=0,
   ButtonLeft,
   ButtonMiddle,
   ButtonRight
} SLMouseButton;
//-----------------------------------------------------------------------------
//! Enumeration for text alignment in a box
typedef enum
{  topLeft, topCenter, topRight,
   centerLeft, centerCenter, centerRight,
   bottomLeft, bottomCenter, bottomRight
} SLTextAlign;
//-----------------------------------------------------------------------------
#define UNUSED_PARAMETER(r)  ((void)(x))

//-----------------------------------------------------------------------------
// Some debuging and error handling functions and macros 
#define SL_EXIT_MSG(M)  SL::exitMsg((M), __LINE__, __FILE__)
#define SL_WARN_MSG(M)  SL::warnMsg((M), __LINE__, __FILE__)
#ifdef SL_OS_ANDROID
#define SL_LOG(...)  __android_log_print(ANDROID_LOG_INFO, "SLProject", __VA_ARGS__) 
#else
#define SL_LOG(...)  printf(__VA_ARGS__);
#endif

//-----------------------------------------------------------------------------
// c-string compare class
struct eqstr
{
   bool operator()(const char* s1, const char* s2) const
   {
      return strcmp(s1, s2) == 0;
   }
};
//-----------------------------------------------------------------------------
//! Class SL with static error message and exit functions.
class SL
{  public:

   //! SL::Exit terminates the application with a message. No leak cheching.
   static void exitMsg(const SLchar* msg, const SLint line, const SLchar* file);
   
   //! SL::Warn message output
   static void warnMsg(const SLchar* msg, const SLint line, const SLchar* file);
};
//-----------------------------------------------------------------------------
#endif
