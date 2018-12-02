/* Board Shader. */
#version 460
#package "Game\GameBuffer"

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int Index;
layout (location = 2) in float Dot;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D BoardTexture;
layout (binding = 1) uniform sampler2D ScoreTexture;
layout (binding = 2) uniform sampler2D TimeTexture;


void main()
{		
	switch (Index) {
	case 0:		
		const float pulseAmount = calcPulseAmount(gl_FragCoord.y);
		const vec3 boardColor = (colorScheme * pulseAmount) * (colorScheme * pulseAmount) * (colorScheme / M_PI) * Dot;
		FragColor = vec4( boardColor, 1 );
		FragColor.xyz += (FragColor.xyz * excitementLinear) * multiplier;
		break;
	case 1:
		FragColor = texture(BoardTexture, TexCoord);
		break;
	case 2:
		FragColor = vec4(texture(ScoreTexture, TexCoord).xyz, 1);
		break;
	case 3:
		FragColor = vec4(texture(TimeTexture, TexCoord).xyz, 1);
		break;
	};
}