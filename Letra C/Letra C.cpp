/**
 * @file brazo_robotico.cpp
 * @brief Proyecto final del curso de Gráficas por Computadora (CSI01).
 * @author Diego Flores Chávez.
 */

 /*
 * CONTROLES.
 *
 *	Movimiento de cámara:
 *		- FLECHA ARRIBA    -> Rotación vertical (arriba).
 *		- FLECHA ABAJO     -> Rotación vertical (abajo).
 *		- FLECHA IZQUIERDA -> Rotación horizontal (izquierda).
 *		- FLECHA DERECHA   -> Rotación horizontal (derecha).
 *		- D                -> Movimiento en eje vertical (arriba).
 *		- F                -> Movimiento en eje vertical (abajo).
 *		- A                -> Movimiento en eje horizontal (izquierda).
 *		- S                -> Movimiento en eje horizontal (derecha).
 *		- W                -> Movimiento en eje saliente (positivo).
 *		- Q                -> Movimiento en eje saliente (negativo).
 *
 *	Movimiento de objeto articulado:
 *		- F5               -> Rotación hacia la izquierda (brazo).
 *		- F6               -> Rotación hacia la derecha (brazo).
 *		- F7               -> Rotación hacia la izquierda (antebrazo).
 *		- F8               -> Rotación hacia la derecha (antebrazo).
 *
 *	Movimiento de luz:
 *		- PAGE UP          -> Movimiento en eje horizontal (izquierda).
 *		- PAGE DOWN        -> Movimiento en eje horizontal (derecha).
 *		- F11              -> Movimiento en eje saliente (positivo).
 *		- F12              -> Movimiento en eje saliente (negativo).
 *
 *	Efectos especiales:
 *		- F1               -> Interruptor de efecto 'cull'.
 *		- F2               -> Interruptor de efecto 'depth'.
 *		- F3               -> Interruptor de efecto 'outline'.
 *		- F4               -> Interruptor de efecto 'shading'.
 */

#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>

#define WINDOW_HEIGHT 700
#define WINDOW_WIDTH 700
#define IMAGE_HEIGHT 256
#define IMAGE_WIDTH 256

#define ALPHA 0.75

#define BUFFER_SIZE 512

#define FOREARM	2
#define ARM	1

GLfloat ex = 0.0f;
GLfloat ey = 20.0f;
GLfloat ez = -50.0f;

GLfloat rotation_speed_1;
GLfloat rotation_speed_2;
int rotation_1;
int rotation_2;

GLfloat delta_r = 0.01f;
GLfloat delta = 0.01f;

GLfloat x_rotation = 0.45f;
GLfloat y_rotation = 0.35f;

GLboolean outline = (GLboolean)true;
GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
GLboolean cull = glIsEnabled(GL_CULL_FACE);

GLenum shade_mode = GL_FLAT;

static GLubyte check_image[IMAGE_HEIGHT][IMAGE_WIDTH][4];
static GLuint texture_name[1];

GLfloat shadow_matrix[16];

GLfloat texture_points[2][2][2] = {
	{
		{0.0, 0.0},
		{0.0, 2.0}
	},
	{
		{3.0, 0.0},
		{3.0, 2.0}
	}
};

GLfloat colors[][3] = {
	{ .5,0.0,0.5 },
	{ 1.0,0.0,0.0 },
	{ 1.0,1.0,0.0 },
	{ 0.0,1.0,0.0 },
	{ 0.0,0.0,1.0 },
	{ 1.0,0.0,1.0 },
	{ 1.0,1.0,1.0 },
	{ 0.0,1.0,1.0 }
};

GLfloat control_points[4][4][3] = {
	{
		{-1.5, -1.5, 4.0},
		{-0.5, -3.5, 2.0},
		{0.5, -1.5, -1.0},
		{1.5, -1.5, 2.0}
	},
	{
		{-1.5, -0.5, 1.0},
		{-0.5, -0.5, 3.0},
		{0.5, -0.5, 0.0},
		{1.5, -0.5, 4.0}
	},
	{
		{-1.5, 2.5, 2.0},
		{-0.5, -1.5, 2.0},
		{0.5, 1.5, 0.0},
		{1.5, 1.5, -1.0}
	}
};

GLfloat ambient_light[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat diffuse_light[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat mat_shininess[] = { 50.0f };
GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat floor_position[] = { 0, 1,0, 20 };
GLfloat light_position[] = { 0., 8., 0. , 1.0 };

/**
 * @brief Genera la imagen a cuadros utilizada como textura en la aplicación.
 */
void makeCheckImages() {
	int i, j, c;

	for (i = 0; i < IMAGE_HEIGHT; i++) {
		for (j = 0; j < IMAGE_WIDTH; j++) {
			c = ((((i & 0x12) == 0) ^ ((j & 0x12)) == 0)) * 255;
			check_image[i][j][0] = (GLubyte)c;
			check_image[i][j][3] = (GLubyte)255;
		}
	}
}

/**
 * @brief Inicializa los parámetros y configuraciones iniciales de OpenGL.
 */
void Initialize() {
	rotation_speed_1 = 3;
	rotation_speed_2 = 3;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &texture_points[0][0][0]);
	glEnable(GL_MAP2_TEXTURE_COORD_2);

	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &control_points[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3);

	glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);

	glFrontFace(GL_CW);

	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_BACK, GL_FILL);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
	glMateriali(GL_FRONT, GL_SHININESS, 128);
	makeCheckImages();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, texture_name);
	glBindTexture(GL_TEXTURE_2D, texture_name[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, check_image);

	glFrontFace(GL_CW);
}

/**
 * @brief Normaliza un vector en 3D.
 *
 * @param v El vector a normalizar.
 */
void Normalize(GLfloat v[3]) {
	GLfloat d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (d == 0.0) {
		exit(100);
		return;
	}
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
}

/**
 * @brief Calcula el producto cruz entre dos vectores en 3D y normaliza el resultado.
 *
 * @param v1     El primer vector.
 * @param v2     El segundo vector.
 * @param out    El vector resultante del producto cruz.
 */
void CrossProduct(GLfloat v1[3], GLfloat v2[3], GLfloat out[3]) {
	out[0] = v1[1] * v2[2] - v1[2] * v2[1];
	out[1] = v1[2] * v2[0] - v1[0] * v2[2];
	out[2] = v1[0] * v2[1] - v1[1] * v2[0];
	Normalize(out);
}

/**
 * @brief Calcula el vector normal a partir de tres puntos en 3D.
 *
 * @param v0     El primer punto.
 * @param v1     El segundo punto.
 * @param v2     El tercer punto.
 * @param norm   El vector normal resultante.
 */
void GetNormalVector(GLfloat v0[3], GLfloat v1[3], GLfloat v2[3], GLfloat norm[3]) {
	GLfloat d1[3], d2[3];

	for (int j = 0; j < 3; j++) {
		d1[j] = v0[j] - v1[j];
		d2[j] = v1[j] - v2[j];
	}

	CrossProduct(d1, d2, norm);
}

/**
 * @brief Crea una matriz de sombra utilizando la ecuación del plano, la posición de la luz y la matriz de destino.
 *
 * @param vPlaneEquation   La ecuación del plano de sombra.
 * @param vLightPos        La posición de la luz.
 * @param destMat          La matriz de destino para almacenar el resultado.
 */
void MakeShadowMatrix(GLfloat vPlaneEquation[], GLfloat vLightPos[], GLfloat destMat[]) {
	GLfloat dot;

	dot = vPlaneEquation[0] * vLightPos[0] + vPlaneEquation[1] * vLightPos[1] + vPlaneEquation[2] * vLightPos[2] + vPlaneEquation[3] * vLightPos[3];

	destMat[0] = dot - vLightPos[0] * vPlaneEquation[0];
	destMat[4] = 0.0f - vLightPos[0] * vPlaneEquation[1];
	destMat[8] = 0.0f - vLightPos[0] * vPlaneEquation[2];
	destMat[12] = 0.0f - vLightPos[0] * vPlaneEquation[3];

	destMat[1] = 0.0f - vLightPos[1] * vPlaneEquation[0];
	destMat[5] = dot - vLightPos[1] * vPlaneEquation[1];
	destMat[9] = 0.0f - vLightPos[1] * vPlaneEquation[2];
	destMat[13] = 0.0f - vLightPos[1] * vPlaneEquation[3];

	destMat[2] = 0.0f - vLightPos[2] * vPlaneEquation[0];
	destMat[6] = 0.0f - vLightPos[2] * vPlaneEquation[1];
	destMat[10] = dot - vLightPos[2] * vPlaneEquation[2];
	destMat[14] = 0.0f - vLightPos[2] * vPlaneEquation[3];

	destMat[3] = 0.0f - vLightPos[3] * vPlaneEquation[0];
	destMat[7] = 0.0f - vLightPos[3] * vPlaneEquation[1];
	destMat[11] = 0.0f - vLightPos[3] * vPlaneEquation[2];
	destMat[15] = dot - vLightPos[3] * vPlaneEquation[3];
}

/**
 * @brief Dibuja un cuadrado en el plano XY.
 */
void Square() {
	glBegin(GL_POLYGON);

	GLfloat v1[] = { 1, 1, 0 }, v2[] = { 1, -1, 0 }, v3[] = { -1, -1, 0 }, norm[3];
	GetNormalVector(&v1[0], &v3[0], &v2[0], norm);
	glNormal3fv(norm);

	glVertex2f(1., 1.);
	glVertex2f(1., -1.);
	glVertex2f(-1., -1.);
	glVertex2f(-1., 1.);
	glEnd();
}

/**
 * @brief Dibuja un cubo sin color.
 */
void ColorlessCube() {
	glPushMatrix();
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glPushMatrix();
	glRotatef(90., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glPushMatrix();
	glRotatef(180., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glPushMatrix();
	glRotatef(270., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glPushMatrix();
	glRotatef(90., 1., 0., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90., 1., 0., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();
}

/**
 * @brief Dibuja un cubo con un color específico para cada cara.
 *        Las caras están coloreadas con el tercer color de la paleta de colores.
 */
void ColorCubeOne() {
	glColor3fv(colors[4]);
	glPushMatrix();
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[4]);
	glPushMatrix();
	glRotatef(90., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[4]);
	glPushMatrix();
	glRotatef(180., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[4]);
	glPushMatrix();
	glRotatef(270., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[4]);
	glPushMatrix();
	glRotatef(90., 1., 0., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[4]);
	glPushMatrix();
	glRotatef(-90., 1., 0., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();
}

/**
 * @brief Dibuja un cubo con un color específico para cada cara.
 *        Las caras están coloreadas con el segundo color de la paleta de colores.
 */
void ColorCubeTwo() {
	glColor3fv(colors[1]);
	glPushMatrix();
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[1]);
	glPushMatrix();
	glRotatef(90., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[1]);
	glPushMatrix();
	glRotatef(180., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[1]);
	glPushMatrix();
	glRotatef(270., 0., 1., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[1]);
	glPushMatrix();
	glRotatef(90., 1., 0., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();

	glColor3fv(colors[1]);
	glPushMatrix();
	glRotatef(-90., 1., 0., 0);
	glTranslatef(0, 0, 1.);
	Square();
	glPopMatrix();
}

/**
 * @brief Dibuja un brazo sin colores.
 *        El brazo consta de un cubo grande como antebrazo y un cubo pequeño como antebrazo.
 *        También se dibujan tres pequeños cubos para representar los dedos.
 */
void ArmWithoutColor() {
	glPushName(ARM);

	glPushMatrix();
	glRotatef(rotation_1, 0.0, 0.0, 1.0);
	glTranslatef(1.0, 0.0, 0.0);
	glPushMatrix();
	glScalef(2, 0.5, 0.5);
	ColorlessCube();
	glPopMatrix();

	glPushName(FOREARM);

	glTranslated(2, 0.0, 0.0);
	glRotatef(rotation_2, 0.0, 0.0, 1.0);
	glTranslatef(1.0, 0.0, 0.0);
	glPushMatrix();
	glScalef(1.0, 0.5, 0.5);
	ColorlessCube();
	glPopMatrix();

	glPopMatrix();
}

/**
 * @brief Dibuja un brazo con colores en cada componente.
 *        El brazo consta de un cubo grande como antebrazo y un cubo pequeño como antebrazo.
 *        También se dibujan tres pequeños cubos para representar los dedos.
 */
void ColorArm() {
	glPushName(ARM);

	glPushMatrix();
	glRotatef(rotation_1, 0.0, 0.0, 1.0);
	glTranslatef(1.0, 0.0, 0.0);
	glPushMatrix();
	glScalef(2, 0.5, 0.5);
	ColorCubeOne();
	glPopMatrix();

	glPushName(FOREARM);

	glTranslated(2, 0.0, 0.0);
	glRotatef(rotation_2, 0.0, 0.0, 1.0);
	glTranslatef(1.0, 0.0, 0.0);
	glPushMatrix();
	glScalef(1.0, 0.5, 0.5);
	ColorCubeTwo();
	glPopMatrix();

	glPopMatrix();
}

/**
 * @brief Dibuja una superficie NURBS con textura.
 *        La superficie se define mediante una malla evaluada con NURBS y se aplica una textura.
 */
void NURBS() {
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_AUTO_NORMAL);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, texture_name[0]);

	glPushMatrix();
	glTranslatef(-10, 0, 3);
	glScalef(1, 1, 1);
	glRotatef(0, 1, 0, 0);
	glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_AUTO_NORMAL);
}

/**
 * @brief Dibuja una superficie NURBS sin colores ni texturas.
 *        La superficie se define mediante una malla evaluada con NURBS.
 */
void NURBSWithoutColor() {
	glPushMatrix();
	glTranslatef(-10, 0, 3);
	glScalef(1, 1, 1);
	glRotatef(0, 1, 0, 0);
	glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	glPopMatrix();
}

/**
 * @brief Crea los objetos sin colores en la escena.
 *        Dibuja un brazo sin colores y una superficie NURBS sin colores ni texturas.
 */
void CreateObjectsWithoutColor() {
	ArmWithoutColor();
	NURBSWithoutColor();
}

/**
 * @brief Crea los objetos en la escena.
 *        Dibuja un brazo con colores y una superficie NURBS con textura.
 */
void CreateObjects() {
	ColorArm();
	NURBS();
}

/**
 * @brief Dibuja líneas horizontales y verticales formando una cuadrícula.
 */
void Lines() {
	glColor3f(0.0, 0., 0.);
	glBegin(GL_LINES);
	for (int i = 0; i < 10; i++) {
		glVertex2f(-1. + i * 2. / 9., 1.);
		glVertex2f(-1. + i * 2. / 9., -1.);
		glVertex2f(1, -1. + i * 2. / 9.);
		glVertex2f(-1., -1. + i * 2. / 9.);
	}
	glEnd();
}

/**
 * @brief Dibuja el suelo de la escena.
 *        El suelo es un cuadrado con un color gris y puede tener una cuadrícula.
 */
void Floor() {
	glColor4f(0.66, 0.66, 0.66, ALPHA);
	glPushMatrix();
	glTranslatef(0, -floor_position[3], 0);
	glRotatef(-90., 1., 0, 0);
	glScalef(40., 40, 40.);
	Square();
	glDisable(GL_DEPTH_TEST);
	Lines();
	if (depth) {
		glEnable(GL_DEPTH_TEST);
	}
	glPopMatrix();
}

/**
 * @brief Dibuja una fuente de luz en la escena.
 *        La fuente de luz se representa como una esfera de color.
 */
void Light() {
	glColor3fv(colors[2]);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glPushMatrix();
	glTranslatef(light_position[0], light_position[1], light_position[2]);
	glutSolidSphere(0.8, 20, 20);
	glPopMatrix();
}

/**
 * @brief Dibuja los objetos con su sombra proyectada.
 *        Aplica una matriz de sombra a los objetos y los dibuja sin colores.
 * @param datosdeLuz Datos de la fuente de luz para calcular la sombra.
 * @param datosdePlano Datos del plano sobre el cual se proyecta la sombra.
 */
void ShadowObjects(GLfloat datosdeLuz[], GLfloat datosdePlano[]) {
	glColor4f(0.33, 0.33, 0.33, ALPHA);
	glPushMatrix();
	MakeShadowMatrix(datosdePlano, datosdeLuz, shadow_matrix);
	glMultMatrixf(shadow_matrix);
	CreateObjectsWithoutColor();
	glPopMatrix();
}

/**
 * @brief Configura el stencil para generar efectos de sombra.
 *        Configura el stencil para renderizar solo en áreas donde la sombra se proyecta en el suelo.
 */
void StencilConfiguration() {
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glEnable(GL_STENCIL_TEST);

	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);

	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	Floor();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

/**
 * @brief Renderiza la escena en el modo especificado.
 * @param mode Modo de renderizado (GL_RENDER para renderizado normal).
 *        Si mode es GL_RENDER, se realiza una nueva proyección y se establece la vista de la cámara.
 *        Si mode no es GL_RENDER, se mantienen la proyección y la vista actuales.
 */
void RenderScene(int mode) {
	glMatrixMode(GL_PROJECTION);
	if (mode == GL_RENDER) {
		glLoadIdentity();
	}

	gluPerspective(60.0, 1.0, 1.5, 500.0);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	int iPivot = 1;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (cull) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	if (depth) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	if (outline) {
		glPolygonMode(GL_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_BACK, GL_FILL);
	}

	glPushMatrix();

	glRotatef(x_rotation, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rotation, 0.0f, 1.0f, 0.0f);

	StencilConfiguration();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LIGHTING);
	Floor();
	glFrontFace(GL_CCW);
	glPushMatrix();
	glTranslatef(0., -floor_position[3], 0.);
	glScalef(1., -1., 1.);
	glTranslatef(0., floor_position[3], 0.);
	Light();
	glEnable(GL_LIGHTING);
	CreateObjects();
	glDisable(GL_LIGHTING);
	glPopMatrix();
	glDisable(GL_DEPTH_TEST);

	ShadowObjects(light_position, floor_position);
	if (depth)  glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glDisable(GL_STENCIL_TEST);


	Light();
	glEnable(GL_LIGHTING);
	CreateObjects();
	glDisable(GL_LIGHTING);

	glPopMatrix();

	glutSwapBuffers();
}

/**
 * @brief Analiza el nombre de un objeto seleccionado y realiza acciones en consecuencia.
 * @param name Nombre del objeto seleccionado.
 *        Si el nombre es ARM, se incrementa la velocidad de rotación del brazo.
 *        Si el nombre es FOREARM, se incrementa la velocidad de rotación del antebrazo.
 */
void AnalizeName(int name) {
	int i;

	if (name == ARM) {
		for (i = 0; i < 3; i++) {
			rotation_speed_1 += 1;
		}

	}
	else if (name == FOREARM) {
		for (i = 0; i < 3; i++) {
			rotation_speed_2 += 1;
		}
	}

	glutPostRedisplay();
}

/**
 * @brief Procesa los resultados de la selección de objetos.
 * @param hits Número de hits (objetos seleccionados).
 * @param buffer Búfer que contiene los nombres de los objetos seleccionados.
 *        Analiza los nombres de los objetos seleccionados y realiza acciones en consecuencia.
 *        Llama a la función AnalizeName() para cada nombre de objeto seleccionado.
 */
void ProcessesHits(GLint hits, GLuint buffer[]) {
	if (hits == 0) {
		Initialize();
		return;
	}

	unsigned int i, j;
	GLuint names;
	GLuint* pointer;

	pointer = (GLuint*)buffer;
	for (i = 0; i < hits; i++) {
		names = *pointer;
		pointer += 3;
		for (j = 0; j < names; j++)
			AnalizeName(*pointer++);
	}
}

/**
 * @brief Realiza una acción cuando se selecciona un objeto.
 * @param button Botón del mouse que se presionó.
 * @param state Estado del botón del mouse.
 * @param x Coordenada x de la posición del mouse.
 * @param y Coordenada y de la posición del mouse.
 *        Si el botón presionado es el botón izquierdo del mouse en el estado de presionado,
 *        se realiza la selección de objetos en la posición del mouse.
 *        Utiliza la función gluPickMatrix para establecer una región de selección y llama a RenderScene(GL_SELECT).
 *        Procesa los resultados de la selección llamando a ProcessesHits() y solicita una actualización de la ventana con glutPostRedisplay().
 */
void SelectAction(int button, int state, int x, int y) {
	GLuint select_buffer[BUFFER_SIZE];
	GLint viewport[4];
	GLint hits;

	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) {
		return;
	}

	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(BUFFER_SIZE, select_buffer);

	(void)glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport);

	RenderScene(GL_SELECT);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode(GL_RENDER);
	ProcessesHits(hits, select_buffer);
	glutPostRedisplay();

}

/**
 * @brief Realiza una acción especial cuando se presiona una tecla especial del teclado.
 * @param key Tecla especial presionada.
 * @param x Coordenada x de la posición del mouse.
 * @param y Coordenada y de la posición del mouse.
 *        Realiza diversas acciones en función de la tecla especial presionada:
 *        - Cambia la posición de la cámara (arriba, abajo, izquierda, derecha).
 *        - Cambia la configuración de renderizado (culling, profundidad, contorno, sombreado).
 *        - Modifica la velocidad de rotación de los objetos.
 *        - Modifica la posición de la luz.
 *        Solicita una actualización de la ventana con glutPostRedisplay().
 */
void SpecialKeys(int key, int x, int y) {
	GLfloat dx, dy, dz;

	switch (key) {
	case GLUT_KEY_UP: {
		dy = -ez;
		dz = ey;
		GLfloat s = sqrtf(ey * ey + ex * ex + ez * ez);
		ey += delta * dy;
		ez += delta * dz;
		GLfloat s1 = sqrtf(ey * ey + ex * ex + ez * ez) / s;
		ey /= s1;
		ex /= s1;
		ex /= s1;
	}
					break;

	case GLUT_KEY_DOWN: {
		dy = -ez;
		dz = ey;
		GLfloat s = sqrtf(ey * ey + ex * ex + ez * ez);
		ey -= delta * dy;
		ez -= delta * dz;
		GLfloat s1 = sqrtf(ey * ey + ex * ex + ez * ez) / s;
		ey /= s1;
		ex /= s1;
		ex /= s1;
	}
					  break;

	case GLUT_KEY_LEFT: {
		dx = -ez;
		dz = ex;
		GLfloat s = sqrtf(ex * ex + ey * ey + ez * ez);
		ex += delta * dx;
		ez += delta * dz;
		GLfloat s1 = sqrtf(ex * ex + ey * ey + ez * ez) / s;
		ex /= s1;
		ey /= s1;
		ey /= s1;
	}
					  break;

	case GLUT_KEY_RIGHT: {
		dx = -ez;
		dz = ex;
		GLfloat s = sqrtf(ex * ex + ey * ey + ez * ez);
		ex -= delta * dx;
		ez -= delta * dz;
		GLfloat s1 = sqrtf(ex * ex + ey * ey + ez * ez) / s;
		ex /= s1;
		ey /= s1;
		ey /= s1;
	}
					   break;

	case GLUT_KEY_F1:
		cull = !cull;
		break;

	case GLUT_KEY_F2:
		depth = !depth;
		break;

	case GLUT_KEY_F3:
		outline = !outline;
		break;

	case GLUT_KEY_F4:
		if (shade_mode == GL_FLAT) {
			shade_mode = GL_SMOOTH;
		}
		else {
			if (shade_mode == GL_SMOOTH) {
				shade_mode = GL_FLAT;
			}
		}
		glShadeModel(shade_mode);
		break;

	case GLUT_KEY_F5:
		rotation_1 = (rotation_1 - (int)rotation_speed_1) % 360;
		break;

	case GLUT_KEY_F6:
		rotation_1 = (rotation_1 + (int)rotation_speed_1) % 360;
		break;

	case GLUT_KEY_F7:
		rotation_2 = (rotation_2 - (int)rotation_speed_2) % 360;
		break;

	case GLUT_KEY_F8:
		rotation_2 = (rotation_2 + (int)rotation_speed_2) % 360;
		break;

	case GLUT_KEY_F11:
		light_position[2] += 0.5;
		break;

	case GLUT_KEY_F12:
		light_position[2] -= 0.5;
		break;

	case GLUT_KEY_PAGE_UP:
		light_position[0] += 0.5;
		break;

	case GLUT_KEY_PAGE_DOWN:
		light_position[0] -= 0.5;
		break;

	case GLUT_KEY_INSERT:
		light_position[1] += 0.5;
		break;

	case GLUT_KEY_HOME:
		light_position[1] -= 0.5;
		break;
	}

	glutPostRedisplay();
}


/**
 * @brief Realiza una acción cuando se presiona una tecla del teclado.
 * @param key Tecla presionada.
 * @param x Coordenada x de la posición del mouse.
 * @param y Coordenada y de la posición del mouse.
 *        Realiza diversas acciones en función de la tecla presionada:
 *        - Modifica la posición de la cámara (movimiento en los ejes x, y y z).
 *        Solicita una actualización de la ventana con glutPostRedisplay().
 */
void KeyboardDown(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
	case 'A':
		ex += (1.0f + delta_r);
		break;

	case 's':
	case 'S':
		ex -= (1.0f + delta_r);
		break;

	case 'd':
	case 'D':
		ey += (1.0f + delta_r);
		break;

	case 'f':
	case 'F':
		ey -= (1.0f + delta_r);
		break;

	case 'q':
	case 'Q':
		ex *= (1.0f + delta_r);
		ey *= (1.0f + delta_r);
		ez *= (1.0f + delta_r);
		break;

	case 'w':
	case 'W':
		ex *= (1.0f - delta_r);
		ey *= (1.0f - delta_r);
		ez *= (1.0f - delta_r);
		break;

	case 27:
		exit(EXIT_SUCCESS);
		break;
	}

	glutPostRedisplay();
}

/**
 * @brief Función de cambio de tamaño de la ventana.
 * @param width Ancho de la ventana después del cambio.
 * @param height Altura de la ventana después del cambio.
 *        Esta función se llama cuando se cambia el tamaño de la ventana.
 *        Ajusta el viewport y las matrices de proyección y modelo para adaptarse al nuevo tamaño de la ventana.
 */
void Reshape(int width, int height) {
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief Función de visualización.
 *        Esta función se llama para mostrar la escena en la ventana.
 *        Llama a RenderScene(GL_RENDER) para renderizar la escena en modo de renderización.
 */
void Display() {
	RenderScene(GL_RENDER);
}

/**
 * @brief Función principal del programa.
 * @param argc Cantidad de argumentos de línea de comandos.
 * @param argv Argumentos de línea de comandos.
 * @return Código de salida del programa.
 *         Esta función inicializa GLUT y crea la ventana de la aplicación.
 *         Configura el modo de visualización y tamaño de la ventana.
 *         Establece las funciones de devolución de llamada para el dibujo, cambio de tamaño de ventana, teclado, mouse, etc.
 *         Inicia el bucle principal de GLUT con glutMainLoop().
 */
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_STENCIL | GLUT_DEPTH);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("2203031129 | CSI01");
	Initialize();
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(KeyboardDown);
	glutMouseFunc(SelectAction);
	glutMainLoop();

	return EXIT_SUCCESS;
}