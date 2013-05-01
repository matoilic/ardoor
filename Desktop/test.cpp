#include "CameraCalibration.h"

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

ARDoor::CameraCalibration calibration;
cv::Size boardSize = cv::Size(9, 6);
GLfloat rotAngle;
GLfloat rotX;
GLfloat rotY;
GLfloat rotZ;
int glutwin = -1;

void init(void)
{
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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 100.0);
    glViewport(0, 0, 640, 480);

    /* Use depth buffering for hidden surface elimination. */
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawCubeModel()
{
    static const GLfloat LightAmbient[]=  { 0.25f, 0.25f, 0.25f, 1.0f };    // Ambient Light Values
    static const GLfloat LightDiffuse[]=  { 0.1f, 0.1f, 0.1f, 1.0f };    // Diffuse Light Values
    static const GLfloat LightPosition[]= { 0.0f, 0.0f, 2.0f, 1.0f };    // Light Position

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);

    glColor3f(0.0f, 0.0f, 0.0f);         // Full Brightness, 50% Alpha ( NEW )
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

void display(void)
{

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);



    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidTeapot(1);

    glutSwapBuffers();
}


void initGL(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutwin = glutCreateWindow("test");
    glutDisplayFunc(display);
    init();
}


void test()
{
    std::vector<std::string> fileList;
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000013.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000014.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000015.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000016.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000017.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000018.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000019.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000020.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000021.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000022.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000023.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000024.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000025.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000026.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000027.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000028.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000029.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000030.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000031.jpg");
    fileList.push_back("/home/david/Desktop/chessboard-pictures/WP_000032.jpg");

    calibration.addChessboardPoints(fileList, boardSize);

    std::vector<std::string>::iterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        std::string fileName = *it;
        std::cout << fileName << std::endl;

        cv::Mat img = cv::imread(fileName);

        cv::Size imageSize = img.size();
        calibration.calibrate(imageSize);


        std::vector<cv::Point2f> corners;
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

        bool found = cv::findChessboardCorners(img, boardSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

        cv::drawChessboardCorners(img, boardSize, corners, found);
        //cv::imshow("chessboard", img);
        //cv::waitKey();

        if (found)
        {
            cv::Mat M = calibration.getIntrinsicsMatrix();
            cv::Mat D = calibration.getDistortionCoeffs();

            // Declaration of rotation and translation Vector
            cv::Mat R(3, 1, CV_64F);
            cv::Mat T(3, 1, CV_64F);

            cv::Mat Rvec;
            cv::Mat_<float> Tvec;
            cv::Mat raux, taux;
            cv::solvePnP(cv::Mat(_3DPoints), cv::Mat(corners), M, D, raux, taux);		//Calculate the Rotation and Translation vector

            raux.convertTo(Rvec, CV_32F);
            taux.convertTo(Tvec, CV_32F);

            cv::Mat_<float> rotMat(3,3);
            cv::Rodrigues(Rvec, rotMat);

            for (int x = 0; x < rotMat.rows; x++) {
                for (int y = 0; y < rotMat.rows; y++) {
                    std::cout << rotMat.at<double>(x, y) << " ";
                }
                std::cout << std::endl;
            }

            cv::Mat projectionMatrix(4, 4, CV_32F);
            projectionMatrix(0, 0) = rotMat.at<float>(0, 0);
            projectionMatrix(0, 1) = rotMat.at<float>(0, 1);
            projectionMatrix(0, 2) = rotMat.at<float>(0, 2);
            projectionMatrix(0, 3) = Rvec.at<float>(0, 0);

            projectionMatrix(1, 0) = rotMat.at<float>(1, 0);
            projectionMatrix(1, 1) = rotMat.at<float>(1, 1);
            projectionMatrix(1, 2) = rotMat.at<float>(1, 2);
            projectionMatrix(1, 3) = Rvec.at<float>(1, 0);

            projectionMatrix(2, 0) = rotMat.at<float>(2, 0);
            projectionMatrix(2, 1) = rotMat.at<float>(2, 1);
            projectionMatrix(2, 2) = rotMat.at<float>(2, 2);
            projectionMatrix(2, 3) = Rvec.at<float>(2, 0);

            projectionMatrix(3, 0) = 0;
            projectionMatrix(3, 1) = 0;
            projectionMatrix(3, 2) = 0;
            projectionMatrix(3, 3) = 1;

            for (int x = 0; x < projectionMatrix.rows; x++) {
                for (int y = 0; y < projectionMatrix.rows; y++) {
                    std::cout << projectionMatrix.at<double>(x, y) << " ";
                }
                std::cout << std::endl;
            }

            //std::cout << T.at<double>(0, 0) << "," << T.at<double>(1, 0) << "," << T.at<double>(2, 0) << std::endl;
            double theta = sqrt((R.at<double>(0,0)*R.at<double>(0,0))+
                                                  (R.at<double>(1,0)*R.at<double>(1,0))+
                                                  (R.at<double>(2,0)*R.at<double>(2,0)));

            rotAngle = (theta*180.0f)/M_PI;
            rotX = R.at<double>(0,0);
            rotY = R.at<double>(1,0);
            rotZ = R.at<double>(2,0);

            std::cout << theta*180/M_PI << ", ";
            std::cout << "x:" << R.at<double>(0,0) << " ";
            std::cout << "y:" << R.at<double>(1,0) << " ";
            std::cout << "z:" << R.at<double>(2,0) << " ";
            std::cout << std::endl;
            std::cout.flush();
        }
    }

    initGL(0, NULL);
    glutMainLoop();
}
