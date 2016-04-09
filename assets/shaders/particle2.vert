#version 300 es

precision lowp float; 
in vec3 Position;
in vec4 VertexColor;
in vec4 Normal; // Center position

uniform mat4 Projectionm;
uniform mat4 Viewm;

out vec4 pColor;

void main()
{
    gl_Position = Projectionm * (Viewm * vec4(Normal.xyz, 1.0f) + vec4(Position * Normal.w, 0.0f));

    pColor = VertexColor;
}