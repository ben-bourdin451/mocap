/*******************************************************\
*                                                       *
*  DRAW.C                                               *
*  Used for display function in display.c               *
*                                                       *
*  Displays various objects on screen                   *
*  Written as a resource for CM20219 coursework         *
*                                                       *
*  Benjamin Bourdin, bb247, University of Bath          *
*  December  2008                                       *
*                                                       *
\*******************************************************/


#include "draw.h"

/* Global variables */
GLuint floorTexture;	/* Holds the chequer board texture for the floor */

void drawJoints(BONE* bone, MOCAP* gMo, int frame, int referenceFrame)
{
	int i = 0;
	float x, y, z;	/* Next joint coordinates */
	float f, g, h;	/* Axis vector coordinates */

	/* Set up joint and vector coordinates */
	x = bone->direction.x*bone->length;
	y = bone->direction.y*bone->length;
	z = bone->direction.z*bone->length;
	f = bone->axis.x;
	g = bone->axis.y;
	h = bone->axis.z;
	

	/* Save current matrix projection and apply K'TRK matrix chain to the MODELVIEW matrix.
	 * */
	glPushMatrix();

	if(referenceFrame)
		drawReferenceFrame(2);

	/* K 
	 * Rotate into arbitrary axis
	 */
	glRotatef(h, 0, 0, 1);
	glRotatef(g, 0, 1, 0);
	glRotatef(f, 1, 0, 0);


	/* R 
	 * Rotate bone according to mocap data
	 */
	glRotatef(gMo->bones_orient[frame][bone->id].z, 0, 0, 1);
	glRotatef(gMo->bones_orient[frame][bone->id].y, 0, 1, 0);
	glRotatef(gMo->bones_orient[frame][bone->id].x, 1, 0, 0);


	/* Draw the bone, i.e. connection between the joints (cylinder) */
	drawCylinder(bone);

	/* T */
	glTranslatef(x, y, z);

	/* Draw joint (sphere) */
	glColor3f(0, 1.0, 0);
	glutSolidSphere(SPHERE_RAD, SLICES, STACKS);

	
	/* K' 
	 * Take out K
	 */
	glRotatef(-f, 1, 0, 0);
	glRotatef(-g, 0, 1, 0);
	glRotatef(-h, 0, 0, 1);


	/* Do the same for all the bones children */
	for(i; i<bone->children_enum; i++)
	{
		drawJoints(bone->children[i], gMo, frame, referenceFrame);
	}


	/* Load previous MODELVIEW matrix.
	 * Goes back to the parent reference frame.
	 */
	glPopMatrix();

}

void drawSkeleton(SKELETON* gSkel, MOCAP* gMo, int frame, int referenceFrame)
{
	int i = 0;

	/* Save current MODELVIEW so that drawing the skeleton
	 * doesn't alter the projection matrix.
	 */
	glPushMatrix();

	/* Rotate 90 degrees on the X-axis so that Skeleton is drawn upwards (up the z-axis) */
	glRotatef(90, 1, 0, 0);

	/*
	 *	Translate and rotate refrence frame by root_pos and root_orient respectively
	 *	then draw the root (red coloured sphere)
	*/
	glTranslatef(gMo->root_pos[frame].x, gMo->root_pos[frame].y, gMo->root_pos[frame].z);
	glRotatef(gMo->root_orient[frame].z, 0, 0, 1);
	glRotatef(gMo->root_orient[frame].y, 0, 1, 0);
	glRotatef(gMo->root_orient[frame].x, 1, 0 ,0);
	

	
	glColor3f(1.0, 0, 0);
	glutSolidSphere(SPHERE_RAD, SLICES, STACKS);
	
	/* For all children of the root node call the recursive drawJoints() function */
	for(i; i < gSkel->children_enum; i++)
	{
		drawJoints(gSkel->children[i], gMo, frame, referenceFrame);
	}

	/* Load the initial (world) reference frame */
	glPopMatrix();



}

void drawFloor(float w, float h)
{
	/* Divide width and height by two because we are drawing the floor centered on the origin.
	 * For example a square of length 20 will have coordinates ranging from 0 to 10.
	 */
	w = w/2;
	h = h/2;

	/* Load texture in white and enable texturing */
	glColor3f(1, 1, 1);
	floorTexture = loadTexture();
	glEnable(GL_TEXTURE_2D);

	/* Map the texture to a rectangle and draw it at height 0 */
	glBindTexture(GL_TEXTURE_2D, floorTexture);
		glBegin(GL_QUADS);
		glNormal3f(0,0,1);
		glTexCoord2f(0,0); glVertex3f(-w,-h,0);
		glTexCoord2f(1,0); glVertex3f(w,-h,0);
		glTexCoord2f(1,1); glVertex3f(w,h,0);
		glTexCoord2f(0,1); glVertex3f(-w,h,0);
		glEnd();


	glDisable(GL_TEXTURE_2D);

}

void drawInitialPose(SKELETON *gSkel, int referenceFrame)
{
	/* This functions uses the same techniques as drawSkeleton() except we translate
	 * and rotate the reference frame by the initial gSkel parameters:
	 * init_orientation and init_posistion.
	 */

	int i = 0;

	glPushMatrix();

	glRotatef(90, 1, 0, 0);

	/* Translate and rotate by initial parameters */
	glTranslatef(gSkel->init_position.x, gSkel->init_position.y, gSkel->init_position.z);
	glRotatef(gSkel->init_orientation.z, 0, 0, 1);
	glRotatef(gSkel->init_orientation.y, 0, 1, 0);
	glRotatef(gSkel->init_orientation.x, 1, 0, 0);

	glColor3f(1, 0, 0);
	glutSolidSphere(SPHERE_RAD, SLICES, STACKS);

	for(i; i < gSkel->children_enum; i++)
	{
		drawInitialJoints(gSkel->children[i], referenceFrame);
	}

	glPopMatrix();
}

void drawInitialJoints(BONE* bone, int referenceFrame)
{
	/* This function is the same as drawJoints() without K and R */

	int i = 0;
	float x, y, z;

	x = bone->direction.x*bone->length;
	y = bone->direction.y*bone->length;
	z = bone->direction.z*bone->length;
	
	glPushMatrix();

	if(referenceFrame)
		drawReferenceFrame(2);
	
	// Draw the bone, ie connection between the joints (cylinder)
	drawCylinder(bone);

	/* T */
	glTranslatef(x, y, z);
	
	// Draw joint (sphere)
	glColor3f(0, 1.0, 0);
	glutSolidSphere(SPHERE_RAD, SLICES, STACKS);
	

	for(i; i<bone->children_enum; i++)
	{
		drawInitialJoints(bone->children[i], referenceFrame);
	}


	glPopMatrix();

}



void drawReferenceFrame(unsigned int scale)
{
	/* This function scales the reference frame by function parameter size
	 * and then uses GL_LINES to draw the X,Y,Z axis
	 */
	glPushMatrix();
    glScalef(scale, scale, scale);
    glBegin(GL_LINES);
	glLineWidth(3);
    glColor3f(1,0,0);
    glVertex3i(0,0,0);
    glVertex3i(1,0,0);
    glColor3f(0,1,0);
    glVertex3i(0,0,0);
    glVertex3i(0,1,0);
	glColor3f(0,0,1);
	glVertex3i(0,0,0);
	glVertex3i(0,0,1);
    glEnd();
    glPopMatrix();
}

void drawCylinder(BONE* bone)
{

	/* Cartesian coordinates x,y,z and corresponding
	 * Spherical coordinates r,phi,theta
	 */
	float x, y, z, theta, phi, r;


	/* Object used to draw a cylinder */
	GLUquadric* param = gluNewQuadric();

	/* Set up x,y,z */
	x = bone->direction.x*bone->length;
	y = bone->direction.y*bone->length;
	z = bone->direction.z*bone->length;
		
	/* Calculate spherical coordinates */
	r = sqrt((x*x) + (y*y) + (z*z));
	phi = atan2(y, x);
	theta = acos(z/r);
		
	/* Convert the angles to degrees */
	theta*=180/PI;
	phi*=180/PI;


	/* Save current projection matrix and rotate Z-axis by phi and Y-axis by theta
	 * so the Z-axis is aligned with P(x,y,z) and draw the cylinder.
	 */
	glPushMatrix();	
	glRotatef(phi, 0, 0, 1);
	glRotatef(theta, 0, 1, 0);

	glColor3f(1,1,0);
	gluCylinder(param, CYLINDER_RAD, CYLINDER_RAD, bone->length, SLICES, STACKS);
	glPopMatrix();

	
	gluDeleteQuadric(param);
}

GLuint loadTexture()
{
	GLuint texture;

	/* Beginning of code taken from the lectures.
	 * This generates a chequerboard texture of size 256x256
	 */
	unsigned char* tex_data;
    unsigned char* ptr;
    int tex_sizex,tex_sizey;
    int flg;
    int i,j;

    tex_sizex=tex_sizey=256;
    tex_data=(unsigned char*)calloc(tex_sizex*tex_sizey*3,sizeof(unsigned char));

    ptr=tex_data;
    for (j=0; j<tex_sizey; j++) {
		for (i=0; i<tex_sizex*3; i++) {
			flg=0;

            if ((i/(3*20))%2)
				flg++;
			if ((j/20)%2)
				flg++;

			*(ptr++)=flg*128;

		}
	}
	/* End of code taken from the lectures */

	/* Give texture a name and select it */
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);


	/* Set up environment and texture wrapping */
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* Use mip mapping */
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,tex_sizex,tex_sizey,GL_RGB,GL_UNSIGNED_BYTE,tex_data);

    free (tex_data);

	return texture;
}

