#ifndef COLLOMOSSE_MOCAP_PARSER_INCLUDED
#define COLLOMOSSE_MOCAP_PARSER_INCLUDED

/*******************************************************\
*                                                       *
*  PARSER.H                                             *
*  Acclaim Motion Capture (ASF/AMC) Parser Library      *
*                                                       *
*  Parses Skeleton (ASF) and motion capture (AMC) files *
*  Written as a resource for CM20219 coursework         *
*                                                       *
*  (c) John Collomosse, University of Bath              *
*  September 2008                                       *
*                                                       *
\*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI (3.141)

#ifdef WIN32
	#include "windows.h"
	#define strcasecmp stricmp
#endif

/* A basic type for representing 3D quantities e.g. points, vectors and Euler angles */
typedef struct _3dcoord {

	float x;
	float y;
	float z;

} POINT3D;


/* Type for representing bones */
typedef struct _bone {

	int		id;
	char*	name;
	POINT3D direction;
	float	length;
	POINT3D axis;
	int		xyzflags;
	int		children_enum;
	struct _bone**	children;
	struct _bone*	parent;

} BONE;


/* Type for representing the skeleton (collection of bones) */
typedef struct _skeleton {

	POINT3D init_position;			/* Initial translation of the root (world) reference frame */
	POINT3D init_orientation;		/* Initial orientation of the root (world) reference frame */
	int		children_enum;			/* How many children bones off the root ? */
	struct _bone**	children;		/* Array of pointers to children bones e.g. children[0 to children_enum] */

	/* You aren't likely to need these next two fields for your coursework */
	int		bonearray_enum;			/* Number of bones in bonearray */
	struct _bone* bonearray;		/* Not needed for coursework - array of all bones in skeleton bonearray[0 to bonearray_enum]*/

} SKELETON;


/* Type for representing a motion capture set (how the skeleton moves) */
typedef struct _mocap {

	int			frames_enum;	/* Number of frames of animation */
	POINT3D*	root_pos;		/* Translation of root (world) reference frame - root_pos[0] to root_pos[frames_enum-1] */
	POINT3D*	root_orient;	/* Orientation of root (world) reference frame - root_orient[0] to root_orient[frames_enum-1] */
	POINT3D**	bones_orient;	/* Orientation of bones - bones_orient[framenumber][bonenumber] */

} MOCAP;


SKELETON*	parser_loadSkeleton(char* argFilename);
MOCAP*		parser_loadMocap(char* argFilename, SKELETON* skel);
void		parser_debugskeletonTree(SKELETON* skel);
void		parser_free_skeleton(SKELETON* skel);
void		parser_free_mocap(MOCAP* mocap);
void RotateBoneDirToLocalCoordSystem(SKELETON* skel);
void vector_rotationXYZ(POINT3D* v, float a, float b, float c);
void matrix_transform_affine(double m[4][4], double x, double y, double z, POINT3D* v);
void rotationX(double r[][4], float a);
void rotationY(double r[][4], float a);
void rotationZ(double r[][4], float a);



#endif 