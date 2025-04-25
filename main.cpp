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

struct point3d {
    double x, y, z;
};

struct triangle { 
    point3d p1, p2, p3;
};

struct normal {
    double x, y, z;
};

vector<Position> controlPoints;
int bcs[4][4];

Position camera;

int WIDTH_WINDOWS;
int HEIGHT_WINDOWS;

double m_slide=90;

bool bezierSurface = true;

double rot = 0;
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

void setup()
{
    gluLookAt(45, 45, 45, 0, 0, 0, 0, 1, 0);
    glClearColor(0, 0, 0, 1.0); // *should* display black background
 
    {// generate the 4*4 control points.
        int i, j;
        for( i = 0; i <= 60; i += 20){
            for(j = 0; j <= 60; j += 20){
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
    return bcs[n][k] * pow(u, k) * pow((1.0-u), n-k);
}

void plotPoint (double x, double y, double z)
{
    glBegin(GL_POINTS);
        glVertex3f(x, y, z);
    glEnd();
}

void modifyControlPoints(int a, int b, int c, int d) {
    controlPoints[2*4 + 1].y = controlPoints[2*4 + 1].y + a;
    controlPoints[1*4 + 1].y = controlPoints[1*4 + 1].y + b;
    controlPoints[2*4 + 2].y = controlPoints[2*4 + 2].y + c;
    controlPoints[1*4 + 2].y = controlPoints[1*4 + 2].y + d;
}

void genTriangles(const vector<vector<point3d>> &pts, vector<triangle> &tr) {
    for (int i = 0; i < pts.size()-1; i++) {
        for (int j = 0; j < pts[0].size()-1; j++) {
            tr.push_back({pts[i][j], pts[i+1][j+1], pts[i][j+1]});
            tr.push_back({pts[i][j], pts[i+1][j], pts[i][j+1]});
        }
    }
}

void DrawBezierSurface() {
    // compute binomCoeffs
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            bcs[i][j] = binomCoeff(i, j);
        }
    }
    // compute bezier surface points
    vector<vector<point3d>> pt_buffer;
    int indi = 0, indj = 0;
    for (double u = 0; u <= 60; u += .5) { // surface boundaries
        vector<point3d> tmp_buffer;
        for (double v = 0; v <= 60; v += .5) {
            indj++;
            double x = 0, y = 0, z = 0;
            for (int j = 0; j <= 3; j++) { // for all control points
                double jBezierBlend = bezierBlend(v/60.0, j, 3);
                for (int k = 0; k <= 3; k++) {
                    double bezierBlendResult = jBezierBlend * bezierBlend(u/60.0, k, 3);
                    x += controlPoints[k*4 + j].x * bezierBlendResult;
                    y += controlPoints[k*4 + j].y * bezierBlendResult;
                    z += controlPoints[k*4 + j].z * bezierBlendResult;
                }
            }
            // store point
            point3d p = {x, y, z};
            tmp_buffer.push_back(p);
        }
        pt_buffer.push_back(tmp_buffer);
    }

    // find triangles from point grid
    vector<triangle> triangles;
    genTriangles(pt_buffer, triangles);

    // determine triangle normals
    vector<normal> norms;
    for (int i = 0; i < triangles.size(); i++) {
        point3d U = {triangles[i].p2.x-triangles[i].p1.x, triangles[i].p2.y-triangles[i].p1.y, triangles[i].p2.z-triangles[i].p1.z};
        point3d V = {triangles[i].p3.x-triangles[i].p1.x, triangles[i].p3.y-triangles[i].p1.y, triangles[i].p3.z-triangles[i].p1.z};
        double Nx = U.y*V.z - U.z*V.y;
        double Ny = U.z*V.x - U.x*V.z;
        double Nz = U.x*V.y - U.y*V.x;
        norms.push_back({Nx, Ny, Nz});
    }

    // calculate dot product (to get sign)
    vector<bool> vis;
    normal view = {camera.x, camera.y, camera.z};
    for (int i = 0; i < norms.size(); i++) {
        double dot = view.x*norms[i].x + view.y*norms[i].y + view.z*norms[i].z;
        if (dot <= 0) vis.push_back(true);
        else vis.push_back(false);
    }
    
    // plot points in visible triangles
    glPointSize(1);
    // draw all points
    glBegin(GL_POINTS);
    int drawn = 0;
    glColor3f(1.0, 0, 1.0);
    for (int i = 0; i < triangles.size(); i++) {
        if (vis[i]) {
            drawn++;
            glVertex3f(triangles[i].p1.x, triangles[i].p1.y, triangles[i].p1.z);
            glVertex3f(triangles[i].p2.x, triangles[i].p2.y, triangles[i].p2.z);
            glVertex3f(triangles[i].p3.x, triangles[i].p3.y, triangles[i].p3.z);
        }
    }
    glEnd();
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
  
    if(bezierSurface){
        //set gluLookAt and gluPerspective
        projection(WIDTH_WINDOWS, HEIGHT_WINDOWS, 1); // set projection.
       // gluLookAt(100, 100, m_slide, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        camera.x = camera.y = camera.z = m_slide;
        gluLookAt(camera.x, camera.y, camera.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        glRotatef(rot, 0, 1, 0);
        drawAxes();
        /* if(bezierSurfaceMapping || bezierSurfaceLighting){
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

        } */
       
        DrawBezierScene();

        rot=rot+0.3;
        if(rot>360) rot=rot-360;
    }
    glutSwapBuffers(); // display newly drawn image in window


}

void keyHandler(unsigned char key, int x, int y) {
    switch (key) {
        case 'e':
            modifyControlPoints(1, 0, 0, 0);
            break;
        case 'r':
            modifyControlPoints(0, 1, 0, 0);
            break;
        case 'd':
            modifyControlPoints(0, 0, 1, 0);
            break;
        case 'f':
            modifyControlPoints(0, 0, 0, 1);
            break;
        case 'j':
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void idleRedisplay() { // for continuous rotation
    glutPostRedisplay();
}

int main(int argc, char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
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
    glutIdleFunc(idleRedisplay);
    //Creates Menu on Right Click
    // CreateMenu();

     glutMainLoop();
     return 0;

}