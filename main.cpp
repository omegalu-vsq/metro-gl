#include "camera.hh"

#include <GL/glew.h>
#include <GL/freeglut.h>

bool init_glut(int* argc, char* argv[]) {
    glutInit(argc, argv);
    glutInitContextVersion(4,5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("OPENGL XX");

    return true;
}

bool init_glew() {
    return glewInit() == GLEW_OK;
}

bool init_gl() {
    glClearColor(0.0f, 1.0f, 0.1f, 1.0f);

    return true;
}

int main(int argc, char* argv[]) {
    init_glut(&argc, argv);
    init_glew();
    init_gl();

    return 0;
}
