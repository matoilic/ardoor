#include "RenderingContext.h"

#include <GL/gl.h>
#include <GL/glu.h>
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

    GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0};  /* Red diffuse light. */
    GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
    GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */

    /* Enable a single OpenGL light. */
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

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

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);


    std::vector<cv::Point2f> corners;
    cv::Size boardSize = cv::Size(9, 6);

    float a = 0.2f;						// The widht/height of each square of the chessboard object
    std::vector<cv::Point3f> _3DPoints;	// Vector that contains the 3D coordinates for each chessboard corner

    // Initialising the 3D-Points for the chessboard
    float rot = 0.0f;
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

    bool found = cv::findChessboardCorners(m_backgroundImage, boardSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
    if (found)
    {
        cv::Mat M = m_calibration->getIntrinsicsMatrix();
        if (M.data == NULL) {
            cv::Size imageSize = m_backgroundImage.size();
            m_calibration->calibrate(imageSize);

            M = m_calibration->getIntrinsicsMatrix();
        }
        cv::Mat D = m_calibration->getDistortionCoeffs();

        // Declaration of rotation and translation Vector
        cv::Mat R(3, 1, CV_64F);
        cv::Mat T(3, 1, CV_64F);

        cv::solvePnP(cv::Mat(_3DPoints), cv::Mat(corners), M, D, R, T);		//Calculate the Rotation and Translation vector

        //std::cout << T.at<double>(0, 0) << "," << T.at<double>(1, 0) << "," << T.at<double>(2, 0) << std::endl;
        double theta = sqrt((R.at<double>(0,0)*R.at<double>(0,0))+
                                              (R.at<double>(1,0)*R.at<double>(1,0))+
                                              (R.at<double>(2,0)*R.at<double>(2,0)));

        glRotatef((theta*180.0f)/3.14159f, R.at<double>(0,0), R.at<double>(1,0), R.at<double>(2,0));

        std::cout << theta << std::endl;
        std::cout.flush();
    }

    drawCameraFrame();

   // drawAugmentedScene();

    glFlush();
}

void RenderingContext::drawCameraFrame()
{
    // Initialize texture for background image
    if (!m_isTextureInitialized)
    {
        glGenTextures(1, &m_backgroundTextureId);
        glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        m_isTextureInitialized = true;
    }

    int w = m_backgroundImage.cols;
    int h = m_backgroundImage.rows;

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

    // Upload new texture data:
    if (m_backgroundImage.channels() == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_backgroundImage.data);
    else if(m_backgroundImage.channels() == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_backgroundImage.data);
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
    drawCoordinateAxis();
    drawCubeModel();
}

void RenderingContext::drawCoordinateAxis()
{
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

}
