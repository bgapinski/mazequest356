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
void draw_wall();
void draw_tile();

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

// Globals
int win_width, win_height ;
int nrows, ncols;
maze_t* maze;
cell_t* start; 
cell_t* end;
vector3_t eye;
float eye_radius = 10.0;
float eye_phi = 0.0;
point3_t look_at = {0.0, 0.0, 0.0};
vector3_t north = {0.0, 1.0, 0.0};

unsigned char dir[] = {NORTH, EAST, SOUTH, WEST};

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

    // Initialize GL.
    init_gl() ;

    // GLUT callbacks.
    glutDisplayFunc(handle_display) ;
    glutReshapeFunc(handle_reshape) ;
    glutKeyboardFunc(handle_key) ;
    glutSpecialFunc(handle_special_key) ;
    glutCloseFunc(handle_close);

    init_maze();

    // Start the event loop.
    glutMainLoop() ;

    // A no-op, because glutMainLoop() will never return.  But required,
    // because GCC does not know that glutMainLoop never returns.
    return EXIT_SUCCESS ;
}

void init_maze() {
    // Use the current time as the seed
    long seed;
    time(&seed);

    maze = make_maze(nrows, ncols, seed);

    start = get_start(maze);
    end = get_end(maze);

    eye.x = start->c;
    eye.y = start->r;
    eye.z = 5.0;
}

/**  init_gl:  Initialize OpenGL.
 */
void init_gl() {
    // Background color.
    glClearColor(0.0, 0.0, 0.0, 1.0) ;
    
    //glEnable(GL_DEPTH_TEST) ;
    glEnable(GL_LIGHTING) ;
    glEnable(GL_LIGHT0) ;
    glShadeModel(GL_SMOOTH) ;
    
    set_camera() ;
    set_lighting() ;
   
    //dlist_id = glGenLists(1) ;
}

void set_camera() {
    //TODO: Currently just testing stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //TODO: Make this not use lookat
    gluLookAt(nrows, ncols, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    //gluLookAt(3.0, 3.0, 15.0, nrows/2.0, ncols/2.0, -1.0, 0.0, 0.0, 1.0);
}

void set_lighting() {
}

void set_projection_viewport() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // TODO: Abstract this
    //gluPerspective(60.0, (GLdouble)win_width/win_height, 4.0, 100.0);
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
    glVertex3f(0.5, 0.125, 0.5);
    glVertex3f(0.5, -0.125, 0.5);
    glVertex3f(-0.5, -0.125, 0.5);
    glVertex3f(-0.5, 0.125, 0.5);

    // z = -1 plane
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(0.5, 0.125, -0.5);
    glVertex3f(-0.5, 0.125, -0.5);
    glVertex3f(-0.5, -0.125, -0.5);
    glVertex3f(0.5, -0.125, -0.5);

    // x = 1 plane
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(0.5, 0.125, 0.5);
    glVertex3f(0.5, 0.125, -0.5);
    glVertex3f(0.5, -0.125, -0.5);
    glVertex3f(0.5, -0.125, 0.5);

    // x = -1 plane
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-0.5, 0.125, 0.5);
    glVertex3f(-0.5, -0.125, 0.5);
    glVertex3f(-0.5, -0.125, -0.5);
    glVertex3f(-0.5, 0.125, -0.5);

    // y = 1 plane
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-0.5, 0.125, -0.5);
    glVertex3f(0.5, 0.125, -0.5);
    glVertex3f(0.5, 0.125, 0.5);
    glVertex3f(-0.5, 0.125, 0.5);

    // y = -1 plane
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-0.5, -0.125, -0.5);
    glVertex3f(-0.5, -0.125, 0.5);
    glVertex3f(0.5, -0.125, 0.5);
    glVertex3f(0.5, -0.125, -0.5);

    glEnd();
}

/** Draws the basic tile surface.
 *  Tiles are a 2x2 square in the xy-plane
 *  centered at the origin.
 */
void draw_tile() {
    float color[4] = {0.0, 1.0, 0.0, 0.2};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

    glBegin(GL_QUADS);

    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.0);

    glEnd();
}

/** Draw the screen
 */
void handle_display() {
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            cell_t* cell = get_cell(maze, i, j);
            if (cell_cmp(start, cell) == 0) {
                debug("Drawing start at coordinates %d, %d", i, j);
                glPushMatrix();
                glTranslatef(i, j, 0);
                glScalef(0.5, 0.5, 1.0);
                draw_tile();
                glPopMatrix();
            }
            for (int d = 0; d < 4; ++d) {
                if (has_wall(maze, cell, dir[d])) {
                    debug("Drawing wall!!!\n %d, %d, %d", i, j, d);
                    glPushMatrix();
                    //glScalef(1, 1, 3.0);
                    glTranslatef(i, j, 0.5);
                    if (d % 2 == 0) {
                        // Rotating works
                        glRotatef(90, 0, 0, 1); 
                    }
                    draw_wall();
                    glPopMatrix();
                }
            }
        }
    }
    //draw_tile();
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

    //// Digits:  specify the recursion depth.
    //if ('1' <= key && key <= '9') {
    //    max_depth = key-'0' ;
    //}

    //else {
    //    switch(key) {
    //        case '+':
    //            eye_dist += .1 ;
    //            set_camera() ;
    //            break ;
    //        case '-':
    //            eye_dist -= .1 ;
    //            set_camera() ;
    //            break ;
    //        case 'n':
    //            frust_near -= .1 ;
    //            set_projection_viewport() ;
    //            set_camera() ;
    //            break ;
    //        case 'N':
    //            frust_near += .1 ;
    //            set_projection_viewport() ;
    //            set_camera() ;
    //            break ;
    //        case 'f':
    //            frust_far -= .1 ;
    //            set_projection_viewport() ;
    //            set_camera() ;
    //            break ;
    //        case 'F':
    //            frust_far += .1 ;
    //            set_projection_viewport() ;
    //            set_camera() ;
    //            break ;
    //        case ' ':
    //            if (light0_pos == light0_point) light0_pos = light0_dist ;
    //            else light0_pos = light0_point ;
    //            break ;
    //        case 'L':
    //            do_dlist = !(do_dlist) ;
    //            if (do_dlist) {
    //              glNewList(dlist_id, GL_COMPILE) ;
    //              draw_sphere() ;
    //              glEndList() ;
    //            }
    //            break ;

    //    }
    //}
    //glutPostRedisplay() ;
}

/** Handle special key events.
 *  
 *  @param key the key.
 *  @param x the x-coordinate of the pointer when the key event occurred.
 *  @param y the x-coordinate of the pointer when the key event occurred.
 */
void handle_special_key(int key, int x, int y) {
    debug("handle_special_key()\n") ;
    //switch(key) {
    //    case GLUT_KEY_UP:       // Move eye up.
    //        theta += 2.0 ;
    //        break ;
    //    case GLUT_KEY_DOWN:     // Move eye down.
    //        theta -= 2.0 ;
    //        break ;
    //    case GLUT_KEY_LEFT:
    //        phi -= 2.0 ;
    //        break ;
    //    case GLUT_KEY_RIGHT:
    //        phi += 2.0 ;
    //        break ;
    //}
    //set_camera() ;
    //glutPostRedisplay() ;
}

/** Cleans up the program before exiting
 */
void handle_close() {
    free(maze);
    printf("Goodbye!\n");
}
