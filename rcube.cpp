/*
  rcube.cpp

  > cl rcube.cpp glut32.lib glpng.lib -EHsc -I.
    -link -LIBPATH:. -NODEFAULTLIB:libc

  -EHsc destroy local objects on catch any exception
  -NODEFAULTLIB:libc recent VC++ links 'libcmt.lib' instead of 'libc.lib'
  'GL/glut.h' 'glut32.lib' 'glut32.dll' from glut-3.7.6-bin.zip
  'GL/glpng.h' 'glpng.lib' from http://openports.se/graphics/glpng
*/

#include <iostream>
#include <string>
#include <cstdarg>
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
int angle = 0;

string RESOURCE_DIR = "resource";
string TEXIMG_FACE = "f%d.png";
string TEXIMG_HINT = "72dpi.png";
string TEXIMG_CHAR[] = {"72dpi_ascii_reigasou_16x16.png"};

string path_join(int n, ...); // string *

void LoadTextures(void);
void Initialize(void);
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
    if(c > 0) s += "\\";
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
  glutIdleFunc(Idle);
  glutDisplayFunc(Display);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(KeyPress);
  Initialize();
  glutMainLoop();
  return 0;
}

void LoadTextures(void)
{
  string *a[] = {&TEXIMG_HINT, &TEXIMG_CHAR[0]};
  for(int i = 0; i < sizeof(a) / sizeof(string *); i++){
    pngInfo pi;
    Gtextures[i] = pngBind(path_join(2, &RESOURCE_DIR, a[i]).c_str(),
      PNG_NOMIPMAP, PNG_ALPHA, &pi, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    if(!Gtextures[i]){
      cerr << "Can't load file: " << *a[i];
      exit(1);
    }
  }
}

void Initialize(void)
{
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST); // glutInitDisplayMode() GLUT_DEPTH
  LoadTextures();
}

void Idle(void)
{
  Update();
}

void Update(void)
{
  angle = (angle + 1) % 360;
  glutPostRedisplay();
}

void Display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
//  glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
//  glPushMatrix();
    glTranslatef(0.0, 0.0, -5.0);
    glRotatef(angle, 0, 1, 0);
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
  // glFlush(); // glutInitDisplayMode() GLUT_SINGLE ?
  glutSwapBuffers(); // glutInitDisplayMode() GLUT_DOUBLE (double buffering)
}

void Reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30, (float)w/h, 1, 100);
  glMatrixMode(GL_MODELVIEW);
  Display();
}

void KeyPress(unsigned char key, int x, int y)
{
  if(key == 27){ // ESC
    glutDestroyWindow(glutGetWindow());
    exit(0);
  }
}
