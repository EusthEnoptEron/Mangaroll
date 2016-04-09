#version 300 es

precision lowp float;

in vec4 pcolor;
in mat2 particleRot;
uniform sampler2D Texture0;
out vec4 outColor;
void main()
{
    vec2 texCoord = particleRot * ( gl_PointCoord - vec2( 0.5 ) ); //use our rotation matrix to modyfi texture coordinates
    outColor = texture( Texture0 , texCoord + vec2( 0.5 ) ) * pcolor + vec4(1.0f,1.0f,1.0f,1.0f);
	
}