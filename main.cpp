#include "camera.hh"
#include "program.hh"
#include "metrogl.hh"

#include <GL/glew.h>
#include <GL/freeglut.h>

static gl::MetroGL app;

void display_master() {
    app.display();
    glutSwapBuffers();
}

void window_resize(int width, int height) {
    glViewport(0, 0, width, height);
    app.resize(width, height);
}

void keyboard_key(unsigned char key, int, int) {
    app.handle_key(key);
}

void keyboard_key_up(unsigned char key, int, int) {
    app.handle_key_up(key);
}

void mouse_motion(int x, int y) {
    app.handle_mouse_motion(x, y);
    glutPostRedisplay();
}

void tick(int) {
    if (app.update_controls()) {
        glutPostRedisplay();
    }

    glutTimerFunc(16, tick, 0);
}

bool init_glut(int* argc, char* argv[]) {
    glutInit(argc, argv);
    glutInitContextVersion(4,5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

    const int window_width = 1920;
    const int window_height = 1080;
    const int screen_width = glutGet(GLUT_SCREEN_WIDTH);
    const int screen_height = glutGet(GLUT_SCREEN_HEIGHT);
    const int window_x = screen_width > window_width ? (screen_width - window_width) / 2 : 0;
    const int window_y = screen_height > window_height ? (screen_height - window_height) / 2 : 0;

    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(window_x, window_y);
    glutCreateWindow("MétroGL");
    glutDisplayFunc(display_master);
    glutReshapeFunc(window_resize);
    glutKeyboardFunc(keyboard_key);
    glutKeyboardUpFunc(keyboard_key_up);
    glutPassiveMotionFunc(mouse_motion);
    glutIgnoreKeyRepeat(1);
    glutTimerFunc(16, tick, 0);

    return true;
}

bool init_glew() {
    return glewInit() == GLEW_OK;
}

bool init_gl() {
    glClearColor(0.0f, 1.0f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return true;
}

int main(int argc, char* argv[]) {
    init_glut(&argc, argv);
    init_glew();
    init_gl();

    app.setup();

    glutMainLoop();

    return 0;
}
