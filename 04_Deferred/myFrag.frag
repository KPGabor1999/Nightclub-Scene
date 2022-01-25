#version 400

// per-fragment attributes coming from the pipeline
in vec3 vs_out_normal;
in vec2 vs_out_tex0;

// multiple outputs are directed into different color textures by the FBO
layout(location=0) out vec4 fs_out_diffuse;
layout(location=1) out vec3 fs_out_normal;

// Different geometries may be drawn with different textures attached
uniform sampler2D texImage;

void main(void) {
	fs_out_diffuse = vec4(texture(texImage, vs_out_tex0).xyz, 1);
	
	if(vs_out_normal == vec3(0)){
		fs_out_normal = vs_out_normal;				// Don't scale zero vectors (background pixels).
	} else {
		fs_out_normal = vs_out_normal / 2.0 + 0.5;	// Scale rgb values between 0 and 1.
	}
}