#include <ARDoorLib.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Logging.h"

cv::Mat inframe;
cv::Mat outframe;
cv::Mat rgbFrame;

GLfloat vVertices[] = {
	   -0.5f,  0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f
};

GLfloat vTexture[] = {
	    1.0f, 1.0f,
	    0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
};

GLuint program;

cv::VideoCapture cap;

void checkGLErrors(const char *label) {
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        LOGE("OpenGL ERROR at %s(): 0x%x", label, errCode);
    }
}

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;

   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
        return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );

   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled )
   {
      GLint infoLen = 0;
      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char* infoLog = (char*) malloc (sizeof(char) * infoLen );
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         LOGE(infoLog);
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

// Function turn a cv::Mat into a texture, and return the texture ID as a GLuint for use
GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Catch silly-mistake texture interpolation method for magnification
	if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
	    magFilter == GL_LINEAR_MIPMAP_NEAREST ||
	    magFilter == GL_NEAREST_MIPMAP_LINEAR ||
	    magFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		std::cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << std::endl;
		magFilter = GL_LINEAR;
	}

	// Set texture interpolation methods for minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_RGB;
	if (mat.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}

	// Create the texture
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
	             0,                 // Pyramid level (for mip-mapping) - 0 is the top level
	             GL_RGB,            // Internal colour format to convert to
	             mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
	             mat.rows,          // Image height i.e. 480 for Kinect in standard mode
	             0,                 // Border width in pixels (can either be 1 or 0)
	             inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
	             GL_UNSIGNED_BYTE,  // Image data type
	             mat.ptr());        // The actual image data itself

	// If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
	if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
	    minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	    minFilter == GL_NEAREST_MIPMAP_LINEAR ||
	    minFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	return textureID;
}

cv::Mat* ARDoorLib::processCanny(cv::Mat& frame)
{
	cv::Mat* result = new cv::Mat();
	cv::Mat gray;
	cv::cvtColor(frame, gray, CV_RGBA2GRAY);
	cv::Canny(gray, *result, 30, 150);
	return result;
}

void ARDoorLib::onSurfaceCreated()
{
	static const char vShaderStr[] = "attribute vec4 a_position;   \n"
	        "attribute vec2 a_texCoord;   \n"
	        "varying vec2 v_texCoord;     \n"
	        "void main()                  \n"
	        "{                            \n"
	        "   gl_Position = a_position; \n"
	        "   v_texCoord = a_texCoord;  \n"
	        "}                            \n";

	static const char fShaderStr[] =
	                "precision mediump float;                            \n"
	                        "varying vec2 v_texCoord;                            \n"
	                        "uniform sampler2D s_texture;                        \n"
	                        "void main()                                         \n"
	                        "{                                                   \n"
	                        "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
	                        "}\n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   checkGLErrors("after vertexShader");

   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );
   checkGLErrors("after fragmentShader");

   // Create the program object
   programObject = glCreateProgram ( );
   checkGLErrors("glCreateProgram");

   if ( programObject == 0 )
	  return;

   glAttachShader ( programObject, vertexShader );
   checkGLErrors("glAttachShader");

   glAttachShader ( programObject, fragmentShader );
   checkGLErrors("glAttachShader 2");

   // Bind vPosition to attribute 0
   glBindAttribLocation ( programObject, 0, "vPosition" );
   checkGLErrors("glBindAttribLocation");

   // Link the program
   glLinkProgram ( programObject );
   checkGLErrors("glLinkProgram");

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );
   checkGLErrors("glGetProgramiv");

   if ( !linked )
   {
	  GLint infoLen = 0;
	  glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

	  if ( infoLen > 1 )
	  {
		 char* infoLog = (char*) malloc (sizeof(char) * infoLen );
		 glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
		 LOGE(infoLog);
		 free ( infoLog );
	  }

	  glDeleteProgram ( programObject );
	  return;
   }

   // Store the program object
   program = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
}

void ARDoorLib::onSurfaceChanged(int width, int height)
{
	glViewport(0, 0, width, height);

	cap.open(CV_CAP_ANDROID + 0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
}

void ARDoorLib::onDrawFrame()
{
	if (!cap.isOpened()) {
		return;
	}
	cap.read(inframe);

	cvtColor(inframe, outframe, CV_BGR2RGB);

	cv::Size dstSize = cv::Size(1024, 1024);
	cv::resize(outframe, rgbFrame, dstSize, 0, 0, cv::INTER_LINEAR);


	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	GLuint tex = matToTexture(rgbFrame, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
	checkGLErrors("after matToTexture");

	// Use the program object
	glUseProgram ( program );
	checkGLErrors("glUseProgram");

    int ph = glGetAttribLocation(program, "vPosition");
    checkGLErrors("glGetAttribLocation");
    int tch = glGetAttribLocation ( program, "vTexCoord" );
    checkGLErrors("glGetAttribLocation 2");
    int th = glGetUniformLocation ( program, "sTexture" );
    checkGLErrors("glGetUniformLocation");

    glActiveTexture(GL_TEXTURE0);
    checkGLErrors("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, tex);
    checkGLErrors("glBindTexture");
    glUniform1i(th, 0);
    checkGLErrors("glUniform1i");

    LOGD("%d", ph);
	glVertexAttribPointer(ph, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), vVertices);
	checkGLErrors("glVertexAttribPointer");
    glVertexAttribPointer(tch, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vTexture );
    checkGLErrors("glVertexAttribPointer 2");
    glEnableVertexAttribArray(ph);
    checkGLErrors("glEnableVertexAttribArray");
    glEnableVertexAttribArray(tch);
    checkGLErrors("glEnableVertexAttribArray 2");

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	checkGLErrors("glDrawArrays");
}

