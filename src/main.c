/*******************************************************\
*                                                       *
*  MAIN.C                                               *
*                                                       *
*  Entry point for motion capture coursework            *
*                                                       *
*  (c) John Collomosse, University of Bath              *
*  September 2008                                       *
*                                                       *
\*******************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "display.h"

#define EXITCODE_SUCCESS	(0)
#define EXITCODE_BADSYNTAX	(1)
#define EXITCODE_BADSKEL	(2)
#define EXITCODE_BADMOCAP	(3)


int main (int argc, char** argv) {

	SKELETON* model=NULL;		/* Stores the skeleton (from ASF file) */
	MOCAP*	  motion=NULL;		/* Stores the motion capture data (from AMC file) */
	int		  delay=0;			/* Stores the optional delay used to slow down animation on fast PCs */

	/* Check we have both command line arguments */
	if (argc<2 || argc>4) {
		printf("Use MOCAPTEST <asf file> <amc file> [optional delay]\n");
		return (EXITCODE_BADSYNTAX);
	}
	
	/* Load the ASF file (skeleton) into 'model' */
	if (!(model=parser_loadSkeleton(argv[1]))) {
		printf("FATAL:  Failed to load skeleton from file\n");
		return (EXITCODE_BADSKEL);
	}

	/* Print out the skeleton hierarchy just for info */
	parser_debugskeletonTree(model);


	/* Load the AMC file (motion capture data) into 'mocap' */
	if (!(motion=parser_loadMocap(argv[2],model))) {
		printf("FATAL:  Failed to load mocap data from file\n");
		return (EXITCODE_BADMOCAP);
	}

	if (argc==4) {
		delay=atoi(argv[3]);
		printf("Pausing %dms at each cycle\n",delay);
	}

	/* TODO - Render an animation of the moving skeleton */
	dorender(argc,argv,model,motion,delay);

	/* Actually the dorender(..) call will never return from the GLUT loop so this line is redundant */

	return (EXITCODE_SUCCESS);
}