#version 300 es
in lowp vec4 fragmentColor;
uniform sampler2D Texture0;
in lowp vec2 oTexCoord;
out lowp vec4 outColor;
uniform lowp float DisplayTime;
uniform lowp float Contrast;
uniform lowp float Brightness;

void main()
{
	lowp vec4 col = texture( Texture0, oTexCoord );
	//lowp float value = ((col.r + col.g + col.b) / 3.0f);
	//col.a *= DisplayTime + oTexCoord.s > 1.0f ? 1.0f : 0.0f;
	col.a = clamp(DisplayTime * 2.0f, 0.0f, 1.0f);
	col.rgb = (col.rgb - 0.5f) * (Contrast) + 0.5f;
	col.rgb += Brightness; 
	outColor = col;
	//outColor = fragmentColor;
}