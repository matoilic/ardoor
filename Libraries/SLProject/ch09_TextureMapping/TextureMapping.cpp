//#############################################################################
//  File:      TextureMapping.cpp
//  Purpose:   Core profile OpenGL application for ambient-diffuse-specular
//             lighting shaders with Textures.
//  Date:      September 2011 (HS11)
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

//-----------------------------------------------------------------------------
//! Struct defintion for vertex attributes
struct VertexPNT
{  
   SLVec3f p;  // vertex position [x,y,z]
   SLVec3f n;  // vertex normal [x,y,z]
   SLVec2f t;  // vertex texture coord. [s,t]
};
//-----------------------------------------------------------------------------
// GLobal application variables
SLMat4f  _modelViewMatrix;          //!< 4x4 modelview matrix
SLMat4f  _projectionMatrix;         //!< 4x4 projection matrix

GLuint    _numI = 0;                //!< NO. of vertex indexes for triangles
GLuint    _vboV = 0;                //!< ID of the VBO for vertex array
GLuint    _vboI = 0;                //!< ID of the VBO for vertex index array
GLint     _resolution;              //!< resolution of sphere stack & slices

float    _camZ;                     //!< z-distance of camera
float    _rotX, _rotY;              //!< rotation angles around x & y axis
int      _deltaX, _deltaY;                  //!< delta mouse motion
int      _startX, _startY;          //!< x,y mouse start positions
int      _mouseX, _mouseY;          //!< current mouse position
bool     _mouseLeftDown;            //!< Flag if mouse is down
GLuint   _modifiers = 0;            //!< modifier bit flags
const GLuint NONE  = 0;             //!< constant for no modifier
const GLuint SHIFT = 0x00200000;    //!< constant for shift key modifier
const GLuint CTRL  = 0x00400000;    //!< constant for control key modifier
const GLuint ALT   = 0x00800000;    //!< constant for alt key modifier

SLVec4f  _globalAmbi;               //!< global ambient intensity
SLVec3f  _lightPos;                 //!< Light position in world space
SLVec3f  _lightDir;                 //!< Light direction in world space
SLVec4f  _lightAmbient;             //!< Light ambient intensity   
SLVec4f  _lightDiffuse;             //!< Light diffuse intensity   
SLVec4f  _lightSpecular;            //!< Light specular intensity
SLVec4f  _matAmbient;               //!< Material ambient reflection coeff.
SLVec4f  _matDiffuse;               //!< Material diffuse reflection coeff.
SLVec4f  _matSpecular;              //!< Material specular reflection coeff.
SLVec4f  _matEmissive;              //!< Material emissive coeff.
float    _matShininess;             //!< Material shininess exponent

GLuint   _shaderVertID = 0;         //!< vertex shader id
GLuint   _shaderFragID = 0;         //!< fragment shader id
GLuint   _shaderProgID = 0;         //!< shader program id
GLuint   _textureID = 0;            //!< texture id
  
GLint    _pLoc;            //!< attribute location for vertex position
GLint    _nLoc;            //!< attribute location for vertex normal
GLint    _tLoc;            //!< attribute location for vertex texcoords
GLint    _mvpMatrixLoc;    //!< uniform location for modelview-projection matrix
GLint    _mvMatrixLoc;     //!< uniform location for modelview matrix
GLint    _nMatrixLoc;      //!< uniform location for normal matrix
GLint    _globalAmbiLoc;   //!< uniform location for global ambient intensity
GLint    _lightPosVSLoc;   //!< uniform location for light position in VS 
GLint    _lightDirVSLoc;   //!< uniform location for light direction in VS 
GLint    _lightAmbientLoc; //!< uniform location for ambient light intensity 
GLint    _lightDiffuseLoc; //!< uniform location for diffuse light intensity 
GLint    _lightSpecularLoc;//!< uniform location for specular light intensity
GLint    _matAmbientLoc;   //!< uniform location for ambient light reflection
GLint    _matDiffuseLoc;   //!< uniform location for diffuse light reflection
GLint    _matSpecularLoc;  //!< uniform location for specular light reflection
GLint    _matEmissiveLoc;  //!< uniform location for light emission
GLint    _matShininessLoc; //!< uniform location for shininess

GLint    _texture0Loc;     //!< uniform location for texture 0

static const SLfloat PI = 3.14159265358979f;

//-----------------------------------------------------------------------------
/*!
buildSphere creates the vertex attributes for a sphere and creates the VBO
at the end. The sphere is built in stacks & slices.
*/
void buildSphere(float radius, int stacks, int slices)
{  
   assert(stacks > 3 && slices > 3);

   // create vertex array
   SLuint numV = (stacks+1) * (slices+1);
   VertexPNT* v = new VertexPNT[numV];

   float  theta, dtheta; // angles around x-axis
   float  phi, dphi;     // angles around z-axis
   int    i, j;          // loop counters
   GLuint iv = 0;
   
   // init start values
   theta = 0.0f;
   dtheta = PI / stacks;
   dphi = 2.0f * PI / slices;
   
   // Define vertex position & normals by looping through all stacks
   for (i=0; i<=stacks; ++i)
   {  
      float sin_theta  = sin(theta);
      float cos_theta  = cos(theta);
      phi = 0.0f;

      // Loop through all slices
      for (j = 0; j<=slices; ++j)
      {  
         if (j==slices) phi = 0.0f;

         // define first the normal with length 1
         v[iv].n.x = sin_theta * cos(phi);
         v[iv].n.y = sin_theta * sin(phi);
         v[iv].n.z = cos_theta;

         // set the vertex position w. the scaled normal
         v[iv].p.x = radius * v[iv].n.x;
         v[iv].p.y = radius * v[iv].n.y;
         v[iv].p.z = radius * v[iv].n.z;
         
         // set the texture coords.
         v[iv].t.x = 0; // ???
         v[iv].t.y = 0; // ???

         phi += dphi;
         iv++;
      }
      theta += dtheta;
   }

   // create Index array x
   _numI = slices * stacks * 2 * 3;
   GLuint* x = new GLuint[_numI];
   GLuint ii = 0, iV1, iV2;

   for (i=0; i<stacks; ++i)
   {  
      // index of 1st & 2nd vertex of stack
      iV1 = i * (slices+1);
      iV2 = iV1 + slices + 1;

      for (j = 0; j<slices; ++j)
      {  // 1st triangle ccw         
         x[ii++] = iV1+j;
         x[ii++] = iV2+j;
         x[ii++] = iV2+j+1;
         // 2nd triangle ccw
         x[ii++] = iV1+j;
         x[ii++] = iV2+j+1;
         x[ii++] = iV1+j+1;
      }
   }

   // Create vertex buffer objects
   _vboV = glUtils::buildVBO(v,  numV, 8, sizeof(GLfloat), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
   _vboI = glUtils::buildVBO(x, _numI, 1, sizeof(GLuint), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

   // Delete arrays on heap
   delete[] v;
   delete[] x;
}
//-----------------------------------------------------------------------------
/*!
buildSquare creates the vertex attributes for a textured square and VBO.
*/
void buildSquare()
{
   // create vertex array for interleaved position, normal and texCoord
	//            Position,  Normal  , texCrd,
   float v[] = {-1, 0, -1,  0, -1, 0,  0,  0, // Vertex 0
                 1, 0, -1,  0, -1, 0,  1,  0, // Vertex 1
                 1, 0,  1,  0, -1, 0,  1,  1, // Vertex 2
                -1, 0,  1,  0, -1, 0,  0,  1};// Vertex 3
   _vboV = glUtils::buildVBO(v, 6, 8, sizeof(GLfloat), GL_ARRAY_BUFFER, GL_STATIC_DRAW);

   // create index array for GL_TRIANGLES
   _numI = 6;
   GLuint i[] = {0, 1, 2,  0, 2, 3};
   _vboI = glUtils::buildVBO(i, _numI, 1, sizeof(GLuint), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
}
//-----------------------------------------------------------------------------
/*!
calcFPS determines the frame per second measurement by averaging 60 frames.
*/
float calcFPS(float deltaTime)
{  const  SLint   FILTERSIZE = 60;
   static SLfloat frameTimes[FILTERSIZE];
   static SLuint  frameNo = 0;

   frameTimes[frameNo % FILTERSIZE] = deltaTime;
   float sumTime = 0.0f;
   for (SLuint i=0; i<FILTERSIZE; ++i) sumTime += frameTimes[i];
   frameNo++;
   float frameTimeSec = sumTime / (SLfloat)FILTERSIZE;
   float fps = 1 / frameTimeSec;

   return fps;
}
//-----------------------------------------------------------------------------
/*!
onInit initializes the global variables and builds the shader program. It
should be called after a window with a valid OpenGL context is present.
*/
void onInit()
{  
   // Define sphere
   _resolution = 64;
   
   //buildSphere(1.0f, _resolution, _resolution);
   buildSquare();

   // Set light parameters
   _globalAmbi.set(0.0f, 0.0f, 0.0f);
   _lightPos.set( 0.0f, 0.0f, 100.0f);   
   _lightDir.set( 0.0f, 0.0f,-1.0f);   
   _lightAmbient.set( 0.1f, 0.1f, 0.1f);  
   _lightDiffuse.set( 1.0f, 1.0f, 1.0f);
   _lightSpecular.set( 1.0f, 1.0f, 1.0f);
   _matAmbient.set( 1.0f, 1.0f, 1.0f);    
   _matDiffuse.set( 1.0f, 1.0f, 1.0f);    
   _matSpecular.set( 1.0f, 1.0f, 1.0f);    
   _matEmissive.set( 0.0f, 0.0f, 0.0f);
   _matShininess = 100.0f; 
   
   // backwards movement of the camera
   _camZ = -3.0f;      

   // Mouse rotation paramters
   _rotX = 0;
   _rotY = 0;
   _deltaX = 0;
   _deltaY = 0;
   _mouseLeftDown = false;

   // Load textures
   _textureID = glUtils::buildTexture("../_data/images/textures/earth2048_C.jpg");

   // Load, compile & link shaders
   _shaderVertID = glUtils::buildShader("../_globals/oglsl/ADSTex.vert", GL_VERTEX_SHADER);
   _shaderFragID = glUtils::buildShader("../_globals/oglsl/ADSTex.frag", GL_FRAGMENT_SHADER);
   _shaderProgID = glUtils::buildProgram(_shaderVertID, _shaderFragID);

   // Activate the shader programm
   glUseProgram(_shaderProgID); 

   // Get the variable locations (identifiers) within the program
   _pLoc            = glGetAttribLocation (_shaderProgID, "a_position");
   _nLoc            = glGetAttribLocation (_shaderProgID, "a_normal");
   _tLoc            = glGetAttribLocation (_shaderProgID, "a_texCoord");
   _mvMatrixLoc     = glGetUniformLocation(_shaderProgID, "u_mvMatrix");
   _mvpMatrixLoc    = glGetUniformLocation(_shaderProgID, "u_mvpMatrix");
   _nMatrixLoc      = glGetUniformLocation(_shaderProgID, "u_nMatrix");
   _globalAmbiLoc   = glGetUniformLocation(_shaderProgID, "u_globalAmbi");
   _lightPosVSLoc   = glGetUniformLocation(_shaderProgID, "u_lightPosVS");
   _lightDirVSLoc   = glGetUniformLocation(_shaderProgID, "u_lightDirVS");
   _lightAmbientLoc = glGetUniformLocation(_shaderProgID, "u_lightAmbient");
   _lightDiffuseLoc = glGetUniformLocation(_shaderProgID, "u_lightDiffuse");
   _lightSpecularLoc= glGetUniformLocation(_shaderProgID, "u_lightSpecular");
   _matAmbientLoc   = glGetUniformLocation(_shaderProgID, "u_matAmbient");
   _matDiffuseLoc   = glGetUniformLocation(_shaderProgID, "u_matDiffuse");
   _matSpecularLoc  = glGetUniformLocation(_shaderProgID, "u_matSpecular");
   _matEmissiveLoc  = glGetUniformLocation(_shaderProgID, "u_matEmissive");
   _matShininessLoc = glGetUniformLocation(_shaderProgID, "u_matShininess");
   _texture0Loc     = glGetUniformLocation(_shaderProgID, "u_texture0");      

   // Set some OpenGL states
   glClearColor(0.0f, 0.0f, 0.0f, 1);  // Set the background color         
   glEnable(GL_DEPTH_TEST);            // Enables depth test
   glEnable(GL_CULL_FACE);             // Enables the culling of back faces
   GET_GL_ERROR;                       // Check for OpenGL errors
}
//-----------------------------------------------------------------------------
/*!
onClose is called when the user closes the window and can be used for proper
deallocation of resources.
*/
int onClose()
{
   // Delete shaders & programs on GPU
   glDeleteShader(_shaderVertID);
   glDeleteShader(_shaderFragID);
   glDeleteProgram(_shaderProgID);
   
   // Delete arrays & buffers on GPU
   glDeleteBuffers(1, &_vboV);
   glDeleteBuffers(1, &_vboI);

   // Remove callback functions
   glfwSetWindowSizeCallback (NULL);
   glfwSetWindowCloseCallback(NULL);
   glfwSetMouseButtonCallback(NULL);
   glfwSetMousePosCallback   (NULL);
   glfwSetMouseWheelCallback (NULL);
   glfwSetWindowCloseCallback(NULL);

   // Return true for closing window
   return GL_TRUE;
}
//-----------------------------------------------------------------------------
/*!
onPaint does all the rendering for one frame from scratch with OpenGL.
*/
bool onPaint()
{  
   // Clear the color & depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Start with identity every frame
   _modelViewMatrix.identity();
   
   // View transform: move the coordinate system away from the camera
   _modelViewMatrix.translate(0, 0, _camZ);

   // View transform: rotate the coordinate system increasingly
   _modelViewMatrix.rotate(_rotX + _deltaX, 1,0,0);
   _modelViewMatrix.rotate(_rotY + _deltaY, 0,1,0);

   // Transform light position & direction into view space
   SLVec3f lightPosVS = _modelViewMatrix * _lightPos;
   
   // The light dir is not a position. We only take the rotation of the mv matrix.
   SLMat3f viewRot    = _modelViewMatrix.mat3();
   SLVec3f lightDirVS = viewRot * _lightDir;

   // Rotate the model so that we see it
   _modelViewMatrix.rotate(90, -1,0,0);

   // Build the combined modelview-projection matrix
   SLMat4f mvp(_projectionMatrix);
   mvp.multiply(_modelViewMatrix);

   // Build normal matrix
   SLMat3f nm(_modelViewMatrix.inverseTransposed());

   // Pass the matrix uniform variables
   glUniformMatrix4fv(_mvMatrixLoc,  1, 0, (float*)&_modelViewMatrix);
   glUniformMatrix3fv(_nMatrixLoc,   1, 0, (float*)&nm);
   glUniformMatrix4fv(_mvpMatrixLoc, 1, 0, (float*)&mvp);

   // Pass lighting uniforms variables
   glUniform4fv(_globalAmbiLoc,     1, (float*)&_globalAmbi);
   glUniform3fv(_lightPosVSLoc,     1, (float*)&lightPosVS);
   glUniform3fv(_lightDirVSLoc,     1, (float*)&lightDirVS);
   glUniform4fv(_lightAmbientLoc,   1, (float*)&_lightAmbient);
   glUniform4fv(_lightDiffuseLoc,   1, (float*)&_lightDiffuse);
   glUniform4fv(_lightSpecularLoc,  1, (float*)&_lightSpecular);
   glUniform4fv(_matAmbientLoc,     1, (float*)&_matAmbient); 
   glUniform4fv(_matDiffuseLoc,     1, (float*)&_matDiffuse); 
   glUniform4fv(_matSpecularLoc,    1, (float*)&_matSpecular); 
   glUniform4fv(_matEmissiveLoc,    1, (float*)&_matEmissive);
   glUniform1f (_matShininessLoc,   _matShininess);
   glUniform1i (_texture0Loc,       0);
     
   //////////////////////
   // Draw with 2 VBOs //
   //////////////////////

   // Enable all of the vertex attribute arrays
   glEnableVertexAttribArray(_pLoc);
   glEnableVertexAttribArray(_nLoc);
   glEnableVertexAttribArray(_tLoc);

   // Activate VBOs
   glBindBuffer(GL_ARRAY_BUFFER, _vboV);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboI);

   // Activate Texture
   glBindTexture(GL_TEXTURE_2D, _textureID);

   // For VBO only offset instead of data pointer
   GLsizei stride  = sizeof(VertexPNT);
   GLsizei offsetN = sizeof(SLVec3f);
   GLsizei offsetT = sizeof(SLVec3f) + sizeof(SLVec3f);
   glVertexAttribPointer(_pLoc, 3, GL_FLOAT, GL_FALSE, stride, 0);
   glVertexAttribPointer(_nLoc, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetN);
   glVertexAttribPointer(_tLoc, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetT);
   
   ////////////////////////////////////////////////////////
   // Draw cube model triangles by indexes
   glDrawElements(GL_TRIANGLES, _numI, GL_UNSIGNED_INT, 0);
   ////////////////////////////////////////////////////////

   // Deactivate buffers
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   // Disable the vertex arrays
   glDisableVertexAttribArray(_pLoc);
   glDisableVertexAttribArray(_nLoc);
   glDisableVertexAttribArray(_tLoc);
   
   // Check for errors from time to time
   //GET_GL_ERROR;

   // Fast copy the back buffer to the front buffer. This is OS dependent.
   glfwSwapBuffers();

   // Calculate frames per second
   char title[255];
   static float lastTimeSec = 0;
   float timeNowSec = (float)glfwGetTime();
   float fps = calcFPS(timeNowSec-lastTimeSec);
   sprintf(title, "Sphere, %d x %d, fps: %4.0f", _resolution, _resolution, fps);
   glfwSetWindowTitle(title);
   lastTimeSec = timeNowSec;

   // Return true to get an immediate refresh 
   return true;
}
//-----------------------------------------------------------------------------
/*!
onResize: Event handler called on the resize event of the window. This event
should called once before the onPaint event. Do everything that is dependent on
the size and ratio of the window.
*/
void onResize(int width, int height)
{  
   double w = (double)width;
   double h = (double)height;
   
   // define the projection matrix
   _projectionMatrix.perspective(45, w/h, 0.01f, 10.0f);
   
   // define the viewport
   glViewport(0, 0, width, height);

   onPaint();
}
//-----------------------------------------------------------------------------
/*!
Mouse button down & release eventhandler starts and end mouse rotation
*/
void onMouseButton(int button, int state)
{
   SLint x = _mouseX;
   SLint y = _mouseY;
      
   _mouseLeftDown = (state==GLFW_PRESS);
   if (_mouseLeftDown)
   {  _startX = x;
      _startY = y;

      // Renders only the lines of a polygon during mouse moves
      if (button==GLFW_MOUSE_BUTTON_RIGHT)
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   } else
   {  _rotX += _deltaX;
      _rotY += _deltaY;
      _deltaX = 0;
      _deltaY = 0;

      // Renders filled polygons
      if (button==GLFW_MOUSE_BUTTON_RIGHT)
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }
}
//-----------------------------------------------------------------------------
/*!
Mouse move eventhandler tracks the mouse delta since touch down (_deltaX/_deltaY)
*/
void onMouseMove(int x, int y)
{     
   _mouseX  = x;
   _mouseY  = y;
   
   if (_mouseLeftDown)
   {  _deltaY = x - _startX;
      _deltaX = y - _startY;
      onPaint();
   }
}
//-----------------------------------------------------------------------------
/*!
Mouse wheel eventhandler that moves the camera foreward or backwards
*/
void onMouseWheel(int wheelPos)
{  
   static int lastMouseWheelPos = 0;
   int delta = wheelPos-lastMouseWheelPos;
   lastMouseWheelPos = wheelPos;

   if (_modifiers == NONE)
   {
      _camZ += (SLfloat)SL_sign(delta)*0.1f;
      onPaint();
   }
}
//-----------------------------------------------------------------------------
/*!
Key action eventhandler handles key down & release events
*/
void onKey(int GLFWKey, int action)
{         
   if (action==GLFW_PRESS)
   {  
      switch (GLFWKey)
      {
         case GLFW_KEY_ESC:
            onClose();
            glfwCloseWindow();
            break;
         case GLFW_KEY_UP:
            _resolution = _resolution<<1;
            buildSphere(1.0f, _resolution, _resolution);
            break;
         case GLFW_KEY_DOWN:
            if (_resolution > 4) _resolution = _resolution>>1;
            buildSphere(1.0f, _resolution, _resolution);
            break;
         case GLFW_KEY_LSHIFT: _modifiers = _modifiers|SHIFT; break;
         case GLFW_KEY_RSHIFT: _modifiers = _modifiers|SHIFT; break;
         case GLFW_KEY_LCTRL:  _modifiers = _modifiers|CTRL; break; 
         case GLFW_KEY_RCTRL:  _modifiers = _modifiers|CTRL; break; 
         case GLFW_KEY_LALT:   _modifiers = _modifiers|ALT; break;
         case GLFW_KEY_RALT:   _modifiers = _modifiers|ALT; break;
      }
   } else
   if (action == GLFW_RELEASE)
   {  switch (GLFWKey)
      {  case GLFW_KEY_LSHIFT: _modifiers = _modifiers&~SHIFT; break;
         case GLFW_KEY_RSHIFT: _modifiers = _modifiers&~SHIFT; break;
         case GLFW_KEY_LCTRL:  _modifiers = _modifiers&~CTRL; break; 
         case GLFW_KEY_RCTRL:  _modifiers = _modifiers&~CTRL; break; 
         case GLFW_KEY_LALT:   _modifiers = _modifiers&~ALT; break;
         case GLFW_KEY_RALT:   _modifiers = _modifiers&~ALT; break;
      }
   }
}

//-----------------------------------------------------------------------------
/*!
The C main procedure running the GLFW GUI application.
*/
int main()
{  
   // Init the GLFW library for the GUI interface
   if (!glfwInit())
   {  fprintf(stderr, "Failed to initialize GLFW\n");
      exit(1);
   }
   
   // Enable fullscreen antialiasing with 4 samples
   glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

   // Create the window with the GLFW library
   if (!glfwOpenWindow(640, 480, 0,0,0,0, 32, 0, GLFW_WINDOW))
   {  glfwTerminate();
      fprintf(stderr, "Failed to create GLFW window");
      exit(1);
   }
   
   cout << "\n--------------------------------------------------------------\n";
   cout << "Texture Mapping with OpenGL and C++\n";
      
   // Init the GLEW library for the OpenGL functions
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return -1;
   }

   onInit();
 
   glfwEnable(GLFW_MOUSE_CURSOR);            // Show the cursor
   glfwSwapInterval(1);                      // 1=Enable vertical sync

   // Set callback functions
   glfwSetWindowSizeCallback (onResize);
   glfwSetMouseButtonCallback(onMouseButton);
   glfwSetMousePosCallback   (onMouseMove);
   glfwSetMouseWheelCallback (onMouseWheel);
   glfwSetKeyCallback        (onKey);
   glfwSetWindowCloseCallback(onClose);
   
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
