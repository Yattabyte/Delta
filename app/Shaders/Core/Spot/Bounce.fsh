#version 460
#pragma optionNV(unroll all)
#package "camera"

struct Light_Struct {
	mat4 mMatrix;
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float ShadowSize_Recip;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;
};

struct Volume_Data 
{
	vec4 BBox_Max;
	vec4 BBox_Min;
	int samples;
	int resolution;
	float spread;
	float R_wcs;
	float factor;
};

layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};

layout (std430, binding = 6) readonly buffer Light_Buffer {
	Light_Struct buffers[];
};

layout (std430, binding = 7) readonly buffer GI_Volume_Attribs {			
	Volume_Data volume_data;
};

layout (location = 0) flat in int BufferIndex;

layout (location = 0) out vec4 GI_Out1; 
layout (location = 1) out vec4 GI_Out2; 
layout (location = 2) out vec4 GI_Out3; 
layout (location = 3) out vec4 GI_Out4; 

layout (binding = 0) uniform sampler2DArray ShadowPos;			// RSM position
layout (binding = 1) uniform sampler2DArray ShadowNormal; 		// RSM normals
layout (binding = 2) uniform sampler2DArray ShadowFlux;  		// RSM vpl flux
layout (binding = 3) uniform sampler2DArray ShadowPos_Static;	// RSM position
layout (binding = 4) uniform sampler2DArray ShadowNormal_Static;// RSM normals
layout (binding = 5) uniform sampler2DArray ShadowFlux_Static;	// RSM vpl flux
layout (binding = 6) uniform sampler2D CameraDepth;		  		// Camera depth buffer
layout (binding = 7) uniform sampler3D Noise;       			// A pre-computed 3D noise texture (32X32X32). Value range (r,g,b): [0,1] 
// #define DEPTH_OCCL    // if defined, depth-based RSM sample occlusion is enabled.


vec4 SHBasis (const in vec3 dir) 
{ 
    float L00  					= 0.282095; 
    float L1_1 					= 0.488603 * dir.y; 
    float L10  					= 0.488603 * dir.z; 
    float L11  					= 0.488603 * dir.x; 
    return vec4 				(L11, L1_1, L10, L00); 
}

void RGB2SH (in vec3 dir, in vec3 L, out vec4 sh_r, out vec4 sh_g, out vec4 sh_b) 
{ 
    vec4 sh 					= SHBasis (dir); 
    sh_r 						= L.r * sh; 
    sh_g 						= L.g * sh; 
    sh_b 						= L.b * sh; 
}

vec3 PointWCS2CSS(in vec3 p) 
{ 
    vec4 p_css = cameraBuffer.pMatrix * cameraBuffer.vMatrix * vec4(p,1); 
    return p_css.xyz/p_css.w; 
} 

vec2 ShadowProjection( in vec3 WorldPos ) 
{ 	
	vec4 LightSpacePos 			= buffers[indexes[BufferIndex]].lightPV * vec4(WorldPos, 1.0f);
	
	vec3 ProjCoords 			= LightSpacePos.xyz / LightSpacePos.w;                                  
    vec2 UVCoords 				= 0.5f * ProjCoords.xy + 0.5f; 
    return 						clamp(UVCoords, vec2(0.0f), vec2(1.0f));
} 

void TextureLookup(vec2 uv, int ShadowSpot, mat4 InversePV, out vec3 rsmPos, out vec3 rsmColor, out vec3 rsmNormal) 
{
	const vec3 uv_array_lookup	= vec3(uv, ShadowSpot);
	const float depth1			= texture(ShadowPos, uv_array_lookup).r; 
	const float depth2			= texture(ShadowPos_Static, uv_array_lookup).r; 
	
	if (depth1 > depth2) {	
		const vec4 World_Pos		= InversePV * vec4(vec3(uv, depth1) * 2.0f - 1.0f, 1.0f);	
		rsmPos						= World_Pos.xyz / World_Pos.w;
		rsmColor					= texture(ShadowFlux, uv_array_lookup).rgb; 
		rsmNormal 					= normalize(texture(ShadowNormal, uv_array_lookup).rgb);	
	}	
	else {
		const vec4 World_Pos		= InversePV * vec4(vec3(uv, depth2) * 2.0f - 1.0f, 1.0f);	
		rsmPos						= World_Pos.xyz / World_Pos.w;
		rsmColor					= texture(ShadowFlux_Static, uv_array_lookup).rgb; 
		rsmNormal 					= normalize(texture(ShadowNormal_Static, uv_array_lookup).rgb);	
	}
}

void BounceFromShadow(in vec3 extents, in vec3 RHCellSize, in vec3 RHCenter, in vec2 RHUV, in mat4 InversePV, in int ShadowSpot)
{
	// Variable Initialization
    float dist, dist_min = volume_data.R_wcs, dist_max = 0.0f, dist_ave = 0.0, FF; 
	vec3 rsmColor, rsmPos, rsmNormal, color; 
    vec4 SH_dist_ave 			= vec4(0.0);
    vec4 SHr 					= vec4(0.0);  
    vec4 SHg 					= vec4(0.0); 
    vec4 SHb 					= vec4(0.0); 
	int totalSamples 			= volume_data.samples;
	
	for (int i = 0; i < totalSamples; ++i) 
    { 
		// produce a new sample location on the RSM texture
        vec3 rnd 				= 2.0f * texture(Noise, 14 * RHCenter / extents + vec3(i,0,0) / totalSamples).xyz - 1.0f; 
		vec2 uv 				= RHUV + vec2( rnd.x * volume_data.spread * cos(6.283 * rnd.y), rnd.x * volume_data.spread * sin(6.283 * rnd.y) ); 		
		vec3 uv_array_lookup	= vec3(uv, ShadowSpot);
		
		// Lookup all 3 textures
		TextureLookup(uv, ShadowSpot, InversePV, rsmPos, rsmColor, rsmNormal);
		
        // produce a new sampling location in the RH stratum
		vec3 samplePos			= RHCenter + (0.5f * rnd) * RHCellSize;		
		
		// Normalize distance to RSM sample
        dist 					= distance(samplePos, rsmPos) / volume_data.R_wcs; 
		
        // Determine the incident direction. 
        // Avoid very close samples (and numerical instability problems)
        vec3 dir 				= ( dist <= 0.007f ) ? vec3(0.0f) : normalize( samplePos - rsmPos );
		float dotprod 			= max(dot(dir, rsmNormal), 0.0f); 
		FF 						= dotprod / (0.1f + dist * dist);	
		
		#ifdef DEPTH_OCCL 
		// ---- Depth-buffer-based RSM sample occlusion
		// Initialize visibility to 1
		float depth_visibility = 1.0; 

		// set number of visibility samples along the line of sight. Can be set with #define
		float vis_samples = 8.0; // 3 to 8
		vec3 Qj; 
		vec3 Qcss; 
		
		// Check if the current RH point is hidden from view. If it is, then "visible" line-of-sight 
		// samples should also be hidden from view. If the RH point is visible to the camera, the 
		// same should hold for the visibility samples in order not to attenuate the light.
		Qcss = PointWCS2CSS(samplePos); 
		float rh_visibility = Qcss.z < (2.0 * texture(CameraDepth, 0.5 * Qcss.xy + vec2(0.5)).r -1.0) * 1.1 ? 1.0 : -1.0; 

		// Estimate attenuation along line of sight
		for (int j = 1; j < vis_samples; ++j) { 
			// determine next point along the line of sight
			Qj = rsmPos + (j / vis_samples) * (RHCenter - rsmPos); 
			Qcss = PointWCS2CSS(Qj); 
			// modulate the visibility according to the number of hidden LoS samples
			depth_visibility -= rh_visibility * Qcss.z < rh_visibility * (2.0 * texture(CameraDepth, 0.5 * Qcss.xy + vec2(0.5)).r - 1.0) ? 0.0 : 1.0 / vis_samples; 
		} 
		depth_visibility = clamp(depth_visibility, 0, 1);
		FF *= depth_visibility;
		#endif 
		
		color 					= rsmColor * FF;
		
		// encode radiance into SH coefs and accumulate to RH		
		vec4 shr, shg, shb; 		
		RGB2SH					(dir, color, shr, shg, shb); 
		SHr  				   += shr; 
		SHg					   += shg; 
		SHb					   += shb; 
		
		// update distance measurements
		dist_max				= ( dist > dist_max ) ? dist : dist_max; 
		dist_min				= ( dist < dist_min ) ? dist : dist_min; 
		dist_ave	   		   += dist; 
	} 

	// cast samples to float to resolve some weird compiler issue. 
	SHr						   /= float( 3.14159f * float(totalSamples) ); 
	SHg						   /= float( 3.14159f * float(totalSamples) ); 
	SHb						   /= float( 3.14159f * float(totalSamples) ); 
	dist_ave				   /= float( totalSamples ); 	
	
	GI_Out1 					= vec4( dist_min, volume_data.R_wcs - dist_max, dist_ave, 1.0f );
	GI_Out2 					= SHr; 
	GI_Out3 					= SHg; 
	GI_Out4 					= SHb; 
}

void main()
{	
	// Get current RH's world pos
	vec3 bbox_max 				= volume_data.BBox_Max.xyz;
	vec3 bbox_min 				= volume_data.BBox_Min.xyz;
	vec3 pos					= vec3(gl_FragCoord.x, gl_FragCoord.y, gl_Layer);
    vec3 extents 				= (bbox_max - bbox_min).xyz; 
	vec3 RHCellSize				= extents / (volume_data.resolution);
    vec3 RHCenter 				= bbox_min + pos * RHCellSize; 	
	
	// RH -> light space, get sampling disk center
    vec2 RHUV 					= ShadowProjection(RHCenter); 
	
	// Perform light bounce operation
	BounceFromShadow(extents, RHCellSize, RHCenter, RHUV, buffers[indexes[BufferIndex]].InverseLightPV, buffers[indexes[BufferIndex]].Shadow_Spot);
}