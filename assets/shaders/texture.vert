#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (location = 0) out vec4 frag_position;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec2 frag_uv;
layout (location = 3) out float distance;
layout (location = 4) out vec3 spherical_harmonics;
layout (location = 5) out vec3 eye_dir;
layout (location = 6) out vec3 light_dir;
layout (location = 7) out mat3 TBN;

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


const float scale_factor = 0.8f;
const float C1 = 0.429043f;
const float C2 = 0.511664f;
const float C3 = 0.743125f;
const float C4 = 0.886227f;
const float C5 = 0.247708f;

// old town square lighting
const vec3 L00  = vec3( 0.871297,  0.875222,  0.864470);
const vec3 L1m1 = vec3( 0.175058,  0.245335,  0.312891);
const vec3 L10  = vec3( 0.034675,  0.036107,  0.037362);
const vec3 L11  = vec3(-0.004629, -0.029448, -0.048028);
const vec3 L2m2 = vec3(-0.120535, -0.121160, -0.117507);
const vec3 L2m1 = vec3( 0.003242,  0.003624,  0.007511);
const vec3 L20  = vec3(-0.028667, -0.024926, -0.020998);
const vec3 L21  = vec3(-0.077539, -0.086325, -0.091591);
const vec3 L22  = vec3(-0.161784, -0.191783, -0.219152);

void main() 
{
  	frag_normal = ( vec4( normal, 0.0f) * instance.normal_matrix ).xyz;
  	frag_normal = normalize( frag_normal );

	spherical_harmonics =  C1 * L22 * (frag_normal.x * frag_normal.x - frag_normal.y * frag_normal.y) +
            C3 * L20 * frag_normal.z * frag_normal.z +
            C4 * L00 -
            C5 * L20 +
            2.0 * C1 * L2m2 * frag_normal.x * frag_normal.y +
            2.0 * C1 * L21  * frag_normal.x * frag_normal.z +
            2.0 * C1 * L2m1 * frag_normal.y * frag_normal.z +
            2.0 * C2 * L11  * frag_normal.x +
            2.0 * C2 * L1m1 * frag_normal.y +   
            2.0 * C2 * L10  * frag_normal.z;


	frag_position = instance.mvp * vec4(position.xyz, 1.0);
	distance = frag_position.z * 0.001f;
	distance = clamp( distance, 0.0f, 1.0f );
  	frag_uv = uv;

	TBN = transpose( mat3( tangent, bitangent, frag_normal ));

	vec4 pos = vec4( position, 1.0f) * instance.model;
	eye_dir = normalize( pos.xyz - world.eye_view );

	light_dir = normalize( -world.light_pos );
  	light_dir = ( vec4( light_dir, 0.0f) * instance.normal_matrix ).xyz;

	gl_Position = frag_position, 1.0f;
}
