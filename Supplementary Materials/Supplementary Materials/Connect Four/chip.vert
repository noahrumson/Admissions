#version 330

layout(location = 0) in vec4 position;

uniform mat4 mvp;
uniform vec4 colors[42];
uniform vec2 translations[42];

flat out vec4 fragColor;

void main()
{
	fragColor = colors[gl_InstanceID];
	gl_Position = mvp * (position + vec4(translations[gl_InstanceID], 0.0f, 0.0f));
}