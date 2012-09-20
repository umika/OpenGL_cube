/*
  rcube.cpp

  > cl rcube.cpp glut32.lib -I. /link -LIBPATH:.
  ('GL/glut.h' 'glut32.lib' 'glut32.dll' from glut-3.7.6-bin.zip)
*/

#include <GL/glut.h>

int WindowPositionX = 100, WindowPositionY = 100;
int WindowWidth = 512, WindowHeight = 512;
char WindowTitle[] = "Hello, world!";

void Initialize(void);
void Display(void);
void Idle(void);

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitWindowPosition(WindowPositionX, WindowPositionY);
  glutInitWindowSize(WindowWidth, WindowHeight);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(WindowTitle);
  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  Initialize();
  glutMainLoop();
  return 0;
}

void Initialize(void)
{
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_DEPTH_TEST); // glutInitDisplayMode() GLUT_DEPTH
}

void Display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glutSwapBuffers(); // glutInitDisplayMode() GLUT_DOUBLE (double buffering)
}

void Idle(void)
{
  // nothing
}
