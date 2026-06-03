#include "camera.hh"
#include "program.hh"
#include "metrogl.hh"

#include <GL/glew.h>
#include <GL/freeglut.h>

//static gl::ShaderProgram* prog;
static gl::MetroGL app;

void display_master() {
    app.display();
    glutSwapBuffers();
}

void window_resize(int width, int height) {
    glViewport(0, 0, width, height);
}

void special_key(int key, int, int) {
    app.handle_special_key(key);
    glutPostRedisplay();
}

bool init_glut(int* argc, char* argv[]) {
    glutInit(argc, argv);
    glutInitContextVersion(4,5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("MétroGL");
    glutDisplayFunc(display_master);
    glutReshapeFunc(window_resize);
    glutSpecialFunc(special_key);

    return true;
}

bool init_glew() {
    return glewInit() == GLEW_OK;
}

bool init_gl() {
    glClearColor(0.0f, 1.0f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return true;
}

int main(int argc, char* argv[]) {
    init_glut(&argc, argv);
    init_glew();
    init_gl();

    app.setup();

    glutMainLoop();

    //prog->use();
    //prog->init_vao();
    //prog->init_object();
    //prog->init_POV();

    /*
    if (prog->is_ready()) {
        prog->display();
        glutMainLoop();
    }
    else
        std::cout << "Ca marche pas lol";
    */

    return 0;
}
