#version 300 es
in lowp vec4 fragmentColor;
uniform sampler2D Texture0;
in lowp vec2 oTexCoord;
in lowp vec2 oPosition;
out lowp vec4 outColor;
uniform lowp vec3 uTint;
uniform lowp vec3 uFalloffDistance;
uniform lowp vec3 uTime;

void main()
{
	lowp float dist = length(oPosition);
	lowp float strength = 0.0f;
	//lowp float halfFalloff = uFalloffDistance * 0.5f;
	//strength = dist > uFalloffDistance.y 
	//	? clamp(1.0f - ((dist - halfFalloff) / halfFalloff), 0.0f, 1.0f)
	//	: clamp((dist / halfFalloff), 0.0f, 1.0f);

	strength = clamp(
				clamp(1.0f - ((dist - uFalloffDistance.y) / (uFalloffDistance.x - uFalloffDistance.y)), 0.0f, 1.0f)
		      * clamp((dist - uFalloffDistance.z) / (uFalloffDistance.y - uFalloffDistance.z), 0.0f, 1.0f)
			  , 0.0f, 1.0f);


	lowp vec4 col = (texture( Texture0, oTexCoord * 150.0f ) 
					* vec4(uTint, 1.0f)) 
					* strength;
	//col.rgb = vec3(0.75f) - col.rgb;			
	//col.rgb = col.rgb * 0.97f + vec3(0.03f);
	//col.rgba = vec4(0.0f,1.0f,1.0f,1.0f);
	outColor = col;
}