#version 300 es

in vec3 Position;
in vec4 VertexColor;
in vec2 TexCoord;
uniform mat4 Viewm;
uniform mat4 Projectionm;
uniform mat4 Modelm;
out vec4 fragmentColor;
out vec2 oTexCoord;
out vec2 oPosition;

void main()
{
	gl_Position = Projectionm * Viewm * vec4( Position, 1.0 );
	fragmentColor = VertexColor;
	oTexCoord = TexCoord;
	oPosition = Position.xz;
}