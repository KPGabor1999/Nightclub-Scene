#version 130

vec4 positions[4] = vec4[4](
	vec4(-1,-1, 0, 1),
	vec4( 1,-1, 0, 1),
	vec4(-1, 1, 0, 1),
	vec4( 1, 1, 0, 1)
);

vec2 texCoords[4] = vec2[4](
	vec2(0,0),
	vec2(1,0),
	vec2(0,1),
	vec2(1,1)
);

in vec3 in_eye_pos;
out vec2 vs_out_tex;
out vec3 out_eye_pos;

void main()
{
	gl_Position = positions[gl_VertexID];
	vs_out_tex	= texCoords[gl_VertexID];
	out_eye_pos = in_eye_pos;
}