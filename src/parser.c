/*******************************************************\
*                                                       *
*  PARSER.C                                             *
*  Acclaim Motion Capture (ASF/AMC) Parser Library      *
*                                                       *
*  Parses Skeleton (ASF) and motion capture (AMC) files *
*  Written as a resource for CM20219 coursework         *
*                                                       *
*  (c) John Collomosse, University of Bath              *
*  September 2008                                       *
*                                                       *
\*******************************************************/

/* CM20219 students - You should not need to edit/view 
   this file for your coursework */

#include "parser.h"

/* ASF/AMC parser states */
#define PARSESTATE_UNKNOWN	(0)
#define PARSESTATE_VERSION	(1)
#define PARSESTATE_NAME		(2)
#define PARSESTATE_UNITS	(3)
#define PARSESTATE_DOCS		(4)
#define PARSESTATE_ROOT		(5)
#define PARSESTATE_BONEDATA	(6)
#define PARSESTATE_HIERARCHY (7)
#define PARSESTATE_DEGREES  (8)

/* Presence flags for DOF keywords in :bonedata of ASF file */
#define DOF_FLAG_RX (0x01)
#define DOF_FLAG_RY (0x02)
#define DOF_FLAG_RZ (0x04)

/* Buffer size for reading each line of ASF/AMC file */
#define READ_BUFFERLEN		(1024)

/* Prototypes for internal functions */

void	trim			(char*);		/* Trim whitespace off string */
int		changemode		(char*);		/* Check for change of parser state */
int		nextwht			(char*);		/* Find next whitespace character in string */

int		decode_bonedata	(FILE*, BONE**, int*);				/* Decoder for ASF :bonedata state */
int		decode_dummyfield(FILE*);							/* Decoder for ASF/AMC dummy/invalid state */
int		decode_hierarchy(FILE*, BONE*, int, SKELETON*);		/* Decoder for ASF :hierarchy state */
int		getboneindex (BONE*, int, char*);					/* Resolve bone name to bone index */
void	debugskeletonTree_recur(BONE* bn, int recurctr);	/* Print skeleton hierarchy for debugging */
void	parser_free_skeleton_helper(BONE* bn);				/* Recursive helper for freeing skeleton structure */
int		decode_root(FILE* fp, SKELETON* skel);				/* Decoder for ASF :root state */
int		decode_degrees(FILE* fp, MOCAP* mocap, SKELETON* skel);/* Decoder for AMC :degrees state */
void	rotateVector(POINT3D*, float, float, float);	/* Rotate vector by X, Y, Z Euler angles */

SKELETON* parser_loadSkeleton(char* argFilename) {

	FILE* fp;					/* file to be parsed */
	int	  ps;				/* parser state */
	char  buf [READ_BUFFERLEN];	/* parse buffer */
	SKELETON* skel;				/* the skeleton */
	BONE*		bones;			/* bone collection */
	int			bone_enum;		/* count of bones in collection */
	int		i;
	
	if (!(fp=fopen(argFilename,"rt")))
		return NULL;

	skel=(SKELETON*)calloc(1,sizeof(SKELETON));
	skel->children_enum=0;
	skel->children=NULL;
	skel->bonearray=NULL;

	bones=NULL;
	bone_enum=0;
	
	ps=PARSESTATE_UNKNOWN;
	while (!feof(fp)) {

		if (ps==PARSESTATE_UNKNOWN) {
			fgets(buf,READ_BUFFERLEN,fp);
			trim(buf);
			ps=changemode(buf);
		}

		
		/* Handle modes */
		switch (ps) {
			case PARSESTATE_BONEDATA:
				ps=decode_bonedata(fp,&bones,&bone_enum);
				break;
			case PARSESTATE_HIERARCHY:
				ps=decode_hierarchy(fp,bones,bone_enum,skel);
				break;
			case PARSESTATE_ROOT:
				ps=decode_root(fp,skel);
				break;
			default:
				ps=decode_dummyfield(fp);
				break;
		}

	}

	
	fclose(fp);

	/* Rewrite the bone direction vectors (which are in global i.e. root frame coords) to the local/axis coord system
	   which is more convenient when performing recursion later on */

	for (i=0; i<skel->bonearray_enum; i++) {
		rotateVector(&(skel->bonearray[i].direction), -skel->bonearray[i].axis.x, -skel->bonearray[i].axis.y, -skel->bonearray[i].axis.z); 
	}

	return skel;

}

int changemode (char* argstr) {

	unsigned int i;
	char tmp[READ_BUFFERLEN];
	strcpy(tmp,argstr);

	for (i=0; i<strlen(tmp); i++) {
		if (tmp[i]<=0x20 || tmp[i]>=0x7f) {
			tmp[i]='\0';
			break;
		}
	}

	if (!strcasecmp(tmp,":version"))
		return PARSESTATE_VERSION;
	else if (!strcasecmp(tmp,":name"))
		return PARSESTATE_NAME;
	else if (!strcasecmp(tmp,":units"))
		return PARSESTATE_UNITS;
	else if (!strcasecmp(tmp,":documentation"))
		return PARSESTATE_DOCS;
	else if (!strcasecmp(tmp,":root"))
		return PARSESTATE_ROOT;
	else if (!strcasecmp(tmp,":bonedata"))
		return PARSESTATE_BONEDATA;
	else if (!strcasecmp(tmp,":hierarchy"))
		return PARSESTATE_HIERARCHY;
	else if (!strcasecmp(tmp,":degrees"))
		return PARSESTATE_DEGREES;


	return PARSESTATE_UNKNOWN;
}

void trim(char* argstr) {

	int len=strlen(argstr)+1;
	char* tmpstr=(char*)malloc(len);
	char* origtmpstr=tmpstr;
	int i;
	int offsetL;

	strcpy(tmpstr,argstr);

	offsetL=0;
	for (i=0; i<strlen(tmpstr); i++) {
		if (tmpstr[i]<=0x20 || tmpstr[i]>=0x7f) {
			offsetL++;
		}
		else {
			break;
		}
	}
	tmpstr+=offsetL;

	for (i=strlen(tmpstr); i>=0; i--) {
		if (tmpstr[i]<=0x20 || tmpstr[i]>=0x7f) {
			tmpstr[i]='\0';
		}
		else {
			break;
		}
	}

	strcpy(argstr,tmpstr);
	free (origtmpstr);

}

int nextwht (char* argstr) {

	unsigned int i;

	for (i=0; i<strlen(argstr); i++) {
		if (argstr[i]<=0x20 || argstr[i]>=0x7f) {
			break;
		}
	}

	return i;

}

int decode_dummyfield(FILE* fp) {

	int  newps;
	char buf[READ_BUFFERLEN];

	while (!feof(fp)) {
		fgets(buf,READ_BUFFERLEN,fp);
		trim(buf);
		
		newps=changemode(buf);
		if (newps) {
			/* Mode change - leave this decoder */
			return newps;
		}
	}	

	return PARSESTATE_UNKNOWN;

}
int decode_bonedata(FILE* fp, BONE** bones, int* bone_ctr) {

	int  newps;
	int  operand;
	BONE thisbone;
	char buf[READ_BUFFERLEN];
	char firstword[READ_BUFFERLEN];
	char strbuf[READ_BUFFERLEN];
	BONE* tmpbones;

	while (!feof(fp)) {
		fgets(buf,READ_BUFFERLEN,fp);
		trim(buf);
		newps=changemode(buf);
		if (newps) {
			/* Mode change - leave this decoder */
			return newps;
		}
		/* Decode */
		
		memset(firstword,0,sizeof(READ_BUFFERLEN));
		operand=nextwht(buf);
		memcpy(firstword,buf,operand);
		firstword[operand]='\0';
		trim(firstword);

		if (!strcasecmp(firstword,"begin")) {
			thisbone.id=*bone_ctr;
			thisbone.direction.x=thisbone.direction.y=thisbone.direction.z=0;
			thisbone.length=0;
			thisbone.name=NULL;
			thisbone.axis.x=thisbone.axis.y=thisbone.axis.z=0;
			thisbone.xyzflags=0;
			thisbone.children=NULL;
			thisbone.children_enum=0;
			thisbone.parent=NULL;
		}
		else if (!strcasecmp(firstword,"end")) {
			tmpbones=(BONE*)calloc((*bone_ctr)+1,sizeof(BONE));
			memcpy(tmpbones,(*bones),sizeof(BONE)*(*bone_ctr));
			free(*bones);
			tmpbones[(*bone_ctr)++]=thisbone;
			*bones=tmpbones;
		}
		else if (!strcasecmp(firstword,"id")) {
			/*sscanf(buf+operand,"%d",&(thisbone.id));* - disable, must use internal id now */
		}
		else if (!strcasecmp(firstword,"direction")) {
			sscanf(buf+operand,"%f %f %f",&(thisbone.direction.x),&(thisbone.direction.y),&(thisbone.direction.z));
		}
		else if (!strcasecmp(firstword,"axis")) {
			sscanf(buf+operand,"%f %f %f",&(thisbone.axis.x),&(thisbone.axis.y),&(thisbone.axis.z));
		}
		else if (!strcasecmp(firstword,"length")) {
			sscanf(buf+operand,"%f",&(thisbone.length));
		}
		else if (!strcasecmp(firstword,"name")) {
			strcpy(strbuf,buf+operand);
			trim(strbuf);
			thisbone.name=(char*)malloc(strlen(strbuf)+1);
			strcpy(thisbone.name,strbuf);
		}
		else if (!strcasecmp(firstword,"dof")) {
			strcpy(strbuf,buf+operand);
			trim(strbuf);
			thisbone.xyzflags=0;
			/* Check strbuf for rx ry and rz indicators */
			if (strstr(strbuf,"rx")) {
				thisbone.xyzflags|=DOF_FLAG_RX;
			}
			if (strstr(strbuf,"ry")) {
				thisbone.xyzflags|=DOF_FLAG_RY;
			}
			if (strstr(strbuf,"rz")) {
				thisbone.xyzflags|=DOF_FLAG_RZ;
			}
		}

	}	

	return PARSESTATE_UNKNOWN;

}


int decode_root(FILE* fp, SKELETON* skel) {

	int  newps;
	int  operand;
	char buf[READ_BUFFERLEN];
	char firstword[READ_BUFFERLEN];

	while (!feof(fp)) {
		fgets(buf,READ_BUFFERLEN,fp);
		trim(buf);
		newps=changemode(buf);
		if (newps) {
			/* Mode change - leave this decoder */
			return newps;
		}
		/* Decode */
		
		memset(firstword,0,sizeof(READ_BUFFERLEN));
		operand=nextwht(buf);
		memcpy(firstword,buf,operand);
		firstword[operand]='\0';
		trim(firstword);

		if (!strcasecmp(firstword,"orientation")) {
			sscanf(buf+operand,"%f %f %f",&(skel->init_orientation.x),&(skel->init_orientation.y),&(skel->init_orientation.z));
		}
		else if (!strcasecmp(firstword,"position")) {
			sscanf(buf+operand,"%f %f %f",&(skel->init_position.x),&(skel->init_position.y),&(skel->init_position.z));
		}

	}	

	return PARSESTATE_UNKNOWN;

}


int getboneindex (BONE* bones, int bone_ctr, char* bonename) {

	int boneid;

	for (boneid=0; boneid<bone_ctr; boneid++) {
		if (!strcasecmp(bones[boneid].name,bonename))
			break;
	}

	if (boneid==bone_ctr)
		return -1;
	else
		return boneid;

}

int decode_hierarchy(FILE* fp, BONE* bones, int bone_ctr, SKELETON* skel) {

	int  newps;
	int  parentid,boneid;
	int  operand;
	char buf[READ_BUFFERLEN];
	char firstword[READ_BUFFERLEN];

	int*	children_enum;
	BONE***	children;

	skel->bonearray=bones;
	skel->bonearray_enum=bone_ctr;

	while (!feof(fp)) {
		fgets(buf,READ_BUFFERLEN,fp);
		trim(buf);
		newps=changemode(buf);
		if (newps) {
			/* Mode change - leave this decoder */
			return newps;
		}
		/* Decode */
		memset(firstword,0,sizeof(READ_BUFFERLEN));
		operand=nextwht(buf);
		memcpy(firstword,buf,operand);
		firstword[operand]='\0';
		trim(firstword);

		if (!strcasecmp("begin",firstword) || !strcasecmp("end",firstword))
			continue;

		/* Which node? */
		if (!strcasecmp("root",firstword)) {
			children=&(skel->children);
			children_enum=&(skel->children_enum);
			parentid=-1;
		}
		else {
			parentid=getboneindex(bones,bone_ctr,firstword);
			if (parentid==-1) {
				printf("WARNING: Skeleton hierarchy - undefined bone name [%s] as parent\n",firstword);
				continue;
			}
			else {	
				children=&(bones[parentid].children);
				children_enum=&(bones[parentid].children_enum);
			}
		}
		/* Add on all specified children */
		while (1) {
			memmove(buf,buf+operand,READ_BUFFERLEN-operand);
			trim(buf);
			operand=nextwht(buf);
			memset(firstword,0,sizeof(READ_BUFFERLEN));
			memcpy(firstword,buf,operand);
			firstword[operand]='\0';
			trim(firstword);
			if (strlen(firstword)==0)
				break;
			/* Firstword contains name of child */
			boneid=getboneindex(bones,bone_ctr,firstword);
			if (boneid==-1) {
				printf("WARNING:  Skeleton hierarchy - undefined bone name [%s] as child\n",firstword);
			}
			else {
				*children=(BONE**)realloc(*children,sizeof(BONE*)*((*children_enum)+1));
				(*children)[(*children_enum)++]=bones+boneid;
				if (parentid>-1)
					bones[boneid].parent=bones+parentid;
				else
					bones[boneid].parent=NULL;
			}
		}
	}	

	return PARSESTATE_UNKNOWN;

}

void parser_debugskeletonTree(SKELETON* skel) {

	int i=0;

	printf("root\n");
	for (i=0; i<skel->children_enum; i++) {
		debugskeletonTree_recur(skel->children[i],1);
		if (skel->children[i]->parent!=NULL) {
			printf("Hierarchy FAILED %p\n",skel->children[i]->parent);
			exit(1);
		}
	}
}

void debugskeletonTree_recur(BONE* bn, int recurctr) {

	int i;

	for (i=0; i<recurctr; i++) {
		printf("  ");
	}
	printf("%s\n",bn->name);
	for (i=0; i<bn->children_enum; i++) {
		debugskeletonTree_recur(bn->children[i],recurctr+1);
		if (bn->children[i]->parent!=bn) {
			printf("HIERARCHY FAILED 2\n");
			exit(1);
		}

	}

}

void parser_free_skeleton(SKELETON* skel) {

	int i=0;

	for (i=0; i<skel->children_enum; i++) {
		parser_free_skeleton_helper(skel->children[i]);
	}
	free(skel->bonearray);
	free(skel);

}

void parser_free_skeleton_helper(BONE* bn) {

	int i=0;

	for (i=0; i<bn->children_enum; i++) {
		if (bn->children[i]) {
			parser_free_skeleton_helper(bn->children[i]);
		}
	}
	free(bn->name);

}

MOCAP*	parser_loadMocap(char* argFilename, SKELETON* skel) {


	FILE* fp;					/* file to be parsed */
	int	  ps;				/* parser state */
	char  buf [READ_BUFFERLEN];	/* parse buffer */
	MOCAP* momodel;				/* the skeleton */
	
	if (!(fp=fopen(argFilename,"rt")))
		return NULL;

	momodel=(MOCAP*)calloc(1,sizeof(MOCAP));
	momodel->bones_orient=NULL;
	momodel->frames_enum=0;
	momodel->root_pos=NULL;
	momodel->root_orient=NULL;
	
	ps=PARSESTATE_UNKNOWN;
	while (!feof(fp)) {

		if (ps==PARSESTATE_UNKNOWN) {
			fgets(buf,READ_BUFFERLEN,fp);
			trim(buf);
			ps=changemode(buf);
		}

		
		/* Handle modes */
		switch (ps) {
			case PARSESTATE_DEGREES:
				ps=decode_degrees(fp,momodel,skel);
				break;
			default:
				ps=decode_dummyfield(fp);
				break;
		}

	}

	
	fclose(fp);
	return momodel;


}


int decode_degrees(FILE* fp, MOCAP* mocap, SKELETON* skel) {

	int	 frmnum=-1;
	int  newps;
	int  boneid;
	int  operand;
	char buf[READ_BUFFERLEN];
	char firstword[READ_BUFFERLEN];
	int	 psd,idx;
	float	 r[3];


	while (!feof(fp)) {
		fgets(buf,READ_BUFFERLEN,fp);
		trim(buf);
		newps=changemode(buf);
		if (newps) {
			/* Mode change - leave this decoder */
			return newps;
		}
		/* Decode */
		memset(firstword,0,sizeof(READ_BUFFERLEN));
		operand=nextwht(buf);
		memcpy(firstword,buf,operand);
		firstword[operand]='\0';
		trim(firstword);

		if (atoi(firstword)>0) {
			/* New frame */
			frmnum=atoi(firstword);
			if (frmnum>mocap->frames_enum) {
				mocap->bones_orient=(POINT3D**)realloc(mocap->bones_orient,sizeof(POINT3D*)*frmnum);
				mocap->root_orient=(POINT3D*)realloc(mocap->root_orient,sizeof(POINT3D)*frmnum);
				mocap->root_pos=(POINT3D*)realloc(mocap->root_pos,sizeof(POINT3D)*frmnum);
				mocap->frames_enum=frmnum;
				mocap->bones_orient[frmnum-1]=(POINT3D*)calloc(skel->bonearray_enum,sizeof(POINT3D));
				for (boneid=0; boneid<skel->bonearray_enum; boneid++) {
					mocap->bones_orient[frmnum-1][boneid].x=0;
					mocap->bones_orient[frmnum-1][boneid].y=0;
					mocap->bones_orient[frmnum-1][boneid].z=0;
				}
			}
			continue;
		}
		else {
			if (frmnum==-1) {
				printf("FATAL:  Data out of sync with frame number\n");
				return PARSESTATE_UNKNOWN;
			}
		}


		/* Which node? */
		if (!strcasecmp("root",firstword)) {
			sscanf(buf+operand,"%f %f %f %f %f %f\n",
				&(mocap->root_pos[frmnum-1].x),&(mocap->root_pos[frmnum-1].y),&(mocap->root_pos[frmnum-1].z),
				&(mocap->root_orient[frmnum-1].x),&(mocap->root_orient[frmnum-1].y),&(mocap->root_orient[frmnum-1].z));
		}
		else {
			boneid=getboneindex(skel->bonearray,skel->bonearray_enum,firstword);
			if (boneid==-1) {
				printf("WARNING: MOCAP file - undefined bone name [%s] in datastream\n",firstword);
				continue;
			}
			else {	
				r[0]=r[1]=r[2]=0;
				psd=sscanf(buf+operand,"%f %f %f",&(r[0]),&(r[1]),&(r[2]));

				idx=0;
				if (skel->bonearray[boneid].xyzflags & DOF_FLAG_RX) {
					mocap->bones_orient[frmnum-1][boneid].x=r[idx++];
				}
				if (skel->bonearray[boneid].xyzflags & DOF_FLAG_RY) {
					mocap->bones_orient[frmnum-1][boneid].y=r[idx++];
				}
				if (skel->bonearray[boneid].xyzflags & DOF_FLAG_RZ) {
					mocap->bones_orient[frmnum-1][boneid].z=r[idx++];
				}
				
			}
		}

	}	

	return PARSESTATE_UNKNOWN;
	

}


void matrix_transform_affine(double m[4][4],
							 double x, double y, 
							 double z, POINT3D* pt) 
{
    pt->x = m[0][0]*x + m[0][1]*y + m[0][2]*z + m[0][3];
    pt->y = m[1][0]*x + m[1][1]*y + m[1][2]*z + m[1][3];
    pt->z = m[2][0]*x + m[2][1]*y + m[2][2]*z + m[2][3];
}


void rotateVector(POINT3D* v, float a, float b, float c)
{
    double Rx[4][4], Ry[4][4], Rz[4][4];

	//Rz is a rotation matrix about Z axis by angle c, same for Ry and Rx
    rotationZ(Rz, c);
    rotationY(Ry, b);
    rotationX(Rx, a);

	//Matrix vector multiplication to generate the output vector v.
    matrix_transform_affine(Rz, v->x, v->y, v->z, v);
    matrix_transform_affine(Ry, v->x, v->y, v->z, v);
    matrix_transform_affine(Rx, v->x, v->y, v->z, v);
}

void rotationZ(double r[][4], float a)
{
    a=a*PI/180.;
    r[0][0]=cos(a); r[0][1]=-sin(a); r[0][2]=0; r[0][3]=0;
    r[1][0]=sin(a); r[1][1]=cos(a);  r[1][2]=0; r[1][3]=0;
    r[2][0]=0;      r[2][1]=0;       r[2][2]=1; r[2][3]=0;
    r[3][0]=0;      r[3][1]=0;       r[3][2]=0; r[3][3]=1;
}

void rotationY(double r[][4], float a)
{
    a=a*PI/180.;
    r[0][0]=cos(a);  r[0][1]=0;       r[0][2]=sin(a); r[0][3]=0;
    r[1][0]=0;       r[1][1]=1;       r[1][2]=0;      r[1][3]=0;
    r[2][0]=-sin(a); r[2][1]=0;       r[2][2]=cos(a); r[2][3]=0;
    r[3][0]=0;       r[3][1]=0;       r[3][2]=0;      r[3][3]=1;
}

void rotationX(double r[][4], float a)
{
    a=a*PI/180.;
    r[0][0]=1;       r[0][1]=0;       r[0][2]=0;       r[0][3]=0;
    r[1][0]=0;       r[1][1]=cos(a);  r[1][2]=-sin(a); r[1][3]=0;
    r[2][0]=0;       r[2][1]=sin(a);  r[2][2]=cos(a);  r[2][3]=0;
    r[3][0]=0;       r[3][1]=0;       r[3][2]=0;       r[3][3]=1;
}

void parser_free_mocap(MOCAP* mocap) {

	int i;

	free (mocap->root_orient);
	free (mocap->root_pos);
	for (i=0; i<mocap->frames_enum; i++) {
		free (mocap->bones_orient[i]);
	}
	free (mocap->bones_orient);



}

