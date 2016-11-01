#version 330

layout(location = 0) in vec4 position;

uniform mat4 mvpMat;

void main()
{
	gl_Position = mvpMat * position;
}