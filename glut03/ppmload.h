#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct RGBColor{
	union {
		struct {
			unsigned char R, G, B;
		};
		unsigned char dens;
	};
};

typedef struct {
	double m[3][3];
} MATRIX;

struct ppmimg{
	int iwidth;
	int iheight;
	int depth;
	int cmode;
	//ascii color(P3):3
	//ascii gray (P2):1
	char magicnamber[10];//テキスト形式カラーとグレイスケールのみ(P3,P2)
	unsigned char* dat;
};

void loadppmimage(char *imagename, struct ppmimg* simg);
void saveppmimage(struct ppmimg* simg, char *imagename);

void setPnmPixel(struct ppmimg* simg, int x, int y, struct RGBColor _rgb);
struct RGBColor getPnmPixel(struct ppmimg* simg, int x, int y);
struct ppmimg *cloneppmimage(struct ppmimg* simg);
struct ppmimg *createppmimage(int width, int height, int mode);

void affinetransform(struct ppmimg* simg, struct ppmimg* dimg, double theta, double xtrans, double ytrans);
void setaffineMatrix(MATRIX *m, double theta, double xtrans, double ytrans);
int MatrixInverse(MATRIX *pM, MATRIX *pOUT);
void MatrixIdentity(MATRIX *pOut);

void loadppmimage(char *imagename, struct ppmimg* simg){
	FILE *fp;
	char buff[255];
	char head[70];
	int imgwidth, imgheight;
	int imagedepth;
	int datasize;
	int datamode = 0;//0:ascii 1:binary
	errno_t error;
	if (error = fopen_s(&fp, imagename, "rt") != 0){
		exit(1);
	}
	fgets(buff, 255, fp);
	while (buff[0] == '#'){
		fgets(buff, 255, fp);
	}
	if (buff[0] == 'P'){
		sprintf_s(simg->magicnamber, "%s", buff);
		simg->magicnamber[2] = '\0';//fgetsは
	}
	fgets(buff, 255, fp);
	while (buff[0] == '#'){
		fgets(buff, 255, fp);
	}
	sscanf_s(buff, "%d %d", &imgwidth, &imgheight);
	fgets(buff, 255, fp);
	while (buff[0] == '#'){
		fgets(buff, 255, fp);
	}
	sscanf_s(buff, "%d", &imagedepth);
	//fgets(buff,255,fp);
	//while(buff[0]=='#'){
	//	fgets(buff,255,fp);
	//}
	simg->iwidth = imgwidth;
	simg->iheight = imgheight;
	simg->depth = imagedepth;

	printf("%d\n", strcmp(simg->magicnamber, "P2"));
	if (strcmp(simg->magicnamber, "P3") == 0){
		simg->cmode = 3;
		datamode = 0;
	}
	else if (strcmp(simg->magicnamber, "P2") == 0){
		simg->cmode = 1;
		datamode = 0;
	}
	else if (strcmp(simg->magicnamber, "P6") == 0){
		simg->cmode = 3;
		datamode = 1;
		fclose(fp);
		errno_t error;
		if (error = fopen_s(&fp, imagename, "rb") != 0){
			exit(1);
		}
		fgets(head, 70, fp);
		if (strncmp(head, "P6", 2)) {
			fprintf(stderr, "%s: Not a raw PPM file\n", imagename);
		}
		int i = 0, w, h, d;
		/* grab the three elements in the header (width, height, maxval). */
		while (i < 3) {
			fgets(head, 70, fp);
			if (head[0] == '#')     /* skip comments. */
				continue;
			if (i == 0)
				i += sscanf_s(head, "%d %d %d", &w, &h, &d);
			else if (i == 1)
				i += sscanf_s(head, "%d %d", &h, &d);
			else if (i == 2)
				i += sscanf_s(head, "%d", &d);
		}
	}

	datasize = (simg->iwidth)*(simg->iheight)*simg->cmode;
	if (simg->dat != NULL)free(simg->dat);
	simg->dat = (unsigned char *)malloc(sizeof(unsigned char)*datasize);
	if (simg->cmode == 3 && datamode == 0){
		for (int i = 0; i<datasize; i += simg->cmode){
			fscanf_s(fp, "%d %d %d", &(simg->dat[i]), &(simg->dat[i + 1]), &(simg->dat[i + 2]));
		}
	}
	else if (simg->cmode == 1 && datamode == 0){
		for (int i = 0; i<datasize; i += simg->cmode){
			fscanf_s(fp, "%d ", &(simg->dat[i]));
		}
	}
	else if (simg->cmode == 3 && datamode == 1){
		fread(simg->dat, sizeof(unsigned char), (simg->iwidth)*(simg->iheight)*simg->cmode, fp);
	}
	else {
		printf("data error \nI don't know this format\n");
		exit(1);
	}
	fclose(fp);
}

struct ppmimg *cloneppmimage(struct ppmimg* simg){
	struct ppmimg *tempimg = (struct ppmimg *)malloc(sizeof(struct ppmimg));
	tempimg->dat = (unsigned char *)malloc(sizeof(unsigned char)*simg->iwidth*simg->iheight*simg->cmode);
	tempimg->cmode = simg->cmode;
	tempimg->depth = simg->depth;
	tempimg->iheight = simg->iheight;
	tempimg->iwidth = simg->iwidth;
	strcpy_s(tempimg->magicnamber, _countof(tempimg->magicnamber), simg->magicnamber);
	for (int j = 0; j<simg->iheight; j++){
		for (int i = 0; i<simg->iwidth; i++){
			struct RGBColor trgb = { 0 };
			setPnmPixel(tempimg, i, j, trgb);
		}
	}
	return(tempimg);
}

struct ppmimg *createppmimage(int width, int height, int mode){
	struct ppmimg *tempimg = (struct ppmimg *)malloc(sizeof(struct ppmimg));
	tempimg->dat = (unsigned char *)malloc(sizeof(unsigned char)*width*height*mode);
	tempimg->cmode = mode;
	tempimg->depth = 255;
	tempimg->iheight = height;
	tempimg->iwidth = width;
	if (mode == 1)sprintf_s(tempimg->magicnamber, "P2");
	else sprintf_s(tempimg->magicnamber, "P3");
	for (int j = 0; j<height; j++){
		for (int i = 0; i<width; i++){
			struct RGBColor trgb;
			if (mode == 1)trgb.dens = 0;
			else{
				trgb.R = 0; trgb.G = 0; trgb.B = 0;
			}
			setPnmPixel(tempimg, i, j, trgb);
		}
	}
	return(tempimg);
}

void saveppmimage(ppmimg *simg, char *imagename){
	FILE *ofp;
	fopen_s(&ofp, imagename, "w");
	fprintf_s(ofp, "%s\n%d %d\n%d\n", simg->magicnamber, simg->iwidth, simg->iheight, simg->depth);
	for (int j = 0; j<simg->iheight; j++){
		for (int i = 0; i<simg->iwidth; i++){
			if (simg->cmode == 3){
				struct RGBColor trgb = getPnmPixel(simg, i, j);
				fprintf(ofp, "%d %d %d", trgb.R, trgb.G, trgb.B);
				if (i != simg->iwidth - 1)fprintf(ofp, " ");
			}
			else{
				struct RGBColor trgb = getPnmPixel(simg, i, j);
				fprintf(ofp, "%d ", trgb.dens);
			}
		}
		fprintf_s(ofp, "\n");
	}
	fclose(ofp);
}


void setPnmPixel(struct ppmimg* simg, int x, int y, struct RGBColor _rgb){
	if (simg->cmode == 3){
		int rindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode;
		int gindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode + 1;
		int bindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode + 2;
		simg->dat[rindex] = _rgb.R;
		simg->dat[gindex] = _rgb.G;
		simg->dat[bindex] = _rgb.B;
	}
	else{
		int gindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode;
		simg->dat[gindex] = _rgb.dens;
	}
}

struct RGBColor getPnmPixel(struct ppmimg* simg, int x, int y){
	struct RGBColor colordat;
	if (simg->cmode == 3){
		int rindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode;
		int gindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode + 1;
		int bindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode + 2;
		colordat.R = simg->dat[rindex];
		colordat.G = simg->dat[gindex];
		colordat.B = simg->dat[bindex];
	}
	else{
		int gindex = (simg->iwidth)*y*simg->cmode + x*simg->cmode;
		colordat.dens = simg->dat[gindex];
	}
	return colordat;
}

//affine変換
void affinetransform(struct ppmimg* simg, struct ppmimg* dimg, double theta, double xtrans, double ytrans){
	MATRIX af, iaf;
	double rad = theta * 3.14159 / 180.0;
	MatrixIdentity(&af);
	setaffineMatrix(&af, rad, xtrans, ytrans);
	MatrixInverse(&af, &iaf);
	struct RGBColor tRGB;
	double a, b, c, d, e, f;
	a = iaf.m[0][0]; b = iaf.m[1][0];
	c = iaf.m[0][1]; d = iaf.m[1][1];
	e = iaf.m[0][2]; f = iaf.m[1][2];
	for (int j = 0; j<simg->iheight; j++){
		for (int i = 0; i<simg->iwidth; i++){
			double pdx, pdy;
			double tx = (double)i, ty = (double)(j);
			pdx = a*tx + c*ty + e;
			pdy = b*tx + d*ty + f;
			if ((pdx<0 || pdx >= simg->iwidth) || (pdy<0 || pdy >= simg->iheight)){
				if (simg->cmode == 1){
					tRGB.dens = 0;
				}
				else{
					tRGB.R = 0; tRGB.G = 0; tRGB.B = 0;
				}
				setPnmPixel(dimg, i, j, tRGB);
			}
			else{
				tRGB = getPnmPixel(simg, (int)pdx, (int)pdy);
				setPnmPixel(dimg, i, j, tRGB);
			}
		}
	}
}
//affine変換マトリクスの設定
void setaffineMatrix(MATRIX *m, double theta, double xtrans, double ytrans){
	m->m[0][0] = cos(theta); m->m[0][1] = -sin(theta); m->m[0][2] = xtrans;
	m->m[1][0] = sin(theta); m->m[1][1] = cos(theta); m->m[1][2] = ytrans;
	m->m[2][0] = 0; m->m[2][1] = 0; m->m[2][2] = 1;
}

//行列計算用の関数たち
//m[行][列]
//m[0][0] m[0][1] m[0][2]
//m[1][0] m[1][1] m[1][2]
//m[2][0] m[2][1] m[2][2]


void MatrixIdentity(MATRIX *pOut)
{
	int y;
	double *p1;

	for (y = 0; y < 3; y++) {
		p1 = pOut->m[y];
		*p1 = 0.0f;
		*(p1 + 1) = 0.0f;
		*(p1 + 2) = 0.0f;
	}
	pOut->m[0][0] = 1.0f;
	pOut->m[1][1] = 1.0f;
	pOut->m[2][2] = 1.0f;
}


int MatrixInverse(MATRIX *pM, MATRIX *pOUT){
	double detA = pM->m[0][0] * pM->m[1][1] * pM->m[2][2] + pM->m[1][0] * pM->m[2][1] * pM->m[0][2] + pM->m[2][0] * pM->m[0][1] * pM->m[1][2] \
		- pM->m[0][0] * pM->m[2][1] * pM->m[1][2] - pM->m[2][0] * pM->m[1][1] * pM->m[0][2] - pM->m[1][0] * pM->m[0][1] * pM->m[2][2];
	if (detA != 0){
		pOUT->m[0][0] = (pM->m[1][1] * pM->m[2][2] - pM->m[1][2] * pM->m[2][1]) / detA;
		pOUT->m[0][1] = (pM->m[0][2] * pM->m[2][1] - pM->m[0][1] * pM->m[2][2]) / detA;
		pOUT->m[0][2] = (pM->m[0][1] * pM->m[1][2] - pM->m[0][2] * pM->m[1][1]) / detA;

		pOUT->m[1][0] = (pM->m[1][2] * pM->m[2][0] - pM->m[1][0] * pM->m[2][2]) / detA;
		pOUT->m[1][1] = (pM->m[0][0] * pM->m[2][2] - pM->m[0][2] * pM->m[2][0]) / detA;
		pOUT->m[1][2] = (pM->m[0][2] * pM->m[1][0] - pM->m[0][0] * pM->m[1][2]) / detA;

		pOUT->m[2][0] = (pM->m[1][0] * pM->m[2][1] - pM->m[1][1] * pM->m[2][0]) / detA;
		pOUT->m[2][1] = (pM->m[0][1] * pM->m[2][1] - pM->m[0][0] * pM->m[2][1]) / detA;
		pOUT->m[2][2] = (pM->m[0][0] * pM->m[1][1] - pM->m[0][1] * pM->m[1][0]) / detA;
		return 1;
	}
	else{
		return 0;
	}
}


