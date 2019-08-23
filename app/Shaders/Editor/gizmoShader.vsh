/* Gizmo shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in uint meshID;

layout (location = 0) uniform mat4 pvmMat;

layout (location = 0) out vec2 UV;
layout (location = 1) flat out uint axisID;

void main()
{	
	UV = texcoord;
	axisID = meshID;
	gl_Position = pvmMat * vec4(vertex, 1.0);	
}

