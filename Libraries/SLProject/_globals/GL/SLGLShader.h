//#############################################################################
//  File:      SLGLShader.h
//  Author:    Marcus Hudritsch 
//             Mainly based on Martin Christens GLSL Tutorial
//             See http://www.clockworkcoders.com
//  Date:      February 2013
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLGLSHADER_H
#define SLGLSHADER_H

#include <stdafx.h>

//-----------------------------------------------------------------------------
//! Shader type enumeration for vertex or fragment (pixel) shader
typedef enum 
{  SLNoShader=0,
   SLVertexShader=1, 
   SLFragmentShader=2
} SLShaderType;
//-----------------------------------------------------------------------------
//! Encapsulation of an OpenGL shader object
/*!
The SLGLShader class represents a shader object of the OpenGL Shading Language
(GLSL). It can load & compile an GLSL shader file and is later on attached
to an OpenGL shader program (SLGLShaderProg).
*/
class SLGLShader : public SLObject
{  friend class SLGLShaderProg;
   public:
                        SLGLShader(SLstring filename, 
                                   SLShaderType shaderType);
                       ~SLGLShader();
                       
         SLbool         createAndCompile();
         void           load(SLstring filename);
         void           loadFromMemory(SLstring program);
         
   protected:         
         SLShaderType   _shaderType;      //!< Shader type enumeration
         SLuint         _shaderObjectGL;  //!< Program Object
         SLstring       _shaderSource;    //!< ASCII Source-Code
         SLstring       _shaderFile;      //!< Path & filename of shader
};
//-----------------------------------------------------------------------------
#endif // SLSHADEROBJECT_H
