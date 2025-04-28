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

struct point2d {
    double u, v;
};

struct triTex {
    point2d t1, t2, t3;
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

int activeCtrlPoint = 5;

double m_slide = 90;

bool bezierSurface = true;
bool show_backface = true;
bool show_texture = false;

// texture data
#define header 54
int WIDTH_IMG, HEIGHT_IMG;
vector<unsigned char> pixelData;
static GLuint texName;

double rot = 0;
// lighting parameters.
bool flatShading = true;
bool bezierSurfaceLighting = true;
bool wireframe = false;
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
GLfloat lightPosition[] = {30, 100, 30, 1.0};
GLfloat shininess       = 50.0f;
// for the materials
GLfloat matAmbient [] = {0.0, 0.7, 0.0, 1.0};
GLfloat matDiffuse [] = {0, 0.7, 0, 1.0};
GLfloat matSpecular[] = {1.0, 1.0, 1.0, 0.1};

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

void printState() {
    string on = "on";
    string off = "off";
    string bfcState = show_backface ? off : on;
    string lightingState = bezierSurfaceLighting ? on : off;
    string renderMode = wireframe ? "wireframe" : "polygon";
    string shadingMode = flatShading ? "flat" : "smooth";
    string textureMode = show_texture ? "texture" : "color";
    cout << " BFC: " << bfcState << " | Lighting: " << lightingState << " | Render Mode: " << renderMode << " | Shading: " << shadingMode << " | Surface: " << textureMode << "   \r" << std::flush;
}

void readBMP(char *filename) {

    FILE *fd;
    if ((fd = fopen(filename, "rb")) == NULL) {
        printf("Error happens\n");
    }
    unsigned char info[header];
    fread(info, sizeof(unsigned char), header, fd); // read the header-byte header
    // extract the  heght and width of the image from the header info.

    WIDTH_IMG = *(int *)&info[18];
    HEIGHT_IMG = *(int *)&info[22];
    int size = 3 * WIDTH_IMG * HEIGHT_IMG;
    //printf("%d, %d\n", WIDTH_IMG, HEIGHT_IMG);

    unsigned char *pixel = (unsigned char *)malloc(sizeof(unsigned char) * size);
    fread(pixel, sizeof(unsigned char), size, fd); // read the data
    fclose(fd);

    // restore pixel from BGR to RGB.
    //
    for (int i = 0; i < size; i += 3) {
        unsigned char temp = pixel[i];
        pixel[i] = pixel[i + 2];
        pixel[i + 2] = temp;
    }

    for (int i = 0; i < size; i++) {
        pixelData.push_back(pixel[i]);
    }

    //printf("%lu\n", pixelData.size());
    // return data;
}

void setup()
{
    printState();
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

    const char *texFile = "./jacob.bmp";
    readBMP((char*) texFile);
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH_IMG, HEIGHT_IMG, 0, GL_RGB,
        GL_UNSIGNED_BYTE, pixelData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void modifyControlPoints(int id, int x, int y, int z) {
    // 9, 5, 10, 6
    controlPoints[id].x += x;
    controlPoints[id].y += y;
    controlPoints[id].z += z;
}

void genTriangles(const vector<vector<point3d>> &pts, const vector<vector<point2d>> &tex_coords, vector<triangle> &tr, vector<triTex> &tr_tex) {
    for (int i = 0; i < pts.size()-1; i++) {
        for (int j = 0; j < pts[0].size()-1; j++) {
            tr.push_back({pts[i][j], pts[i+1][j+1], pts[i][j+1]});
            triTex tmp = {tex_coords[i][j], tex_coords[i+1][j+1], tex_coords[i][j+1]};
            tr_tex.push_back(tmp); // also store texCoord for later
            tr.push_back({pts[i][j], pts[i+1][j], pts[i+1][j+1]});
            triTex tmp2 = {tex_coords[i][j], tex_coords[i+1][j], tex_coords[i+1][j+1]};
            tr_tex.push_back(tmp2);
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
    vector<vector<point2d>> texCoordBuffer; // for texture coordinates
    int indi = 0, indj = 0;
    for (double u = 0; u <= 60; u += .5) { // surface boundaries
        vector<point3d> tmp_buffer;
        vector<point2d> tex_tmp_buffer;
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
            point2d tex_p = {u/60, v/60}; // get current texture coordinates
            tex_tmp_buffer.push_back(tex_p);
        }
        pt_buffer.push_back(tmp_buffer);
        texCoordBuffer.push_back(tex_tmp_buffer);
    }

    // find triangles from point grid (and tex coordinates)
    vector<triangle> triangles;
    vector<triTex> tri_tex_coords;
    genTriangles(pt_buffer, texCoordBuffer, triangles, tri_tex_coords);

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
    normal view = {camera.x, camera.y, camera.z}; // constant view vector
    for (int i = 0; i < norms.size(); i++) {
        double dot = view.x*norms[i].x + view.y*norms[i].y + view.z*norms[i].z;
        if (dot <= 0) vis.push_back(true);
        else vis.push_back(false);
    }
    
    // plot lines from visible triangles (some repeated)
    glPointSize(1);
    int num_tri = triangles.size();
    if (wireframe) {  // WIREFRAME
        for (int i = 0; i < num_tri; i++) {
            if (vis[i] || show_backface) {
                glColor3f((double) i/num_tri, 1-(double) i/num_tri, .5);
                glBegin(GL_LINES);
                glVertex3f(triangles[i].p2.x, triangles[i].p2.y, triangles[i].p2.z);
                glVertex3f(triangles[i].p3.x, triangles[i].p3.y, triangles[i].p3.z);
                glEnd();
            }
        }
    } else { // POLYGONS (filled triangles)
        if (show_texture) {
            glEnable(GL_TEXTURE_2D);
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            glColor3f(0.0, 0.5, 0);
        }
        
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < num_tri; i++) {
            if (vis[i] || show_backface) {
                GLfloat norm[3] = {-1*(float)norms[i].x, -1*(float)norms[i].y, -1*(float)norms[i].z};
                glNormal3fv(norm);
                glTexCoord2f(tri_tex_coords[i].t2.u, tri_tex_coords[i].t2.v); // always bind tex coords even if not showing
                glVertex3f(triangles[i].p2.x, triangles[i].p2.y, triangles[i].p2.z);
               
                glNormal3fv(norm);
                glTexCoord2f(tri_tex_coords[i].t1.u, tri_tex_coords[i].t1.v);
                glVertex3f(triangles[i].p1.x, triangles[i].p1.y, triangles[i].p1.z);
               
                glNormal3fv(norm);
                glTexCoord2f(tri_tex_coords[i].t3.u, tri_tex_coords[i].t3.v);
                glVertex3f(triangles[i].p3.x, triangles[i].p3.y, triangles[i].p3.z);
            }
        }
        glEnd();
        if (show_texture) glDisable(GL_TEXTURE_2D);
    }   
}

void DrawBezierScene(){
  // draw your own Bezier Surface here.

    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glPointSize(5);
    glBegin(GL_POINTS); // draw active control point
    glColor3f(1.0f, 0, 0);
    glVertex3d(controlPoints[activeCtrlPoint].x, controlPoints[activeCtrlPoint].y, controlPoints[activeCtrlPoint].z);
    glEnd();

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
    glEnable(GL_COLOR);
    glColor3f(0.5, 0.5, 0.5);
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
   projection(WIDTH_WINDOWS, HEIGHT_WINDOWS, 1); // set projection.
    // gluLookAt(100, 100, m_slide, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    camera.x = camera.y = camera.z = m_slide;
    gluLookAt(camera.x, camera.y, camera.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glMatrixMode(GL_MODELVIEW);
   
  
    if(bezierSurface){
        glRotatef(rot, 0, 1, 0);

        glPushMatrix(); // draw light source independent of lighting
            glDisable(GL_LIGHTING);
            glColor3f(1.0, 1.0, 1.0); 
            glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
            glutSolidSphere(2.0, 10, 10);
            glEnable(GL_LIGHTING); 
        glPopMatrix();

        glDisable(GL_LIGHTING); // draw axes independent of lighting
            drawAxes();
        glEnable(GL_LIGHTING);

        if(bezierSurfaceLighting){
            // lighting
            glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
            glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
            //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);

            glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
            
            //flat shading or smooth shading
            if(flatShading) glShadeModel(GL_FLAT);
            else            glShadeModel(GL_SMOOTH);
        } else {
            glDisable(GL_LIGHTING);
        }
        DrawBezierScene();
    }
    glutSwapBuffers(); // display newly drawn image in window
}

void keyHandler(unsigned char key, int x, int y) {
    switch (key) {
        case 'r':
            if (activeCtrlPoint == 5) activeCtrlPoint = 9;
            else if (activeCtrlPoint == 9) activeCtrlPoint = 10;
            else if (activeCtrlPoint == 10) activeCtrlPoint = 6;
            else if (activeCtrlPoint == 6) activeCtrlPoint = 5;
            break;
        case 'd':
            modifyControlPoints(activeCtrlPoint, 0, -10, 0);
            break;
        case 'f':
            modifyControlPoints(activeCtrlPoint, 0, 10, 0);
            break;
        case 'j':
            exit(0);
            break;
        case 'q':
            rot=rot+5;
            if(rot>360) rot=rot-360;
            break;
        case 'w':
            rot=rot-5;
            if(rot<0) rot=360+rot;
            break;
        case 'u':
            if (wireframe) wireframe = false;
            else wireframe = true;
            printState();
            break;
        case 'l':
            if (bezierSurfaceLighting) bezierSurfaceLighting = false;
            else bezierSurfaceLighting = true;
            printState();
            break;
        case 'k':
            if (flatShading) flatShading = false;
            else flatShading = true;
            printState();
            break;
        case 's':
            shininess += 5;
            if (shininess>128) shininess = 128;
            break;
        case 'a':
            shininess -= 5;
            if (shininess<1) shininess = 1;
            break;
        case 'b':
            if (show_backface) show_backface = false;
            else show_backface = true;
            printState();
            break;
        case 'm':
            matDiffuse[1] += .1;
            if (matDiffuse[1]>1.0) matDiffuse[1] = 1.0;
            break;
        case 'n':
            matDiffuse[1] -= .1;
            if (matDiffuse[1]<0) matDiffuse[1] = 0;
            break;
        case 't':
            if (show_texture) show_texture = false;
            else show_texture = true;
            printState();
            break;
        case 'z':
            lightPosition[0] += 5;
            lightPosition[2] += 5;
            break;
        case 'x':
            lightPosition[0] -= 5;
            lightPosition[2] -= 5;
            break;
    }
    glutPostRedisplay();
}

void arrowKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            modifyControlPoints(activeCtrlPoint, 0, 0, -10);
            break;
        case GLUT_KEY_DOWN:
            modifyControlPoints(activeCtrlPoint, 0, 0, 10);
            break;
        case GLUT_KEY_LEFT:
            modifyControlPoints(activeCtrlPoint, -10, 0, 0);
            break;
        case GLUT_KEY_RIGHT:
            modifyControlPoints(activeCtrlPoint, 10, 0, 0);
            break;
    }
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
    glutSpecialFunc(arrowKeys);
    //Creates Menu on Right Click
    // CreateMenu();

     glutMainLoop();
     return 0;

}