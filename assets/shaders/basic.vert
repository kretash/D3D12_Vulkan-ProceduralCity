#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (binding = 0) uniform UBO1
{
  mat4 mvp;
  mat4 model;
  mat4 normal_matrix;

  uint d_texture_id;
  uint n_texture_id;
  uint s_texture_id;

  uint pad[13];
} istance;

layout (binding = 1) uniform UBO2
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

layout (location = 0) out vec3 frag_normal;

void main() 
{
	frag_normal = normal;

  	frag_normal = ( vec4( normal, 0.0f) * inverse(istance.model) ).xyz;
  	frag_normal = normalize( frag_normal );

	gl_Position = istance.mvp * vec4(position.xyz, 1.0);
}
