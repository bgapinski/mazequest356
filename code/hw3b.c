/**
 *  Brian Gapinski
 *  Alexander Van Patten
 *  COMP356
 *  hw3b.c
 */

/* misc includes */
#include "debug.h"
#include "geom356.h"
#include "list356.h"
#include "maze.h"

/* Standard includes */
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>

/* OpenGL Includes */
#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

// Defaults for the window.
#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_XORIG 0
#define DEFAULT_WINDOW_YORIG 0
#define WINDOW_TITLE "MazeQuest 356"
#define PHI_INCR 2.0*M_PI/180

/* OpenGL Initialization */
void init_gl();

/* GLUT callbacks */
void handle_display() ;
void handle_reshape(int, int) ;
void handle_key(unsigned char key, int x, int y) ;
void handle_special_key(int key, int x, int y) ;
void handle_close();

/* Functions */
void set_camera();
void set_lighting();
void set_projection_viewport();
void init_maze();

/* Drawing functions */
void draw_wall();
void draw_tile(int);
void draw_maze();

/* Movement Functions */
void move_forward();
void move_backward();
void rotate_clockwise();
void rotate_counter_clockwise();

// Type of materials
typedef struct _material_t {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat phong_exp[4];
} material_t;

typedef struct _light_t {
    GLfloat position[4];
    GLfloat color[4];
} light_t;

GLfloat BLACK[4] = {0.0, 0.0, 0.0, 1.0};

light_t far_light = {
    {20.0, 10.0, 0.0, 0.0},
    {0.75, 0.75, 0.75, 1.0}
};

// Globals
int win_width, win_height ;
int nrows, ncols;
maze_t* maze;
cell_t* start; 
cell_t* end;
point3_t eye;
float eye_radius = 10.0; // How far ahead we can see.
float phi = 0.0; // The angle from north we are looking at.
point3_t look_at;
vector3_t up = {0.0, 1.0, 0.0};
bool bird_eye = false; // If the eye is in the maze or bird_eye

unsigned char dir[] = {NORTH, EAST, SOUTH, WEST};

void debug_eye() {
    debug("Eye:\n\tx: %f\n\ty: %f\n\tz: %f", eye.x, eye.y, eye.z);
}

/** Draw the coordinate axes as line segments from -100 to +100 along
 *  *  the corresponding axis.
 *   */
void draw_axes() {
    glBegin(GL_LINES) ;
    glColor3f(0.0, 0.0, 1.0) ;
    glVertex3f(0.0f, 0.0f, -100.0f) ;
    glVertex3f(0.0f, 0.0f, 100.0f) ;
    glColor3f(1.0, 0.0, 0.0) ;
    glVertex3f(-100.0f, 0.0f, 0.0f) ;
    glVertex3f(100.0f, 0.0f, 0.0f) ;
    glColor3f(0.0, 1.0, 0.0) ;
    glVertex3f(0.0f, -100.0f, 0.0f) ;
    glVertex3f(0.0f, 100.0f, 0.0f) ;
    glEnd() ;
}

int main(int argc, char** argv) {
    // Make sure there are the minimum number of arguments.
    if (argc < 3) {
        printf("hw3b requires at least 2 arguments!\n\nUsage: ./hw3b nrows ncols [GL options]\n");

        return EXIT_FAILURE;
    }

    nrows = atoi(argv[1]);
    ncols = atoi(argv[2]);

    // GLUT initialization.
    glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) ;
    glutInitWindowPosition(DEFAULT_WINDOW_XORIG, DEFAULT_WINDOW_YORIG) ;
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH) ;
    glutInit(&argc, argv) ;
    if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
        fprintf(stderr, "Cannot get requested display mode; exiting.\n") ;
        exit(EXIT_FAILURE) ;
    }

    glutCreateWindow(WINDOW_TITLE) ;

    // GLUT callbacks.
    glutDisplayFunc(handle_display) ;
    glutReshapeFunc(handle_reshape) ;
    glutKeyboardFunc(handle_key) ;
    glutSpecialFunc(handle_special_key) ;
    glutCloseFunc(handle_close);

    // Initialize the maze.
    init_maze();

    // Initialize GL.
    init_gl() ;

    // Start the event loop.
    glutMainLoop() ;

    // A no-op, because glutMainLoop() will never return.  But required,
    // because GCC does not know that glutMainLoop never returns.
    return EXIT_SUCCESS ;
}

/** Sets up some globals based on the maze.
 */
void init_maze() {
    // Use the current time as the seed
    //long seed;
    //time(&seed);
    long seed = 5555;

    maze = make_maze(nrows, ncols, seed);

    start = get_start(maze);
    end = get_end(maze);

    //debug("Start: %d \t %d \n", start->c, start->r);
    eye.x = start->r;
    eye.y = 0.0;
    eye.z = start->c;
    //debug("Eye: %f \t %f \t %f\n", eye.x, eye.y, eye.z);
}

/**  init_gl:  Initialize OpenGL.
 */
void init_gl() {
    // Background color.
    
    glEnable(GL_DEPTH_TEST) ;
    glEnable(GL_LIGHTING) ;
    glEnable(GL_LIGHT0) ;
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    //glShadeModel(GL_SMOOTH) ;
    glClearColor(0.0, 0.0, 0.0, 0.0) ;

    set_camera() ;
    set_lighting() ;
   
    //dlist_id = glGenLists(1) ;
}

/** Sets the camera frame.
 *  This will be either the birds eye view
 *  or the in-maze point of view.
 */
void set_camera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    debug_eye();
    if (bird_eye) {
        debug("Setting camera to bird view");
        gluLookAt(eye.x, 3.0, eye.z,
                  //start->r, 0.0, start->c,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);
    }
    else {
        debug("Setting camera");
        look_at.x = eye_radius * cos(phi) + eye.x;
        look_at.y = eye.y;
        look_at.z = eye_radius * sin(phi) + eye.z;

        vector3_t u, v, w;
        pv_subtract(&eye, &look_at, &w);
        normalize(&w);
        cross(&up, &w, &u);
        normalize(&u);
        cross(&w, &u, &v);

        float camera_matrix[16] = {u.x, u.y, u.z, 0.0,
                                   v.x, v.y, v.z, 0.0,
                                   w.x, w.y, w.z, 0.0,
                                   0.0, 0.0, 0.0, 1.0
                                  };
        float eye_matrix[16] = {1.0, 0.0, 0.0, -eye.x,
                                0.0, 1.0, 0.0, -eye.y,
                                0.0, 0.0, 1.0, -eye.z,
                                0.0, 0.0, 0.0, 1.0
                               };

        glMultMatrixf(camera_matrix);
        glMultMatrixf(eye_matrix);
    }
}

/** Setup the lighting.
 */
void set_lighting() {
    light_t* light = &far_light;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light->color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, BLACK);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light->color);
}

/** Setup the projection viewport matrix.
 */
void set_projection_viewport() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // TODO: Abstract this
    //gluPerspective(60.0, (GLdouble)win_width/win_height, 0.0, 50.0);
    glOrtho(-10, 10, -10, 10, 1, 100);

    glViewport(0, 0, win_width, win_height);
}

/** Draws the basic wall surface.
 *  The wall is a rectangualr solid of length 1,
 *  width 0.25, and height 1 along the x-axis
 *  centered at the origin.
 */
void draw_wall() {
    // TODO: Move colors to material type
    float color[4] = {0.0, 0.0, 1.0, 1.0};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

    glBegin(GL_QUADS);
    // z = 1 plane
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(-0.5, 0.5, 0.125);
    glVertex3f(-0.5, -0.5, 0.125);
    glVertex3f(0.5, -0.5, 0.125);

    // z = -1 plane
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(0.5, -0.5, -0.125);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(-0.5, 0.5, -0.125);

    // x = 1 plane
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(0.5, -0.5, 0.125);
    glVertex3f(0.5, -0.5, -0.125);
    glVertex3f(0.5, 0.5, -0.125);

    // x = -1 plane
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-0.5, 0.5, 0.125);
    glVertex3f(-0.5, 0.5, -0.125);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(-0.5, -0.5, 0.125);

    // y = 1 plane
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(-0.5, 0.5, -0.125);
    glVertex3f(-0.5, 0.5, 0.125);

    // y = -1 plane
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(0.5, -0.5, 0.125);
    glVertex3f(-0.5, -0.5, 0.125);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(0.5, -0.5, -0.125);

    glEnd();
}

/** Draws the basic tile surface.
 *  Tiles are a 2x2 square in the xy-plane
 *  centered at the origin.
 */
void draw_tile(int i) {
    float GREEN[4] = {0.0, 1.0, 0.0, 1.0};
    float RED[4] = {1.0, 0.0, 0.0, 1.0};
    float BREAD[4] = {0.5, 0.7, 1.0};
    float* color;
    if (i == 0) {
        color = GREEN;
    }
    else if (i == 1) {
        color = RED;
    }
    else {
        color = BREAD; 
    }

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

    glBegin(GL_QUADS);

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(1.0, 0.0, 1.0);
    glVertex3f(1.0, 0.0, -1.0);
    glVertex3f(-1.0, 0.0, -1.0);
    glVertex3f(-1.0, 0.0, 1.0);

    glEnd();
}

/** Draws the maze.
 */
void draw_maze() {
    glMatrixMode(GL_MODELVIEW);
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            cell_t* cell = get_cell(maze, i, j);
            if (cell_cmp(start, cell) == 0) {
                //debug("Drawing start at coordinates %d, %d", i, j);
                glPushMatrix();
                glTranslatef(i, 0.0, j);
                //glScalef(0.5, 0.5, 1.0);
                draw_tile(0);
                glPopMatrix();
            }
            if (cell_cmp(end, cell) == 0) {
                glPushMatrix();
                glTranslatef(i, 0.0, j);
                //glScalef(0.5, 0.5, 1.0);
                draw_tile(1);
                glPopMatrix();
            }
            //for (int d = 0; d < 4; ++d) {
            //    if (has_wall(maze, cell, dir[d])) {
            //        //debug("Drawing wall!!!\n %d, %d, %d", i, j, d);
            //        glPushMatrix();
            //        glTranslatef(i, j, 0.5);
            //        glScalef(1.0, 1.0, 2.0);
            //        if (d % 2 == 0) {
            //            // Rotating works
            //            glRotatef(90, 0, 0, 1); 
            //        }
            //        draw_wall();
            //        glPopMatrix();
            //    }
            //}
            if (has_wall(maze, cell, NORTH)) {
                glPushMatrix();
                glTranslatef(5*i, 0.5, 5*j);
                glScalef(5.0, 5.0, 1.0);
                draw_wall();
                glPopMatrix();
            }
            if (has_wall(maze, cell, EAST)) {
                glPushMatrix();
                glTranslatef(5*i+1.0, 0.5, 5*j);
                glScalef(5.0, 5.0, 1.0);
                glRotatef(90, 0.0, 1.0, 0.0);
                draw_wall();
                glPopMatrix();
            }
            if (has_wall(maze, cell, SOUTH)) {
                glPushMatrix();
                glTranslatef(5*i, 0.5, 5*j+1.0);
                glScalef(5.0, 5.0, 1.0);
                draw_wall();
                glPopMatrix();
            }
            if (has_wall(maze, cell, WEST)) {
                glPushMatrix();
                glTranslatef(5*i, 0.5, 5*j);
                glScalef(5.0, 5.0, 1.0);
                glRotatef(90, 0.0, 1.0, 0.0);
                draw_wall();
                glPopMatrix();
            }
        }
    }
}

void move_forward() {
    if (!bird_eye) {
        debug("Moving forward");
        eye.x += cos(phi);
        eye.z += sin(phi);
    }
}

void move_backward() {
    if (!bird_eye) {
        debug("Moving backward!");
        eye.x -= cos(phi);
        eye.z -= sin(phi);
    }
}

void rotate_clockwise() {
    if (!bird_eye) {
        debug("Turing right!");
        phi += PHI_INCR;
        if (phi >= 2*M_PI) {
            phi -= 2*M_PI;
        }
    }
}

void rotate_counter_clockwise() {
    if (!bird_eye) {
        debug("Turing left");
        phi -= PHI_INCR;
        if (phi < 0) {
            phi += 2*M_PI;
        }
    }
}


/** Draw the screen
 */
void handle_display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightfv(GL_LIGHT0, GL_POSITION, far_light.position);
    draw_axes();
    draw_maze();
    glFlush();
}

/** Handle reshape events by setting the projection transform to match
 *  the aspect ratio of the window.
 *
 *  @param w the window width.
 *  @param h the window height.
 */
void handle_reshape(int w, int h) {
    win_width = w ;
    win_height = h ;
    set_projection_viewport() ;
}

/** Handle key events.
 *  
 *  @param key the key.
 *  @param x the x-coordinate of the pointer when the key event occurred.
 *  @param y the x-coordinate of the pointer when the key event occurred.
 */
void handle_key(unsigned char key, int x, int y) {
    debug("handle_key(%d)", key) ;

    switch(key) {
        case 'W':
        case 'w':
        case 'k':
            move_forward();
            break;
        case 'S':
        case 's':
        case 'j':
            move_backward();
            break;
        case 'D':
        case 'd':
        case 'l':
            rotate_clockwise();
            break;
        case 'A':
        case 'a':
        case 'h':
            rotate_counter_clockwise();
            break;
        case ' ':
            bird_eye = !bird_eye;
            break;
    }
    set_camera();
    glutPostRedisplay() ;
}

/** Handle special key events.
 *  
 *  @param key the key.
 *  @param x the x-coordinate of the pointer when the key event occurred.
 *  @param y the x-coordinate of the pointer when the key event occurred.
 */
void handle_special_key(int key, int x, int y) {
    debug("handle_special_key()\n") ;
    switch(key) {
        case GLUT_KEY_UP:       // Move eye up.
            move_forward();
            break;
        case GLUT_KEY_DOWN:     // Move eye down.
            move_backward();
            break;
        case GLUT_KEY_LEFT:
            rotate_counter_clockwise();
            break;
        case GLUT_KEY_RIGHT:
            rotate_clockwise();
            break;
    }
    set_camera() ;
    glutPostRedisplay() ;
}

/** Cleans up the program before exiting
 */
void handle_close() {
    free(maze);
    printf("Goodbye!\n");
}
