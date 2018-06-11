#version 460
#extension GL_ARB_shader_draw_parameters : enable
#package "camera"

layout(location = 0) in vec4 vertex;

struct Reflection_Struct {
	mat4 mMatrix;
	vec4 BoxCamPos;
	float Radius;
	int CubeSpot;
};
layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 5) readonly buffer Reflection_Buffer {
	Reflection_Struct buffers[];
};

layout (location = 0) out vec3 CubeWorldPos;
layout (location = 1) flat out uint BufferIndex;

void main(void)
{	
	BufferIndex 		= gl_InstanceID;
	const vec4 WorldPos = buffers[indexes[gl_InstanceID]].mMatrix * vec4(vertex.xyz, 1);
	gl_Position 		= pMatrix * vMatrix * WorldPos;	
	CubeWorldPos 		= WorldPos.xyz / WorldPos.w;
}
