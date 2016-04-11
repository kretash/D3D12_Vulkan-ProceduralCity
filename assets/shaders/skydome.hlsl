#define PI 3.1415926535897932384626433832795028841971693993f

struct VSInput {
  float4 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
  float3 tangent : TANGENT;
  float3 bitangent : BITANGENT;
};

struct PSInput {
  float4 position : SV_POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float height : HEIGHT;
  float angle : ANGLE;
  float3 distance : DISTANCE;
};

cbuffer InstanceData : register( b0 ) {
  float4x4 mvp;
  float4x4 model;
  float4x4 normal_matrix;

  uint d_texture_id;
  uint n_texture_id;
  uint s_texture_id;

  uint pad[13];
};

cbuffer FrameData : register( b1 ) {
  float3 light_pos;
  float sky_color;
  float3 fog_color;
  float ambient_intensity;
  float3 sun_light_intensity;
  float3 eye_view;
};

Texture2D textures[2048] : register( t0 );
SamplerState	mySampler	: register( s0 );

static const float3 all_horizon_color[5] =
{
  float3( 69.0f / 256.f, 56.0f / 256.f, 47.0f / 256.f ),    // Night
  float3( 239.0f / 256.f, 96.0f / 256.f, 40.0f / 256.f ),   // Sunset
  float3( 127.0f / 256.f, 190.0f / 256.f, 233.0f / 256.f ), // Day 1
  float3( 137.0f / 256.f, 199.0f / 256.f, 236.0f / 256.f ), // Day 2
  float3( 147.0f / 256.f, 210.0f / 256.f, 243.0f / 256.f )  // Day 3
};
static const float3 all_mid_sky_color[5] =
{
  float3( 52.0f / 256.f, 46.0f / 256.f, 37.0f / 256.f ),    // Night
  float3( 176.0f / 256.f, 155.0f / 256.f, 112.0f / 256.f ), //Sunset
  float3( 64.0f / 256.f, 128.0f / 256.f, 210.0f / 256.f ),  // Day 1
  float3( 74.0f / 256.f, 148.0f / 256.f, 219.0f / 256.f ),  // Day 2
  float3( 84.0f / 256.f, 158.0f / 256.f, 225.0f / 256.f )   // Day 3
};
static const float3 all_top_sky_color[5] =
{
  float3( 42.0f / 256.f, 33.0f / 256.f, 26.0f / 256.f ),   // Night
  float3( 94.0f / 256.f, 124.0f / 256.f, 148.0f / 256.f ), // Sunset
  float3( 53.0f / 256.f, 113.0f / 256.f, 186.0f / 256.f ), // Day
  float3( 63.0f / 256.f, 123.0f / 256.f, 196.0f / 256.f ), // Day
  float3( 73.0f / 256.f, 133.0f / 256.f, 226.0f / 256.f )  // Day
};

PSInput VSMain( VSInput input ) {
  PSInput result;

  result.position = mul( float4( input.position.xyz, 1.0f ), mvp );

  result.normal = input.normal;
  result.uv = input.uv;

  result.distance = ( mul( float4( input.position.xyz, 1.0f ), model ) ).xyz;
  result.height = result.distance.y / 1000.f + 0.25f;

  float3 sky_dome = result.distance / 1000.0f;
  result.angle = dot( sky_dome.xyz, float3( sky_dome.x, 0.0f, sky_dome.z ) );

  result.distance = input.position.xyz;
  return result;
}

float4 PSMain( PSInput input ) : SV_TARGET
{
  float3 final_color = float3( 0.0f, 0.0f, 0.0f );
  float angle = 1.0f - saturate( input.angle );

  if( angle < 0.1f ) final_color = float3( 1.0f, 0.0f, 0.0f );
  if( angle > 0.9f ) final_color = float3( 0.0f, 0.0f, 1.0f );

  float angle_threshold = 0.025f;
  float angle_divider = angle_threshold * 10.0f;
  float o_angle_divider = 10.0f - angle_divider;

  float3 horizon_color = float3( 0.0f, 0.0f, 0.0f );
  float3 mid_sky_color = float3( 0.0f, 0.0f, 0.0f );
  float3 top_sky_color = float3( 0.0f, 0.0f, 0.0f );

  if( sky_color < 1.0f ) {
    horizon_color = all_horizon_color[0] * ( 1.0f - sky_color ) + all_horizon_color[1] * ( sky_color );
    mid_sky_color = all_mid_sky_color[0] * ( 1.0f - sky_color ) + all_mid_sky_color[1] * ( sky_color );
    top_sky_color = all_top_sky_color[0] * ( 1.0f - sky_color ) + all_top_sky_color[1] * ( sky_color );
  } else if( sky_color < 2.0f ) {
    float new_sky_color = sky_color - 1.0f;
    horizon_color = all_horizon_color[1] * ( 1.0f - new_sky_color ) + all_horizon_color[2] * ( new_sky_color );
    mid_sky_color = all_mid_sky_color[1] * ( 1.0f - new_sky_color ) + all_mid_sky_color[2] * ( new_sky_color );
    top_sky_color = all_top_sky_color[1] * ( 1.0f - new_sky_color ) + all_top_sky_color[2] * ( new_sky_color );
  } else if( sky_color < 3.0f ) {
    float new_sky_color = sky_color - 2.0f;
    horizon_color = all_horizon_color[2] * ( 1.0f - new_sky_color ) + all_horizon_color[3] * ( new_sky_color );
    mid_sky_color = all_mid_sky_color[2] * ( 1.0f - new_sky_color ) + all_mid_sky_color[3] * ( new_sky_color );
    top_sky_color = all_top_sky_color[2] * ( 1.0f - new_sky_color ) + all_top_sky_color[3] * ( new_sky_color );
  } else {
    float new_sky_color = sky_color - 3.0f;
    horizon_color = all_horizon_color[3] * ( 1.0f - new_sky_color ) + all_horizon_color[4] * ( new_sky_color );
    mid_sky_color = all_mid_sky_color[3] * ( 1.0f - new_sky_color ) + all_mid_sky_color[4] * ( new_sky_color );
    top_sky_color = all_top_sky_color[3] * ( 1.0f - new_sky_color ) + all_top_sky_color[4] * ( new_sky_color );
  }

  if( angle < angle_threshold ) {
    final_color = lerp( horizon_color, mid_sky_color, ( angle*10.0f ) / angle_divider );
  } else {
    final_color = lerp( mid_sky_color, top_sky_color, ( angle*10.0f ) / o_angle_divider - angle_threshold );
  }

  float3 light_dir = normalize( light_pos );
  light_dir *= 1000.0f;

  float distance = abs( length( input.distance.xyz - light_dir ) );
  float sun = 0.0f;

  if( distance <= 100.f ) {
    float range = saturate( 1.0f - ( distance / 100.0f ) );
    sun = lerp( 0.0f, 0.1f, range );

    if( distance >= 0.0f && distance < 50.0f ) {
      float range = saturate( 1.0f - ( distance / 60.0f ) );
      sun = lerp( 0.0f, 0.3f, range );

      if( distance >= 0.0f && distance < 20.0f ) {
        float range = saturate( 1.0f - ( distance / 40.0f ) );
        sun = lerp( 0.0f, 1.5f, range );
      }
    }
  }

  final_color += sun;
  return float4( final_color, 1.0f );
}
