#include <GL/freeglut.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
#include <process.h>
#endif

#if defined(__unix__)
#include <unistd.h>
#endif

#define TARGET_COUNT 30

enum GAME_STATE {
    START_STATE,
    PLAYING_STATE,
    RESULTS_STATE
} game_state;

int init_width = 800;
int init_height = 600;

int init_play_area_width = 750;
int init_play_area_height = 500;

double scale_x = 1.0;
double scale_y = 1.0;

bool display_target = true;
double target_x = 0.0;
double target_y = 0.0;
double target_r = 0.15;

double play_area_x = 1.5;
double play_area_y = 1.0;

double step_target_r = 0.01;
double min_target_r = 0.08;
double max_target_r = 0.25;

double play_again_x = 0.3;
double play_again_y = 0.1;

int remaining_targets = TARGET_COUNT;
clock_t target_times[TARGET_COUNT];
clock_t initial_time;

bool is_inside_target(double x, double y) {
    return pow(x - target_x, 2) + pow(y - target_y, 2) <= pow(target_r, 2);
}

double get_mouse_x(int x) {
    if (x == 0) x = 1;

    return scale_x * (-1.0 + (2.0 * ((double)x / (double)glutGet(GLUT_WINDOW_WIDTH))));
}

double get_mouse_y(int y) {
    if (y == 0) y = 1;

    return scale_y * (-1.0 + (2.0 * ((double)y / (double)glutGet(GLUT_WINDOW_HEIGHT))));
}

void draw_text(double x, double y, void* font, const char* string)
{
    glColor3d(1.0, 1.0, 1.0);
    glRasterPos2d(x, y);

    glutBitmapString(font, string);
}

void draw_circle(double cx, double cy, double r, int num_segments)
{
    glBegin(GL_LINE_LOOP);

    for(int i = 0; i < num_segments; i++)
    {
        double theta = 2.0f * 3.1415926f * (double)i / (double)num_segments;

        double x = r * cos(theta);
        double y = r * sin(theta);

        glVertex2d(x + cx, y + cy);
    }

    glEnd();
}

void draw_play_again_button(void) {
    glLineWidth(3.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2d(-play_again_x, 0.2 - play_again_y);
    glVertex2d(play_again_x, 0.2 - play_again_y);
    glVertex2d(play_again_x, 0.2 + play_again_y);
    glVertex2d(-play_again_x, 0.2 + play_again_y);
    glEnd();

    draw_text(-0.25, 0.23, GLUT_BITMAP_HELVETICA_18, "Jogar novamente");
}

void draw_play_area(void) {
    glLineWidth(3.0f);

    glBegin(GL_LINE_LOOP);
    glVertex2d(-play_area_x, -play_area_y);
    glVertex2d(play_area_x, -play_area_y);
    glVertex2d(play_area_x, play_area_y);
    glVertex2d(-play_area_x, play_area_y);
    glEnd();
}

void draw_target(void) {
    glLineWidth(3.0f);

    glColor3d(1.0, 1.0, 1.0);

    draw_circle(target_x, target_y, target_r * 3.0/3.0, 50);
    draw_circle(target_x, target_y, target_r * 2.0/3.0, 50);
    draw_circle(target_x, target_y, target_r * 1.0/3.0, 50);

    glBegin(GL_LINES);
    glVertex2d(target_x - target_r, target_y);
    glVertex2d(target_x + target_r, target_y);
    glEnd();

    glBegin(GL_LINES);
    glVertex2d(target_x, target_y - target_r);
    glVertex2d(target_x, target_y + target_r);
    glEnd();
}

void display(GLvoid) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(43.0f/255.0f, 135.0f/255.0f, 209.0f/255.0f, 1.0f);

    if (display_target)
        draw_target();

    draw_play_area();

    if (game_state == START_STATE) {
        char* instructions_buffer = (char*)malloc(256 * sizeof(char));
        snprintf(instructions_buffer, 256, "Para jogar, acerte %d alvos rapidamente.", TARGET_COUNT);

        draw_text(-0.15, -0.4, GLUT_BITMAP_HELVETICA_18, "Aim Trainer");
        draw_text(-0.45, 0.4, GLUT_BITMAP_HELVETICA_18, instructions_buffer);
        draw_text(-0.4, 0.5, GLUT_BITMAP_HELVETICA_18, "Clique no alvo acima para iniciar.");
        draw_text(-0.395, 0.7, GLUT_BITMAP_HELVETICA_18, "Aperte ESC para sair do jogo.");

        draw_text(-play_area_x + 0.1, 0.4, GLUT_BITMAP_HELVETICA_18, "Mudar dificuldade:");
        draw_text(-play_area_x + 0.1, 0.5, GLUT_BITMAP_HELVETICA_18, "Facil (tecla [F])");
        draw_text(-play_area_x + 0.1, 0.6, GLUT_BITMAP_HELVETICA_18, "Normal (tecla [N])");
        draw_text(-play_area_x + 0.1, 0.7, GLUT_BITMAP_HELVETICA_18, "Dificil (tecla [D])");
        draw_text(-play_area_x + 0.1, 0.8, GLUT_BITMAP_HELVETICA_18, "Viciado (tecla [V])");

        draw_text(-play_area_x + 0.1, 0.9, GLUT_BITMAP_HELVETICA_18, "Use a roda do mouse para ajustes finos.");
    } else if (game_state == PLAYING_STATE) {
        char* remaining_message_buffer = (char*)malloc(256 * sizeof(char));
        snprintf(remaining_message_buffer, 256, "Faltam %d alvos", remaining_targets);

        draw_text(-0.1, - play_area_y - 0.1, GLUT_BITMAP_HELVETICA_18, remaining_message_buffer);
    } else if (game_state == RESULTS_STATE) {
        int total_time_ms = 0;
        for (int i = 0; i < TARGET_COUNT; ++i)
            total_time_ms += (int)((float)target_times[i])/(CLOCKS_PER_SEC/1000);

        int average_ms = total_time_ms / TARGET_COUNT;

        char* average_ms_buffer = (char*)malloc(256 * sizeof(char));
        snprintf(average_ms_buffer, 256, "%d ms", average_ms);

        draw_text(-0.4, -0.25, GLUT_BITMAP_HELVETICA_18, "Tempo de reacao medio por alvo");
        draw_text(-0.1, -0.05, GLUT_BITMAP_TIMES_ROMAN_24, average_ms_buffer);

        draw_play_again_button();
    }

    glFlush();
}

void decrease_target_r(void) {
    target_r -= step_target_r;

    if (target_r < min_target_r)
        target_r = min_target_r;
}

void increase_target_r(void) {
    target_r += step_target_r;

    if (target_r > max_target_r)
        target_r = max_target_r;
}

void mouse_passive_motion(int x, int y)
{
    double mouse_x = get_mouse_x(x);
    double mouse_y = get_mouse_y(y);

    if (game_state == RESULTS_STATE
        && mouse_x <= play_again_x
        && mouse_x >= -play_again_x
        && mouse_y <= 0.2 + play_again_y
        && mouse_y >= 0.2 - play_again_y) {
        glutSetCursor(GLUT_CURSOR_INFO);
    }
    else if (display_target && is_inside_target(mouse_x, mouse_y)) {
        glutSetCursor(GLUT_CURSOR_INFO);
    } else {
        glutSetCursor(GLUT_CURSOR_INHERIT);
    }
}

void reshape(int new_width, int new_height)
{
    if (new_height <= init_height)
        new_height = init_height;

    if (new_width <= init_width)
        new_width = init_width;

    glutReshapeWindow(new_width, new_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double init_scale_x = (double)init_width / (double)init_play_area_width;
    double init_scale_y = (double)init_height / (double)init_play_area_height;

    scale_x = init_scale_x * (double)new_width/init_height;
    scale_y = init_scale_y * (double)new_height/init_height;

    gluOrtho2D( -scale_x, scale_x, scale_y, -scale_y);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0,0,new_width,new_height);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            glutExit();
            return;
        case '-':
            decrease_target_r();
            break;
        case '+':
            increase_target_r();
            break;
        case 'F':
        case 'f':
            target_r = 0.25;
            break;
        case 'N':
        case 'n':
            target_r = 0.15;
            break;
        case 'D':
        case 'd':
            target_r = 0.12;
            break;
        case 'V':
        case 'v':
            target_r = 0.08;
            break;
        default:
            break;
    }

    glutPostRedisplay();
}

void init(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(43.0f/255.0f, 135.0f/255.0f, 209.0f/255.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    game_state = START_STATE;

    double init_play_scale_x = (double)init_play_area_width / (double)init_play_area_height;
    double init_play_scale_y = 1.0;

    double init_scale_x = (double)init_width / (double)init_play_area_width;
    double init_scale_y = (double)init_height / (double)init_play_area_height;

    double ortho_x = init_scale_x * init_play_scale_x;
    double ortho_y = init_scale_y * init_play_scale_y;

    gluOrtho2D(-ortho_x, ortho_x, ortho_y, -ortho_y);

#if defined(_WIN32)
    srand(time(NULL) + _getpid());
#endif

#if defined(__unix__)
    srand(time(NULL) + getpid());
#endif

#if !defined(_WIN32) && !defined(__unix__)
    srand(time(NULL));
#endif
}

void mouse(int button, int state, int x, int y)
{
    double mouse_x = get_mouse_x(x);
    double mouse_y = get_mouse_y(y);

    if ((button == 3) || (button == 4))
    {
        if (state == GLUT_UP) return;

        if (button == 3) increase_target_r();
        else decrease_target_r();
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        bool inside_target = is_inside_target(mouse_x, mouse_y);
        if (game_state == START_STATE) {
            if (!inside_target) return;

            initial_time = clock();
            game_state = PLAYING_STATE;
        } else if (game_state == PLAYING_STATE) {
            if (!inside_target) return;

            clock_t time = clock();
            target_times[TARGET_COUNT - remaining_targets] = time - initial_time;
            initial_time = time;
            remaining_targets--;

            if (remaining_targets == 0) {
                game_state = RESULTS_STATE;
                display_target = false;

                target_x = 0.0;
                target_y = 0.0;

                glutPostRedisplay();
                return;
            }
        } else if (game_state == RESULTS_STATE) {
            if (mouse_x <= play_again_x
                && mouse_x >= -play_again_x
                && mouse_y <= 0.2 + play_again_y
                && mouse_y >= 0.2 - play_again_y) {
                game_state = START_STATE;
                target_x = 0.0;
                target_y = 0.0;
                display_target = true;
                remaining_targets = TARGET_COUNT;
                glutPostRedisplay();
                return;
            }
        }

        double range_x = 2.0 * (play_area_x - target_r);
        double range_y = 2.0 * (play_area_y - target_r);

        double rand_x = range_x * ((double)rand() / (double)RAND_MAX); // NOLINT(cert-msc30-c, cert-msc50-cpp)
        double rand_y = range_y * ((double)rand() / (double)RAND_MAX); // NOLINT(cert-msc30-c, cert-msc50-cpp)

        target_x = -play_area_x + target_r + rand_x;
        target_y = -play_area_y + target_r + rand_y;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(init_width, init_height);
    glutInitWindowPosition(200, 200);

    glutCreateWindow("Aim Trainer");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouse_passive_motion);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
