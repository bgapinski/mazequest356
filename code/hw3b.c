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
void draw() ;
void handle_reshape(int, int) ;
void handle_key(unsigned char key, int x, int y) ;
void handle_special_key(int key, int x, int y) ;

int win_width, win_height ;
int nrows, ncols;

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("hw3b requires at least 2 arguments!\n\nSyntax:\n$ ./hw3b nrows ncols [GL options]");

        return EXIT_FAILURE;
    }
    // Set the rows and columns
    nrows = atoi(argv[1]);
    ncols = atoi(argv[2]);

    // GLUT initialization.
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH) ;
    glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) ;
    glutInitWindowPosition(DEFAULT_WINDOW_XORIG, DEFAULT_WINDOW_YORIG) ;
    glutInit(&argc, argv) ;
    if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
        fprintf(stderr, "Cannot get requested display mode; exiting.\n") ;
        exit(EXIT_FAILURE) ;
    }

    glutCreateWindow(WINDOW_TITLE) ;

    // Initialize GL.
    init_gl() ;

    // GLUT callbacks.
    glutDisplayFunc(draw) ;
    glutReshapeFunc(handle_reshape) ;
    glutKeyboardFunc(handle_key) ;
    glutSpecialFunc(handle_special_key) ;

    //// Info to console.
    //printf(
    //    "X/x to place eye on postive/negative x-axis; same for Y/y, Z/z.\n") ;
    //printf("Arrow keys to move eye left/right/up/down.\n") ;
    //printf("n/N and f/F to move near and far clipping planes toward/away "
    //        "from eye.\n") ;
    //printf("Ctrl-g/Ctrl-f to switch between flat and Goraud shading.\n") ;
    //printf("1-9 to set recursion depth for sphere subdivision.\n") ;

    // Start the event loop.
    glutMainLoop() ;

    // A no-op, because glutMainLoop() will never return.  But required,
    // because GCC does not know that glutMainLoop never returns.
    return EXIT_SUCCESS ;
}

/**  init_gl:  Initialize OpenGL.
 */
void init_gl() {
    // Background color.
    glClearColor(0.0, 0.0, 0.0, 1.0) ;
    
    glEnable(GL_DEPTH_TEST) ;
    glEnable(GL_LIGHTING) ;
    glEnable(GL_LIGHT0) ;
    glShadeModel(GL_SMOOTH) ;
    
    //set_camera() ;
    //set_lighting() ;
   
    //dlist_id = glGenLists(1) ;
}

/** Draw the screen
 */
void draw() {

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
    //set_projection_viewport() ;
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
