#include <GL/glut.h>  // GLUT, GLU, GL
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

float angle = 0.0; // Угол вращения
// The coordinates for the vertices of the cube
double x = 0.3;
double y = 0.3;
double z = 0.3;

// Reshape function
void Reshape(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode( GL_PROJECTION);
  glLoadIdentity();
  // gluOrtho2D(-1, 1, -1, 1);
  glOrtho(-1, 1, -1, 1, 0, 5);
  glMatrixMode(GL_MODELVIEW);
}

void drawCube() {
    // Create the 3D cube
    // BACK
    glBegin(GL_QUADS);

    glColor4f(0.5, 1.0, 1.0, 1.0); // голубой
    glVertex3f(x, -y, z);
    glVertex3f(x, y, z);
    glVertex3f(-x, y, z);
    glVertex3f(-x, -y, z);

    // FRONT
    glColor4f(0.0, 0.9, 0.0, 1.0); // темно-зеленый
    glVertex3f(-x, y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f(x, -y, -z);
    glVertex3f(x, y, -z);

    // LEFT
    glColor4f(1.0, 1.0, 0.0, 1.0); // желтый
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y, z);
    glVertex3f(-x, y, z);
    glVertex3f(-x, y, -z);

    // RIGHT
    glColor4f(1.0, 0.5, 0.0, 1.0); // оранжевый
    glVertex3f(x, -y, -z);
    glVertex3f(x, -y, z);
    glVertex3f(x, y, z);
    glVertex3f(x, y, -z);

    // TOP
    glColor4f(0.9, 0.0, 0.0, 1.0);
    glVertex3f(x, y, z);
    glVertex3f(-x, y, z);
    glVertex3f(-x, y, -z);
    glVertex3f(x, y, -z);

    // BOTTOM
    glColor4f(0.9, 0.0, 0.9, 1.0);
    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y, z);
    glVertex3f(x, -y, z);
    glVertex3f(x, -y, -z);

    glEnd();
}

void Draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Очистка буферов
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();  // Сброс матрицы

    // Рисование куба
    glutWireCube(2.0);  // Используйте glutSolidCube для заполненного куба

    glPushMatrix();

    // Перемещение камеры
    glTranslatef(0.0f, 0.0f, -1.0f);
    glRotatef(angle, 1.0f, 1.0f, 1.0f); // Вращение куба
    angle += 1.0;                       // Увеличиваем угол
    if (angle > 359) {
        angle = 0;                      // Ограничиваем угол
    }

    glTranslatef(0.2f, 0.2f, 0.0f);
    glRotatef(angle, 1.0, 1.0, 1.0);

    glRotatef(angle, 1.0, 0.0, 1.0);
    glRotatef(angle, 0.0, 1.0, 1.0);
    glTranslatef(-0.1f, -0.3f, 0.0f);

    drawCube();

    glPopMatrix();
    glFlush();

    glutSwapBuffers();  // Обмен буферов
}

void timer(int value) {
    glutPostRedisplay();            // Запрос перерисовки
    glutTimerFunc(60, timer, 0);    // Периодический вызов таймера
}

void Visibility(int state) {    // Visibility function
    if (state == GLUT_NOT_VISIBLE)
        printf("Window not visible!\n");
    if (state == GLUT_VISIBLE)
        printf("Window visible!\n");
}

void init() {
    glEnable(GL_DEPTH_TEST);            // Включаем тест глубины
    glEnable(GL_TEXTURE_2D);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Цвет фона окна
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // Esc
        exit(0); // Для старых можно использовать exit(0);
    }
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);  // Инициализация GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);  // Двойная буферизация
    glutInitWindowSize(1200, 800);  // Размер окна
    glutInitWindowPosition(300, 50);
    glutCreateWindow("Rotating cube"); // Заголовок окна

    init();  // Инициализация
    glutReshapeFunc(Reshape);
    glEnable(GL_TEXTURE_2D);
    glutDisplayFunc(Draw);  // Установка функции отображения
    glutKeyboardFunc(keyboard);
    glutTimerFunc(60, timer, 0);  // Запуск таймера
    glutVisibilityFunc(Visibility);
    glDisable(GL_TEXTURE_2D);

    glutMainLoop();  // Запуск главного цикла

    return 0;
}
