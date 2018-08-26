#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#define MAX_BONES 100

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct BonesStruct {
	mat4 bones[MAX_BONES];
};
struct Shadow_Struct {	
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;	
	int Shadow_Spot;	
};

layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Visibility_Buffer {
	uint propIndexes[];
};
layout (std430, binding = 5) readonly buffer Skeleton_Buffer {
	BonesStruct skeletonBuffer[];
};
layout (std430, binding = 6) readonly buffer Skeleton_Index_Buffer {
	int skeletonIndexes[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};
	
layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

layout (location = 0) uniform int LightIndex = 0;
layout (location = 1) uniform int ShadowIndex = 0;

void main()
{	
	const bool isStatic = skeletonIndexes[gl_DrawID] == -1 ? true : false;
	gl_Position = shadowBuffers[ShadowIndex].lightPV * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	gl_Layer = shadowBuffers[ShadowIndex].Shadow_Spot + int(isStatic);
	id = gl_DrawID;
}
