/* Multiplier Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float NumberToRender;
layout (location = 2) flat in uint UseBackdrop;
layout (location = 0) out vec4 HeaderColor;

layout (binding = 1) uniform sampler2D Numbers;


void main()
{		
	const ivec2 Size = textureSize(Numbers, 0);
	const float ElementCount = 12.0f;	
	const float AtlasWidth = float(Size.x);
	const float ElementWidth = float(Size.x / ElementCount);
	
	const vec2 DigitIndex = vec2((TexCoord.x / ElementCount) + ((NumberToRender * ElementWidth) / AtlasWidth), TexCoord.y);
	const float pulseAmount = calcPulseAmount(gl_FragCoord.y);
	const vec4 boardColor = vec4((colorScheme * pulseAmount) * (colorScheme * pulseAmount) * (colorScheme / M_PI) * (UseBackdrop != 0 ? 0.5f : 1.0f), 1) * (multiplier > 1 ? 1.0f : 0.0f);	
	const vec4 DigitColor = texture(Numbers, DigitIndex) * boardColor * ((NumberToRender >= -0.5f) ? 1.0f : 0.0f);
	HeaderColor = DigitColor;
}