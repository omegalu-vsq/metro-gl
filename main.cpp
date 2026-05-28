#include "camera.hh"
#include "program.hh"

#include <GL/glew.h>
#include <GL/freeglut.h>

static gl::Program* prog;

void display_master() {
    if (!prog) return;
    prog->display();
    glutSwapBuffers();
}

void window_resize(int width, int height) {
    glViewport(0,0,width,height);
}

bool init_glut(int* argc, char* argv[]) {
    glutInit(argc, argv);
    glutInitContextVersion(4,5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("OPENGL XX");
    glutDisplayFunc(display_master);
    glutReshapeFunc(window_resize);

    return true;
}

bool init_glew() {
    return glewInit() == GLEW_OK;
}

bool init_gl() {
    glClearColor(0.0f, 1.0f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    return true;
}

int main(int argc, char* argv[]) {
    init_glut(&argc, argv);
    init_glew();
    init_gl();

    auto progprog = gl::Program::make_program("vertex.shd", "fragment.shd");
    prog = &progprog;

    prog->use();
    prog->init_object();
    prog->init_POV();

    if (prog->is_ready()) {
        prog->display();
        glutMainLoop();
    }
    else
        std::cout << "Ca marche pas lol";

    return 0;
}
