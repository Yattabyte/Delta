/* Directional light - geometry shadowing shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#define NUM_CASCADES 4
#define MAX_BONES 100
#define TEXTURES_PER_MATERIAL 3

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct BonesStruct {
	mat4 bones[MAX_BONES];
};
struct Light_Struct {
	vec4 LightColor;
	vec4 LightDirection;
	float LightIntensity;
};
struct Shadow_Struct {
	mat4 lightV;
	int Shadow_Spot;
	float CascadeEndClipSpace[NUM_CASCADES];
	mat4 LightVP[NUM_CASCADES];
	mat4 InverseLightVP[NUM_CASCADES];	
};

layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};
layout (std430, binding = 5) readonly buffer Skeleton_Buffer {
	BonesStruct skeletonBuffer[];
};
layout (std430, binding = 6) readonly buffer Skeleton_Index_Buffer {
	int skeletonIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 textureCoordinate;
layout (location = 5) in uint matID;
layout (location = 6) in ivec4 boneIDs;
layout (location = 7) in vec4 weights;

layout (location = 0) uniform int LightIndex = 0;
layout (location = 1) uniform int ShadowIndex = 0;

layout (location = 0) out mat3 WorldTBN;
layout (location = 4) out vec2 TexCoord0;
layout (location = 5) flat out vec3 ColorModifier;
layout (location = 6) flat out uint MaterialOffset;

void main()
{	
	const uint PropIndex 		= propIndexes[gl_DrawID];
	const int SkeletonIndex 	= skeletonIndexes[gl_DrawID];
	mat4 BoneTransform 			= mat4(1.0);
	if (SkeletonIndex != -1) {	
		BoneTransform 			= skeletonBuffer[SkeletonIndex].bones[boneIDs[0]] * weights[0];
		BoneTransform 	   	   += skeletonBuffer[SkeletonIndex].bones[boneIDs[1]] * weights[1];
		BoneTransform          += skeletonBuffer[SkeletonIndex].bones[boneIDs[2]] * weights[2];
		BoneTransform          += skeletonBuffer[SkeletonIndex].bones[boneIDs[3]] * weights[3];
	}
	const mat4 matTrans4 		= propBuffer[PropIndex].mMatrix * BoneTransform;
	const mat3 matTrans3 		= mat3(matTrans4);
	const vec4 WorldVertex		= matTrans4 * vec4(vertex,1.0);
	const vec3 WorldNormal 		= normalize(matTrans3 * normalize(normal));
	const vec3 WorldTangent		= normalize(matTrans3 * normalize(tangent));		
	const vec3 WorldBitangent 	= normalize(matTrans3 * normalize(bitangent));
	WorldTBN					= mat3(WorldTangent, WorldBitangent, WorldNormal);	
	TexCoord0             		= textureCoordinate;		
	ColorModifier 				= lightBuffers[LightIndex].LightColor.xyz * lightBuffers[LightIndex].LightIntensity;
	MaterialOffset				= matID + (propBuffer[PropIndex].materialID * TEXTURES_PER_MATERIAL);
	gl_Position           		= shadowBuffers[ShadowIndex].LightVP[gl_InstanceID] * WorldVertex;		
	gl_Layer 					= shadowBuffers[ShadowIndex].Shadow_Spot + gl_InstanceID;
}