#ifndef PAINTER_H
#define PAINTER_H

#include "Matrix.h"
#include "Vertex.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "gl/glew.h"


// Время от начала запуска программы
float running_time = 0;

// Положение курсора мыши в окне
struct
{
    int x, y;

} mousePosition;


// Матрица поворота, которая изменяется при движении мыши
Matrix rotateMatrix;

void changeRotateMatrix(int dx, int dy)
{
    // Умножение rotateMatrix на матрицу поворота вокруг оси y и на матрицу поворота вокруг оси x
    rotateMatrix = rotateMatrix * Matrix::RotationY(dx/50.0f) * Matrix::RotationX(dy/50.f);
}


// Шейдерная программа
GLuint shaderProgram;

// Функция для создания и компиляции шейдера
GLuint CreateShader(GLenum type, const char* source) 
{
    // Создание шейдерной подпрограммы
    GLuint shader = glCreateShader(type);

    if (shader == 0)
    {
        char error[] = "Ошибка создания шейдера\n";
        fprintf(stderr, error);
        OutputDebugStringA(error);
    }

    // Копирование исходного текста шейдерной подпрограммы в объект шейдера
    glShaderSource(shader, 1, &source, nullptr);

    // Компиляция шейдерной пдпрограммы
    glCompileShader(shader);

    // Проверка на ошибки компиляции
    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        char error[64] = "";
        if (type == GL_VERTEX_SHADER)
        {
            sprintf_s(error, "Ошибка компиляции вершинного шейдера\n");
        }
        if (type == GL_FRAGMENT_SHADER)
        {
            sprintf_s(error, "Ошибка компиляции фрагментного шейдера\n");
        }
        fprintf(stderr, error);
        OutputDebugStringA(error);
  
        // Определяем длину сообщения об ошибке
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
        // Вывод сообщения об ошибке компиляции
        char* errorLog = new char[logLength];
        glGetShaderInfoLog(shader, 512, nullptr, errorLog);
        fprintf(stderr, "Функция %s:\n%s", __FUNCTION__, errorLog);
        OutputDebugStringA(errorLog);
        delete [] errorLog;
    }
    return shader;
}

 
// Функция для инициализации OpenGL
void InitOpenGL() {
    
    // Текст вершинного шейдера
    const char* vertexShaderSource = R"(
        #version 430

        layout (location = 0) in vec3 position;
        uniform mat4 general_matrix;

        void main() {
            gl_Position = general_matrix * vec4(position, 1.0);
        }
    )";

    // Текст фрагментного шейдера
    const char* fragmentShaderSource = R"(
        #version 430

        out vec4 color;

        void main() {
             color = vec4(.3, 1.0, 1.0, 1.0); // Цвет треугольника
       
        }
    )";

    // Компиляция шейдеров
    GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Создание шейдерной программы
    shaderProgram = glCreateProgram();

    // Подключение двух шейдерных подпрограмм 
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Компоновка шейдерной программы
    glLinkProgram(shaderProgram);

    // Проверить результат компоновки
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) 
    {
        char error[] = "Ошибка компоновки шейдерной программы\n";
        fprintf(stderr, error);
        OutputDebugStringA(error);

        // Вывод сообщения об ошибке
        GLint logLen;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0)
        {
            char* errorLog = new char[logLen];
            GLsizei written;
            glGetProgramInfoLog(shaderProgram, logLen, &written, errorLog);
            fprintf(stderr, "Функция %s:\n%s", __FUNCTION__, errorLog);
            OutputDebugStringA(errorLog);
            delete[] errorLog;
        }
    }

    // Удаление шейдеров, поскольку они больше не нужны
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Включение сортировки по глубине
    glEnable(GL_DEPTH_TEST);

    // Режим рисования только лицевых граней
    //glEnable(GL_CULL_FACE);
}


// Основная функция рисования
void Draw(int width, int height) 
{
    // Очистка буфера глубины и буфера цвета
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Определение вершин треугольника
    GLfloat vertices[] = {
        0.0f,  1.0f, 0.0f, // Вершина 1
       -1.0f, -1.0f, 0.0f, // Вершина 2
        1.0f, -1.0f, 0.0f,  // Вершина 3
        0.0f,  1.0f, 1.0f, // Вершина 1
       -1.0f, -1.0f, 1.0f, // Вершина 2
        1.0f, -1.0f, 1.0f  // Вершина 3
    };

    float angle = 0;

    /*glm::mat4 RX = glm::rotate(glm::mat4(1.0f), angle / 8, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 RY = glm::rotate(glm::mat4(1.0f), angle / 16, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 R_MOUSE = glm::make_mat4(&rotateMatrix.M[0][0]);
    glm::mat4 T  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
    glm::mat4 PROJ = glm::frustum(-0.5f * width / height, 0.5f * width / height, -0.5f, 0.5f, 1.0f, 20.0f);

    glm::mat4 general_matrix = PROJ * T * R_MOUSE * RY * RX;*/
    
    Matrix general_matrix =
        (Matrix::RotationX(angle / 8) *      // Поворот куба вокруг оси x
            Matrix::RotationY(angle / 16) *  // Поворот куба вокруг оси y
            rotateMatrix *                   // Поворот куба при зажатой левой кнопке мыши
            Matrix::Translation(0, 0, -4)) * // Перенос куба против оси z
            // Matrix::Ortho(-2.0 * width / height, 2.0 * width / height, -2.0, 2.0 , 1.0f, 100.0f);
            //Matrix::Frustum(-0.5 * width / height, 0.5 * width / height, -0.5, 0.5, 1.0, 20);
            Matrix::Perspective(45.0f, (float)width / height, 0.05f, 6.0f); // Перспективное проектирование
    

    // Получение идентификатора uniform-параметра general_matrix
    GLuint matrixLocation = glGetUniformLocation(shaderProgram, "general_matrix");

    // Задание матрицы general_matrix внутри шейдерной программы
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &general_matrix.M[0][0]);
    //glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(general_matrix));

    GLuint VBO;
    
    // Генерируем буфер для хранения координат вершин
    glGenBuffers(1, &VBO);

    // Заполняем сгенерированный буфер координат
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Создание объекта массива вершин, который будет определять отношения между буферами и входными атрибутами
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Привяжем индекс 0 к буферу с координатами
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    // Активация массива вершинных атрибутов
    glEnableVertexAttribArray(0);

    // Рисование треугольника
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(GLfloat) / 3);

    // Отвяжем объект массива вершин
    glBindVertexArray(0);

    // Очистка
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

#endif // PAINTER_H