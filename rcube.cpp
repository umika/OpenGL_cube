/*
  rcube.cpp

  > cl rcube.cpp glut32.lib glpng.lib -EHsc -I. \
    -link -LIBPATH:. -SUBSYSTEM:console -NODEFAULTLIB:libc

  -EHsc destroy local objects on catch any exception
  -NODEFAULTLIB:libc recent VC++ links 'libcmt.lib' instead of 'libc.lib'
  'GL/glut.h' 'glut32.lib' 'glut32.dll' from glut-3.7.6-bin.zip
  'GL/glpng.h' 'glpng.lib' from http://openports.se/graphics/glpng
*/

#include <iostream>
#include <sstream>
#include <string>
#include <cstdarg>
#include <cstdlib>
#include <GL/glpng.h>
#include <GL/glut.h>

using namespace std;

typedef struct _G_Size {
  int w, h;
} G_Size;

typedef struct _G_Point {
  int x, y;
} G_Point;

G_Size Gwsize = {640, 480};
G_Point Gwpos = {320, 240};
string Gwtitle = "rcube";
GLuint Gtextures[2] = {0};
GLuint Gfps = 0;
int Gangle = 0;

string RESOURCE_DIR = "resource";
string TEXIMG_FACE = "f%d.png";
string TEXIMG_HINT = "72dpi.png";
string TEXIMG_CHAR[] = {"72dpi_ascii_reigasou_16x16.png"};

string path_join(int n, ...); // string *

unsigned char AlphaCallback(unsigned char r, unsigned char g, unsigned char b);
void LoadTextures(void);
void Initialize(void);
void FPS(void);
void FPSdisplay(void);
void Timer(int dt);
void Idle(void);
void Update(void);
void Display(void);
void Reshape(int w, int h);
void KeyPress(unsigned char key, int x, int y);

string path_join(int n, ...) // string *
{
  string s;
  va_list ap;
  va_start(ap, n);
  for(int c = 0; c < n; c++){
    string *p = va_arg(ap, string *);
    if(c > 0) s += '\\';
    s += *p;
  }
  va_end(ap);
  return s;
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(Gwsize.w, Gwsize.h);
  glutInitWindowPosition(Gwpos.x, Gwpos.y);
  glutCreateWindow(Gwtitle.c_str());
  glutTimerFunc(0, Timer, 0);
  glutIdleFunc(Idle);
  glutDisplayFunc(Display);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(KeyPress);
  Initialize();
  glutMainLoop();
  return 0;
}

unsigned char AlphaCallback(unsigned char r, unsigned char g, unsigned char b)
{
  // unsigned a = 255 - (r + g + b) / 3;
  unsigned a = 255 * (r + g + b) / (255 * 3);
  return a & 0x00FF;
}

void LoadTextures(void)
{
  string *a[] = {&TEXIMG_HINT, &TEXIMG_CHAR[0]};
  for(int i = 0; i < sizeof(a) / sizeof(string *); i++){
    string &filename = path_join(2, &RESOURCE_DIR, a[i]);
#if 1 // Using pngLoadRaw and setting texture parameters manually.
    glGenTextures(1, &Gtextures[i]);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Gtextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    pngRawInfo pri;
    if(!pngLoadRaw(filename.c_str(), &pri)){
      cerr << "Can't load file: " << *a[i];
      exit(1);
    }else{
#ifdef _DEBUG
      cout << "File: " << *a[i] << endl;
      cout << " Width: " << pri.Width << endl;
      cout << " Height: " << pri.Height << endl;
      cout << " Depth: " << pri.Depth << endl;
      cout << " Alpha: " << pri.Alpha << endl;
      cout << " Components: " << pri.Components << endl;
      cout << " pData: " << (long)pri.Data << endl;
      cout << " pPalette: " << (long)pri.Palette << endl;
#endif
      if(pri.Data){
        unsigned char buf[4 * 256 * 256];
        for(int y = 0; y < pri.Height; y++){
          for(int x = 0; x < pri.Width; x++){
            int r = y * pri.Width + x;
            int q = r * 4;
            if(pri.Components == 1){
              int p = pri.Data[r] * 3;
              for(int j = 0; j < 3; j++) buf[q + j] = pri.Palette[p + j];
            }else{
              for(int j = 0; j < 3; j++) buf[q + j] = pri.Data[q + j];
            }
            buf[q + 3] = AlphaCallback(buf[q + 0], buf[q + 1], buf[q + 2]);
          }
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, pri.Width, pri.Height, 0,
          GL_RGBA, GL_UNSIGNED_BYTE, buf);
        free(pri.Data);
      }
      if(pri.Palette) free(pri.Palette);
    }
#else
    pngInfo pi;
    pngSetAlphaCallback(AlphaCallback);
    Gtextures[i] = pngBind(filename.c_str(), PNG_NOMIPMAP,
      PNG_CALLBACK /* PNG_ALPHA */, &pi, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    if(!Gtextures[i]){
      cerr << "Can't load file: " << *a[i];
      exit(1);
    }
#endif
  }
}

void Initialize(void)
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST); // glutInitDisplayMode() GLUT_DEPTH
  LoadTextures();
}

void FPS(void)
{
  static GLuint frames = 0; // frames averaged over 1000ms
  static GLuint clk = 0; // milliseconds
  static GLuint pclk = 0; // milliseconds
  static GLuint nclk = 0; // milliseconds
  ++frames;
  clk = glutGet(GLUT_ELAPSED_TIME); // has limited resolution
  if(clk < nclk) return;
  Gfps = frames;
  pclk = clk;
  nclk = clk + 1000;
  frames = 0;
}

void FPSdisplay(void)
{
  ostringstream oss;
  oss << Gfps << " FPS";
  string &s = oss.str();
  glDisable(GL_TEXTURE_2D);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  // glWindowPos2f(0.0, 0.0);
  // glRasterPos2f(0.0, 0.0);
  glRasterPos3f(-0.9, -0.5, -0.9);
  for(string::iterator it = s.begin(); it != s.end(); ++it)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *it);
  glEnable(GL_TEXTURE_2D);
  // reverse top / bottom
  glEnable(GL_TEXTURE_2D);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
  glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, Gtextures[1]);
  glBegin(GL_QUADS);
    glTexCoord2f(0.0625, 0.3125); glVertex3f(-0.1, -0.1, 1.1);
    glTexCoord2f(0.1250, 0.3125); glVertex3f( 0.1, -0.1, 1.1);
    glTexCoord2f(0.1250, 0.2500); glVertex3f( 0.1,  0.1, 1.1);
    glTexCoord2f(0.0625, 0.2500); glVertex3f(-0.1,  0.1, 1.1);
  glEnd();
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
#ifdef _DEBUG
  cout << s << endl;
#endif
}

void Timer(int dt)
{
  const int desiredFPS = 60;
  if(dt > desiredFPS) dt = 0;
  glutTimerFunc(1000 / desiredFPS, Timer, ++dt);
  FPS();
  Update();
}

void Idle(void)
{
}

void Update(void)
{
  Gangle = (Gangle + 1) % 360;
  glutPostRedisplay();
}

void Display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_TEXTURE_2D);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
  glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
  glEnable(GL_BLEND);
//  glPushMatrix();
    glTranslatef(0.0, 0.0, -5.0);
    glRotatef(Gangle, 0, 1, 0);
//  glPopMatrix();
  glBindTexture(GL_TEXTURE_2D, Gtextures[0]);
  glBegin(GL_QUADS);
    // reverse top / bottom
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, -0.5, -1.0);
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.5, -0.5, -1.0);
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.5,  0.5, -1.0);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.5,  0.5, -1.0);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, Gtextures[1]);
  glBegin(GL_QUADS);
    // reverse top / bottom
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, -0.4,  1.0);
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.8, -0.4,  1.0);
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.8,  0.4,  1.0);
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.0,  0.4,  1.0);
  glEnd();
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  FPSdisplay();
  // glFlush(); // glutInitDisplayMode() GLUT_SINGLE ?
  glutSwapBuffers(); // glutInitDisplayMode() GLUT_DOUBLE (double buffering)
}

void Reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, (float)w / (float)h, 1, 1000);
  Display();
}

void KeyPress(unsigned char key, int x, int y)
{
  if(key == 27){ // ESC
    glutDestroyWindow(glutGetWindow());
    exit(0);
  }
}
