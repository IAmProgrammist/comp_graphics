#ifndef PAINTER_H
#define PAINTER_H

#include "Matrix.h"
#include "Vertex.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "gl/glew.h"


// ����� �� ������ ������� ���������
float running_time = 0;

// ��������� ������� ���� � ����
struct
{
    int x, y;

} mousePosition;


// ������� ��������, ������� ���������� ��� �������� ����
Matrix rotateMatrix;

void changeRotateMatrix(int dx, int dy)
{
    // ��������� rotateMatrix �� ������� �������� ������ ��� y � �� ������� �������� ������ ��� x
    rotateMatrix = rotateMatrix * Matrix::RotationY(dx/50.0f) * Matrix::RotationX(dy/50.f);
}


// ��������� ���������
GLuint shaderProgram;

// ������� ��� �������� � ���������� �������
GLuint CreateShader(GLenum type, const char* source) 
{
    // �������� ��������� ������������
    GLuint shader = glCreateShader(type);

    if (shader == 0)
    {
        char error[] = "������ �������� �������\n";
        fprintf(stderr, error);
        OutputDebugStringA(error);
    }

    // ����������� ��������� ������ ��������� ������������ � ������ �������
    glShaderSource(shader, 1, &source, nullptr);

    // ���������� ��������� �����������
    glCompileShader(shader);

    // �������� �� ������ ����������
    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        char error[64] = "";
        if (type == GL_VERTEX_SHADER)
        {
            sprintf_s(error, "������ ���������� ���������� �������\n");
        }
        if (type == GL_FRAGMENT_SHADER)
        {
            sprintf_s(error, "������ ���������� ������������ �������\n");
        }
        fprintf(stderr, error);
        OutputDebugStringA(error);
  
        // ���������� ����� ��������� �� ������
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
        // ����� ��������� �� ������ ����������
        char* errorLog = new char[logLength];
        glGetShaderInfoLog(shader, 512, nullptr, errorLog);
        fprintf(stderr, "������� %s:\n%s", __FUNCTION__, errorLog);
        OutputDebugStringA(errorLog);
        delete [] errorLog;
    }
    return shader;
}

 
// ������� ��� ������������� OpenGL
void InitOpenGL() {
    
    // ����� ���������� �������
    const char* vertexShaderSource = R"(
        #version 430

        layout (location = 0) in vec3 position;
        uniform mat4 general_matrix;

        void main() {
            gl_Position = general_matrix * vec4(position, 1.0);
        }
    )";

    // ����� ������������ �������
    const char* fragmentShaderSource = R"(
        #version 430

        out vec4 color;

        void main() {
             color = vec4(.3, 1.0, 1.0, 1.0); // ���� ������������
       
        }
    )";

    // ���������� ��������
    GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // �������� ��������� ���������
    shaderProgram = glCreateProgram();

    // ����������� ���� ��������� ����������� 
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // ���������� ��������� ���������
    glLinkProgram(shaderProgram);

    // ��������� ��������� ����������
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) 
    {
        char error[] = "������ ���������� ��������� ���������\n";
        fprintf(stderr, error);
        OutputDebugStringA(error);

        // ����� ��������� �� ������
        GLint logLen;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0)
        {
            char* errorLog = new char[logLen];
            GLsizei written;
            glGetProgramInfoLog(shaderProgram, logLen, &written, errorLog);
            fprintf(stderr, "������� %s:\n%s", __FUNCTION__, errorLog);
            OutputDebugStringA(errorLog);
            delete[] errorLog;
        }
    }

    // �������� ��������, ��������� ��� ������ �� �����
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ��������� ���������� �� �������
    glEnable(GL_DEPTH_TEST);

    // ����� ��������� ������ ������� ������
    //glEnable(GL_CULL_FACE);
}


// �������� ������� ���������
void Draw(int width, int height) 
{
    // ������� ������ ������� � ������ �����
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // ����������� ������ ������������
    GLfloat vertices[] = {
        0.0f,  1.0f, 0.0f, // ������� 1
       -1.0f, -1.0f, 0.0f, // ������� 2
        1.0f, -1.0f, 0.0f,  // ������� 3
        0.0f,  1.0f, 1.0f, // ������� 1
       -1.0f, -1.0f, 1.0f, // ������� 2
        1.0f, -1.0f, 1.0f  // ������� 3
    };

    float angle = 0;

    /*glm::mat4 RX = glm::rotate(glm::mat4(1.0f), angle / 8, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 RY = glm::rotate(glm::mat4(1.0f), angle / 16, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 R_MOUSE = glm::make_mat4(&rotateMatrix.M[0][0]);
    glm::mat4 T  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
    glm::mat4 PROJ = glm::frustum(-0.5f * width / height, 0.5f * width / height, -0.5f, 0.5f, 1.0f, 20.0f);

    glm::mat4 general_matrix = PROJ * T * R_MOUSE * RY * RX;*/
    
    Matrix general_matrix =
        (Matrix::RotationX(angle / 8) *      // ������� ���� ������ ��� x
            Matrix::RotationY(angle / 16) *  // ������� ���� ������ ��� y
            rotateMatrix *                   // ������� ���� ��� ������� ����� ������ ����
            Matrix::Translation(0, 0, -4)) * // ������� ���� ������ ��� z
            // Matrix::Ortho(-2.0 * width / height, 2.0 * width / height, -2.0, 2.0 , 1.0f, 100.0f);
            //Matrix::Frustum(-0.5 * width / height, 0.5 * width / height, -0.5, 0.5, 1.0, 20);
            Matrix::Perspective(45.0f, (float)width / height, 0.05f, 6.0f); // ������������� ��������������
    

    // ��������� �������������� uniform-��������� general_matrix
    GLuint matrixLocation = glGetUniformLocation(shaderProgram, "general_matrix");

    // ������� ������� general_matrix ������ ��������� ���������
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &general_matrix.M[0][0]);
    //glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(general_matrix));

    GLuint VBO;
    
    // ���������� ����� ��� �������� ��������� ������
    glGenBuffers(1, &VBO);

    // ��������� ��������������� ����� ���������
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // �������� ������� ������� ������, ������� ����� ���������� ��������� ����� �������� � �������� ����������
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // �������� ������ 0 � ������ � ������������
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    // ��������� ������� ��������� ���������
    glEnableVertexAttribArray(0);

    // ��������� ������������
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(GLfloat) / 3);

    // ������� ������ ������� ������
    glBindVertexArray(0);

    // �������
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

#endif // PAINTER_H