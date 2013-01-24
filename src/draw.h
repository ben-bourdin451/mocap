#ifndef DEF_DRAW
#define DEF_DRAW

/*******************************************************\
*                                                       *
*  DRAW.H                                               *
*  Used for display function in display.c               *
*                                                       *
*  Displays various objects on screen                   *
*  Written as a resource for CM20219 coursework         *
*                                                       *
*  Benjamin Bourdin, bb247, University of Bath          *
*  December  2008                                       *
*                                                       *
\*******************************************************/

#ifdef WIN32
	#include "windows.h"
	#define strcasecmp stricmp
#endif

#include "GL/gl.h"
#include "GL/glut.h"

#include "parser.h"


#include <math.h>

#define SPHERE_RAD 0.5			/* Specifies the radius of the spheres */
#define CYLINDER_RAD 0.2		/* Specifies the width of the cylinders/bones */
#define SLICES 16				/* Specifies the number of slices used for the spheres and cylinders */
#define STACKS 16				/* Specifies the number of stacks used for the spheres and cylinders */
#define PI 3.14159				/* Defines the pi constant used for angles */

/* Prototypes of functions */
void drawInitialPose(SKELETON* gSkel, int referenceFrame);							/* Draws the skeleton in its initial pose */
void drawSkeleton(SKELETON* gSkel, MOCAP *gMo, int frame, int referenceFrame);		/* Draws skeleton under mocap data at specified frame*/

void drawInitialJoints(BONE* bone, int referenceFrame);								/* Draws the joints of the skeleton without any rotations (used for initial pose) */
void drawJoints(BONE* bone, MOCAP* gMo, int frame, int referenceFrame);				/* Draws the joints of the skeleton with the bone orientation
																					 * of the specified frame (under mocap data) */

void drawCylinder(BONE* bone);														/* Draws the bones of the skeleton */
void drawReferenceFrame(unsigned int scale);										/* Draws a reference frame of specified scale/size */
void drawFloor(float w, float h);													/* Draws the floor of the scene */

GLuint loadTexture();																/* Loads a chequerboard texture */

#endif