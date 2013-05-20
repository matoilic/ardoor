#include "RenderingContext.h"
#include "DebugHelper.h"

#if defined(__APPLE__) || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#endif

#include <iostream>
#include <opencv2/video/video.hpp>

namespace ARDoor {

RenderingContext::RenderingContext(CameraCalibration *c)
{
    m_calibration = c;
    m_isTextureInitialized = false;
}

void RenderingContext::updateBackground(const cv::Mat& frame)
{
    frame.copyTo(m_backgroundImage);
}

void RenderingContext::initialize()
{
    std::cout << "initialize()" << std::endl;
    std::cout.flush();

    glClearColor(0.0, 0.0, 0.0, 0.0);

    GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */

    /* Enable a single OpenGL light. */
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 100.0);

    /* Use depth buffering for hidden surface elimination. */
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RenderingContext::draw()
{
    if (m_backgroundImage.data == NULL) {
        return;
    }

    std::cout << "draw()" << std::endl;

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    detectChessboard();
    drawAugmentedScene();
    drawCameraFrame();

    glFlush();
}

void RenderingContext::drawCameraFrame()
{
    std::cout << "drawCameraFrame()" << std::endl;

    // Initialize texture for background image
    if (!m_isTextureInitialized)
    {
        glGenTextures(1, &m_backgroundTextureId);
        glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_isTextureInitialized = true;
    }

    int w = m_backgroundImage.cols;
    int h = m_backgroundImage.rows;
    glPixelStorei(GL_PACK_ALIGNMENT, 3);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 3);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

    // Upload new texture data:
    if (m_backgroundImage.channels() == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, m_backgroundImage.data);
    else if(m_backgroundImage.channels() == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_backgroundImage.data);
    else if (m_backgroundImage.channels()==1)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_backgroundImage.data);

    const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
    const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
    const GLfloat proj[]              = { 0, -2.f/w, 0, 0, -2.f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

    // Update attribute values.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

    glColor4f(1,1,1,1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

void RenderingContext::drawAugmentedScene()
{
    std::cout << "drawAugmentedScene()" << std::endl;

      // Init augmentation projection
      cv::Mat projectionMatrix;
      int w = m_backgroundImage.cols;
      int h = m_backgroundImage.rows;
      buildProjectionMatrix(m_calibration, w, h, projectionMatrix);

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(reinterpret_cast<const GLfloat*>(&projectionMatrix.data[0]));

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      if (isPatternPresent)
      {
        // Set the pattern transformation
        glLoadMatrixf(reinterpret_cast<const GLfloat*>(&objectPosition.data[0]));

        // Render model
        drawCoordinateAxis();
        drawCubeModel();
      }
}


void RenderingContext::buildProjectionMatrix(CameraCalibration* calibration, int screen_width, int screen_height, cv::Mat& projectionMatrix)
{
    std::cout << "buildProjectionMatrix()" << std::endl;

  float nearPlane = 0.01f;  // Near clipping distance
  float farPlane  = 100.0f;  // Far clipping distance

  // Camera parameters
  cv::Mat intrinsics = calibration->getIntrinsicsMatrix();
  float f_x = intrinsics.at<float>(1, 1); // Focal length in x axis
  float f_y = intrinsics.at<float>(0, 0); // Focal length in y axis (usually the same?)
  float c_x = intrinsics.at<float>(0, 2); // Camera primary point x
  float c_y = intrinsics.at<float>(1, 2); // Camera primary point y

  projectionMatrix = cv::Mat(4, 4, CV_32F);

  projectionMatrix.at<float>(0, 0) = -2.0f * f_x / screen_width;
  projectionMatrix.at<float>(0, 1) = 0.0f;
  projectionMatrix.at<float>(0, 2) = 0.0f;
  projectionMatrix.at<float>(0, 3) = 0.0f;

  projectionMatrix.at<float>(1, 0) = 0.0f;
  projectionMatrix.at<float>(1, 1) = 2.0f * f_y / screen_height;
  projectionMatrix.at<float>(1, 2) = 0.0f;
  projectionMatrix.at<float>(1, 3) = 0.0f;

  projectionMatrix.at<float>(2, 0) = 2.0f * c_x / screen_width - 1.0f;
  projectionMatrix.at<float>(2, 1) = 2.0f * c_y / screen_height - 1.0f;
  projectionMatrix.at<float>(2, 2) = -( farPlane + nearPlane) / ( farPlane - nearPlane );
  projectionMatrix.at<float>(2, 3) = -1.0f;

  projectionMatrix.at<float>(3, 0) = 0.0f;
  projectionMatrix.at<float>(3, 1) = 0.0f;
  projectionMatrix.at<float>(3, 2) = -2.0f * farPlane * nearPlane / ( farPlane - nearPlane );
  projectionMatrix.at<float>(3, 3) = 0.0f;
}

void RenderingContext::drawCoordinateAxis()
{
    std::cout << "drawCoordinateAxis()" << std::endl;

    static float lineX[] = {0,0,0,1,0,0};
    static float lineY[] = {0,0,0,0,1,0};
    static float lineZ[] = {0,0,0,0,0,1};

    glLineWidth(2);

    glBegin(GL_LINES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(lineX);
    glVertex3fv(lineX + 3);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(lineY);
    glVertex3fv(lineY + 3);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3fv(lineZ);
    glVertex3fv(lineZ + 3);

    glEnd();
}

void RenderingContext::drawCubeModel()
{
    std::cout << "drawCubeModel()" << std::endl;

    static const GLfloat LightAmbient[]=  { 0.25f, 0.25f, 0.25f, 1.0f };    // Ambient Light Values
    static const GLfloat LightDiffuse[]=  { 0.1f, 0.1f, 0.1f, 1.0f };    // Diffuse Light Values
    static const GLfloat LightPosition[]= { 0.0f, 0.0f, 2.0f, 1.0f };    // Light Position

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);

    glColor4f(0.2f,0.35f,0.3f,0.75f);         // Full Brightness, 50% Alpha ( NEW )
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);       // Blending Function For Translucency Based On Source Alpha
    glEnable(GL_BLEND);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_COLOR_MATERIAL);

    glScalef(0.25,0.25, 0.25);
    glTranslatef(0,0, 1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    // Front Face
    glNormal3f( 0.0f, 0.0f, 1.0f);    // Normal Pointing Towards Viewer
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 1 (Front)
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 2 (Front)
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Front)
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 4 (Front)
    // Back Face
    glNormal3f( 0.0f, 0.0f,-1.0f);    // Normal Pointing Away From Viewer
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Back)
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 2 (Back)
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 3 (Back)
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 4 (Back)
    // Top Face
    glNormal3f( 0.0f, 1.0f, 0.0f);    // Normal Pointing Up
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 1 (Top)
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 2 (Top)
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Top)
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 4 (Top)
    // Bottom Face
    glNormal3f( 0.0f,-1.0f, 0.0f);    // Normal Pointing Down
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Bottom)
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 2 (Bottom)
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 3 (Bottom)
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 4 (Bottom)
    // Right face
    glNormal3f( 1.0f, 0.0f, 0.0f);    // Normal Pointing Right
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 1 (Right)
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 2 (Right)
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Right)
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 4 (Right)
    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);    // Normal Pointing Left
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Left)
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 2 (Left)
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 3 (Left)
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 4 (Left)
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor4f(0.2f,0.65f,0.3f,0.35f); // Full Brightness, 50% Alpha ( NEW )
    glBegin(GL_QUADS);
    // Front Face
    glNormal3f( 0.0f, 0.0f, 1.0f);    // Normal Pointing Towards Viewer
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 1 (Front)
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 2 (Front)
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Front)
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 4 (Front)
    // Back Face
    glNormal3f( 0.0f, 0.0f,-1.0f);    // Normal Pointing Away From Viewer
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Back)
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 2 (Back)
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 3 (Back)
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 4 (Back)
    // Top Face
    glNormal3f( 0.0f, 1.0f, 0.0f);    // Normal Pointing Up
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 1 (Top)
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 2 (Top)
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Top)
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 4 (Top)
    // Bottom Face
    glNormal3f( 0.0f,-1.0f, 0.0f);    // Normal Pointing Down
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Bottom)
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 2 (Bottom)
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 3 (Bottom)
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 4 (Bottom)
    // Right face
    glNormal3f( 1.0f, 0.0f, 0.0f);    // Normal Pointing Right
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 1 (Right)
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 2 (Right)
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Right)
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 4 (Right)
    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);    // Normal Pointing Left
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Left)
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 2 (Left)
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 3 (Left)
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 4 (Left)
    glEnd();

    glPopAttrib();
}

void RenderingContext::detectChessboard()
{
    std::cout << "detectChessboard()" << std::endl;

    cv::Size boardSize = cv::Size(9, 6);

    std::vector<cv::Point2f> corners;
    float a = 0.2f;						// The widht/height of each square of the chessboard object
    std::vector<cv::Point3f> _3DPoints;	// Vector that contains the 3D coordinates for each chessboard corner

    // Initialising the 3D-Points for the chessboard
    cv::Point3f _3DPoint;
    float y = (((boardSize.height-1.0f)/2.0f)*a)+(a/2.0f);
    float x = 0.0f;
    for (int h = 0; h < boardSize.height; h++, y+=a) {
        x = (((boardSize.height-2.0f)/2.0f)*(-a))-(a/2.0f);
        for (int w = 0; w < boardSize.width; w++, x+=a) {
            _3DPoint.x = x;
            _3DPoint.y = y;
            _3DPoint.z = 0.0f;
            _3DPoints.push_back(_3DPoint);
        }
    }

    isPatternPresent = cv::findChessboardCorners(m_backgroundImage, boardSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

    if (isPatternPresent)
    {
        cv::Mat M = m_calibration->getIntrinsicsMatrix();
        cv::Mat D = m_calibration->getDistortionCoeffs();

        cv::Mat_<float> Rvec;
        cv::Mat_<float> Tvec;
        cv::Mat raux, taux;
        cv::solvePnP(cv::Mat(_3DPoints), cv::Mat(corners), M, D, raux, taux);		//Calculate the Rotation and Translation vector

        raux.convertTo(Rvec, CV_32F);
        taux.convertTo(Tvec, CV_32F);

        cv::Mat_<float> rotMat(3, 3);
        cv::Rodrigues(Rvec, rotMat);

        cv::Mat projectionMatrix(4, 4, CV_32F);
        projectionMatrix.at<float>(0, 0) = rotMat.at<float>(0, 0);
        projectionMatrix.at<float>(0, 1) = rotMat.at<float>(0, 1);
        projectionMatrix.at<float>(0, 2) = rotMat.at<float>(0, 2);
        projectionMatrix.at<float>(0, 3) = Rvec.at<float>(0, 0);

        projectionMatrix.at<float>(1, 0) = rotMat.at<float>(1, 0);
        projectionMatrix.at<float>(1, 1) = rotMat.at<float>(1, 1);
        projectionMatrix.at<float>(1, 2) = rotMat.at<float>(1, 2);
        projectionMatrix.at<float>(1, 3) = Rvec.at<float>(1, 0);

        projectionMatrix.at<float>(2, 0) = rotMat.at<float>(2, 0);
        projectionMatrix.at<float>(2, 1) = rotMat.at<float>(2, 1);
        projectionMatrix.at<float>(2, 2) = rotMat.at<float>(2, 2);
        projectionMatrix.at<float>(2, 3) = Rvec.at<float>(2, 0);

        projectionMatrix.at<float>(3, 0) = 0;
        projectionMatrix.at<float>(3, 1) = 0;
        projectionMatrix.at<float>(3, 2) = 0;
        projectionMatrix.at<float>(3, 3) = 1;

        objectPosition = projectionMatrix.inv();
    }

}

}
