#version 330

in vec2 texCoord0;
in vec3 normal0;
in vec3 color0;
in vec3 position0;

uniform vec4 lightColor;
uniform sampler2D sampler1;
uniform vec4 lightDirection;
uniform float Transperancy;
uniform vec3 myColor;

out vec4 Color;
void main()
{
	Color = vec4(myColor, Transperancy);
}
