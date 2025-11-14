#version 330

in vec3 a_Position;
in vec4 a_Color;

out vec4 v_Color;

void main()
{
	vec4 newPosition = vec4(a_Position.x,a_Position.y,a_Position.z,1);
	gl_Position = newPosition;

	v_Color = a_Color;
}
 