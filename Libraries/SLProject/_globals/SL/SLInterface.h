//#############################################################################
//  File:      SLInterface.h
//  Purpose:   Delcaration of the main Scene Library C-Interface. Only these 
//             functions should called by the OS-dependend GUI applications. 
//             These functions can be called from any C, C++ or ObjectiveC GUI 
//             framework or by a native API such as Java Native Interface 
//             (JNI). See the implementation for more information.
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLINTERFACE_H
#define SLINTERFACE_H

#include <stdafx.h>

//-----------------------------------------------------------------------------
bool     slInit            (int    screenWidth, 
                            int    screenHeight,
                            int    dotsPerInch,
                            SLCmd  initScene, 
                            string shaderPath,
                            string modelPath,
                            string texturePath,
                            void*  raytracingCallback=0);
void     slClose           ();
void     slResize          (int width, int height);
bool     slPaint           ();
bool     slMouseDown       (SLMouseButton button, int x, int y, SLKey modifier);
bool     slMouseMove       (int x, int y);
bool     slMouseUp         (SLMouseButton button, int x, int y, SLKey modifier);
bool     slDoubleClick     (SLMouseButton button, int x, int y, SLKey modifier);
bool     slTouch2Down      (int x1, int y1, int x2, int y2);
bool     slTouch2Move      (int x1, int y1, int x2, int y2);
bool     slTouch2Up        (int x1, int y1, int x2, int y2);
bool     slMouseWheel      (int pos, SLKey modifier);
bool     slKeyPress        (SLKey key, SLKey modifier);
bool     slKeyRelease      (SLKey key, SLKey modifier);
bool     slCommand         (SLCmd command);
string   slGetWindowTitle  ();
//-----------------------------------------------------------------------------
#ifdef SL_OS_ANDROID

//! Pointer to JAVA environment used in ray tracing callback
JNIEnv* environment;

/*! Java Native Interface (JNI) function deklarations. These functions are called by
the Java interface class GLES2Lib. The function name follows the pattern
Java_{package name}_{JNI class name}_{function name}(JNIEnv* env, jobject obj, *);
In the function implementations we simply forward the C++ framework.
*/
extern "C"
{  JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onInit       (JNIEnv* env, jobject obj, jint width, jint height, jint dpi, jstring filePath);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onPaint      (JNIEnv* env, jobject obj);
   JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onResize     (JNIEnv* env, jobject obj, jint width, jint height);
   JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMenuButton (JNIEnv* env, jobject obj);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMouseDown  (JNIEnv* env, jobject obj, jint button, jint x, jint y);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMouseUp    (JNIEnv* env, jobject obj, jint button, jint x, jint y);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMouseMove  (JNIEnv* env, jobject obj, jint x, jint y);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onTouch2Up   (JNIEnv* env, jobject obj, jint x1, jint y1, jint x2, jint y2);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onTouch2Down (JNIEnv* env, jobject obj, jint x1, jint y1, jint x2, jint y2);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onTouch2Move (JNIEnv* env, jobject obj, jint x1, jint y1, jint x2, jint y2);
   JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onDoubleClick(JNIEnv* env, jobject obj, jint button, jint x, jint y);
   JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onClose      (JNIEnv* env, jobject obj);
};

//! Native ray tracing callback function that calls the Java class method GLES2Lib.RaytracingCallback
bool Java_renderRaytracingCallback()
{
	jclass klass = environment->FindClass("ch/fhnw/comgr/GLES2Lib");
	jmethodID method = environment->GetStaticMethodID(klass, "RaytracingCallback", "()Z");
   return environment->CallStaticObjectMethod(klass, method);
}

//! Native OpenGL info string print functions used in onInit
static void printGLString(const char *name, GLenum s) 
{
    const char *v = (const char *) glGetString(s);
    SL_LOG("GL %s = %s\n", name, v);
}

JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onInit(JNIEnv* env, jobject obj, jint width, jint height, jint dpi, jstring filePath)
{   
   environment = env;
	const char *nativeString = env->GetStringUTFChars(filePath, 0);
   string devicePath(nativeString);
   env->ReleaseStringUTFChars(filePath, nativeString);

   printGLString("Version", GL_VERSION);
   printGLString("Vendor", GL_VENDOR);
   printGLString("Renderer", GL_RENDERER);
   printGLString("Extensions", GL_EXTENSIONS);
    
   slInit(width, height, dpi, 
          cmdSceneMesh3DS,
          devicePath + "/shaders/",
          devicePath + "/models/", 
          devicePath + "/textures/",
          (void*)&Java_renderRaytracingCallback);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onPaint(JNIEnv* env, jobject obj)
{
   return slPaint();
}

JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onResize(JNIEnv* env, jobject obj,  jint width, jint height)
{
    slResize(width, height);
}

JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMenuButton(JNIEnv* env, jobject obj)
{  
   SL_LOG("onMenuButton");
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMouseDown (JNIEnv* env, jobject obj, jint button, jint x, jint y)
{
   slMouseDown(ButtonLeft, x, y, KeyNone);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMouseUp(JNIEnv* env, jobject obj, jint button, jint x, jint y)
{
   slMouseUp(ButtonLeft, x, y, KeyNone);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onMouseMove(JNIEnv* env, jobject obj, jint x, jint y)
{
   slMouseMove(x, y);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onTouch2Down(JNIEnv* env, jobject obj, jint x1, jint y1, jint x2, jint y2)
{
   slTouch2Down(x1, y1, x2, y2);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onTouch2Up(JNIEnv* env, jobject obj, jint x1, jint y1, jint x2, jint y2)
{
   slTouch2Up(x1, y1, x2, y2);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onTouch2Move(JNIEnv* env, jobject obj, jint x1, jint y1, jint x2, jint y2)
{
   slTouch2Move(x1, y1, x2, y2);
}

JNIEXPORT bool JNICALL Java_ch_fhnw_comgr_GLES2Lib_onDoubleClick(JNIEnv* env, jobject obj, jint button, jint x, jint y)
{
   slDoubleClick(ButtonLeft, x, y, KeyNone);
}

JNIEXPORT void JNICALL Java_ch_fhnw_comgr_GLES2Lib_onClose(JNIEnv* env, jobject obj)
{
   slClose();
}

#endif
//-----------------------------------------------------------------------------
#endif
