#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


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

layout (location = 0) out vec4 frag_color;

layout (location = 0) in vec3 frag_distance;
layout (location = 1) in float frag_height;
layout (location = 2) in float frag_angle;

void main() 
{
  frag_color = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
  float angle = 1.0f - clamp( frag_angle, 0.0f, 1.0f );

  if( angle < 0.1f ) frag_color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
  if( angle > 0.9f ) frag_color = vec4( 0.0f, 0.0f, 1.0f, 1.0f );

  float angle_threshold = 0.025f;
  float angle_divider = angle_threshold * 10.0f;
  float o_angle_divider = 10.0f - angle_divider;

  vec3 horizon_color = vec3( 0.0f, 0.0f, 0.0f );
  vec3 mid_sky_color = vec3( 0.0f, 0.0f, 0.0f );
  vec3 top_sky_color = vec3( 0.0f, 0.0f, 0.0f );

  const vec3 all_horizon_color[5] =
  {
    vec3( 69.0f / 256.f, 56.0f / 256.f, 47.0f / 256.f ),    // Night
    vec3( 239.0f / 256.f, 96.0f / 256.f, 40.0f / 256.f ),   // Sunset
    vec3( 127.0f / 256.f, 190.0f / 256.f, 233.0f / 256.f ), // Day 1
    vec3( 137.0f / 256.f, 199.0f / 256.f, 236.0f / 256.f ), // Day 2
    vec3( 147.0f / 256.f, 210.0f / 256.f, 243.0f / 256.f )  // Day 3
  };
  const vec3 all_mid_sky_color[5] =
  {
    vec3( 52.0f / 256.f, 46.0f / 256.f, 37.0f / 256.f ),    // Night
    vec3( 176.0f / 256.f, 155.0f / 256.f, 112.0f / 256.f ), //Sunset
    vec3( 64.0f / 256.f, 128.0f / 256.f, 210.0f / 256.f ),  // Day 1
    vec3( 74.0f / 256.f, 148.0f / 256.f, 219.0f / 256.f ),  // Day 2
    vec3( 84.0f / 256.f, 158.0f / 256.f, 225.0f / 256.f )   // Day 3
  };
  const vec3 all_top_sky_color[5] =
  {
    vec3( 42.0f / 256.f, 33.0f / 256.f, 26.0f / 256.f ),   // Night
    vec3( 94.0f / 256.f, 124.0f / 256.f, 148.0f / 256.f ), // Sunset
    vec3( 53.0f / 256.f, 113.0f / 256.f, 186.0f / 256.f ), // Day
    vec3( 63.0f / 256.f, 123.0f / 256.f, 196.0f / 256.f ), // Day
    vec3( 73.0f / 256.f, 133.0f / 256.f, 226.0f / 256.f )  // Day
  };

  if( world.sky_color < 1.0f ) {
    horizon_color = all_horizon_color[0] * ( 1.0f - world.sky_color ) + all_horizon_color[1] * ( world.sky_color );
    mid_sky_color = all_mid_sky_color[0] * ( 1.0f - world.sky_color ) + all_mid_sky_color[1] * ( world.sky_color );
    top_sky_color = all_top_sky_color[0] * ( 1.0f - world.sky_color ) + all_top_sky_color[1] * ( world.sky_color );
  } else if( world.sky_color < 2.0f ) {
    float new_sky_color = world.sky_color - 1.0f;
    horizon_color = all_horizon_color[1] * ( 1.0f - new_sky_color ) + all_horizon_color[2] * ( new_sky_color );
    mid_sky_color = all_mid_sky_color[1] * ( 1.0f - new_sky_color ) + all_mid_sky_color[2] * ( new_sky_color );
    top_sky_color = all_top_sky_color[1] * ( 1.0f - new_sky_color ) + all_top_sky_color[2] * ( new_sky_color );
  } else if( world.sky_color < 3.0f ) {
    float new_sky_color = world.sky_color - 2.0f;
    horizon_color = all_horizon_color[2] * ( 1.0f - new_sky_color ) + all_horizon_color[3] * ( new_sky_color );
    mid_sky_color = all_mid_sky_color[2] * ( 1.0f - new_sky_color ) + all_mid_sky_color[3] * ( new_sky_color );
    top_sky_color = all_top_sky_color[2] * ( 1.0f - new_sky_color ) + all_top_sky_color[3] * ( new_sky_color );
  } else {
    float new_sky_color = world.sky_color - 3.0f;
    horizon_color = all_horizon_color[3] * ( 1.0f - new_sky_color ) + all_horizon_color[4] * ( new_sky_color );
    mid_sky_color = all_mid_sky_color[3] * ( 1.0f - new_sky_color ) + all_mid_sky_color[4] * ( new_sky_color );
    top_sky_color = all_top_sky_color[3] * ( 1.0f - new_sky_color ) + all_top_sky_color[4] * ( new_sky_color );
  }

  if( angle < angle_threshold ) {
   frag_color = vec4( mix( horizon_color, mid_sky_color, clamp( ( angle*10.0f ) / angle_divider , 0.0f, 1.0f)), 1.0f);
  } else {
   frag_color = vec4( mix( mid_sky_color, top_sky_color, clamp( ( angle*10.0f ) / o_angle_divider - angle_threshold , 0.0f, 1.0f)), 1.0f);
  }

  //frag_color.xyz = mix( mid_sky_color, vec3( 1.0f, 1.0f, 1.0f ), 0.5f);

   vec3 light_dir = normalize( world.light_pos );
   light_dir *= 1000.0f;

   float distance = abs( length( frag_distance.xyz - light_dir ) );
   float sun = 0.0f;

   if( distance <= 100.f ) {
   float range = clamp( 1.0f - ( distance / 100.0f ), 0.0f, 1.0f  );
   sun = mix( 0.0f, 0.1f, range );

     if( distance >= 0.0f && distance < 50.0f ) {
       float range = clamp( 1.0f - ( distance / 60.0f ), 0.0f, 1.0f  );
       sun = mix( 0.0f, 0.3f, range );

       if( distance >= 0.0f && distance < 20.0f ) {
         float range = clamp( 1.0f - ( distance / 40.0f ), 0.0f, 1.0f  );
         sun = mix( 0.0f, 1.5f, range );
       }
     }
   }

  frag_color += vec4( sun, sun, sun, 1.0f);
  //frag_color = clamp( frag_color, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));

}