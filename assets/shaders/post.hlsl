
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
  float2 uv : TEXCOORD;
  float3 tangent : TANGENT;
  float3 bitangent : BITANGENT;
};

Texture2D textures : register( t0 );
SamplerState m_sampler : register( s0 );

PSInput VSMain( VSInput input ) {
  PSInput result;
  result.position = float4( input.position.xyz, 1.0f );
  result.normal = input.normal;
  result.uv = input.uv;
  result.tangent = input.tangent;
  result.bitangent = input.bitangent;

  return result;
}

static const int screen_height = 720.0f;
static const int screen_width = 1280.0f;

float4 FXAA( float2 uv ) {
  const float fxaa_reduce_min = 1.0f / 128.0f;
  const float fxaa_reduce_mul = 1.0f / 8.0f;
  const float fxaa_span_max = 8.0f;

  float2 pixel = float2( 1.0f / screen_width, 1.0f / screen_height );
  float2 frag_coord = uv * float2( screen_width, screen_height );

  float4 fxaa_uv;
  fxaa_uv.xy = uv.xy;
  fxaa_uv.zw = uv.xy - ( pixel * ( 0.5 ) );

  float4 texture_color = textures.Sample( m_sampler, uv );
  float3 rgbNW = textures.Sample( m_sampler, fxaa_uv.zw ).rgb;
  //texture2D( texture_, fxaa_uv.zw ).xyz;
  float3 rgbNE = textures.Sample( m_sampler, fxaa_uv.zw + float2( 1.0f, 0.0f )*pixel ).rgb;
  //texture2D( texture_, fxaa_uv.zw + ivec2( 1, 0 ) * pixel ).xyz;
  float3 rgbSW = textures.Sample( m_sampler, fxaa_uv.zw + float2( 0.0f, 1.0f )*pixel ).rgb;
  //float3 rgbSW = texture2D( texture_, fxaa_uv.zw + ivec2( 0, 1 ) * pixel ).xyz;
  float3 rgbSE = textures.Sample( m_sampler, fxaa_uv.zw + float2( 1.0f, 1.0f )*pixel ).rgb;
  //float3 rgbSE = texture2D( texture_, fxaa_uv.zw + ivec2( 1, 1 ) * pixel ).xyz;

  float3 rgbM = texture_color.xyz;
  float3 luma = float3( 0.299, 0.587, 0.114 );
  float lumaNW = dot( rgbNW, luma );
  float lumaNE = dot( rgbNE, luma );
  float lumaSW = dot( rgbSW, luma );
  float lumaSE = dot( rgbSE, luma );
  float lumaM = dot( rgbM, luma );
  float lumaMin = min( lumaM, min( min( lumaNW, lumaNE ), min( lumaSW, lumaSE ) ) );
  float lumaMax = max( lumaM, max( max( lumaNW, lumaNE ), max( lumaSW, lumaSE ) ) );

  float2 dir;
  dir.x = -( ( lumaNW + lumaNE ) - ( lumaSW + lumaSE ) );
  dir.y = ( ( lumaNW + lumaSW ) - ( lumaNE + lumaSE ) );

  float dirReduce = max( ( lumaNW + lumaNE + lumaSW + lumaSE ) *
    ( 0.25 * fxaa_reduce_mul ), fxaa_reduce_min );

  float rcpDirMin = 1.0f / ( min( abs( dir.x ), abs( dir.y ) ) + dirReduce );

  dir = min( float2( fxaa_span_max, fxaa_span_max ),
        max( float2( -fxaa_span_max, -fxaa_span_max ),
      dir * rcpDirMin ) ) * pixel;

  float3 rgbA = 0.5 * (
    //texture2D( texture_, frag_coord * pixel + dir * ( 1.0 / 3.0 - 0.5 ) ).xyz +
    textures.Sample( m_sampler, frag_coord * pixel + dir * ( 1.0f / 3.0f - 0.5f ) ).rgb +
    //texture2D( texture_, frag_coord * pixel + dir * ( 2.0 / 3.0 - 0.5 ) ).xyz );
    textures.Sample( m_sampler, frag_coord * pixel + dir * ( 2.0f / 3.0f - 0.5f ) ).rgb);
  float3 rgbB = rgbA * 0.5 + 0.25 * (
    textures.Sample( m_sampler, frag_coord * pixel + dir * -0.5f ).rgb +
    textures.Sample( m_sampler, frag_coord * pixel + dir * 0.5f ).rgb );
    //texture2D( texture_, frag_coord * pixel + dir * -0.5 ).xyz +
    //texture2D( texture_, frag_coord * pixel + dir * 0.5 ).xyz );

  float lumaB = dot( rgbB, luma );
  float4 color;
  if( ( lumaB < lumaMin ) || ( lumaB > lumaMax ) ) {
    return float4( rgbA, texture_color.a );
  } else {
    return float4( rgbB, texture_color.a );
  }
}

float4 PSMain( PSInput input ) : SV_TARGET
{
  float3 texture_color = FXAA( input.uv ).rgb;
  //float3 texture_color = textures.Sample( m_sampler, input.uv ).rgb;
  return float4( texture_color.xyz, 1.0f );
}