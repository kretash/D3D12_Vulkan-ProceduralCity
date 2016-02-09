
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
  float distance : DISTANCE;
  float3 spherical_harmonics : SPHERICAL_HARM;
  float2 uv : TEXCOORD0;
  float3 world : WORLD;
  float3 eye_view : EYE_VIEW;
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

static const float scale_factor = 0.8f;
static const float C1 = 0.429043f;
static const float C2 = 0.511664f;
static const float C3 = 0.743125f;
static const float C4 = 0.886227f;
static const float C5 = 0.247708f;

// old town square lighting
static const float3 L00 = float3( 0.871297, 0.875222, 0.864470 );
static const float3 L1m1 = float3( 0.175058, 0.245335, 0.312891 );
static const float3 L10 = float3( 0.034675, 0.036107, 0.037362 );
static const float3 L11 = float3( -0.004629, -0.029448, -0.048028 );
static const float3 L2m2 = float3( -0.120535, -0.121160, -0.117507 );
static const float3 L2m1 = float3( 0.003242, 0.003624, 0.007511 );
static const float3 L20 = float3( -0.028667, -0.024926, -0.020998 );
static const float3 L21 = float3( -0.077539, -0.086325, -0.091591 );
static const float3 L22 = float3( -0.161784, -0.191783, -0.219152 );

static const float3 horizon_color = float3( 239.0f / 256.f, 96.0f / 256.f, 40.0f / 256.f );
static const float3 mid_sky_color = float3( 176.0f / 256.f, 155.0f / 256.f, 112.0f / 256.f );
static const float3 top_sky_color = float3( 94.0f / 256.f, 124.0f / 256.f, 148.0f / 256.f );

PSInput VSMain( VSInput input ) {
  PSInput result;

  result.position = mul( float4( input.position.xyz, 1.0f ), mvp );

  result.distance = result.position.z * 0.001f;
  result.distance = clamp( result.distance, 0.0f, 1.0f );

  result.normal = mul( float4( input.normal.xyz, 0.0f ), model ).xyz;
  result.uv = input.uv;

  result.world = mul( float4( input.position.xyz, 1.0f ), model ).xyz;
  result.eye_view = eye_view;

  float3 frag_normal = mul( ( float3x3 ) normal_matrix, input.normal.xyz );
  frag_normal = normalize( frag_normal );

  result.spherical_harmonics = C1 * L22 * ( frag_normal.x * frag_normal.x - frag_normal.y * frag_normal.y ) +
    C3 * L20 * frag_normal.z * frag_normal.z +
    C4 * L00 -
    C5 * L20 +
    2.0 * C1 * L2m2 * frag_normal.x * frag_normal.y +
    2.0 * C1 * L21  * frag_normal.x * frag_normal.z +
    2.0 * C1 * L2m1 * frag_normal.y * frag_normal.z +
    2.0 * C2 * L11  * frag_normal.x +
    2.0 * C2 * L1m1 * frag_normal.y +
    2.0 * C2 * L10  * frag_normal.z;

  return result;
}

float4 PSMain( PSInput input ) : SV_TARGET
{
  float3 surface_color = float3( 182.0f / 256.f, 171.0f / 256.f, 153.0f / 256.f );

  float3 ambient_color = float3( 0.0f, 0.0f, 0.0f );
  float3 diffuse_color = float3( 0.0f, 0.0f, 0.0f );
  float3 specular_color = float3( 0.0f, 0.0f, 0.0f );

  float3 light_dir = normalize( light_pos );

  //Ambient
  //float moon_dot_product = dot( input.normal, normalize( float3( -19.0f, 1.0f, 5.3f ) ) );
  //ambient_color *= moon_dot_product;
  //ambient_color = float3( ambient_intensity, ambient_intensity, ambient_intensity );
  ambient_color = input.spherical_harmonics * ambient_intensity;
  ambient_color = max( 0.0f, ambient_color );

  //Diffuse
  float dot_product = dot( input.normal, light_dir );
  diffuse_color = dot_product * surface_color * sun_light_intensity;
  diffuse_color = max( 0.0f, diffuse_color );

  //Specular
  if( dot_product > 0.0f ) {
    float3 vertex_to_eye = normalize( input.eye_view - input.world );
    float3 reflection = normalize( reflect( -light_dir, input.normal ) );
    float specular = dot( vertex_to_eye, reflection );
    if( specular > 0.4f ) {
      specular = pow( specular, 8.0f );
      specular_color = surface_color * specular;
    }
  }

  float3 final_color = ambient_color + diffuse_color + specular_color;
  final_color = lerp( final_color, fog_color, input.distance );

  return float4( final_color, 1.0f );
}
