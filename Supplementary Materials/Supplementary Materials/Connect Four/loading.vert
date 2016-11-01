#version 330

layout(location = 0) in vec4 position;

uniform mat4 mvp;
uniform vec4 colors[3];

flat out vec4 fragColor;

void main()
{
	fragColor = colors[gl_InstanceID];
	gl_Position = mvp * (position + vec4(-7.0f, 0.0f, 0.0f, 0.0f));
}