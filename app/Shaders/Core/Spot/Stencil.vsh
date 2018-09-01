/* Spot light - light stenciling shader. */
#version 460
#package "camera"

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
};

layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};

layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

void main()
{		
	gl_Position = cameraBuffer.pMatrix * cameraBuffer.vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
}

