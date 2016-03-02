#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 frag_normal;
layout (location = 0) out vec4 frag_color;

void main() 
{

	vec3 light_dir = vec3( -0.3f, 0.4f, -0.7f );
	vec3 color = vec3( 1.0f, 0.23f, 0.23f );
	float diffuse =  clamp( dot( frag_normal, light_dir ), 0.0f, 1.0f );

	vec3 c_lum = color * diffuse;

  	frag_color = vec4(c_lum, 1.0);
}