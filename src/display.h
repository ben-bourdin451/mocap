#ifndef COLLOMOSSE_MOCAP_DISPLAY_INCLUDED
#define COLLOMOSSE_MOCAP_DISPLAY_INCLUDED

/*******************************************************\
*                                                       *
*  DISPLAY.H                                            *
*  Outline solution to begin the coursework with        *
*                                                       *
*  Currently just draws a teapot                        *
*  Written as a resource for CM20219 coursework         *
*                                                       *
*  (c) John Collomosse, University of Bath              *
*  September 2008                                       *
*                                                       *
\*******************************************************/


#ifdef WIN32
	#include "windows.h"
	#define strcasecmp stricmp
#endif

#include "GL/gl.h"
#include "GL/glut.h"

#include <math.h>

#include "parser.h"
#include "draw.h"

#define CAMERA_SENS 0.07		/* This is the camera sensibility or the incremental step for the camera angles */
#define PI 3.14159				/* Defines the pi constant used for angles */

void dorender(int argc, char** argv, SKELETON* skel, MOCAP* mo, int delay);

/* GLUT callbacks */
void keyboard(unsigned char key, int x, int y);
void reshape(int w, int h);
void init(void);
void display(void);
void idle(void);

#endif