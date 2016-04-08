#version 300 es
in lowp vec4 fragmentColor;
uniform sampler2D Texture0;
in lowp vec2 oTexCoord;
in lowp vec2 oPosition;
out lowp vec4 outColor;
uniform lowp vec3 uTint;

void main()
{
	lowp float dist = length(oPosition);
	lowp vec4 col = (texture( Texture0, oTexCoord * 150.0f ) 
						* vec4(1.0f, 1.5f, 1.5f, 1.0f)) 
						* clamp(1.0f - (dist / 6.0f), 0.0f, 1.0f);
	//col.rgba = vec4(0.0f,1.0f,1.0f,1.0f);
	outColor = col;
}