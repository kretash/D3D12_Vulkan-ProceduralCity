#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 frag_position;
layout (location = 1) in vec3 frag_normal;
layout (location = 2) in vec2 frag_uv;
layout (location = 3) in float distance;
layout (location = 4) in vec3 spherical_harmonics;
layout (location = 5) in vec3 eye_dir;
layout (location = 6) in vec3 light_dir;
layout (location = 7) in mat3 TBN;

layout (location = 11) in vec3 frag_tangent;

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

layout (set = 1, binding = 1) uniform sampler2D m_texture[64];

void main() 
{
	vec3 ambient_color = vec3( 0.0f, 0.0f, 0.0f );
	vec3 diffuse_color = vec3( 0.0f, 0.0f, 0.0f );
	vec3 specular_color = vec3( 0.0f, 0.0f, 0.0f );

	vec3 texture_color = texture(m_texture[instance.d_texture_id], frag_uv).xyz;
	vec3 normal_map = texture(m_texture[instance.n_texture_id], frag_uv).xyz;
	vec3 specular_map = texture(m_texture[instance.s_texture_id], frag_uv).xyz;

	vec3 normal = normalize( normal_map * 2.0f - 1.0f ) * TBN;
	float specular_map_r = specular_map.x;
	
	//Ambient
	ambient_color = spherical_harmonics * world.ambient_intensity * texture_color;
	ambient_color = max( vec3( 0.0f, 0.0f, 0.0f), ambient_color );
	
	//Diffuse
	float dot_product = clamp( dot( normal, light_dir ), 0.0f, 1.0f );
	diffuse_color = dot_product * world.sun_light_intensity * texture_color;
	diffuse_color = max( vec3( 0.0f, 0.0f, 0.0f), diffuse_color );

	//Specular
	if( dot_product > 0.0 ) {
		vec3 half_vector = normalize( light_dir - eye_dir );
		float specular = max( 0.0f, dot( half_vector, normal ));
		specular_color = vec3( pow( specular, 16.0f ) * specular_map_r );
	}
		
	frag_color.xyz = ambient_color + diffuse_color + specular_color; 
	frag_color.xyz = mix( frag_color.xyz, world.fog_color, distance );
	//frag_color.xyz = specular_color;
	frag_color.w = 1.0f;

	//frag_color.xyz = frag_tangent;
}