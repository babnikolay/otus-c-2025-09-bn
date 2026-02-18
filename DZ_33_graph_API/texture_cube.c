#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float angle = 0.0f; // Угол вращения куба

// Функция для загрузки текстуры
GLuint loadTexture(const char *filename) {
    GLuint texture;
    texture = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y);

    if (texture == 0) {
        printf("Ошибка загрузки текстуры: %s\n", SOIL_last_result());
        return 0;
    }

    // Настройка параметров текстуры
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

// Функция для отрисовки куба
void draw_cube(GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);

    // Передняя грань
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Задняя грань
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    // Верхняя грань
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Нижняя грань
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // Правая грань
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Левая грань
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    glEnd();
}

void draw(GLuint texture)
{
    // Очистка экрана
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Установка матрицы проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);

    // Установка матрицы модели/вида
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Перемещение камеры
    glTranslatef(0.0f, 0.0f, -1.0f);
    glRotatef(angle, 1.0f, 1.0f, 1.0f); // Вращение куба
    angle += 1.0;                       // Увеличиваем угол
    if (angle > 359)
    {
        angle = 0; // Ограничиваем угол
    }

    // Вращение куба
    glTranslatef(0.3f, 0.3f, 0.0f);
    glRotatef(angle, 1.0, 1.0, 1.0);
    glRotatef(angle, 1.0, 0.0, 1.0);
    glRotatef(angle, 0.0, 1.0, 1.0);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glTranslatef(-0.5f, -0.4f, 0.0f);

    // Отрисовка куба
    draw_cube(texture);
}

void keyboard(GLFWwindow * window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE); // Закрываем окно
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Использовать: %s <Путь к файлу PNG с текстурой>\n", argv[0]);
        fprintf(stderr, "Нажмите ESC для выхода из программы.\n");
        return EXIT_FAILURE;
    }

    // Инициализация GLFW
    if (!glfwInit()) {
        printf("Ошибка инициализации GLFW\n");
        return EXIT_FAILURE;
    }

    // Создание окна
    GLFWwindow *window = glfwCreateWindow(1200, 800, "Вращающийся куб с текстурой", NULL, NULL);
    if (!window) {
        printf("Ошибка создания окна\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyboard);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Загрузка текстуры
    GLuint texture = loadTexture(argv[1]);
    if (texture == 0) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw(texture);

        // Обновление окна
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка ресурсов
    glDeleteTextures(1, &texture);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
