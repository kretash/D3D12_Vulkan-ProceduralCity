#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (set = 0, binding = 0) uniform UBO1
{
  mat4 mvp;
  mat4 model;
  mat4 normal_matrix;

  uint d_texture_id;
  uint n_texture_id;
  uint s_texture_id;

  uint pad[13];
} instance;

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

layout (location = 0) out vec3 frag_distance;
layout (location = 1) out float frag_height;
layout (location = 2) out float frag_angle;

void main() 
{
  frag_distance = ( instance.model *vec4( position.xyz, 1.0f ) ).xyz;
  frag_height = frag_distance.y / 1000.f + 0.25f;

  vec3 sky_dome = frag_distance / 1000.0f;
  frag_angle = dot( sky_dome.xyz, vec3( sky_dome.x, 0.0f, sky_dome.z ) );

	gl_Position = instance.mvp * vec4(position.xyz, 1.0);
}
