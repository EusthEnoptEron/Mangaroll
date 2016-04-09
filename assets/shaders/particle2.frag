#version 300 es

precision lowp float;

in vec4 pColor;
uniform sampler2D Texture0;
out vec4 outColor;
void main()
{
   outColor = pColor;
}