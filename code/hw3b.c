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
void draw_tile(GLfloat*);
void draw_maze_walls();
void draw_maze_tiles();
void print_position();

/* Movement Functions */
bool check_collision(float, float, int, int);
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

// Create some lights.
light_t far_light = {
    {50.0, 50.0, 50.0, 0.0},
    {0.75, 0.75, 0.75, 1.0}
};

light_t maze_light = {
    {5.0, 10.0, 5.0, 0.0},
    {0.1, 0.1, 0.1, 1.0}
};

// Define some materials.
material_t blue_plastic = {
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {1000.0f}
};

material_t start_tile = {
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f}
};

material_t end_tile = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f}
};

material_t bread_crumb = {
    {0.3984f, 0.1992f, 0.0f, 1.0f},
    {0.3984f, 0.1992f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f}
};

// Enumerated Types.
typedef enum {FORWARD, BACKWARD, STRAFE_LEFT, STRAFE_RIGHT} move_t;

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
float speed = 0.1;
float view_plane_near = 0.1;
float view_plane_far = 100.0;
int** visited; // Each entry correpsonds to a row/col. 1 if visited 0 else.

unsigned char dir[] = {NORTH, EAST, SOUTH, WEST};

void debug_eye() {
    debug("Eye:\n\tx: %f\n\ty: %f\n\tz: %f", eye.x, eye.y, eye.z);
}

int main(int argc, char** argv) {
    // Make sure there are the minimum number of arguments.
    if (argc < 3) {
        printf("hw3b requires at least 2 arguments!\n\n"
               "Usage: ./hw3b nrows ncols [GL options]\n");

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
    long seed;
    time(&seed);

    maze = make_maze(nrows, ncols, seed);

    start = get_start(maze);
    end = get_end(maze);

    // Set the eye to the start cell.
    eye.x = start->r;
    eye.y = 0.5;
    eye.z = start->c;

    // Initialze the visited array
    visited = (int**) malloc(nrows*ncols*sizeof(int*));
    for (int r = 0; r < nrows; ++r) {
        visited[r] = calloc(ncols, sizeof(int));
    }
}

/**  init_gl:  Initialize OpenGL.
 */
void init_gl() {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    glShadeModel(GL_SMOOTH) ;

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
    debug("Heading: %f\n", phi*180/M_PI);
    if (bird_eye) {
        debug("Setting camera to bird view");
        gluLookAt(eye.x, 20.0, eye.z,
                  nrows/2, 0.0, ncols/2,
                  cos(phi), 0.0, sin(phi));
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

        //Transpose the matrices because that is how GL reads them.
        float camera_matrix[16] = {u.x, v.x, w.x, 0.0,
                                   u.y, v.y, w.y, 0.0,
                                   u.z, v.z, w.z, 0.0,
                                   0.0, 0.0, 0.0, 1.0
                                  };
        float eye_matrix[16] = {1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                -eye.x, -eye.y, -eye.z, 1.0
                               };

        glMultMatrixf(camera_matrix);
        glMultMatrixf(eye_matrix);
    }
}

/** Setup the lighting.
 */
void set_lighting() {
    light_t* light = &far_light;
    light_t* light1 = &maze_light;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light->color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, BLACK);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light->color);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1->color);
    glLightfv(GL_LIGHT1, GL_AMBIENT, BLACK);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1->color);
}

/** Setup the projection viewport matrix.
 */
void set_projection_viewport() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, (GLdouble)win_width/win_height, view_plane_near, 
            view_plane_far);

    glViewport(0, 0, win_width, win_height);
}

/** Draws the basic wall surface.
 *  The wall is a rectangualr solid of length 1,
 *  width 0.25, and height 1 along the x-axis
 *  centered at the origin.
 */
void draw_wall() {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue_plastic.diffuse);

    glBegin(GL_QUADS);
    // positive z plane
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(-0.5, 0.5, 0.125);
    glVertex3f(-0.5, -0.5, 0.125);
    glVertex3f(0.5, -0.5, 0.125);

    // negative z plane
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(0.5, -0.5, -0.125);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(-0.5, 0.5, -0.125);

    // positive x plane
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(0.5, -0.5, 0.125);
    glVertex3f(0.5, -0.5, -0.125);
    glVertex3f(0.5, 0.5, -0.125);

    // negative x plane
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-0.5, 0.5, 0.125);
    glVertex3f(-0.5, 0.5, -0.125);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(-0.5, -0.5, 0.125);

    // positive y plane
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(-0.5, 0.5, -0.125);
    glVertex3f(-0.5, 0.5, 0.125);

    // negative y plane
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
 *
 *  @param color the color to draw the tile.
 */
void draw_tile(GLfloat* color) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

    glBegin(GL_QUADS);

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(1.0, 0.0, 1.0);
    glVertex3f(1.0, 0.0, -1.0);
    glVertex3f(-1.0, 0.0, -1.0);
    glVertex3f(-1.0, 0.0, 1.0);

    glEnd();
}

/** Draws the walls of the maze.
 */
void draw_maze_walls() {
    glMatrixMode(GL_MODELVIEW);
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            cell_t* cell = get_cell(maze, i, j);
            if (has_wall(maze, cell, NORTH)) {
                glPushMatrix();
                glTranslatef(i+0.5, 0.5, j);
                glScalef(1.0, 1.0, 1.25); //Smooth out the corners
                glRotatef(90, 0.0, 1.0, 0.0);
                draw_wall();
                glPopMatrix();
            }
            if (has_wall(maze, cell, EAST)) {
                glPushMatrix();
                glTranslatef(i, 0.5, j+0.5);
                glScalef(1.25, 1.0, 1.0);
                draw_wall();
                glPopMatrix();
            }
            if (has_wall(maze, cell, SOUTH)) {
                glPushMatrix();
                glTranslatef(i-0.5, 0.5, j);
                glScalef(1.0, 1.0, 1.25); //Smooth out the corners
                glRotatef(90, 0.0, 1.0, 0.0);
                draw_wall();
                glPopMatrix();
            }
            if (has_wall(maze, cell, WEST)) {
                glPushMatrix();
                glTranslatef(i, 0.5, j-0.5);
                glScalef(1.25, 1.0, 1.0);
                draw_wall();
                glPopMatrix();
            }
        }
    }
}

/** Draws the tiles for the maze.
 *  This includes the start tile,
 *  the end tile, and the breadcrumbs.
 */
void draw_maze_tiles() {
    glMatrixMode(GL_MODELVIEW);

    // Draw the start tile.
    glPushMatrix();
    glTranslatef(start->r, 0.0, start->c);
    glScalef(0.5, 1.0, 0.5);
    draw_tile(start_tile.diffuse);
    glPopMatrix();

    // Draw the end tile.
    glPushMatrix();
    glTranslatef(end->r, 0.0, end->c);
    glScalef(0.5, 1.0, 0.5);
    draw_tile(end_tile.diffuse);
    glPopMatrix();

    // Draw bread crumbs.
    // This isn't very efficient but we don't have a set :(
    for (int r = 0; r < nrows; ++r) {
        for (int c = 0; c < ncols; ++c) {
            if (visited[r][c] == 1) {
                glPushMatrix();
                glTranslatef(r, 0.0001, c); //Let the tiles sit atop the start cell.
                glScalef(0.125, 2.0, 0.125);
                draw_tile(bread_crumb.diffuse);
                glPopMatrix();
            }
        }
    }
}

/** Print current position and heading.
 */
void print_position() {
    char* s;
    asprintf(&s,
            "Position: %.4f, %.4f\n"
            "Heading (deg): %.4f\n", 
            eye.x, eye.z, phi*180/M_PI);

    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, win_width, 0, win_height);

    glRasterPos2s(10, 50);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, (unsigned char*)s);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_LIGHTING);

    free(s);
}

/** Check to see if moving will collide with a wall.
 *  Cells are centerd on integers which places walls
 *  on the n.5 lines. Since walls are .25 units
 *  thick, we check if our position is in
 *  the range n.375 to n.625 or the range
 *  (n-1).875 to n.125.
 *
 *  @param x the x coordinate of the new position.
 *  @param z the z coordinate of the new position.
 *  @param cur_row the row that we are moving from.
 *  @param cur_col the col that we are moving from.
 *
 *  @return true if the new position collides with a wall,
 *      false otherwise.
 */
bool check_collision(float x, float z, int cur_row, int cur_col) {
    cell_t* cell = get_cell(maze, cur_row, cur_col);
    debug("Row, col: %d, %d", cur_row, cur_col);
    if (cur_row + .375 <= x && x <= cur_row + 0.625) {
        return (has_wall(maze, cell, NORTH));
    }
    if (cur_row - 0.625 <= x && x <= cur_row - 0.375) {
        return (has_wall(maze, cell, SOUTH));
    }
    if (z <= cur_col + 0.625 && cur_col + 0.375 <= z) {
        return (has_wall(maze, cell, EAST));
    }
    if (cur_col - 0.625 <= z && z <= cur_col - 0.375) {
        return (has_wall(maze, cell, WEST));
    }
    return false;
}

/** Handles movement of the eye.
 *  
 *  @param dir the direction we are moving.
 */
void move(move_t dir) {
    if (!bird_eye) {
        float new_x, new_z;
        int r = (int) round(eye.x);
        int c = (int) round(eye.z);
        switch (dir) {
            case FORWARD:
                new_x = eye.x + speed * cos(phi);
                new_z = eye.z + speed * sin(phi);
                break;
            case BACKWARD:
                new_x = eye.x - speed * cos(phi);
                new_z = eye.z - speed * sin(phi);
                break;
            case STRAFE_LEFT:
                new_x = eye.x - speed * cos(phi + 90);
                new_z = eye.z - speed * sin(phi + 90);
                break;
            case STRAFE_RIGHT:
                new_x = eye.x + speed * cos(phi + 90);
                new_z = eye.z + speed * sin(phi + 90);
                break;
        }
        if (!check_collision(new_x, new_z, r, c)) {
            eye.x = new_x;
            eye.z = new_z;
            visited[r][c] = 1;
        }
        else {
            debug("Collision!");
        }
    }
}

/** Rotate the angle from north clockwise.
 */
void rotate_clockwise() {
    if (!bird_eye) {
        phi += PHI_INCR;
        if (phi >= 2*M_PI) {
            phi -= 2*M_PI;
        }
    }
}

/** Rotate the angle from north counterclockwise.
 */
void rotate_counter_clockwise() {
    if (!bird_eye) {
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
    draw_maze_walls();
    draw_maze_tiles();
    print_position();
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
    // Additional movement keys. I like vim movement so those are added.
    switch(key) {
        case 'w':
        case 'k':
            move(FORWARD);
            break;
        case 's':
        case 'j':
            move(BACKWARD);
            break;
        case 'd':
        case 'l':
            rotate_clockwise();
            break;
        case 'a':
        case 'h':
            rotate_counter_clockwise();
            break;
        case 'q':
        case 'u':
            move(STRAFE_LEFT);
            break;
        case 'e':
        case 'o':
            move(STRAFE_RIGHT);
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
            move(FORWARD);
            break;
        case GLUT_KEY_DOWN:     // Move eye down.
            move(BACKWARD);
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
    for (int r = 0; r < nrows; ++r) {
        free(visited[r]);
    }
    free(visited);
    printf("Goodbye!\n");
}
