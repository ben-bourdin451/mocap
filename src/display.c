/*******************************************************\
*                                                       *
*  DISPLAY.C                                            *
*  Outline solution to begin the coursework with        *
*                                                       *
*  Currently just draws a teapot                        *
*  Written as a resource for CM20219 coursework         *
*                                                       *
*  Benjamin Bourdin, bb247, University of Bath          *
*  December  2008                                       *
*                                                       *
\*******************************************************/


#include "display.h"


/* Global variables */
SKELETON* gSkel;			/* Holds skeleton data - available to any function */
MOCAP*	  gMo;				/* Hold motion data - available to any function */
int		  gDelay=0;			/* Holds delay information - available to any function */
int currentFrame = 0;       /* Frame counter */
int initialPose = 0;		/* Boolean for displaying skeleton in initial position (if user presses 'f' key) */
int referenceFrame = 0;


/* Global variables for the camera position */
float rCamera = 70, thetaCamera = PI/4, phiCamera = -PI/2;

/* Entry point from MAIN.C */
void dorender(int argc, char** argv, SKELETON* skel, MOCAP* mo, int delay) {


	/* Create GLUT window */
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (600, 600);
   glutCreateWindow ("Mocap Viewer");

   /* Setup GLUT callbacks */
   glutReshapeFunc (reshape);
   glutKeyboardFunc (keyboard);
   glutDisplayFunc (display);
   glutIdleFunc (idle);

   /* Initialise any OpenGL state */
   init();

	/* Set global variables */
   gSkel=skel;
   gMo=mo;
   gDelay=delay;

   /* Kick off the GLUT main loop */
   /* This call will never return */
   glutMainLoop();
   

}


/* Keyboard callback from GLUT */
void keyboard(unsigned char key, int x, int y)
{
	switch(key) {

			/* If the ESCAPE key is pressed we have to force an exit.
			 * There is no graceful way to exit the GLUT loop unfortunately.
			 */
			case 0x1b:  /* 0x1b (27 decimal) is the ASCII code for the ESCAPE */
						parser_free_skeleton(gSkel);
						parser_free_mocap(gMo);
						exit(0);
						break;

			case 'f':	/* If the 'f' key is pressed enable intial pose view*/
						if (initialPose)
							initialPose = 0;
						else
							initialPose = 1;

						break;


			/* If the 'r' key is pressed change boolean referenceFrames to true
			 * and display the reference frames for each joint
			 */
			case 'r':
						if (referenceFrame)
							referenceFrame = 0;
						else
							referenceFrame = 1;

						break;



			/* Keyboard input to handle camera position. 
			 * 'q' and 'e' are used to zoom in and out. They modify the R (distance from origin) in our polar coordinates.
			 * 'a' and 'd' modify the angle phi (angle between the X-axis and the Y-axis) used to rotate right and left.
			 * 'w' and 's' modify the theta angle (angle between the Z-axis and R) used to rotate up and down.
			 */

			case 'e':
						rCamera-=2;
						if(rCamera < 1)
							rCamera = 1;
						break;

			case 'q':
						rCamera+=2;
						break;

			case 'a':
						phiCamera-=CAMERA_SENS;
						break;

			case 'd':
						phiCamera+=CAMERA_SENS;
						break;

			case 'w':
						thetaCamera-=CAMERA_SENS;
						if (thetaCamera < 0)
							thetaCamera+=CAMERA_SENS;
						break;

			case 's':
						thetaCamera+=CAMERA_SENS;
						if (thetaCamera > PI/2)
							thetaCamera-=CAMERA_SENS;
						break;
	}
}

/* Called back when GLUT idling */
void idle() {
	
	Sleep(gDelay);
	
	if (currentFrame < gMo->frames_enum-1) {
		currentFrame++;
	} else {
		currentFrame = 0;
	}

    glutPostRedisplay();
}

/* Called back by GLUT when the window is resized */
void reshape(int w, int h)
{

   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   
   gluPerspective(60,(GLfloat)w/(GLfloat)h,0.01,1000.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

}

/* Called by us in dorender() to init any OpenGL state */
/* It is good practice to keep this in a separate function from dorender() */
void init(void)
{  

	GLfloat mat_specular[] = {1, 1, 1, 1};
	GLfloat light_position[] = {0, 0.5, 0.5, 0.0};

	glClearColor (0.0, 0.0, 0.0, 0.0);

	/* Material */
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 96);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_COLOR_MATERIAL);

	/* Lighting */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);


	/* OpenGL Rendering engine parameters */
	glShadeModel (GL_SMOOTH);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glEnable(GL_DEPTH_TEST);

}


/* Called when GLUT wants to repaint the screen (we do all our rendering/geometry here) */
void display(void)
{
	float xCamera, yCamera, zCamera;	/* Camera coordinates */
	float xRoot, yRoot, zRoot;			/* Root position */
	
	/* Clear frame buffer and set up MODELVIEW matrix */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Calculate the camera position using polar coordinates */
	xCamera = rCamera*sin(thetaCamera)*cos(phiCamera);
	yCamera = rCamera*sin(thetaCamera)*sin(phiCamera);
	zCamera = rCamera*cos(thetaCamera);

	/* Calculate root postion */
	xRoot = gMo->root_pos[currentFrame].x;
	yRoot = -gMo->root_pos[currentFrame].z;
	zRoot = gMo->root_pos[currentFrame].y;

	if(initialPose) {

		/* Place the camera and draw the skeleton in its initial position */
		gluLookAt(xCamera, yCamera, zCamera, 0, 0, 0, 0, 0, 1);
		drawInitialPose(gSkel, referenceFrame);

	} else {

		/* Place camera at specified position and draw the skeleton under mocap data */
		gluLookAt(xCamera+xRoot, yCamera+yRoot, zCamera+zRoot, xRoot, yRoot, zRoot, 0, 0, 1);
		drawSkeleton(gSkel, gMo, currentFrame, referenceFrame);
	}


	drawFloor(140, 140);
	
	if(referenceFrame)
		drawReferenceFrame(20);
	
	/* Ensure any queued up OpenGL calls are run and swap buffers */
	glFlush();
	glutSwapBuffers();

}

