#version 300 es
in lowp vec4 fragmentColor;
uniform sampler2D Texture0;
uniform int IsSelected;
in lowp vec2 oTexCoord;
out lowp vec4 outColor;
void main()
{
	outColor = IsSelected == 1 ? fragmentColor : texture( Texture0, oTexCoord );
	//outColor = fragmentColor;
}