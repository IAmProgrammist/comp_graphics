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
	rotateMatrix = rotateMatrix * Matrix::RotationY(dx / 50.0f) * Matrix::RotationX(dy / 50.f);
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
		delete[] errorLog;
	}
	return shader;
}


// ������� ��� ������������� OpenGL
void InitOpenGL() {

	// ����� ���������� �������
	const char* vertexShaderSource = R"(
#version 430

// � �����
layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec2 VertexTexPosition;
layout (location=2) in vec3 VertexNormal;

// � ������������� �����
uniform mat4 ModelViewMatrix;                             // �������������� ������ � ����
uniform mat3 NormalMatrix;                                // ModelViewMatrix ** -1
uniform mat4 ProjectionMatrix;                            // ������� ��������
uniform mat4 MVP;                                         // ProjectionMatrix * ModelViewMatrix

// � ����� � �����
struct LightInfo {
    vec4 Position;
    vec3 Intensity;
};
uniform LightInfo lights[3];

// �������������� ���������
uniform vec3 Kd;         // � ��������� ����������� �����
uniform vec3 Ka;         // � ��������� �������� �����
uniform vec3 Ks;         // � ����������� ���������
uniform float Shininess; // ���������� ������� ����������� ���������


// �������� ���������
out vec3 LightIntensity;
out vec2 TexCoord;

vec3 ads( int lightIndex, vec4 position, vec3 norm )
{
    vec3 s = normalize(vec3(lights[lightIndex].Position - position));
    vec3 v = normalize((-position).xyz);
    vec3 r = reflect( -s, norm );
    vec3 I = lights[lightIndex].Intensity;

    return I * ( Ka + Kd * max( dot(s, norm), 0.0 ) + Ks * pow( max( dot(r,v), 0.0 ), Shininess ) );
}

void main()
{
    TexCoord = VertexTexPosition;

	vec3 eyeNorm = normalize( NormalMatrix * VertexNormal);
    vec4 eyePosition = ModelViewMatrix * vec4(VertexPosition,1.0);

    LightIntensity = vec3(0.0);

    for( int i = 0; i < 3; i++ )
       LightIntensity += ads( i, eyePosition, eyeNorm );
   
    gl_Position = MVP * vec4(VertexPosition,1.0);
}
    )";

	// ����� ������������ �������
	const char* fragmentShaderSource = R"(
#version 430

in vec3 LightIntensity;
in vec2 TexCoord;

layout( location = 0 ) out vec4 FragColor;

void main() {
	FragColor = vec4(LightIntensity, 1.0);
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
	float positionData[] = {
		-0.8f, -0.8f, 0.0f,
		0.8f, -0.8f, 0.0f,
		0.0f, 0.8f, 0.0f 
	};

	float normalsData[] = {
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,
	};

	float texCoordsData[] = {
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0
	};

	float angle = 0;

	Matrix modelViewMatrix = Matrix::RotationX(angle / 8) *      // ������� ���� ������ ��� x
		Matrix::RotationY(angle / 16) *  // ������� ���� ������ ��� y
		rotateMatrix *                   // ������� ���� ��� ������� ����� ������ ����
		Matrix::Translation(0, 0, -4);

	Matrix projectionMatrix = Matrix::Perspective(45.0f, (float)width / height, 0.05f, 6.0f);

	float normalMatrix[9] = {
		modelViewMatrix.M[0][0], modelViewMatrix.M[0][1], modelViewMatrix.M[0][2],
		modelViewMatrix.M[1][0], modelViewMatrix.M[1][1], modelViewMatrix.M[1][2],
		modelViewMatrix.M[2][0], modelViewMatrix.M[2][2], modelViewMatrix.M[2][2]
	};

	Matrix generalMatrix = modelViewMatrix * projectionMatrix;

	float Kd[3] = { .5, .5, .5 };
	float Ka[3] = { .5, .5, .5 };
	float Ks[3] = { .5, .5, .5 };


	// ��������� �������������� uniform-��������� ��� ������
	GLuint generalMatrixLocation = glGetUniformLocation(shaderProgram, "MVP");
	GLuint modelViewMatrixLocation = glGetUniformLocation(shaderProgram, "ModelViewMatrix");
	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "ProjectionMatrix");
	GLuint normalMatrixLocation = glGetUniformLocation(shaderProgram, "NormalMatrix");

	glUniformMatrix4fv(generalMatrixLocation, 1, GL_FALSE, &generalMatrix.M[0][0]);
	glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, &modelViewMatrix.M[0][0]);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix.M[0][0]);
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, normalMatrix);

	GLuint KdLocation = glGetUniformLocation(shaderProgram, "Kd");
	GLuint KaLocation = glGetUniformLocation(shaderProgram, "Ka");
	GLuint KsLocation = glGetUniformLocation(shaderProgram, "Ks");
	GLuint ShininessLocation = glGetUniformLocation(shaderProgram, "Shininess");

	glUniform3fv(KdLocation, 1, Kd);
	glUniform3fv(KaLocation, 1, Ka);
	glUniform3fv(KsLocation, 1, Ks);
	glUniform1f(ShininessLocation, 0.5);

	GLuint lights0IntensityLocation = glGetUniformLocation(shaderProgram, "lights[0].Intensity");
	GLuint lights0PositionLocation = glGetUniformLocation(shaderProgram, "lights[0].Position");
	GLuint lights1IntensityLocation = glGetUniformLocation(shaderProgram, "lights[1].Intensity");
	GLuint lights1PositionLocation = glGetUniformLocation(shaderProgram, "lights[1].Position");
	GLuint lights2IntensityLocation = glGetUniformLocation(shaderProgram, "lights[2].Intensity");
	GLuint lights2PositionLocation = glGetUniformLocation(shaderProgram, "lights[2].Position");

	glUniform3f(lights0IntensityLocation, 1, 0, 0);
	glUniform3f(lights1IntensityLocation, 0, 1, 0);
	glUniform3f(lights2IntensityLocation, 0, 0, 1);
	glUniform4f(lights0PositionLocation, 1, 0, 0, 0);
	glUniform4f(lights1PositionLocation, -1, .0, 0, 0);
	glUniform4f(lights2PositionLocation, 0, 1, 0, 0);

	
	
	GLuint vboHandles[4];

	// ���������� ����� ��� �������� ��������� ������
	glGenBuffers(3, vboHandles);
	GLuint positionBufferHandle = vboHandles[0];
	GLuint texCoordsBufferHandle = vboHandles[1];
	GLuint normalsBufferHandle = vboHandles[2];

	// ��������� ��������������� ����� ���������
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STATIC_DRAW);

	// ��������� ��������������� ����� ��������� �������
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordsData), texCoordsData, GL_STATIC_DRAW);

	// ��������� ��������������� ����� ��������� �������
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normalsData), normalsData, GL_STATIC_DRAW);

	// �������� ������� ������� ������, ������� ����� ���������� ��������� ����� �������� � �������� ����������
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// ��������� ������� ��������� ���������
	glEnableVertexAttribArray(0); // ���������� ������
	glEnableVertexAttribArray(1); // ���������� ������� ������
	glEnableVertexAttribArray(2); // ���������� �������� ������

	// �������� ������ 0 � ������ � ������������
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// �������� ������ 2 � ������ � ����������� ������������ ������
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferHandle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// �������� ������ 3 � ������ � ���������
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferHandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// ��������� ������������
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// ������� ������ ������� ������
	glBindVertexArray(0);

	// �������
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, vboHandles);
}

#endif // PAINTER_H