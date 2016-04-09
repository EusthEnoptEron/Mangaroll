#version 300 es

precision lowp float; //mobile device so we need to specify precison (mediup will be good enought)

in vec3 Position;
in vec4 VertexColor;
in vec3 Normal; // Rotvec

uniform mat4 Projectionm;
uniform mat4 Viewm;

out vec4 pcolor;
out mat2 particleRot;

void main()
{
    //we need to build our 2x2 matrix so we can rotate texture coordinates that comes from GPU
    particleRot[0] = vec2( Normal.y , -Normal.x );
    particleRot[1] = Normal.xy;
    /*
    [cos(angle),-sin(angle),
     sin(angle), cos(angle)]
    */

    pcolor = VertexColor;
  
    //to explain this:
    //let say min size is 2 and maximum is 5 and reduction is also 2 so the size will move
    //from 2 -> 5 -> 3 (2->5->(5-2)) so it is close to easyOutBack function

    gl_PointSize = mix( pSize.z * ( Normal.z - 1.0 ) + pSize.y , pSize.x, Normal.z );
    gl_Position = Projectionm * (Viewm * vec4(Position, 1.0f));
}