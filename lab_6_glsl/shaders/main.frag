#version 430

in vec3 LightIntensity;
in vec2 TexCoord;

layout( binding = 0 ) uniform sampler2D BaseTexture;

layout( location = 0 ) out vec4 FragColor;

void main() {
    vec4 texColor = texture(BaseTexture, TexCoord) * vec4(LightIntensity, 1.0);
	FragColor = texColor;
}