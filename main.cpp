#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<string.h>
#include<math.h>
#include<GL/glut.h>   
#include<vector>
#include<assert.h>

using namespace std;

struct Position{
    Position() : x(0), y(0),z(0), u(0), v(0) {}
    Position(float m, float n){
      x=m; y=n; z = 0; u = 0; v = 0; 
    }
    Position(float m, float n, float t){
      x=m; y=n; z = t; u = 0; v = 0; 
    }
    Position(float m, float n,float t, float i, float j){
        x = m; y =n; z = t; u = i; v = j;
    }
    float x;
    float y;
    float z;
    float u;
    float v;
};


vector<Position> cubicPoints;
vector<Position> controlPoints;

Position camera;

int WIDTH_WINDOWS;
int HEIGHT_WINDOWS;

double m_slide=45;

bool cubicSpline = false;
bool bezierSurface = true;


// lighting parameters.
bool flatShading = false;
bool bezierSurfaceMapping = true;
bool bezierSurfaceLighting = true;
// data for the lighting
//
// for x-y-z axis
GLfloat redSurface[]   = {1.0, 0.0, 0.0, 1.0};
GLfloat greenSurface[]   = {0.0, 1.0, 0.0, 1.0};
GLfloat blueSurface[]   = {0.0, 0.0, 1.0, 1.0};
GLfloat darkSurface[]   = {1.0, 0.0, 0.0, 1.0};
//for lighting
GLfloat lightAmbient[] =  {0.1, 0.1, 0.1, 1.0};
GLfloat lightDiffuse[] =  {0.7, 0.7, 0.7, 1.0};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1.0};
GLfloat lightPosition[] = {0, 0, 100.0, 0.0};
GLfloat lightDirection[] ={0.0, 0.0, -1.0};
GLfloat shininess       = 50;
// for the materials
GLfloat matAmbient [] = {0.0, 1.0, 0.0, 1.0};
GLfloat matDiffuse [] = {0.0, 1.0, 0.0, 1.0};
GLfloat matSpecular[] = {1.0, 1.0, 1.0, 1.0};

void projection(int width, int height, int perspectiveORortho){
  float ratio = (float)width/height;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (perspectiveORortho)
      gluPerspective(60, ratio, 1, 1000);
  else 
      glOrtho(-ratio, ratio, -ratio, ratio, 1, 1000);
  glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
}

void DrawStippleLines(Position p1, Position p2){
  glEnable(GL_LINE_STIPPLE);
  {
    glColor3f(255.0,0.0,0.0); //blue dot 
    glLineStipple(1, 0x0FFF); /* dashed */
    glBegin(GL_LINE_STRIP);
       glVertex3f(p1.x, p1.y, p1.z);
       glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
  } 
  glDisable(GL_LINE_STIPPLE);

  glColor3f(255.0, 255.0, 0.0); //blue dot 
  glPointSize(10.0);
  glBegin(GL_POINTS);
       glVertex3f(p1.x, p1.y, p1.z);
       glVertex3f(p2.x, p2.y, p2.z);
  glEnd();
  
  glFlush();
}

void setup()
{
    gluLookAt(45, 45, 45, 0, 0, 0, 0, 1, 0);
    glClearColor(0, 0, 0, 1.0); // *should* display black background

    { 
        // generate some data for the cubicPoints, bezierPoints, bsplinePoints
        cubicPoints.push_back(Position(100, 200, 0));
        cubicPoints.push_back(Position(200, 300, 0));
        cubicPoints.push_back(Position(300, 300, 0));
        cubicPoints.push_back(Position(400, 200, 0));
    }
 
    {// generate the 4*4 control points.
        int i, j;
        for( i = -30; i <= 30; i += 20){
            for(j = -30; j <= 30; j += 20){
                controlPoints.push_back(Position(i, 0, j));
            }
        }
        //controlPoints[10] = Position(0, 50, 40);
    
    }
   
}

void reshape( int w, int h ){
   glViewport( 0, 0, (GLsizei)w, (GLsizei)h ); // set to size of window
   glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    //gluOrtho2D( 0.0, (float)w, 0.0, (float)h );

    glOrtho( 0, w, h, 0, -1, 1 );
    WIDTH_WINDOWS = w;  // records width globally
    HEIGHT_WINDOWS = h; // records height globally

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void DrawCubicSpline(){
    // implement your own cubic spline function here.
    DrawStippleLines(cubicPoints.at(0), cubicPoints.at(1));
    DrawStippleLines(cubicPoints.at(2), cubicPoints.at(3));
}

int fact(int n) {
    int ret = 1;
    for (int i = 2; i <= n; i++) {
        ret *= i;
    }
    return ret;
}

int binomCoeff(int n, int k) {
    return fact(n)/(fact(k)*fact(n-k));
}

double bezierBlend(double u, int k, int n) {
    return binomCoeff(n, k) * pow(u, k) * pow((1.0-u), n-k);
}

void plotPoint (double x, double y, double z)
{
    glBegin(GL_POINTS);
        glColor3d(1.0, 0, 0);
        glVertex3f(x, y, z);
    glEnd();
}

void modifyControlPoints(int a, int b, int c, int d) {
    controlPoints[2*4 + 1].y = controlPoints[2*4 + 1].y + a;
    controlPoints[1*4 + 1].y = controlPoints[1*4 + 1].y + b;
    controlPoints[2*4 + 2].y = controlPoints[2*4 + 2].y + c;
    controlPoints[1*4 + 2].y = controlPoints[1*4 + 2].y + d;
}

void DrawBezierSurface() {
    for (double u = 0; u <= 80; u += 1) { // surface boundaries
        for (double v = 0; v <= 80; v += 1) {
            double x = 0, y = 0, z = 0;
            for (int j = 0; j <= 3; j++) { // for all control points
                for (int k = 0; k <= 3; k++) {
                    x += controlPoints[j*4 + k].x * bezierBlend(v/80.0, j, 4) * bezierBlend(u/80.0, k, 4);
                    y += controlPoints[j*4 + k].y * bezierBlend(v/80.0, j, 4) * bezierBlend(u/80.0, k, 4);
                    z += controlPoints[j*4 + k].z * bezierBlend(v/80.0, j, 4) * bezierBlend(u/80.0, k, 4);
                }
            }
            // plot point
            plotPoint(x, y, z);
        }
    }
}

void DrawBezierScene(){
  // draw your own Bezier Surface here.

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // control points
    glPointSize(5);
    glBegin(GL_POINTS); 
    glColor3f(1.0f, 1.0f, 0);
        for (int i = 0; i < controlPoints.size(); i++) {
            glVertex3d(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z);
        }
    glEnd();
    
    DrawBezierSurface();
}

void drawAxes() {
    glBegin(GL_LINES);
        glColor3f(.5,.5,.5);
        glVertex3d(100, 0, 0);
        glVertex3d(-100, 0, 0);
        glVertex3d(0, 100, 0);
        glVertex3d(0, -100, 0);
        glVertex3d(0, 0, 100);
        glVertex3d(0, 0, -100);
    glEnd();
}

void display(){
   // glClear(GL_COLOR_BUFFER_BIT); // clear window
   glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT );
   glEnable(GL_DEPTH_TEST); 

   glLoadIdentity();

   if(cubicSpline){
        // setting environment 
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0, WIDTH_WINDOWS, HEIGHT_WINDOWS, 0, -1, 1 );
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // draw
        DrawCubicSpline();        
   }
  
    if(bezierSurface){
        //set gluLookAt and gluPerspective
        projection(WIDTH_WINDOWS, HEIGHT_WINDOWS, 1); // set projection.
       // gluLookAt(100, 100, m_slide, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        camera.x = camera.y = camera.z = m_slide;
        gluLookAt(camera.x, camera.y, camera.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        
        drawAxes();
        if(bezierSurfaceMapping || bezierSurfaceLighting){
            // lighting

            glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
            glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
            glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            //flat shading or smooth shading
            if(flatShading) glShadeModel(GL_FLAT);
            else            glShadeModel(GL_SMOOTH);

        }
       
        DrawBezierScene();
    }
    glutSwapBuffers(); // display newly drawn image in window


}

void keyHandler(unsigned char key, int x, int y) {
    switch (key) {
        case 'e':
            modifyControlPoints(10, 0, 0, 0);
            break;
        case 'r':
            modifyControlPoints(0, 10, 0, 0);
            break;
        case 'd':
            modifyControlPoints(0, 0, 10, 0);
            break;
        case 'f':
            modifyControlPoints(0, 0, 0, 10);
            break;
    }
    glutPostRedisplay();
}


int main(int argc, char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(1000,500);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Spline and Surface Demo");
        setup();
        
            // initializing callbacks
     glutReshapeFunc(reshape);
     glutDisplayFunc(display);
     //glutMouseFunc(mouse);  // define your own mouse event.
     //glutMotionFunc(motion);  // define your own motion event, e.g., rotate OBJ model.
    glutKeyboardFunc(keyHandler);
    //Creates Menu on Right Click
    // CreateMenu();

     glutMainLoop();
     return 0;

}