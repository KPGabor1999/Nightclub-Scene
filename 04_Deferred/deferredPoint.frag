#version 130

in vec2 vs_out_tex;
in vec3 out_eye_pos;

out vec4 fs_out_col;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform mat4 projMatrixInv;
uniform mat4 viewMatrixInv;
uniform vec3 lightPos;
uniform vec4 Ld;
uniform float brightness;

// Referring to:
	// https://learnopengl.com/Getting-started/Coordinate-Systems
	// https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
vec3 worldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(vs_out_tex * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = (projMatrixInv * clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = viewMatrixInv * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
	fs_out_col = vec4(0);

	// Diffuse light:
	float depth = texture(depthTexture, vs_out_tex).x;
	vec3 pos = worldPosFromDepth(depth);
	vec3 lightDir = normalize(lightPos - pos);
	vec4 Kd = texture( diffuseTexture, vs_out_tex );
	vec3 n = texture( normalTexture, vs_out_tex ).rgb;
	if (n != vec3(0)) n = normalize(n * 2.0 - 1.0);		//Scaling it back to [-1,1].
	float di = clamp(dot(n, lightDir), 0, 1);
	
	vec4 diffuse_color = Ld*Kd*di;
	float dist = distance(lightPos, pos);
	float falloff = brightness / dist / dist;

	fs_out_col += falloff * diffuse_color;
}