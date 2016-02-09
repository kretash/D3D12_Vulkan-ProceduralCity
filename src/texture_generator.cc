#include "core/texture_generator.hh"
#include "core/engine_settings.hh"
#include "core/renderer.hh"
#include "core/texture.hh"
#include "core/tools.hh"
#include "core/engine.hh"
#include <cassert>

#include "noise/PerlinNoise.h"

using namespace tools;

texel get_window_color( float n1, float n2, float n3 ) {

  int8_t r = 0, g = 0, b = 0;
  {
    int32_t f = int32_t( n1 );
    float noise = n1 - ( float ) f;
    r = 55 + ( int8_t ) ( 128.0f * noise );
  }
  {
    int32_t f = int32_t( n2 );
    float noise = n2 - ( float ) f;
    g = 55 + ( int8_t ) ( 128.0f * noise );
  }
  {
    int32_t f = int32_t( n3 );
    float noise = n3 - ( float ) f;
    b = 55 + ( int8_t ) ( 200.0f * noise );
  }

  //return texel( 255, 0, 0 );
  return texel( r, g, b );
}

texel get_wall_color( float n1, float n2, float n3 ) {
  int8_t r = 0, g = 0, b = 0;
  {
    int32_t f = int32_t( n1 );
    float noise = n1 - ( float ) f;
    r = 64 + ( int8_t ) ( 128.0f * noise );
  }
  {
    int32_t f = int32_t( n2 );
    float noise = n2 - ( float ) f;
    g = 64 + ( int8_t ) ( 128.0f * noise );
  }
  {
    int32_t f = int32_t( n3 );
    float noise = n3 - ( float ) f;
    b = 50 + ( int8_t ) ( 128.0f * noise );
  }

  return texel( r, g, b );
}

texel get_border_color( float n1, float n2, float n3 ) {
  int8_t r = 0, g = 0, b = 0;
  {
    int32_t f = int32_t( n1 );
    float noise = n1 - ( float ) f;
    r = 16 + ( int8_t ) ( 64.0f * noise );
  }
  {
    int32_t f = int32_t( n2 );
    float noise = n2 - ( float ) f;
    g = 16 + ( int8_t ) ( 64.0f * noise );
  }
  {
    int32_t f = int32_t( n3 );
    float noise = n3 - ( float ) f;
    b = 16 + ( int8_t ) ( 64.0f * noise );
  }

  return texel( r, g, b );
}

TextureGenerator::TextureGenerator() {

  m_exit_all_threads.store( false );

  for( int i = 0; i < 6; ++i )
    m_threads.push_back( std::thread( &TextureGenerator::_thread_generate, this ) );
}

void TextureGenerator::generate( Texture* desc ) {
  if( desc->m_ready.load() ) {
    desc->m_ready.store( false );

    m_queue_mutex.lock();
    m_generate_queue.push_back( desc );
    m_queue_mutex.unlock();
  } else {
    std::cout << "Sent active texture" << std::endl;
  }
}

bool TextureGenerator::texture_ready( Texture* desc ) {
  return desc->m_ready.load();
}

void TextureGenerator::gather_texture( Texture* desc ) {
  desc->apply_future_ids();

  while( !desc->m_ready.load() ) {
    std::cout << "Waiting for texture load";
    while( !desc->m_ready.load() ) {
      std::cout << ".";
      std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) );
    }
    std::cout << std::endl;
  }


  //SDL test project should be able to remove D3D12 code
#if 1
  //desc->clear();

  int32_t texture_start = tDIFFUSE;
  int32_t texture_end = tCOUNT;
  for( int32_t e = texture_start; e < texture_end; ++e ) {

    texture_t tt = static_cast< texture_t >( e );
    int32_t w = desc->get_width( tt );
    int32_t h = desc->get_height( tt );
    int32_t c = desc->get_channels( tt );

    GPU::create_texture( k_engine->get_engine_data(),
      desc->get_texture_data( tt ),
      desc->get_texture_pointer( tt ), w, h, c );

    GPU::create_shader_resource_view( k_engine->get_renderer( rTEXTURE )->get_render_data(),
      desc->get_texture_data( tt ), desc->m_future_texture_id[e] );

  }

#endif
}

void TextureGenerator::_thread_generate() {
  while( !m_exit_all_threads.load() ) {
    if( m_queue_mutex.try_lock() ) {
      if( m_generate_queue.size() == 0 ) {

        m_queue_mutex.unlock();
        std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );

      } else {

        Texture* desc = m_generate_queue[0];
        m_generate_queue.erase( m_generate_queue.begin() );
        m_queue_mutex.unlock();

#if 1
        _generate_elements( desc );
        _rasterize_elements( desc );
#else
        //This will be a lot quicker than generating a normal texture
        _rasterize_debug( desc );
#endif

        desc->m_texture_e.clear();

        desc->m_ready.store( true );
      }
    } else {
      std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
    }

  }
}

int32_t _get_p_rand( int32_t min, int32_t max, float sample ) {
  return min + ( ( ( int ) ( sample ) ) % max );
}

float _get_p_rand( float min, float max, float sample ) {
  float range = max - min;

  sample = fmodf( sample, range );
  sample = fabsf( sample );
  sample += min;
  assert( sample >= min && sample <= max && "PRAND FLOAT ERROR" );

  return sample;
}

void TextureGenerator::_generate_elements( Texture* desc ) {

  int32_t width = desc->_get_width() / 2;
  int32_t height = desc->_get_height();

  float n1 = PerlinNoise::noise( desc->m_seed_x / 64.0f, desc->m_seed_y / 64.0f, 2.1f );
  float n2 = PerlinNoise::noise( desc->m_seed_x / 64.0f, desc->m_seed_y / 64.0f, 2.2f );
  float n3 = PerlinNoise::noise( desc->m_seed_x / 64.0f, desc->m_seed_y / 64.0f, 2.3f );

  //n1 = random( 0.0f, 10.0f );
  //n2 = random( 0.0f, 10.0f );
  //n3 = random( 0.0f, 10.0f );

  float thick = _get_p_rand( 0.08f, 0.2f, n1 );
  thick = random( 0.025f, 0.1f );
  int32_t thickness = ( int32_t ) ( thick * ( float ) height );
  int32_t vertical = _get_p_rand( 1, 4, n1 ) * 2 + 1;
  int32_t horizontal = _get_p_rand( 1, 4, n1 ) * 2 + 1;
  vertical = random( 1, 4 ) * 2 + 1;
  horizontal = random( 1, 4 ) * 2 + 1;

  int32_t v_rest_of_space = width - vertical*thickness;
  int32_t v_space = v_rest_of_space / ( vertical - 1 );

  int32_t h_rest_of_space = height - horizontal*thickness;
  int32_t h_space = h_rest_of_space / ( horizontal - 1 );

  texel window_color = get_window_color( n1, n2, n3 );
  texel wall_color = get_wall_color( n1, n2, n3 );
  texel border_color = get_border_color( n1, n2, n3 );

  {
    int32_t counter = 0;

    for( int32_t i = 0; i < vertical; ++i ) {
      texture_element curr = {};

      curr.start = int2( counter, 0 );
      curr.end = int2( counter + thickness, 0 + height );
      if( i == ( vertical - 1 ) ){
        curr.end.x = width;
      }
      curr.color = border_color;
      curr.specular = texel( 0, 0, 0 );

      desc->m_texture_e.push_back( curr );
      counter += ( thickness + v_space );
    }
  }
  {
    bool has_walls = false;
    if( n1 > 0.5f ) has_walls = true;
    bool window_turn = true;
    int32_t windows = ( horizontal - 1 ) / 2;
    int32_t h_offset = ( height / 4 ) / windows;

    int32_t counter = 0;


    for( int32_t i = 0; i < horizontal; ++i ) {
      texture_element curr = {};
      curr.start = int2( 0, counter );
      curr.end = int2( 0 + width, counter + thickness );
      curr.color = border_color;
      curr.specular = texel( 0, 0, 0 );
      desc->m_texture_e.push_back( curr );

      int32_t add_next = h_space;
      if( has_walls ) {
        if( window_turn ) {
          add_next += h_offset;
        } else {
          add_next -= h_offset;
        }
      }

      int32_t w_counter = 0;
      for( int32_t e = 1; e < vertical && i != ( horizontal - 1 ); ++e ) {
        texture_element window = {};
        window.start = int2( thickness + w_counter, counter + thickness );
        window.end = int2( thickness + w_counter + v_space, counter + ( thickness + add_next ) );

        assert( window.start.x >= 0 && window.start.x <= width && "GENERATED WRONG ELEMENT" );
        assert( window.start.y >= 0 && window.start.y <= height && "GENERATED WRONG ELEMENT" );

        assert( window.end.x >= 0 && window.end.x <= width && "GENERATED WRONG ELEMENT" );
        assert( window.end.y >= 0 && window.end.y <= height && "GENERATED WRONG ELEMENT" );


        if( window_turn || !has_walls ) {
          window.color = window_color;
          window.specular = texel( 255, 255, 255 );
        } else {
          window.color = wall_color;
          window.specular = texel( 32, 32, 32 );
          window.frequency = 5.0f;
          window.intensity = 2.0f;
        }

        desc->m_texture_e.push_back( window );
        w_counter += ( thickness + v_space );
      }

      window_turn = !window_turn;

      counter += ( thickness + add_next );
    }
  }
}



void TextureGenerator::_rasterize_elements( Texture* desc ) {
  _rasterize_diffuse( desc );
  _rasterize_specular( desc );
  _rasterize_diffuse_rocks( desc );
  _generate_normal_maps( desc );
}


void TextureGenerator::_rasterize_diffuse( Texture* desc ) {
  int32_t width = desc->_get_width();
  int32_t height = desc->_get_height();

  size_t size = desc->m_texture_e.size();

  for( size_t i = 0; i < size; ++i ) {
    int2 start = desc->m_texture_e[i].start;
    int2 end = desc->m_texture_e[i].end;

    float divx = ( float ) end.x - start.x;
    float divy = ( float ) end.y - start.y;

    for( int32_t x = start.x; x < end.x; ++x ) {
      for( int32_t y = start.y; y < end.y; ++y ) {

        float border = 1.0f;
        //if( x == start.x || x == end.x - 1 || y == start.y || y == end.y - 1 ) border = 2.0f;

        texel p = desc->m_texture_e[i].color;

        float x_seed = ( float ) x / divx;
        x_seed *= 6.0f;
        float y_seed = ( float ) y / divy;
        y_seed *= 6.0f;

        float n = PerlinNoise::noise(
          x_seed * desc->m_texture_e[i].frequency,
          y_seed * desc->m_texture_e[i].frequency,
          desc->m_seed_x * desc->m_texture_e[i].frequency );

        float r_intensity = 0.1f * desc->m_texture_e[i].intensity;
        float g_intensity = 0.1f * desc->m_texture_e[i].intensity;
        float b_intensity = 0.1f * desc->m_texture_e[i].intensity;

        float r = ( float ) p.r;
        float g = ( float ) p.g;
        float b = ( float ) p.b;

        r = r + ( r * n * r_intensity ) * border;
        g = g + ( g * n * g_intensity ) * border;
        b = b + ( b * n * b_intensity ) * border;

        p.cast_and_clamp_texel( r, g, b );

        int32_t* diffuse = ( int32_t* ) desc->get_texture_pointer( tDIFFUSE );

        diffuse[y * width + x] = p.get_texel();
      }
    }
  }
}


void TextureGenerator::_rasterize_diffuse_rocks( Texture* desc ) {
  int32_t width = desc->_get_width();
  int32_t height = desc->_get_height();

  for( uint16_t x = width / 2; x < width; ++x ) {
    for( uint16_t y = 0; y < height; ++y ) {

      int32_t center = y * width + x;

      float fx = ( float ) y * 128.0f;
      float fy = ( float ) x * 128.0f;

      float xx = fx / ( float ) ( width );
      float yy = fy / ( float ) ( height );

      float n1 = PerlinNoise::noise( xx, yy, desc->m_seed_y );

      texel e = get_wall_color( n1, n1, n1 );

      ( ( int32_t* ) desc->get_texture_pointer( tDIFFUSE ) )[center] = e.get_texel();

      // mmmmm....
      ( ( int32_t* ) desc->get_texture_pointer( tSPECULAR ) )[center] = 
        ( uint8_t ) ( luminance( texel::texel_to_float3(e.get_texel()) ) * 255 );
    }
  }
}

void TextureGenerator::_rasterize_specular( Texture* desc ) {
  int32_t width = desc->_get_width();
  int32_t height = desc->_get_height();

  size_t size = static_cast< size_t >( desc->m_texture_e.size() );

  for( size_t i = 0; i < size; ++i ) {

    int2 start = desc->m_texture_e[i].start;
    int2 end = desc->m_texture_e[i].end;

    float divx = ( float ) end.x - start.x;
    float divy = ( float ) end.y - start.y;

    for( int32_t x = start.x; x < end.x; ++x ) {
      for( int32_t y = start.y; y < end.y; ++y ) {
        texel p = desc->m_texture_e[i].specular;

        float x_seed = ( float ) x / divx;
        x_seed *= 6.0f;
        float y_seed = ( float ) y / divy;
        y_seed *= 6.0f;

        float n = PerlinNoise::noise( x_seed, y_seed, x_seed + y_seed );

        float r_intensity = 0.1f;

        float r = ( float ) p.r;

        r = r + ( r * n * r_intensity );

        p.cast_and_clamp_texel( r, r, r );

        ( ( int32_t* ) desc->get_texture_pointer( tSPECULAR ) )[y * width + x] = p.get_texel();
      }
    }
  }
}


void TextureGenerator::_generate_normal_maps( Texture* desc ) {

  int32_t width = desc->_get_width();
  int32_t height = desc->_get_height();
  float scale = 15.0f;

  for( int32_t x = 0; x < width; ++x ) {
    for( int32_t y = 0; y < height; ++y ) {

      int32_t p_x = clamp( x + 1, 0, width - 1 );
      int32_t m_x = clamp( x - 1, 0, width - 1 );
      int32_t p_y = clamp( y + 1, 0, height - 1 );
      int32_t m_y = clamp( y - 1, 0, height - 1 );

      int32_t center = y *   width + x;
      int32_t right = y *   width + p_x;
      int32_t left = y *   width + m_x;
      int32_t bot = m_y * width + x;
      int32_t top = p_y * width + x;

      int32_t* diffuse = ( ( int32_t* ) desc->get_texture_pointer( tDIFFUSE ) );

      float3 center_color = texel::texel_to_float3( diffuse[center] );
      float3 right_color = texel::texel_to_float3( diffuse[right] );
      float3 left_color = texel::texel_to_float3( diffuse[left] );
      float3 bot_color = texel::texel_to_float3( diffuse[bot] );
      float3 top_color = texel::texel_to_float3( diffuse[top] );

      float center_lum = scale * luminance( center_color );
      float right_lum = scale * luminance( right_color );
      float left_lum = scale * luminance( left_color );
      float top_lum = scale * luminance( top_color );
      float bot_lum = scale * luminance( bot_color );

      float3 tangent_2 = float3( 1.0f, 0.0f, center_lum - left_lum );
      float3 tangent_1 = float3( 1.0f, 0.0f, right_lum - center_lum );
      float3 bitangent_2 = float3( 0.0f, 1.0f, center_lum - bot_lum );
      float3 bitangent_1 = float3( 0.0f, 1.0f, top_lum - center_lum );

      tangent_1.normalize();
      tangent_2.normalize();
      bitangent_1.normalize();
      bitangent_2.normalize();

      float3 tangent = tangent_1 + tangent_2;
      float3 bitangent = bitangent_1 + bitangent_2;

      float3 normal_v = float3::cross( tangent, bitangent );
      normal_v.normalize();

      normal_v = normal_v * 0.5f + 0.5f;

      assert( center >= 0 || center <= height*width && "NORMAL MAP SAMPLE ERROR" );

      ( ( int32_t* ) desc->get_texture_pointer( tNORMAL ) )[center] = texel::float3_to_texel( normal_v ).get_texel();

    }
  }
}

void TextureGenerator::_rasterize_debug( Texture* desc ) {

  int32_t width = desc->_get_width();
  int32_t height = desc->_get_height();
  texel color = {};
  if( desc->get_LOD() == 0 ) {
    color = texel( 255, 0, 0, 255 );
  } else if( desc->get_LOD() == 1 ) {
    color = texel( 0, 255, 0, 255 );
  } else if( desc->get_LOD() == 2 ) {
    color = texel( 0, 0, 255, 255 );
  } else if( desc->get_LOD() == 3 ) {
    color = texel( 255, 0, 255, 255 );
  } else if( desc->get_LOD() == 5 ) {
    color = texel( 255, 255, 255, 255 );
  }

  for( int32_t x = 0; x < width; ++x ) {
    for( int32_t y = 0; y < height; ++y ) {

      int32_t* diffuse = ( int32_t* ) desc->get_texture_pointer( tDIFFUSE );
      diffuse[y * width + x] = color.get_texel();

      int32_t* normal = ( int32_t* ) desc->get_texture_pointer( tNORMAL );
      normal[y * width + x] = color.get_texel();

      int32_t* specular = ( int32_t* ) desc->get_texture_pointer( tSPECULAR );
      specular[y * width + x] = color.get_texel();

    }
  }
}

void TextureGenerator::shutdown() {
  m_exit_all_threads.store( true );

  for( int i = 0; i < m_threads.size(); ++i ) {
    m_threads[i].join();
  }

  m_threads.clear();
}

TextureGenerator::~TextureGenerator() {
  m_exit_all_threads.store( true );

  for( int i = 0; i < m_threads.size(); ++i ) {
    m_threads[i].join();
  }

  m_threads.clear();
}

