/*************************
  STUDENT: EFE TANÖZ
  STUDENT ID: 22303006	
  SECTION: 03
  HOMEWORK II
  ADDITIONAL FEATURES:
  -Pacman gets redder every time he hits a ghost.
  -Fire balls changes color depending on their location.
*************************/
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600
#define TIMER_PERIOD   20 // Period for the timer (50 fps)
#define TIMER_ON        1 // 0:Disable timer, 1:Enable timer
#define D2R 0.01745329252
#define PI  3.14159265358

#define MAX_FIRE 10  // 20 fires at a time.
#define FIRE_RATE 8  // After 8 frames you can throw another one.
#define TARGET_RADIUS 20
#define MAX_TARGETS 5

// Global variables for Template File
bool spacebar = false, start = true;
int  winWidth, winHeight; // Current Window width and height
int hitCount = 0;

// User Defined Types and Variables
typedef struct {
	float x, y;
} point_t;

typedef struct {
	point_t pos;    // Position of the object
	float   angle;  // View angle 
	float   r;
} player_t;

typedef struct {
	point_t pos;
	float angle;
	bool active;
} fire_t;

typedef struct {
	int r, g, b;
} color_t;

typedef struct {
	point_t center;
	color_t color;
	float radius;
	float speed;
} target_t;

player_t player = { {-300, 0}, 0, 20 };
fire_t   fire[MAX_FIRE];
target_t targets[MAX_TARGETS];
int fire_rate = 50;
double timerForGame = 20;

// To draw a filled circle, centered at (x,y) with radius r
void circle(int x, int y, int r) {
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++) {
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

// To display text with variables
void vprint(int x, int y, void* font, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(font, str[i]);
}

void drawBg() {
	glColor3f(0, 0, 1);  
	glLineWidth(10);  
	glBegin(GL_LINE_LOOP);
	glVertex2f(-300, -300);
	glVertex2f(-300, 300);
	glVertex2f(300, 300);
	glVertex2f(300, -300);
	glEnd();
	glLineWidth(1);  
	glDisable(GL_LINE_STIPPLE);

}
void pie_filled(float x, float y, float r, float start, float end) {
	glBegin(GL_POLYGON);
	glVertex2f(x, y);
	for (float angle = start; angle < end; angle += 3)
		glVertex2f(r * cos(angle*D2R) + x, r * sin(angle*D2R) + y);
	glVertex2f(r * cos(end*D2R) + x, r * sin(end*D2R) + y);
	glEnd();
}

void pie_wire(float x, float y, float r, float start, float end) {
	glBegin(GL_LINE_LOOP);
	glVertex2f(x, y);
	for (float angle = start; angle < end; angle += 3)
		glVertex2f(r * cos(angle*D2R) + x, r * sin(angle*D2R) + y);
	glVertex2f(r * cos(end*D2R) + x, r * sin(end*D2R) + y);
	glEnd();
}

void myCircle_filled(float x, float y, float r) {
	glBegin(GL_POLYGON);
	for (float angle = 0; angle < 360; angle += 3)
		glVertex2f(r * cos(angle*D2R) + x, r * sin(angle*D2R) + y);
	glEnd();
}

void drawPlayer(player_t tp) {
	glColor3f(1, 1 - hitCount * 1 / 40.0, 0);
	glLineWidth(5);
	glBegin(GL_LINES);
	glVertex2f(tp.pos.x, tp.pos.y);
	glVertex2f(tp.pos.x + 1 * cos(tp.angle * D2R), tp.pos.y + 1 * sin(tp.angle * D2R));
	glEnd();
	glLineWidth(1);

	glColor3f(0.3, 0.3, 1.0);
	circle(tp.pos.x, tp.pos.y, tp.r-10);
	glColor3f(1.0, 0.3, 0.3);
	circle(tp.pos.x, tp.pos.y, tp.r - 14);

	glColor3f(1, 1, 1);
	vprint(tp.pos.x - 12, tp.pos.y - 5, GLUT_BITMAP_8_BY_13, "%.0f", tp.angle);

	//draw pacman

	glColor3f(1 , 1-hitCount*1/40.0, 0);
	pie_filled(-250, tp.pos.y, 50, 30, 330);
	glColor3f(0, 0, 0);
	glLineWidth(3);
	pie_wire(-250 , tp.pos.y, 50, 30, 330);
	myCircle_filled(-240, 25+tp.pos.y, 6);
}

void drawFires() {
	for (int i = 0; i < MAX_FIRE; i++) {
		if (fire[i].active) {
			glColor3f(fire[i].pos.x, fire[i].pos.y, 0.4);
			circle(fire[i].pos.x, fire[i].pos.y, 6);
		}
	}
}

void drawTarget(target_t t) {
	glColor3ub(t.color.r, t.color.g, t.color.b);
	circle(t.center.x, t.center.y, t.radius);
	glColor3ub(t.color.r, t.color.g, t.color.b);
	glRectf(t.center.x - t.radius, t.center.y, t.center.x + 20, t.center.y - 30);
	glColor3f(0, 0, 0);
	circle(t.center.x+5,t.center.y , 3);
	glColor3f(0, 0, 0);
	circle(t.center.x-5, t.center.y, 3);
	glColor3ub(t.color.r, t.color.g, t.color.b);
	circle(t.center.x, t.center.y + 20, 10);
}

// To display onto window using OpenGL commands
void display() {
	// Clear window to black
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawBg();

	if (timerForGame > 0){
		drawFires();
	drawPlayer(player);
	for (int i = 0; i < MAX_TARGETS; i++) {
		drawTarget(targets[i]);
	}
}
	vprint(-290, 280, GLUT_BITMAP_8_BY_13, "Angle: %.0f degrees", player.angle);
	glColor3f(1, 1, 1);
	vprint(-290, 260, GLUT_BITMAP_8_BY_13, "Hit Count: %d",hitCount);
	vprint(-290, 240, GLUT_BITMAP_8_BY_13, "Time Left: %0.2f", timerForGame);

	glColor3f(1, 1, 1);
	vprint(-290, -230, GLUT_BITMAP_8_BY_13, "Upwards Arrow: Move Upwards");
	vprint(-290, -250, GLUT_BITMAP_8_BY_13, "Downwards Arrow: Move Downwards");
	vprint(-290, -270, GLUT_BITMAP_8_BY_13, "A or D: Shooting Direction");
	vprint(-290, -290, GLUT_BITMAP_8_BY_13, "Space: Shoot");


	glutSwapBuffers();
}

// In the fire array, check if any fire is available.
// If there is a fire that we can use for firing, return its index.
int findAvailableFire() {
	for (int i = 0; i < MAX_FIRE; i++)
		if (fire[i].active == false) return i;
	return -1;
}

void resetTarget(target_t *t) {
	float xPos = rand() % 300;
	t->center = { xPos, -300 };
	t->color.r = rand() % 256;
	t->color.g = rand() % 256;
	t->color.b = rand() % 256;
	t->radius = TARGET_RADIUS;
	t->speed = 3.0 * (rand() % 100) / 100.0 + 0.5;
}

void resetTargets() {
	for (int i = 0; i < MAX_TARGETS; i++) {
		resetTarget(&targets[i]);
	}
}

void turnPlayer(player_t *tp, float inc) {
	tp->angle += inc;
	if (tp->angle > 360) tp->angle -= 360;
	if (tp->angle < 0) tp->angle += 360;
}

void movePlayer(player_t *tp, float speed) {
	float dy = speed;
	float r = tp->r;
	tp->pos.y += dy;
	
}

bool testCollision(fire_t fr, target_t t) {
	float dx = t.center.x - fr.pos.x;
	float dy = t.center.y - fr.pos.y;
	float d = sqrt(dx * dx + dy * dy);
	return d <= t.radius;
}

// Key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
void onKeyDown(unsigned char key, int x, int y) {
	// Exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (key == ' ')
		spacebar = true;
	if (key == 'a' || key == 'A')
		if(player.angle<6 || player.angle>353)
			turnPlayer(&player, 1);
	if (key == 'd' || key=='D')
		if(player.angle>354 || player.angle<7)
		turnPlayer(&player, -1);

}

void onKeyUp(unsigned char key, int x, int y) {
	if (key == 27)
		exit(0);
	if (key == ' ')
		spacebar = false;
}

// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
void onSpecialKeyDown(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
			movePlayer(&player, 3);
			break;
	case GLUT_KEY_DOWN:
		movePlayer(&player, -3);
		break;
	case GLUT_KEY_F1:
		start = !start;
	}
	// To refresh the window it calls display() function
	glutPostRedisplay();
}

// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
void onResize(int w, int h) {
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // Refresh window
}

#if TIMER_ON == 1
void onTimer(int v) {
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	if (start)
	{


		if (timerForGame > 0) {
			timerForGame = timerForGame - 0.01;
		}
		if (spacebar && fire_rate == 0) {
			int availFire = findAvailableFire();
			if (availFire != -1) {
				fire[availFire].pos = player.pos;
				fire[availFire].angle = player.angle;
				fire[availFire].active = true;
				fire_rate = FIRE_RATE;
			}
		}
		if (fire_rate > 0) fire_rate--;

		// Move all fires that are active.
		for (int i = 0; i < MAX_FIRE; i++) {
			if (fire[i].active) {
				fire[i].pos.x += 10 * cos(fire[i].angle * D2R);
				fire[i].pos.y += 10 * sin(fire[i].angle * D2R);
				if (fire[i].pos.x > winWidth / 2 || fire[i].pos.x < -winWidth / 2 || fire[i].pos.y > winHeight / 2 || fire[i].pos.y < -winHeight / 2)
					fire[i].active = false;
				for (int j = 0; j < MAX_TARGETS; j++) {
					if (testCollision(fire[i], targets[j])) {
						fire[i].active = false;
						resetTarget(&targets[j]);
						hitCount++;
					}
				}
			}
		}

		// Move all targets from left to right
		for (int i = 0; i < MAX_TARGETS; i++) {
			targets[i].center.y += targets[i].speed;
			if (targets[i].center.y > 280) {
				resetTarget(&targets[i]);
			}
		}
	}
	



	// To refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void init() {
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	resetTargets();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(500, 200);
	glutCreateWindow("PAC-MAN vs GHOSTS| EFE TANÖZ");

	// Window Events
	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	// Keyboard Events
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialFunc(onSpecialKeyDown);

#if TIMER_ON == 1
	// Timer Event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	init();
	glutMainLoop();
	return 0;
}