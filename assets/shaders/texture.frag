#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 frag_normal;
layout (location = 0) out vec4 frag_color;

layout (set = 0, binding = 0) uniform UBO1
{
  mat4 mvp;
  mat4 model;
  mat4 normal_matrix;

  uint d_texture_id;
  uint n_texture_id;
  uint s_texture_id;

  uint pad[13];
} istance;

layout (set = 1, binding = 0) uniform UBO2
{
  vec3            light_pos;
  float           sky_color;
  vec3            fog_color;
  float           ambient_intensity;
  vec3            sun_light_intensity;
  float           padding;
  vec3            eye_view;
  mat4            view;

} world;

void main() 
{

	vec3 light_dir = vec3( -0.3f, 0.4f, -0.7f );
	vec3 color = vec3( 0.23f, 0.23f, 0.23f );
	float diffuse =  clamp( dot( frag_normal, light_dir ), 0.0f, 1.0f );

	vec3 c_lum = diffuse * color;

  	frag_color = vec4(c_lum, 1.0);
}