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