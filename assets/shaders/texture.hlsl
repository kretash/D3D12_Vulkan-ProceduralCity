
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
  float3 eye_dir : EYE_DIR;
  float3 light_dir : LIGHT_DIR;
  float3x3 TBN : TBN;

  float3 tangent : TANGENT;
};

cbuffer InstanceData : register( b0, space0 ) {
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
  float padding;
  float3 eye_view;
  float4x4 view;
};

Texture2D textures[2048] : register( t0 );
SamplerState samplers[2]	: register( s0 );

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
  result.uv = input.uv;

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

  //TBN
  result.normal = frag_normal;

  float3 tangent = normalize( input.tangent );
  float3 bitangent = normalize( input.bitangent );

  float3x3 TBN = transpose( float3x3( tangent, bitangent, frag_normal ) );
  result.TBN = TBN;

  float3 pos = mul( float4( input.position.xyz, 1.0f ), model ).xyz;
  pos = input.position.xyz;
  result.eye_dir = normalize( pos - eye_view );

  result.light_dir = normalize( -light_pos );
  //result.light_dir.x = -result.light_dir.x;
  //result.light_dir.z = -result.light_dir.z;
  result.light_dir = mul( float4( result.light_dir, 0.0f ), normal_matrix ).xyz;

  result.tangent = tangent;

  return result;
}

float4 PSMain( PSInput input ) : SV_TARGET
{
  float3 ambient_color = float3( 0.0f, 0.0f, 0.0f );
  float3 diffuse_color = float3( 0.0f, 0.0f, 0.0f );
  float3 specular_color = float3( 0.0f, 0.0f, 0.0f );

  float3 texture_color = textures[d_texture_id].Sample( samplers[1], input.uv ).rgb;
  float3 normal_map = textures[n_texture_id].Sample( samplers[1], input.uv ).rgb;
  float3 specular_map = textures[s_texture_id].Sample( samplers[1], input.uv ).rgb;

  float3 normal = mul( normalize( normal_map * 2.0f - 1.0f ), input.TBN );
  float specular_map_r = specular_map.x;

  //Ambient
  ambient_color = input.spherical_harmonics * ambient_intensity * texture_color;
  ambient_color = max( 0.0f, ambient_color );

  //Diffuse
  float dot_product = clamp( dot( normal, input.light_dir ), 0.0f, 1.0f );
  diffuse_color = dot_product * sun_light_intensity * texture_color;
  diffuse_color = max( 0.0f, diffuse_color );

  //Specular
  if( dot_product > 0.0 ) {
    float3 half_vector = normalize( input.light_dir - input.eye_dir );
    float specular = max( 0.0f, dot( half_vector, normal ));
    specular_color = pow( specular, 16.0f ) * specular_map_r;
  }

  float3 final_color = ambient_color + diffuse_color + specular_color; 
  final_color = lerp( final_color, fog_color, input.distance );

  return float4( final_color, 1.0f );
  //return float4( input.tangent, 1.0f );
}
