#ifndef PAINTER_H
#define PAINTER_H

#include "Matrix.h"
#include "Vertex.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "gl/glew.h"

#include "BMP.h"
#include "AudioFile.h"


// Время от начала запуска программы
float running_time = 0;
float begin_time = 0;
float last_time = 0;
int preferred_tiles_amount = 64;
int actual_tiles_amount = 0;
double gravity = 0.001;
double tiles_spacing = 0.01;
double MAX_FREQUENCY = 20000;
double MIN_FREQUENCY = 20;
double SENSITIVITY = 0.005;

std::vector<double> tiles_amplitudes(preferred_tiles_amount, 0);

AudioFile<double> audioFile;

// Положение курсора мыши в окне
struct
{
	int x, y;

} mousePosition;


// Матрица поворота, которая изменяется при движении мыши
Matrix rotateMatrix;
BMP textureBackground;
BMP textureTile;

void changeRotateMatrix(int dx, int dy)
{
	// Умножение rotateMatrix на матрицу поворота вокруг оси y и на матрицу поворота вокруг оси x
	rotateMatrix = rotateMatrix * Matrix::RotationY(dx / 50.0f) * Matrix::RotationX(dy / 50.f);
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
		delete[] errorLog;
	}
	return shader;
}


// Функция для инициализации OpenGL
void InitOpenGL() {
	std::fstream vertHFile("C:\\Users\\vladi\\Workspace\\C++\\comp_graphics\\lab_6_glsl\\shaders\\main.vert", std::ios::in | std::ios::binary);
	if (!vertHFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

	vertHFile.seekg(0, std::ios::end);
	std::size_t vertLength = vertHFile.tellg();
	vertHFile.seekg(0, std::ios::beg);
	std::vector<char> vertFileInfo(vertLength);
	vertHFile.read(reinterpret_cast<char*>(vertFileInfo.data()), 54);

	std::fstream fragHFile("C:\\Users\\vladi\\Workspace\\C++\\comp_graphics\\lab_6_glsl\\shaders\\main.frag", std::ios::in | std::ios::binary);
	if (!fragHFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

	fragHFile.seekg(0, std::ios::end);
	std::size_t fragLength = fragHFile.tellg();
	fragHFile.seekg(0, std::ios::beg);
	std::vector<char> fragFileInfo(fragLength);
	fragHFile.read(reinterpret_cast<char*>(fragFileInfo.data()), 54);

	// Текст вершинного шейдера
	const char* vertexShaderSource = R"(
#version 430

// О точке
layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec2 VertexTexPosition;
layout (location=2) in vec3 VertexNormal;

// О трансформации точки
uniform mat4 ModelViewMatrix;                             // Преобразование модели в мире
uniform mat3 NormalMatrix;                                // ModelViewMatrix ** -1
uniform mat4 ProjectionMatrix;                            // Матрица проекции
uniform mat4 MVP;                                         // ProjectionMatrix * ModelViewMatrix

// О свете в сцене
struct LightInfo {
    vec4 Position;
    vec3 Intensity;
};
uniform LightInfo lights[3];

// Характеристики материала
uniform vec3 Kd;         // К отражения рассеянного света
uniform vec3 Ka;         // К отражения фонового света
uniform vec3 Ks;         // К зеркального отражения
uniform float Shininess; // Показатель степени зеркального отражения
uniform vec3 Emission;   // Свечение


// Выходные параметры
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

    LightIntensity = Emission;

    for( int i = 0; i < 3; i++ )
       LightIntensity += ads( i, eyePosition, eyeNorm );
   
    gl_Position = MVP * vec4(VertexPosition,1.0);
}
    )";

	// Текст фрагментного шейдера
	const char* fragmentShaderSource = R"(
#version 430

in vec3 LightIntensity;
in vec2 TexCoord;

layout( binding = 0 ) uniform sampler2D BaseTexture;

layout( location = 0 ) out vec4 FragColor;

void main() {
    vec4 texColor = texture(BaseTexture, TexCoord) * vec4(LightIntensity, 1.0);
	FragColor = texColor;
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
	vertHFile.close();
	fragHFile.close();

	// Включение сортировки по глубине
	glEnable(GL_DEPTH_TEST);

	// Режим рисования только лицевых граней
	//glEnable(GL_CULL_FACE);
}

void DrawSoundTiles(GLuint shaderProgram, int width, int height) {
	std::vector<float> positionData;
	std::vector<float> normalsData;
	std::vector<float> texCoordsData;
	float tile_width = (2 + tiles_spacing * (1 - actual_tiles_amount)) / actual_tiles_amount;

	for (int i = 0; i < actual_tiles_amount; i++) {
		float x_left = -1 + i * (tile_width + tiles_spacing);
		float x_rght = x_left + tile_width;
		float ampl = tiles_amplitudes[i];
		float z_tile_width = tile_width / 2;

		std::vector<float> tmp_position_data = {
			x_rght, 0, z_tile_width,
			x_rght, ampl, z_tile_width,
			x_left, 0, z_tile_width,

			x_rght, ampl, z_tile_width,
			x_left, 0, z_tile_width,
			x_left, ampl, z_tile_width,


			x_rght, 0, -z_tile_width,
			x_rght, ampl, -z_tile_width,
			x_left, 0, -z_tile_width,

			x_rght, ampl, -z_tile_width,
			x_left, 0, -z_tile_width,
			x_left, ampl, -z_tile_width,

			x_rght, 0, -z_tile_width,
			x_rght, ampl, z_tile_width,
			x_rght, 0, z_tile_width,

			x_rght, ampl, z_tile_width,
			x_rght, 0, -z_tile_width,
			x_rght, ampl, -z_tile_width,

			x_left, 0, -z_tile_width,
			x_left, ampl, z_tile_width,
			x_left, 0, z_tile_width,

			x_left, ampl, z_tile_width,
			x_left, 0, -z_tile_width,
			x_left, ampl, -z_tile_width,

			x_rght, ampl, z_tile_width,
			x_left, ampl, -z_tile_width,
			x_left, ampl, z_tile_width,

			x_rght, ampl, z_tile_width,
			x_left, ampl, -z_tile_width,
			x_rght, ampl, -z_tile_width,

			x_rght, 0, z_tile_width,
			x_left, 0, -z_tile_width,
			x_left, 0, z_tile_width,

			x_rght, 0, z_tile_width,
			x_left, 0, -z_tile_width,
			x_rght, 0, -z_tile_width,
		};

		std::vector<float> tmp_normals_data = {
			.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,

		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,

		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,

		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,

		1.0f, .0f, .0f,
		1.0f, .0f, .0f,
		1.0f, .0f, .0f,

		1.0f, .0f, .0f,
		1.0f, .0f, .0f,
		1.0f, .0f, .0f,

		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,

		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,

		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,

		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,

		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,

		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,
		};

		std::vector<float> tmp_tex_coords_data = {
			1.0, 0.0,
			1.0, 1.0,
			0.0, 0.0,

			1.0, 1.0,
			0.0, 0.0,
			0.0, 1.0,

			1.0, 0.0,
			1.0, 1.0,
			0.0, 0.0,

			1.0, 1.0,
			0.0, 0.0,
			0.0, 1.0,


			0.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,

			1.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,

			0.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,

			1.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,


			1.0, 1.0,
			0.0, 0.0,
			0.0, 1.0,

			1.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,

			1.0, 1.0,
			0.0, 0.0,
			0.0, 1.0,

			1.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,
		};

		positionData.insert(positionData.end(), tmp_position_data.begin(), tmp_position_data.end());
		normalsData.insert(normalsData.end(), tmp_normals_data.begin(), tmp_normals_data.end());
		texCoordsData.insert(texCoordsData.end(), tmp_tex_coords_data.begin(), tmp_tex_coords_data.end());
	}

	Matrix modelViewMatrix =
		Matrix::Scale(0.2, .2, .2) *
		Matrix::Translation(0, -0.1, -.3);

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


	// Получение идентификатора uniform-параметра для матриц
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
	GLuint EmissionLocation = glGetUniformLocation(shaderProgram, "Emission");

	glUniform3fv(KdLocation, 1, Kd);
	glUniform3fv(KaLocation, 1, Ka);
	glUniform3fv(KsLocation, 1, Ks);
	glUniform1f(ShininessLocation, 0.5);
	glUniform3f(EmissionLocation, .1, .1, .1);

	GLuint lights0IntensityLocation = glGetUniformLocation(shaderProgram, "lights[0].Intensity");
	GLuint lights0PositionLocation = glGetUniformLocation(shaderProgram, "lights[0].Position");
	GLuint lights1IntensityLocation = glGetUniformLocation(shaderProgram, "lights[1].Intensity");
	GLuint lights1PositionLocation = glGetUniformLocation(shaderProgram, "lights[1].Position");
	GLuint lights2IntensityLocation = glGetUniformLocation(shaderProgram, "lights[2].Intensity");
	GLuint lights2PositionLocation = glGetUniformLocation(shaderProgram, "lights[2].Position");

	glUniform3f(lights0IntensityLocation, 0, 0, 1);
	glUniform3f(lights1IntensityLocation, 1, 1, 1);
	glUniform3f(lights2IntensityLocation, 0, 0, 0);
	glUniform4f(lights0PositionLocation, 1, 0, 0, 0);
	glUniform4f(lights1PositionLocation, -1, .0, 0, 0);
	glUniform4f(lights2PositionLocation, 0, 0, 0, 0);

	GLuint vboHandles[4];

	// Генерируем буфер для хранения координат вершин
	glGenBuffers(3, vboHandles);
	GLuint positionBufferHandle = vboHandles[0];
	GLuint texCoordsBufferHandle = vboHandles[1];
	GLuint normalsBufferHandle = vboHandles[2];

	// Заполняем сгенерированный буфер координат
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positionData.size(), positionData.data(), GL_STATIC_DRAW);

	// Заполняем сгенерированный буфер координат текстур
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texCoordsData.size(), texCoordsData.data(), GL_STATIC_DRAW);

	// Заполняем сгенерированный буфер координат текстур
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normalsData.size(), normalsData.data(), GL_STATIC_DRAW);

	// Создание объекта массива вершин, который будет определять отношения между буферами и входными атрибутами
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Активация массива вершинных атрибутов
	glEnableVertexAttribArray(0); // Координаты вершин
	glEnableVertexAttribArray(1); // Координаты текстур вершин
	glEnableVertexAttribArray(2); // Координаты нормалей вершин

	// Привяжем индекс 0 к буферу с координатами
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Привяжем индекс 2 к буферу с текстурными координатами вершин
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferHandle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Привяжем индекс 3 к буферу с нормалями
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferHandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	textureTile.bindGLTexture(GL_TEXTURE0);

	// Рисование треугольника
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, positionData.size() / 3);

	textureTile.unbindGLTexture();

	// Отвяжем объект массива вершин
	glBindVertexArray(0);

	// Очистка
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(3, vboHandles);
}

void DrawSkybox(GLuint shaderProgram, int width, int height) {
	// Определение вершин треугольника
	float positionData[] = {
		1, -1, 1,
		1, 1, 1,
		-1, -1, 1,

		1, 1, 1,
		-1, -1, 1,
		-1, 1, 1,


		1, -1, -1,
		1, 1, -1,
		-1, -1, -1,

		1, 1, -1,
		-1, -1, -1,
		-1, 1, -1,

		1, -1, -1,
		1, 1, 1,
		1, -1, 1,

		1, 1, 1,
		1, -1, -1,
		1, 1, -1,

		-1, -1, -1,
		-1, 1, 1,
		-1, -1, 1,

		-1, 1, 1,
		-1, -1, -1,
		-1, 1, -1,

		1, 1, 1,
		-1, 1, -1,
		-1, 1, 1,

		1, 1, 1,
		-1, 1, -1,
		1, 1, -1,

		1, -1, 1,
		-1,-1, -1,
		-1, -1, 1,

		1, -1, 1,
		-1, -1, -1,
		1, -1, -1,
	};

	float normalsData[] = {
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,

		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,
		.0f, .0f, 1.0f,

		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,

		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,
		.0f, .0f, -1.0f,

		1.0f, .0f, .0f,
		1.0f, .0f, .0f,
		1.0f, .0f, .0f,

		1.0f, .0f, .0f,
		1.0f, .0f, .0f,
		1.0f, .0f, .0f,

		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,

		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,
		-1.0f, .0f, .0f,

		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,

		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,
		.0f, 1.0f, .0f,

		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,

		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,
		.0f, -1.0f, .0f,


	};

	float texCoordsData[] = {
		1.0, 0.0,
		1.0, 1.0,
		0.0, 0.0,

		1.0, 1.0,
		0.0, 0.0,
		0.0, 1.0,

		1.0, 0.0,
		1.0, 1.0,
		0.0, 0.0,

		1.0, 1.0,
		0.0, 0.0,
		0.0, 1.0,


		0.0, 0.0,
		1.0, 1.0,
		0.0, 1.0,

		1.0, 1.0,
		0.0, 0.0,
		1.0, 0.0,

		0.0, 0.0,
		1.0, 1.0,
		0.0, 1.0,

		1.0, 1.0,
		0.0, 0.0,
		1.0, 0.0,


		1.0, 1.0,
		0.0, 0.0,
		0.0, 1.0,

		1.0, 1.0,
		0.0, 0.0,
		1.0, 0.0,

		1.0, 1.0,
		0.0, 0.0,
		0.0, 1.0,

		1.0, 1.0,
		0.0, 0.0,
		1.0, 0.0,
	};

	float angle = running_time;

	Matrix modelViewMatrix =
		Matrix::Scale(1, 1, 1) *
		Matrix::RotationX(angle / 8) *      // Поворот куба вокруг оси x
		Matrix::RotationY(angle / 16) *  // Поворот куба вокруг оси y
		Matrix::RotationZ(angle / 4) *  // Поворот куба вокруг оси y
		Matrix::Translation(0, 0, 0);

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


	// Получение идентификатора uniform-параметра для матриц
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
	GLuint EmissionLocation = glGetUniformLocation(shaderProgram, "Emission");

	glUniform3fv(KdLocation, 1, Kd);
	glUniform3fv(KaLocation, 1, Ka);
	glUniform3fv(KsLocation, 1, Ks);
	glUniform1f(ShininessLocation, 0.5);
	glUniform3f(EmissionLocation, 1, 1, 1);

	GLuint lights0IntensityLocation = glGetUniformLocation(shaderProgram, "lights[0].Intensity");
	GLuint lights0PositionLocation = glGetUniformLocation(shaderProgram, "lights[0].Position");
	GLuint lights1IntensityLocation = glGetUniformLocation(shaderProgram, "lights[1].Intensity");
	GLuint lights1PositionLocation = glGetUniformLocation(shaderProgram, "lights[1].Position");
	GLuint lights2IntensityLocation = glGetUniformLocation(shaderProgram, "lights[2].Intensity");
	GLuint lights2PositionLocation = glGetUniformLocation(shaderProgram, "lights[2].Position");

	glUniform3f(lights0IntensityLocation, 0, 0, 0);
	glUniform3f(lights1IntensityLocation, 0, 0, 0);
	glUniform3f(lights2IntensityLocation, 0, 0, 0);
	glUniform4f(lights0PositionLocation, 1, 0, 0, 0);
	glUniform4f(lights1PositionLocation, -1, .0, 0, 0);
	glUniform4f(lights2PositionLocation, 0, 1, 0, 0);

	GLuint vboHandles[4];

	// Генерируем буфер для хранения координат вершин
	glGenBuffers(3, vboHandles);
	GLuint positionBufferHandle = vboHandles[0];
	GLuint texCoordsBufferHandle = vboHandles[1];
	GLuint normalsBufferHandle = vboHandles[2];

	// Заполняем сгенерированный буфер координат
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STATIC_DRAW);

	// Заполняем сгенерированный буфер координат текстур
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordsData), texCoordsData, GL_STATIC_DRAW);

	// Заполняем сгенерированный буфер координат текстур
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normalsData), normalsData, GL_STATIC_DRAW);

	// Создание объекта массива вершин, который будет определять отношения между буферами и входными атрибутами
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Активация массива вершинных атрибутов
	glEnableVertexAttribArray(0); // Координаты вершин
	glEnableVertexAttribArray(1); // Координаты текстур вершин
	glEnableVertexAttribArray(2); // Координаты нормалей вершин

	// Привяжем индекс 0 к буферу с координатами
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Привяжем индекс 2 к буферу с текстурными координатами вершин
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferHandle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Привяжем индекс 3 к буферу с нормалями
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferHandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	textureBackground.bindGLTexture(GL_TEXTURE0);

	// Рисование треугольника
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	textureBackground.unbindGLTexture();

	// Отвяжем объект массива вершин
	glBindVertexArray(0);

	// Очистка
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(3, vboHandles);
}


// Основная функция рисования
void Draw(int width, int height)
{
	// Очистка буфера глубины и буфера цвета
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	DrawSoundTiles(shaderProgram, width, height);
	DrawSkybox(shaderProgram, width, height);
}

#endif // PAINTER_H