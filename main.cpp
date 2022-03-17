#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <cmath>
#include <iostream>

using namespace std;

#define PI 3.14159265

const int width = 800;
const int height = 600;
const float rat = 1.0*width/height;

GLfloat eye[3] = {0,5,70};
GLfloat look[3] = {0,5,40};

float rot = 0;
float fan_rot = 0, door_rot = 0, clock_rot = 0, controller = 10;
GLfloat gate = 3;
bool gate_open = false, gate_close = false, fan_mill = true;
bool ambient_flag = true, diffuse_flag = true, specular_flag = true;

bool day_flag = true, night_flag = false, birds_eye = false, wire = false, door_flag = false, door_flag1 = false;

///for the curves
const int L=8;
const int dgre=3;
int ncpt=L+1;
int clikd=0;
const int nt = 10;				//number of slices along x-direction
const int ntheta = 20;

GLint viewport[4]; //var to hold the viewport info
GLdouble modelview[16]; //var to hold the modelview info
GLdouble projection[16]; //var to hold the projection matrix info
int flag=0;

unsigned int ID;

class BmpLoader
{
public:
    unsigned char* textureData;
    int iWidth, iHeight;

    BmpLoader(const char* filename)
    {
        FILE *file=0;
        file=fopen(filename, "rb");
        if(!file)
            std::cout<<"File not found"<<std::endl;
        fread(&bfh, sizeof(BITMAPFILEHEADER),1,file);
        if(bfh.bfType != 0x4D42)
            std::cout<<"Not a valid bitmap"<<std::endl;
        fread(&bih, sizeof(BITMAPINFOHEADER),1,file);
        if(bih.biSizeImage==0)
            bih.biSizeImage=bih.biHeight*bih.biWidth*3;
        textureData = new unsigned char[bih.biSizeImage];
        fseek(file, bfh.bfOffBits, SEEK_SET);
        fread(textureData, 1, bih.biSizeImage, file);
        unsigned char temp;
        for(int i=0; i<bih.biSizeImage; i+=3)
        {
            temp = textureData[i];
            textureData[i] = textureData[i+2];
            textureData[i+2] = temp;

        }

        iWidth = bih.biWidth;
        iHeight = bih.biHeight;
        fclose(file);
    }
    ~BmpLoader(){
        delete [] textureData;
    }

private:
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
};

static GLfloat v_ac[8][3] =
{
    {0,0.2,0},
    {0,0,1},
    {0,0.6,0},
    {0,1,1},

    {1,0.2,0},
    {1,0,1},
    {1,0.6,0},
    {1,1,1}
};

static GLfloat v_cube[8][3] =
{
    {0,0,0},
    {0,0,1},
    {0,1,0},
    {0,1,1},

    {1,0,0},
    {1,0,1},
    {1,1,0},
    {1,1,1}
};

static GLubyte c_ind[6][4] =
{
    {3,1,5,7},  //front
    {6,4,0,2},  //back
    {2,3,7,6},  //top
    {1,0,4,5},  //bottom
    {7,5,4,6},  //right
    {2,0,1,3}   //left
};

static void getNormal3p(GLfloat x1, GLfloat y1, GLfloat z1,
                        GLfloat x2, GLfloat y2, GLfloat z2,
                        GLfloat x3, GLfloat y3, GLfloat z3)
{
    GLfloat Ux, Uy, Uz, Vx, Vy, Vz, Nx, Ny, Nz;

    Ux = x2-x1;
    Uy = y2-y1;
    Uz = z2-z1;

    Vx = x3-x1;
    Vy = y3-y1;
    Vz = z3-z1;

    Nx = Uy*Vz - Uz*Vy;
    Ny = Uz*Vx - Ux*Vz;
    Nz = Ux*Vy - Uy*Vx;

    glNormal3f(Nx,Ny,Nz);
}

void set_mat_prop(float colR=0, float colG=1, float colB=0, bool em=false, float shine=30, float trans = 1.0)
{
    GLfloat no_mat[] = { 0.0, 0.0, 0.0, trans };
    GLfloat mat_ambient[] = { colR/2, colG/2, colB/2, trans };
    GLfloat mat_diffuse[] = { colR, colG, colB, trans };
    GLfloat mat_specular[] = { 0.5, 0.5, 0.5, trans };
    GLfloat mat_emission[] = {colR, colG, colB, trans };
    GLfloat mat_shininess[] = {shine};

    glMaterialfv( GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv( GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess);

    if(em & night_flag)
        glMaterialfv( GL_FRONT, GL_EMISSION, mat_emission);
    else
        glMaterialfv( GL_FRONT, GL_EMISSION, no_mat);
}

void matColor(float kdr, float kdg, float kdb,  float shiny, int frnt_Back, float ambFactor, float specFactor, bool em=false)
{
    GLfloat no_mat[] = { 0.0, 0.0, 0.0};
    const GLfloat mat_ambient[]    = { kdr*ambFactor, kdg*ambFactor, kdb*ambFactor, 1.0f };
    const GLfloat mat_diffuse[]    = { kdr, kdg, kdb, 1.0f };
    const GLfloat mat_specular[]   = { 1.0f*specFactor, 1.0f*specFactor, 1.0f*specFactor, 1.0f };
    GLfloat mat_emission[] = {kdr, kdg, kdb };
    const GLfloat high_shininess[] = { shiny };
    if(em & night_flag)
        glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
    else
        glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, no_mat);
    if(frnt_Back==0)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    }
    else if(frnt_Back==1)
    {
        glMaterialfv(GL_BACK, GL_AMBIENT,   mat_ambient);
        glMaterialfv(GL_BACK, GL_DIFFUSE,   mat_diffuse);
        glMaterialfv(GL_BACK, GL_SPECULAR,  mat_specular);
        glMaterialfv(GL_BACK, GL_SHININESS, high_shininess);
    }
    else if(frnt_Back==2)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, high_shininess);
    }

}

void cube(float colR=1, float colG=1, float colB=1, float trans=1.0, bool em=false, float shine=60)
{
    set_mat_prop(colR,colG,colB,em,shine,trans);

    glBegin(GL_QUADS);
    for (GLint i = 0; i <6; i++)
    {
        getNormal3p(v_cube[c_ind[i][0]][0], v_cube[c_ind[i][0]][1], v_cube[c_ind[i][0]][2],
                    v_cube[c_ind[i][1]][0], v_cube[c_ind[i][1]][1], v_cube[c_ind[i][1]][2],
                    v_cube[c_ind[i][2]][0], v_cube[c_ind[i][2]][1], v_cube[c_ind[i][2]][2]);

        glTexCoord2f(0,1);
        glVertex3fv(&v_cube[c_ind[i][0]][0]);
        glTexCoord2f(0,0);
        glVertex3fv(&v_cube[c_ind[i][1]][0]);
        glTexCoord2f(1,0);
        glVertex3fv(&v_cube[c_ind[i][2]][0]);
        glTexCoord2f(1,1);
        glVertex3fv(&v_cube[c_ind[i][3]][0]);
    }
    glEnd();
}

void ac(float colR=0.5, float colG=0.5, float colB=0.5, float shine=30)
{
    GLfloat m_ambient[] = {colR/2.0,colG/2.0,colB/2.0,1};
    GLfloat m_diffuse[] = {colR,colG,colB,1};
    GLfloat m_specular[] = {colR,colG,colB,1};
    GLfloat m_shine[] = {shine};

    glMaterialfv(GL_FRONT, GL_AMBIENT, m_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, m_shine);

    glBegin(GL_QUADS);
    for (GLint i = 0; i <6; i++)
    {
        getNormal3p(v_ac[c_ind[i][0]][0], v_ac[c_ind[i][0]][1], v_ac[c_ind[i][0]][2],
                    v_ac[c_ind[i][1]][0], v_ac[c_ind[i][1]][1], v_ac[c_ind[i][1]][2],
                    v_ac[c_ind[i][2]][0], v_ac[c_ind[i][2]][1], v_ac[c_ind[i][2]][2]);

        for (GLint j=0; j<4; j++)
        {
            glVertex3fv(&v_ac[c_ind[i][j]][0]);
        }
    }
    glEnd();
}

///curve control points
GLfloat ctrlpoints[L+1][3] =
{
    { 0.0, -0.25, 0.0}, { 0.5, -0.25, 0.0}, { 1.0, -0.25, 0.0}, { 1.5, -0.25, 0.0},
    { 2.0, -0.25, 0.0}, {2.5, -1.0, 0.0}, {3.0, -2.0, 0.0},
    { 3.5, -2.0, 0.0},{ 4.0, -2.0, 0.0},
};

GLfloat ctrlpoints1[L+1][3] =
{
    { 0.0, -2.0, 0.0}, { 0.5, -1.75, 0.0}, { 1.0, -1.5, 0.0}, { 1.5, -1.25, 0.0},
    { 2.0, -1.0, 0.0}, {2.5, -0.75, 0.0}, {3.0, -0.5, 0.0},
    { 3.5, -0.25, 0.0},{ 4.0, 0.0, 0.0},
};

GLfloat ctrlpoints2[L+1][3] =
{
    { 0.0, -0.5, 0.0}, { 0.5, -0.5, 0.0}, { 1.0, -0.5, 0.0}, { 1.5, -0.5, 0.0},
    { 2.0, -0.5, 0.0}, {2.5, -0.5, 0.0}, {3.0, -0.5, 0.0},
    { 3.5, -0.5, 0.0},{ 4.0, -0.5, 0.0},
};


void glusphereDraw(/*int id,*/ float radius, int a, int b, GLfloat r=1, GLfloat g=1, GLfloat b1=1,bool em = false)
{
    set_mat_prop(r,g,b1,em);
    GLUquadric* sphere;
    sphere = gluNewQuadric();
    glPushMatrix();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, radius, a, b);

    glPopMatrix();

}

//control points
long long nCr(int n, int r)
{
    if(r > n / 2) r = n - r; // because C(n, r) == C(n, n - r)
    long long ans = 1;
    int i;

    for(i = 1; i <= r; i++)
    {
        ans *= n - r + i;
        ans /= i;
    }

    return ans;
}

//polynomial interpretation for N points
void BezierCurve ( double t,  float xy[2], GLint iid)
{
    double y=0;
    double x=0;
    t=t>1.0?1.0:t;
    for(int i=0; i<=L; i++)
    {
        int ncr=nCr(L,i);
        double oneMinusTpow=pow(1-t,double(L-i));
        double tPow=pow(t,double(i));
        double coef=oneMinusTpow*tPow*ncr;
        if(iid==1){
            x+=coef*ctrlpoints[i][0];
            y+=coef*ctrlpoints[i][1];
        }
        if(iid==2){
            x+=coef*ctrlpoints1[i][0];
            y+=coef*ctrlpoints1[i][1];
        }
        if(iid==3){
            x+=coef*ctrlpoints2[i][0];
            y+=coef*ctrlpoints2[i][1];
        }


    }
    xy[0] = float(x);
    xy[1] = float(y);

    //return y;
}

void setNormal(GLfloat x1, GLfloat y1,GLfloat z1, GLfloat x2, GLfloat y2,GLfloat z2, GLfloat x3, GLfloat y3,GLfloat z3)
{
    GLfloat Ux, Uy, Uz, Vx, Vy, Vz, Nx, Ny, Nz;

    Ux = x2-x1;
    Uy = y2-y1;
    Uz = z2-z1;

    Vx = x3-x1;
    Vy = y3-y1;
    Vz = z3-z1;

    Nx = Uy*Vz - Uz*Vy;
    Ny = Uz*Vx - Ux*Vz;
    Nz = Ux*Vy - Uy*Vx;

    glNormal3f(-Nx,-Ny,-Nz);
}

void bottleBezier(GLint iid=1)
{
    if(wire) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    int i, j;
    float x, y, z, r;				//current coordinates
    float x1, y1, z1, r1;			//next coordinates
    float theta;
    const float startx = 0;
    float endx;
    if(iid==1) endx = ctrlpoints[L][0];
    if(iid==2) endx = ctrlpoints1[L][0];
    if(iid==3) endx = ctrlpoints2[L][0];
    //number of angular slices
    const float dx = (endx - startx) / nt;	//x step size
    const float dtheta = 2*PI / ntheta;		//angular step size

    float t=0;
    float dt=1.0/nt;
    float xy[2];
    BezierCurve( t,  xy, iid);
    x = xy[0];
    r = xy[1];
    //rotate about z-axis
    float p1x,p1y,p1z,p2x,p2y,p2z;
    for ( i = 0; i < nt; ++i )  			//step through x
    {
        theta = 0;
        t+=dt;
        BezierCurve( t,  xy, iid);
        x1 = xy[0];
        r1 = xy[1];

        //draw the surface composed of quadrilaterals by sweeping theta
        glBegin( GL_QUAD_STRIP );
        for ( j = 0; j <= ntheta; ++j )
        {
            theta += dtheta;
            double cosa = cos( theta );
            double sina = sin ( theta );
            y = r * cosa;
            y1 = r1 * cosa;	//current and next y
            z = r * sina;
            z1 = r1 * sina;	//current and next z

            //edge from point at x to point at next x
            glVertex3f (x, y, z);

            if(j>0)
            {
                setNormal(p1x,p1y,p1z,p2x,p2y,p2z,x, y, z);
            }
            else
            {
                p1x=x;
                p1y=y;
                p1z=z;
                p2x=x1;
                p2y=y1;
                p2z=z1;

            }
            glVertex3f (x1, y1, z1);

            //forms quad with next pair of points with incremented theta value
        }
        glEnd();
        x = x1;
        r = r1;
    } //for i
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void light(GLfloat l, GLfloat w)
{
    GLfloat no_light[] = {0, 0, 0, 1.0};
    ///ambients
    GLfloat l_ambient[] = {0.5, 0.5, 0.5, 1.0};
    GLfloat l_ambient_spot[] = {0.5, 0.5, 0.5, 1.0};
    ///diffuse and specular
    GLfloat l_diffuse[] = {1,1,1,1};
    GLfloat l_specular[] = {1,1,1,1};
    ///positions
    GLfloat l_position[] = {0, 60, 0, 0};
    GLfloat l_position_spot_1[] = {3.7,10,(w/2)+25, 1};
    GLfloat l_position_spot_2[] = {-3.7,10,(w/2)+45, 1};
    GLfloat l_position_spot_3[] = {(l/2), 8, -(w/2), 1};
    GLfloat l_position_spot_4[] = {-(l/2), 8, (w/2), 1};
    ///direction
    GLfloat spot_direction1[] = {0,-1,0,1};
    GLfloat spot_direction2[] = {0,-1,0,1};
    GLfloat spot_direction3[] = {-1,-1,0.2,1};
    GLfloat spot_direction4[] = {1,-1,-0.2,1};

    ///daylight
    glEnable(GL_LIGHT0);
    ///spotlights
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);

    ///daylight properties
    if(ambient_flag) glLightfv(GL_LIGHT0, GL_AMBIENT, l_ambient);
    else glLightfv(GL_LIGHT0, GL_AMBIENT, no_light);

    if(diffuse_flag) glLightfv(GL_LIGHT0, GL_DIFFUSE, l_diffuse);
    else glLightfv(GL_LIGHT0, GL_DIFFUSE, no_light);

    if(specular_flag) glLightfv(GL_LIGHT0, GL_SPECULAR, l_specular);
    else glLightfv(GL_LIGHT0, GL_SPECULAR, no_light);

    glLightfv(GL_LIGHT0, GL_POSITION, l_position);

    ///spotlight
    GLfloat cut_off[] = {40};
    ///1
    glLightfv(GL_LIGHT1, GL_AMBIENT, l_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, l_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, l_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, l_position_spot_1);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction1);
    glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, cut_off);
    ///2
    glLightfv(GL_LIGHT2, GL_AMBIENT, l_ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, l_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, l_specular);
    glLightfv(GL_LIGHT2, GL_POSITION, l_position_spot_2);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction2);
    glLightfv(GL_LIGHT2, GL_SPOT_CUTOFF, cut_off);
    ///3
    glLightfv(GL_LIGHT3, GL_AMBIENT, l_ambient);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, l_diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, l_specular);
    glLightfv(GL_LIGHT3, GL_POSITION, l_position_spot_3);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spot_direction3);
    glLightfv(GL_LIGHT3, GL_SPOT_CUTOFF, cut_off);
    ///4
    glLightfv(GL_LIGHT4, GL_AMBIENT, l_ambient);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, l_diffuse);
    glLightfv(GL_LIGHT4, GL_SPECULAR, l_specular);
    glLightfv(GL_LIGHT4, GL_POSITION, l_position_spot_4);
    glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction4);
    glLightfv(GL_LIGHT4, GL_SPOT_CUTOFF, cut_off);

    if(day_flag){
        glEnable(GL_LIGHT0);
    }
    else {
        glDisable(GL_LIGHT0);
    }
    if(night_flag) {glEnable(GL_LIGHT1);glEnable(GL_LIGHT2);glEnable(GL_LIGHT3);glEnable(GL_LIGHT4);}
    else {glDisable(GL_LIGHT1);glDisable(GL_LIGHT2);glDisable(GL_LIGHT3);glDisable(GL_LIGHT4);}

}

void LoadTexture(const char*filename)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    BmpLoader bl(filename);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, bl.iWidth, bl.iHeight, GL_RGB, GL_UNSIGNED_BYTE, bl.textureData );
}

void axes()
{
    float length = 15;
    float width = 0.3;

    // X-axis
    glPushMatrix();
    glTranslatef(length/2,0,0);
    glScalef(length,width,width);
    glTranslatef(-0.5,-0.5,-0.5);
    cube(0.8,0.1,0.1);
    glPopMatrix();

    // Y-axis
    glPushMatrix();
    glTranslatef(0,length/2,0);
    glScalef(width,length,width);
    glTranslatef(-0.5,-0.5,-0.5);
    cube(0.1,0.8,0.1);
    glPopMatrix();

    // Z-axis
    glPushMatrix();
    glTranslatef(0,0,length/2);
    glScalef(width,width,length);
    glTranslatef(-0.5,-0.5,-0.5);
    cube(0.1,0.1,0.8);
    glPopMatrix();
}

void base_with_field(GLfloat l,GLfloat w)
{
    glPushMatrix();
    ///field
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,1);
    glPushMatrix();
    glTranslatef(-(l/2),0,-(w/2));
    glScalef(l,0.1,w);
    cube();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///the track
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glPushMatrix();
    glTranslatef(-(l/2)-2,-0.1,-(w/2)-2);
    glScalef(l+4,0.1,w+4);
    cube();
    glPopMatrix();
    ///base
    glPushMatrix();
    glTranslatef(-l*2,-0.2,-2*w);
    glScalef(4*l,0.1,7*w);
    cube(0.0, 0.6, 0.0);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///track boundary
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,10);
    ///back
    glPushMatrix();
    glTranslatef(-(l/2)-2,0,-(w/2)-2);
    glScalef(l+4,1.5,0.125);
    cube(0.5,0.5,0.5);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-(l/2)-2,0,-(w/2)-2);
    glScalef(0.125,1.5,w+4);
    cube(0.5,0.5,0.5);
    glPopMatrix();
    glPushMatrix();
    glTranslatef((l/2)+2,0,-(w/2)-2);
    glScalef(0.125,1.5,w+4);
    cube(0.5,0.5,0.5);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void boundary(GLfloat l,GLfloat w)
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);

    ///left wall
    glBindTexture(GL_TEXTURE_2D,3);
    glPushMatrix();
    glTranslatef(-(l/2)-0.125,0,-(w/2));
    glScalef(0.125,1,w);
    cube(1,1,1);
    glPopMatrix();

    ///right wall
    glBindTexture(GL_TEXTURE_2D,4);
    glPushMatrix();
    glTranslatef((l/2)+0.125,0,-(w/2));
    glScalef(0.125,1,w);
    cube(1,1,1);
    glPopMatrix();

    ///back wall
    glBindTexture(GL_TEXTURE_2D,4);
    glPushMatrix();
    glTranslatef(-(l/2),0,-(w/2));
    glScalef((l/2),1,0.125);
    cube(1,1,1);
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D,3);
    glPushMatrix();
    glTranslatef(0,0,-(w/2));
    glScalef((l/2),1,0.125);
    cube(1,1,1);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void post(){
    ///bars
    glPushMatrix();
    glTranslatef(0,1.5,0);
    glScalef(0.1,0.1,3);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glScalef(0.1,1.5,0.1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0,2.9);
    glScalef(0.1,1.5,0.1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    ///net
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,10);
    glPushMatrix();
    glTranslatef(0,1.5,0);
    glScalef(1,0.1,3);
    cube(1,1,1,0.3);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0,2.9);
    glScalef(1,1.5,0.1);
    cube(1,1,1,0.3);
    glPopMatrix();
    glPushMatrix();
    glScalef(1,1.5,0.1);
    cube(1,1,1,0.3);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.9,0,0);
    glScalef(0.1,1.5,3);
    cube(1,1,1,0.3);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void goalPosts(GLfloat l){
    glPushMatrix();
    glTranslatef((l/2)-1.3,0,-1.5);
    post();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-(l/2)+1.3,0,1.5);
    glRotatef(180,0,1,0);
    post();
    glPopMatrix();
}

void flood_light(){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,2);
    glPushMatrix();
    glTranslatef(0,7,0);
    glScalef(4,2,0.25);
    cube(1,1,1,1,true);
    glPopMatrix();
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D,11);
    glTranslatef(1.75,0,0);
    glScalef(0.25,7,0.25);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void flood_lights(GLfloat l, GLfloat w){
    ///back-right
    glPushMatrix();
    glTranslatef((l/2)+1,0,-(w/2)-1.5);
    glRotatef(-45,0,1,0);
    flood_light();
    glPopMatrix();
    ///back left
    glPushMatrix();
    glTranslatef(-(l/2)-4,0,-(w/2)+1);
    glRotatef(45,0,1,0);
    flood_light();
    glPopMatrix();
    ///front right
    glPushMatrix();
    glTranslatef((l/2)+2,0,(w/2)+2);
    glRotatef(45,0,1,0);
    flood_light();
    glPopMatrix();
    ///front left
    glPushMatrix();
    glTranslatef(-(l/2)-3.5,0,(w/2)-1);
    glRotatef(-45,0,1,0);
    flood_light();
    glPopMatrix();
}

void score_board(){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,5);
    glPushMatrix();
    glScalef(0.1,4,6);
    cube(1,1,1,1,true);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void viewer(GLfloat l){
    glPushMatrix();
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0.5,-0.5);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,1,-1);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,1.5,-1.5);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,2,-2);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,2.5,-2.5);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,3,-3);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,3.5,-3.5);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,4,-4);
    glScalef(l,1,1);
    cube(0.37, 0.44, 0.44);
    glPopMatrix();
}

void upper_gallery(GLfloat l, GLfloat w){
    ///front
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,9);
    glPushMatrix();
    glTranslatef((l/2)+3,0,(w/2)+5);
    glRotatef(180,0,1,0);
    viewer(l+6);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///floor front
    glPushMatrix();
    glTranslatef(-(l/2)-4,-0.4,(w/2)+2);
    glScalef(l+8,0.5,7);
    cube(0.5,0.5,0.5);
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,10);
    glPushMatrix();
    glTranslatef(-(l/2)-4,0,(w/2)+2);
    glScalef(0.5,1,7);
    cube(0.5,0.5,0.5);
    glPopMatrix();
    glPushMatrix();
    glTranslatef((l/2)+3.5,0,(w/2)+2);
    glScalef(0.5,1,3);
    cube(0.5,0.5,0.5);
    glPopMatrix();
    ///front boundary
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D,3);
    glTranslatef(-(l/2)-4,0,(w/2)+2);
    glScalef((l/2)+4,1,0.5);
    cube(0.5,0.5,0.5,1,true,120);
    glPopMatrix();
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D,4);
    glTranslatef(0,0,(w/2)+2);
    glScalef((l/2)+4,1,0.5);
    cube(0.5,0.5,0.5,1,true,120);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void viewers_bench(GLfloat l, GLfloat w){
    ///back
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,9);
    glPushMatrix();
    glTranslatef(-(l/2)-3,0,-(w/2)-5);
    viewer(l+6);
    glPopMatrix();
    ///left
    glBindTexture(GL_TEXTURE_2D,9);
    glPushMatrix();
    glTranslatef(-3,0,0);
    glPushMatrix();
    glTranslatef(-(l/2)-2,0,(w/2)+3);
    glRotatef(90,0,1,0);
    viewer(w+6);
    glPopMatrix();
    ///corner back-left
    glBindTexture(GL_TEXTURE_2D,8);
    glPushMatrix();
    glTranslatef(-(l/2)-3.5,0,-(w/2));
    glRotatef(45,0,1,0);
    viewer(w/2);
    glPopMatrix();
    glPopMatrix();
    ///right
    glBindTexture(GL_TEXTURE_2D,9);
    glPushMatrix();
    glTranslatef(1.6,0,0);
    glPushMatrix();
    glTranslatef((l/2)+4,0,-(w/2)-3);
    glRotatef(-90,0,1,0);
    viewer(w+6);
    glPopMatrix();
    ///corner back-right
    glBindTexture(GL_TEXTURE_2D,8);
    glPushMatrix();
    glTranslatef((l/2)-2,0,-(w/2)-6);
    glRotatef(-45,0,1,0);
    viewer(w/2);
    glPopMatrix();
    glPopMatrix();
    ///corner left-front
    glBindTexture(GL_TEXTURE_2D,8);
    glPushMatrix();
    glTranslatef(-(l/2)-2,0,(w/2)+6);
    glRotatef(135,0,1,0);
    viewer(w/2);
    glPopMatrix();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0,5,0);
    upper_gallery(l,w);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,5,0);
    glRotatef(180,0,1,0);
    upper_gallery(l,w);
    glPopMatrix();

    ///corner right-front
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,8);
    glPushMatrix();
    glTranslatef((l/2)+6,0,(w/2));
    glRotatef(-135,0,1,0);
    viewer(w/2);
    glPopMatrix();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void bench(){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,11);
    ///roof
    glPushMatrix();
    glTranslatef(0,1.5,0);
    glScalef(5,0.1,1);
    cube();
    glPopMatrix();
    ///back
    glPushMatrix();
    glScalef(5,1.5,0.1);
    cube(0.37, 0.44, 0.44,0.5);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///base chair
    glPushMatrix();
    glTranslatef(0,0.4,0);
    glScalef(5,0.15,1);
    cube(0.14, 0.41, 0.23);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0.4,0);
    glScalef(5,.5,.15);
    cube(0.14, 0.41, 0.23);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0,0.9);
    glScalef(0.1,0.4,0.1);
    cube();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(4.9,0,0.9);
    glScalef(0.1,0.4,0.1);
    cube();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.4,0,0.9);
    glScalef(0.1,0.4,0.1);
    cube();
    glPopMatrix();
}

void players_bench(GLfloat l, GLfloat w){
    glPushMatrix();
    glTranslatef(1+4,0,(w/2)+1.5);
    glRotatef(180,0,1,0);
    bench();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1-4,0,(w/2)+1.5);
    glRotatef(180,0,1,0);
    bench();
    glPopMatrix();
}

void outer_wall(GLfloat l, int id = 7){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
    glScalef(l,10,0.8);
    cube(1,1,1,1,false);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,10,0);
    glScalef(l,0.5,10);
    cube(1,1,1,1,false);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void outer_walls(GLfloat l, GLfloat w){
    ///back right
    glPushMatrix();
    glTranslatef(-(l/2)-11,-0.05,(-w/2)-3);
    glRotatef(45,0,1,0);
    outer_wall(w-4);
    glPopMatrix();
    ///back
    glPushMatrix();
    glTranslatef(-(l/2)-3,0,(-w/2)-11.5);
    outer_wall(l+6);
    glPopMatrix();
    ///back left
    glPushMatrix();
    glTranslatef((l/2)+3,-0.05,(-w/2)-11.5);
    glRotatef(-45,0,1,0);
    outer_wall(w-4);
    glPopMatrix();
    ///left
    glPushMatrix();
    glTranslatef(-(l/2)-11,0,(w/2)+3);
    glRotatef(90,0,1,0);
    outer_wall(w+6);
    glPopMatrix();
    ///score
    glPushMatrix();
    glTranslatef(-(l/2)-1,6,-3);
    score_board();
    glPopMatrix();
    ///right
    glPushMatrix();
    glTranslatef((l/2)+11.4,0,-(w/2)-3);
    glRotatef(-90,0,1,0);
    outer_wall(w+6);
    glPopMatrix();
    ///score
    glPushMatrix();
    glTranslatef((l/2)+2,6,-3);
    score_board();
    glPopMatrix();
    ///front left
    glPushMatrix();
    glTranslatef(-(l/2)-2.4,-0.05,(w/2)+11.5);
    glRotatef(135,0,1,0);
    outer_wall(w-4);
    glPopMatrix();
    ///front right
    glPushMatrix();
    glTranslatef((l/2)+11.4,-0.05,(w/2)+3);
    glRotatef(-135,0,1,0);
    outer_wall(w-4);
    glPopMatrix();
    ///front
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,7);
    glPushMatrix();
    glTranslatef((l/2)+3,10,(w/2)+11.5);
    glRotatef(180,0,1,0);
    //outer_wall(l+6);
    glScalef(l+6,0.8,10);
    cube();
    glPopMatrix();
    glPushMatrix();
    glTranslatef((l/2)+3,0,(w/2)+11.5);
    glRotatef(180,0,1,0);
    glScalef((l/2),10,0.8);
    cube();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-2,0,(w/2)+11.5);
    glRotatef(180,0,1,0);
    glScalef((l/2),10,0.8);
    cube();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(3,6.75,(w/2)+11.5);
    glRotatef(180,0,1,0);
    glScalef(5,3.25,0.8);
    cube(0.2,0.2,0.8);
    glPopMatrix();
}

void cone(GLfloat r, GLfloat g, GLfloat b, bool em = false){
    glPushMatrix();
    glRotatef( 90, 0.0, 0.0, 1.0);
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); //get the modelview info
    matColor(r,g,b,120,2,0.45,1, em);
    bottleBezier(2);
    glPopMatrix();
}

void cylinder(GLfloat r, GLfloat g, GLfloat b, bool em = false){
    glPushMatrix();
    glRotatef( 90, 0.0, 0.0, 1.0);
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); //get the modelview info
    matColor(r,g,b,120,2,0.45,1, em);
    bottleBezier(3);
    glPopMatrix();
}

void trophy(GLfloat r, GLfloat g, GLfloat b, bool em = false){
    glPushMatrix();
    glTranslatef(0,0.75,0);
    glRotatef( 90, 0.0, 0.0, 1.0);
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); //get the modelview info
    matColor(r,g,b,120,2,0.45,1, em);
    bottleBezier();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.75,0,-0.75);
    glScalef(1.5,0.75,1.5);
    cube(0.3,0.18,0.09);
    glPopMatrix();
}

void name(){
        glLineWidth(20);

        glPushMatrix();
        set_mat_prop(1,0,0,1,true);
        glScalef(0.01,0.01,0.01);
        string msg;
        msg = "WELCOME";
        for(int i=0; i<msg.size(); i++)
         // glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,msg[i]);
            glutStrokeCharacter(GLUT_STROKE_ROMAN,msg[i]);
        glPopMatrix();
        glLineWidth(1.0);

}

void entrance(GLfloat l, GLfloat w){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,11);
    glPushMatrix();
    glTranslatef(-(l/4), 6, (w/2)+11);
    glScalef((l/2), 0.8, 5);
    cube();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-(l/4), 0, (w/2)+15);
    glScalef(0.5, 6, 0.5);
    cube();
    glPopMatrix();
    glPushMatrix();
    glTranslatef((l/4)-0.5, 0, (w/2)+15);
    glScalef(0.5, 6, 0.5);
    cube();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///decorations
    glPushMatrix();
    glTranslatef(-(l/4)+1, 6.8, (w/2)+15.5);
    glScalef(0.5,0.5,0.5);
    trophy(0.183, 0.455, 0.433, true);
    glPopMatrix();
    glPushMatrix();
    glTranslatef((l/4)-1, 6.8, (w/2)+15.5);
    glScalef(0.5,0.5,0.5);
    trophy(0.183, 0.455, 0.433, true);
    glPopMatrix();
    ///sign
    glPushMatrix();
    glTranslatef((l/4)-8.75, 6.9, (w/2)+16);
    name();
    glPopMatrix();

    ///corridore
    glPushMatrix();
    glTranslatef(-3,0,(w/2)+2.1);
    glScalef(6,0.5,9.35);
    cube(0.15,0.15,0.15);
    glPopMatrix();

    ///gate
    glPushMatrix();
    glTranslatef(gate,0,(w/2)+11.4);
    glRotatef(180,0,1,0);
    glScalef(5,6.75,0.8);
    cube(0.7,0.7,0.7);
    glPopMatrix();

}

void trophy_stand(GLfloat w){
    glPushMatrix();
    glTranslatef(-1.5,0,(w/2)-0.75);
    ///champion
    glPushMatrix();
    ///base
    glScalef(0.75,1,0.75);
    cube(0.14,0.07,0.01);
    glPopMatrix();
    ///trophy
    glPushMatrix();
    glTranslatef(0.375,1,0.375);
    glScalef(0.25,0.25,0.25);
    trophy(0.68,0.58,0);
    glPopMatrix();
    ///runners-up
    glPushMatrix();
    glTranslatef(-0.75,0,0);
    ///base
    glPushMatrix();
    glScalef(0.75,0.75,0.75);
    cube(0.14,0.07,0.01);
    glPopMatrix();
    ///trophy
    glPushMatrix();
    glTranslatef(0.375,0.75,0.375);
    glScalef(0.2,0.2,0.2);
    trophy(0.83,0.83,0.83);
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void room(GLfloat l, GLfloat w){
    ///floor
    glPushMatrix();
    glTranslatef(0,-0.5,0);
    glScalef((l/4)+3,0.5,9);
    cube(0.3,0.3,0.3);
    glPopMatrix();
    ///ceiling
    glPushMatrix();
    glTranslatef(0,5,0);
    glScalef((l/4)+3,0.5,9);
    cube(0.3,0.3,0.3);
    glPopMatrix();
    ///left wall
    glPushMatrix();
    glTranslatef((l/4)+2.5,0,0);
    glScalef(0.5,5,9);
    cube(0.25,0.44,0.64);
    glPopMatrix();
    ///right wall with a door
    glPushMatrix();
    glScalef(0.5,3.5,3.5);
    cube(0.25,0.44,0.64);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0,5.5);
    glScalef(0.5,3.5,3.5);
    cube(0.25,0.44,0.64);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,3.5,0);
    glScalef(0.5,1.5,9);
    cube(0.25,0.44,0.64);
    glPopMatrix();
    ///door
    glPushMatrix();
    glTranslatef(0,0,3.5);
    glRotatef(door_rot,0,1,0);
    glScalef(0.5,3.5,2);
    cube(0.3,0.18,0.09);
    glPopMatrix();
}

void sofa(){
    glPushMatrix();
    glScalef(1,0.75,1);
    cube(0.3, 0.52, 0.15);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.2,0,0);
    glScalef(0.2,1,1);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1,0,0);
    glScalef(0.2,1,1);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0,1);
    glScalef(1,1.25,0.2);
    cube(0.3, 0.52, 0.15);
    glPopMatrix();
}

void fan(){
    glPushMatrix();
    glTranslatef(-0.2,1.8,-0.2);
    glScalef(0.4,0.2,0.4);
    cube(0.2,0.2,0.3);
    glPopMatrix();
    glPushMatrix();
    glScalef(0.05,2,0.05);
    cube(0.2,0.2,0.3);
    glPopMatrix();
    glPushMatrix();
    glRotatef(fan_rot,0,1,0);
    ///base
    glPushMatrix();
    glTranslatef(-0.25,0,-0.25);
    glScalef(0.5,0.2,0.5);
    cube(0.02,0.32,0.31);
    glPopMatrix();
    ///blade 1
    glPushMatrix();
    glTranslatef(-1.5,0,-0.25);
    glScalef(3,0.1,0.5);
    cube(0.26, 0.7, 0.6);
    glPopMatrix();
    ///blade 2
    glPushMatrix();
    glTranslatef(-0.25,0,-1.5);
    glScalef(0.5,0.1,3);
    cube(0.26, 0.7, 0.6);
    glPopMatrix();
    glPopMatrix();
}

void clock(){
    glPushMatrix();
    glTranslatef(-0.25,0,0);
    glPushMatrix();
    glTranslatef(-1,-1.5,0);
    glScalef(2.5,3,0.5);
    cube(1,1,1,1,true);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1.25,-1.75,0.2);
    glScalef(3,3.5,0.5);
    cube(0.2,0.2,0.2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,-0.25,-0.3);
    glScalef(0.5,0.5,0.2);
    cube(0.183, 0.455, 0.433, 1, true);
    glPopMatrix();
    glPopMatrix();
    ///arms
    glPushMatrix();
    glRotatef(clock_rot,0,0,1);
    glTranslatef(0,0,-0.2);
    glScalef(1,0.2,0.2);
    cube(0.2,0.2,0.2);
    glPopMatrix();
    glPushMatrix();
    glRotatef(clock_rot*20,0,0,1);
    glTranslatef(0,0,-0.2);
    glScalef(1.25,0.2,0.2);
    cube(0.2,0.2,0.2);
    glPopMatrix();

}

void vip(GLfloat l, GLfloat w){
    glPushMatrix();
    glTranslatef(3,0,(w/2)+2.1);
    room(l,w);
    ///sofa set
    glPushMatrix();
    glTranslatef(5,0,3);
    sofa();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(3,0,3);
    sofa();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(6,0,2);
    glRotatef(90,0,1,0);
    sofa();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.7,0,1);
    glRotatef(-90,0,1,0);
    sofa();
    glPopMatrix();
    ///table
    ///base
    glPushMatrix();
    glTranslatef(3,0,1);
    glPushMatrix();
    glTranslatef(0,0.65,0);
    glScalef(2.5,0.1,1.5);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    ///leg
    glPushMatrix();
    glTranslatef(1.05,0,0.55);
    glScalef(0.2,0.65,0.2);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.65,0,0.55);
    glScalef(1,0.2,0.2);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.05,0,0.1);
    glScalef(0.2,0.2,1);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPopMatrix();

    ///scoreboard on the wall
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 5);
    glPushMatrix();
    glTranslatef((l/4)+2.2,2,2);
    glScalef(0.3,2,3);
    cube(1,1,1,1,true);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef((l/4)+2.3,1.9,1.9);
    glScalef(0.3,2.2,3.2);
    cube(0,0,0);
    glPopMatrix();

    ///wall front
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 4);
    glPushMatrix();
    glTranslatef(0.5,0,0);
    glScalef((l/4)+2, 0.75,0.1);
    cube(1,1,1,1,true);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///wall back
    glPushMatrix();
    glTranslatef(0.5,0,7);
    glScalef((l/4)+2, 5,0.1);
    cube(0.4,0.4,0.4);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2,3,6);
    glScalef(5,1,1);
    ac(0.5,0.5,0.5);
    glPopMatrix();

    ///fan
    glPushMatrix();
    glTranslatef(5,3,4);
    fan();
    glPopMatrix();

    ///glass front
    glPushMatrix();
    glTranslatef(0.5,0.75,0);
    glScalef((l/4)+2, 4.25,0.1);
    cube(0,0,0.5,0.3);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();
}

void commentry(GLfloat l, GLfloat w){
    glPushMatrix();
    glTranslatef(-3,0,w+3.1);
    glRotatef(180, 0,1,0);
    room(l,w);

    ///set of table and chair
    glPushMatrix();
    glTranslatef(0,0,7);
    ///table
    glPushMatrix();
    glTranslatef(1,1.5,0);
    glScalef(6,0.1,1.5);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1,0,0);
    glScalef(0.1,1.5,1.5);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(6.9,0,0);
    glScalef(0.1,1.5,1.5);
    cube(0.3,0.18,0.09);
    glPopMatrix();
    ///screen
    glPushMatrix();
    glTranslatef(2.5,2,0.5);
    glScalef(1.5,1,0.2);
    cube(1,1,1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.4,1.9,0.6);
    glScalef(1.7,1.2,0.2);
    cube(0.2,0.2,0.2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(3,1.4,0.6);
    glScalef(0.5,0.5,0.2);
    cube(0.2,0.2,0.2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2.75,1.5,0.6);
    glScalef(1,0.2,0.2);
    cube(0.2,0.2,0.2);
    glPopMatrix();
    ///sofas
    glPushMatrix();
    glTranslatef(2.5,0,1);
    glRotatef(180,0,1,0);
    sofa();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(4.25,0,1);
    glRotatef(180,0,1,0);
    sofa();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(6,0,1);
    glRotatef(180,0,1,0);
    sofa();
    glPopMatrix();
    glPopMatrix();

    ///wall front
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 3);
    glPushMatrix();
    glTranslatef(0.5,0,8.9);
    glScalef((l/4)+2, 0.75,0.1);
    cube(1,1,1,1,true);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    ///wall back
    glPushMatrix();
    glTranslatef(0.5,0,2);
    glScalef((l/4)+2, 5,0.1);
    cube(0.4,0.4,0.4);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(5,3,2.5);
    glScalef(0.5,0.5,1);
    glRotatef(180,0,1,0);
    clock();
    glPopMatrix();

    ///scoreboard on the wall
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 5);
    glPushMatrix();
    glTranslatef((l/4)+2.2,2,4.5);
    glScalef(0.3,2,3);
    cube(1,1,1,1,true);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef((l/4)+2.3,1.9,4.4);
    glScalef(0.3,2.2,3.2);
    cube(0,0,0);
    glPopMatrix();

    ///fan
    glPushMatrix();
    glTranslatef(6,3.5,4);
    fan();
    glPopMatrix();

    ///glass front
    glPushMatrix();
    glTranslatef(0.5,0.75,8.9);
    glScalef((l/4)+2, 4.25,0.1);
    cube(0,0,0.5,0.3);
    glPopMatrix();



    glPopMatrix();
}

void lamp_post(){
    ///post
    glPushMatrix();
    glScalef(0.2,1.5,0.2);
    cylinder(0.83,0.83,0.83);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,4.5,0);
    glRotatef(-45,0,0,1);
    glScalef(0.2,0.5,0.2);
    cylinder(0.83,0.83,0.83);
    glPopMatrix();
    ///light
    glPushMatrix();
    glTranslatef(1.25,5.75,-0.25);

    glPushMatrix();
    glScalef(2,0.2,0.5);
    cube(0.7,0.7,0.7);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.25,-0.1,0.05);
    glScalef(1.5,0.2,0.3);
    cube(1,1,1,1,true);
    glPopMatrix();

    glPopMatrix();
}

void road(GLfloat l, GLfloat w){
    glPushMatrix();
    glPushMatrix();
    glTranslatef(-7,-0.4,(w/2)+10);
    glScalef(14,0.5,50);
    cube(0.05,0.05,0.05);
    glPopMatrix();

    ///lamp posts
    glPushMatrix();
    glTranslatef(-3.7,0,(w/2)+25);
    lamp_post();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.7,0,(w/2)+45);
    lamp_post();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.7,0,(w/2)+35);
    glRotatef(180,0,1,0);
    lamp_post();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.7,0,(w/2)+55);
    glRotatef(180,0,1,0);
    lamp_post();
    glPopMatrix();

    for(int i=10;i<=40;i+=10){
        glPushMatrix();
        glTranslatef(-0.2,-0.3,(w/2)+6+i);
        glScalef(0.5,0.5,6);
        cube(1,1,1);
        glPopMatrix();
    }
    ///walkway
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glPushMatrix();
    glTranslatef(-7,0,(w/2)+10);
    glScalef(3,1,10);
    cube(1,1,1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-7,0,(w/2)+25);
    glScalef(3,1,35);
    cube(1,1,1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(4,0,(w/2)+10);
    glScalef(3,1,50);
    cube(1,1,1);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    ///parking
    glPushMatrix();

    glTranslatef(-27,-0.4,(w/2)+15);
    glPushMatrix();
    glScalef(20,0.5,30);
    cube(0.05,0.05,0.05);
    glPopMatrix();
    ///boundary
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glPushMatrix();
    glTranslatef(-1,0,0);
    glScalef(1,3,30);
    cube(0,1,0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1,0,-1);
    glScalef(21,3,1);
    cube(0,1,0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1,0,30);
    glScalef(21,3,1);
    cube(0,1,0);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    for(int i=5;i<=25;i+=5){
        glPushMatrix();
        glTranslatef(0,0.1,i);
        glScalef(8,0.5,0.3);
        cube(1,1,1);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(12,0.1,i);
        glScalef(8,0.5,0.3);
        cube(1,1,1);
        glPopMatrix();
    }

    glPopMatrix();


    glPopMatrix();
}

void tree(){
    glPushMatrix();
    cylinder(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,2,0);
    cone(0.01,0.5,0.01);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,4,0);
    cone(0.02,0.6,0.02);
    glPopMatrix();
}

void tree1(){
    ///trunk
    glPushMatrix();
    glScalef(0.75,1,0.75);
    cylinder(0.3,0.18,0.09);
    glPopMatrix();
    ///branches
    glPushMatrix();
    glTranslatef(0,3,0);
    glRotatef(45,0,0,1);
    glScalef(0.5,0.5,0.5);
    cylinder(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,3,0);
    glRotatef(-45,0,0,1);
    glScalef(0.5,0.5,0.5);
    cylinder(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,3,0);
    glRotatef(45,1,0,0);
    glScalef(0.5,0.5,0.5);
    cylinder(0.3,0.18,0.09);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,3,0);
    glRotatef(-45,1,0,0);
    glScalef(0.5,0.5,0.5);
    cylinder(0.3,0.18,0.09);
    glPopMatrix();
    ///leaves
    glPushMatrix();
    glTranslatef(-2,5,0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.5,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,5,-2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.5,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,5,2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.5,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2,5,0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.5,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,7,0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.75,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1,6,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.25,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1,6,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.25,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1,6,-1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.25,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-1,6,-1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    glusphereDraw(1.25,20,20,0,1,0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void trees(){
    for(int i=25; i<=65; i+=5){
        glPushMatrix();
        glTranslatef(8,0,i);
        tree();
        glPopMatrix();
    }
    glPushMatrix();
    glTranslatef(-8,0,55);
    tree1();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-16,0,55);
    tree1();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-24,0,55);
    tree1();
    glPopMatrix();
    for(int i=20;i<=52;i+=8){
        glPushMatrix();
        glTranslatef(-30,0,i);
        tree1();
        glPopMatrix();
    }
}

void boundary_gate(GLfloat l, GLfloat w){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,6);
    ///back
    glPushMatrix();
    glTranslatef(-l,0,(-w/2)-14);
    glScalef(l*2,6,1);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();
    ///left
    glPushMatrix();
    glTranslatef(-(l/2)-13,0,-w-6);
    glScalef(1,6,(w*2)+7);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-(l/2)-23,0,(w/2)+8);
    glScalef(1,6,50);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();
    ///right
    glPushMatrix();
    glTranslatef((l/2)+12,0,-w-6);
    glScalef(1,6,(w*2)+10);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(12,0,(w/2)+13);
    glScalef(1,6,45);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();
    ///front
    glPushMatrix();
    glTranslatef(-(l*1.5)+2,0,(w/2)+8);
    glScalef(l/2.5,6,1);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();

    glPushMatrix();
    glTranslatef((l/2),0,(w/2)+12);
    glScalef((l/2)+1,6,1);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();

    ///more front
    glPushMatrix();
    glTranslatef(-(l*1.5)+1,0,(w/2)+58);
    glScalef(l+7.1,6,1);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.9,0,(w/2)+58);
    glScalef(9,6,1);
    cube(0.1, 0.4, 0.433);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    ///poles
    glPushMatrix();
    glTranslatef(3.7,0,(w/2)+58);
    glScalef(0.5,2.5,0.5);
    cylinder(0.8,0.8,0.8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-3.7,0,(w/2)+58);
    glScalef(0.5,2.5,0.5);
    cylinder(0.8,0.8,0.8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-4,7,(w/2)+58);
    glScaled(8,0.3,0.3);
    cube(0.8,0.8,0.8,true);
    glPopMatrix();

    ///name
    glPushMatrix();
    glTranslatef(-3, 7.5, (w/2)+58);
    name();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void sky(){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,12);
    glPushMatrix();
    glTranslatef(0,0,30);
    glRotatef(90,1,0,0);
    glusphereDraw(100,50,50);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void windmill(){
    glPushMatrix();
    glScalef(1,5,1);
    cylinder(0.8,0.8,0.8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.75,20,0);
    glScalef(1.5,1.5,3);
    cube(0.8,0.8,0.8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,20,3);
    ///blades
    glPushMatrix();
    glRotatef(fan_rot/2,0,0,1);

    glPushMatrix();
    glTranslatef(-10,-0.5,0);
    glScalef(20,1,1);
    cube(0.9,0.9,0.9);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.5,-10,0);
    glScalef(1,20,1);
    cube(0.9,0.9,0.9);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();
}

void windmills(GLfloat l, GLfloat w){

    glPushMatrix();
    glTranslatef(-l+2,0,w);
    windmill();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(l-2,0,w);
    windmill();
    glPopMatrix();
}

static void resize(int width, int height)
{
    float rat_new = 1.0*width/height;
    float width_new = height*rat;
    float height_new = width/rat;
    float x_t = 0;
    float y_t = 0;

    if(rat<rat_new)
    {
        x_t = (width-width_new)/2;
        width=width_new;
    }
    else
    {
        y_t = (height-height_new)/2;
        height=height_new;
    }

    glViewport(x_t, y_t, width, height);
}

void drawText(const char *text, int length, int x, int y)
{
    glMatrixMode(GL_PROJECTION);
    double *matrix = new double[16];
    glGetDoublev(GL_PROJECTION_MATRIX, matrix);
    glLoadIdentity();

    glOrtho(0,800,0,600,-5,5);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
        glLoadIdentity();
        glRasterPos2i(x,y);
        for(int i=0;i<length;i++)
        {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]);
        }
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix);
    glMatrixMode(GL_MODELVIEW);
}

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-4, 4, -3, 3, 3.0, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0],eye[1],eye[2], look[0],look[1],look[2], 0,1,0);

    glRotatef(rot, 0,1,0);

    GLfloat l = 24, w = 16;
    light(l,w);
    ///axes();
    entrance(l,w);
    base_with_field(l,w);
    boundary(l,w);
    goalPosts(l);
    flood_lights(l,w);
    viewers_bench(l,w);
    players_bench(l,w);
    outer_walls(l,w);
    trophy_stand(w);
    vip(l,w);
    commentry(l,w);
    road(l,w);
    trees();
    boundary_gate(l,w);
    sky();
    windmills(l,w);

    glPushMatrix();
    string text = "B->Birds Eye, V->Entrance, G->Inside, M->Day/Night, N->Night Light";
    glColor3f(1,1,1);
    drawText(text.data(),text.size(),100,200);
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}

static void key(unsigned char key, int x, int y)
{
    GLfloat o_lx, o_lz,theta=-3, n_lx, n_lz, step = 0.35;
    GLfloat diff_x = look[0]-eye[0];
    GLfloat diff_y = look[1]-eye[1];
    GLfloat diff_z = look[2]-eye[2];
    GLfloat unit = sqrt(diff_x*diff_x + diff_y*diff_y + diff_z*diff_z);
    diff_x = diff_x /unit;
    diff_y = diff_y /unit;
    diff_z = diff_z /unit;
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case 'a':
        o_lx = look[0]-eye[0];
        o_lz = look[2]-eye[2];
        theta = (theta*PI)/180;
        n_lx = o_lx*cos(theta)-o_lz*sin(theta);
        n_lz = o_lx*sin(theta)+o_lz*cos(theta);
        look[0] = n_lx + eye[0];
        look[2] = n_lz + eye[2];
        break;
    case 'd':
        o_lx = look[0]-eye[0];
        o_lz = look[2]-eye[2];
        theta = (theta*PI)/180;
        n_lx = o_lx*cos(theta)+o_lz*sin(theta);
        n_lz = -o_lx*sin(theta)+o_lz*cos(theta);
        look[0] = n_lx + eye[0];
        look[2] = n_lz + eye[2];
        break;
    case 'w':
        eye[0] += diff_x*step;
        eye[1] += diff_y*step;
        eye[2] += diff_z*step;
        look[0] += diff_x*step;
        look[1] += diff_y*step;
        look[2] += diff_z*step;
        break;
    case 's':
        eye[0] -= diff_x*step;
        eye[1] -= diff_y*step;
        eye[2] -= diff_z*step;
        look[0] -= diff_x*step;
        look[1] -= diff_y*step;
        look[2] -= diff_z*step;
        break;
        case 'i':
        o_lx = look[1]-eye[1];
        o_lz = look[2]-eye[2];
        theta = (theta*PI)/180;
        n_lx = o_lx*cos(theta)-o_lz*sin(theta);
        n_lz = o_lx*sin(theta)+o_lz*cos(theta);
        look[1] = n_lx + eye[1];
        look[2] = n_lz + eye[2];
        break;
    case 'k':
        o_lx = look[1]-eye[1];
        o_lz = look[2]-eye[2];
        theta = (theta*PI)/180;
        n_lx = o_lx*cos(theta)+o_lz*sin(theta);
        n_lz = -o_lx*sin(theta)+o_lz*cos(theta);
        look[1] = n_lx + eye[1];
        look[2] = n_lz + eye[2];
        break;
    case '+':
        eye[1]+=0.2;
        look[1]+=0.2;
        break;
    case '-':
        eye[1]-=0.2;
        look[1]-=0.2;
        break;
    case 'm':
        day_flag =! day_flag;
        break;
    case 'n':
        night_flag =! night_flag;
        break;
    case 'b':
        eye[0] = -20;
        eye[1] = 30;
        eye[2] = 40;
        look[0] = 0;
        look[1] = 5;
        look[2] = -10;
        break;
    case 'v':
        eye[0] = 0;
        eye[1] = 5;
        eye[2] = 70;
        look[0] = 0;
        look[1] = 5;
        look[2] = 40;
        break;
    case 'g':
        eye[0] = 0;
        eye[1] = 5;
        eye[2] = 0;
        look[0] = 0;
        look[1] = 5;
        look[2] = -20;
        break;
    case 'c':
        wire =! wire;
        break;
    case 'x':
        door_flag =! door_flag;
        break;
    case 'X':
        door_flag1 =! door_flag1;
        break;
    case 'z':
        gate_open =! gate_open;
        break;
    case 'Z':
        gate_close =! gate_close;
        break;
    case 'f':
        controller-=2;
        break;
    case 'h':
        controller+=2;
        break;
    case '1':
        ambient_flag =! ambient_flag;
        break;
    case '2':
        diffuse_flag =! diffuse_flag;
        break;
    case '3':
        specular_flag =! specular_flag;
        break;
    }

    glutPostRedisplay();
}

static void idle(void)
{
    if(birds_eye){
        rot+=0.5;
        if(rot==360) rot = 0;
    }
    if(door_flag){
        door_rot += 0.5;
        if(door_rot==80) door_flag=false;
    }
    if(door_flag1){
        door_rot -= 0.5;
        if(door_rot==0) door_flag1=false;
    }
    clock_rot += 0.1;
    fan_rot += controller;
    if(gate_open){
        gate += 0.5;
        if(gate==10) gate_open = false;
    }
    if(gate_close){
        gate -= 0.5;
        if(gate==3) gate_close = false;
    }
    glutPostRedisplay();
}


int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(width,height);
    glutInitWindowPosition(30,30);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("GLUT Shapes");

    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    glutReshapeFunc(resize);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);

    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\field.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\light.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\i.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\ps5.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\score.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\track1.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\allianz.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\chairs.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\chairs1.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\net.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\tin.bmp");
    LoadTexture("C:\\Users\\USER\\Downloads\\Texture-20210518T050920Z-001\\Texture\\sky.bmp");

    cout<<"\t\t\t------Key Guide------"<<endl;
    cout<<endl<<"\t\t------Movement-----"<<endl;
    cout<<"w -> Move forward"<<endl<<"s -> Move backwards"<<endl<<"+ -> Move up"<<endl<<"- -> Move down"<<endl;
    cout<<endl<<"\t\t------Looking-----"<<endl;
    cout<<"a -> Look left"<<endl<<"d -> Look down"<<endl<<"i/k -> Look up/down"<<endl;
    cout<<endl<<"\t\t------Positioning-----"<<endl;
    cout<<"b -> Birds eye"<<endl<<"v -> Entrance"<<endl<<"g -> Inside"<<endl;
    cout<<endl<<"\t\t------Lights-----"<<endl;
    cout<<"m -> Toggle day/night mode"<<endl<<"n -> Toggle night lights"<<endl<<"1/2/3 -> Toggle Ambient/Diffuse/Specular property of lights"<<endl;
    cout<<endl<<"\t\t------Others-----"<<endl;
    cout<<"z/Z -> Open/Close main door"<<endl<<"x/X -> Open/Close room doors"<<endl<<"c -> Toggle wire frame mode"<<endl<<"f/h-> Control fan/windmill";

    glutMainLoop();

    return EXIT_SUCCESS;
}
