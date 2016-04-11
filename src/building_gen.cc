#include "core/building_gen.hh"
#include "core/GPU_pool.hh"
#include "core/tools.hh"
#include "core/engine_settings.hh"
#include "core/math/float4x4.hh"
#include <vector>
#include <cassert>

namespace kretash {

  BuildingGen::BuildingGen() :
    n_sides( 0 ),
    n_floors( 0 ),
    n_floor_height( 0.0f ),
    c_angle( 0.0f ),
    a_id( 0 ),
    p_id( 0 ),
    n_elem_offset( 0 ),
    m_radius( 0.0f ),
    m_vertex_length( 0 ),
    m_vertex_buffer( nullptr ),
    m_elem_buffer( nullptr ) {
  }

  void BuildingGen::generate( building_settings s ) {

    n_sides = s.n_sides;
    n_floors = s.n_floors;
    n_floor_height = s.n_floor_height;
    center_pos = float3( 0.0f, 0.0f, 0.0f );
    n_base_size = s.n_base_size;

    a_id = s.n_angle_seed;
    p_id = s.n_size_seed;

    c_angle = 0.0f;
    n_angles = new float[n_sides];
    n_side_size = new float[n_sides];
    c_pos = { 0.0f, 0.0f, 0.0f };
    c_norm = { 0.0f, 0.0f, 0.0f };
    n_uv = { 0.0f, 0.0f };

    uv_x_min = 0.0f;
    uv_y_min = 0.0f;
    uv_x_max = 1.0f;
    uv_y_max = s.n_vertical_uv;

    if( s.n_texture_set == eLEFT ) { uv_x_max = 0.5f; } else if( s.n_texture_set == eRIGHT ) { uv_x_min = 0.5f; }

    if( s.n_group == eSingle ) {
      _generate_angles_1( n_angles, n_sides );
      _generate_sizes_1( n_side_size, n_sides );
    } else if( s.n_group == eDouble ) {
      _generate_angles_2( n_angles, n_sides );
      _generate_sizes_2( n_side_size, n_sides );
    } else if( s.n_group == eTriple ) {
      _generate_angles_3( n_angles, n_sides );
      _generate_sizes_3( n_side_size, n_sides );
    } else if( s.n_group == eQuad ) {
      _generate_angles_4( n_angles, n_sides );
      _generate_sizes_4( n_side_size, n_sides );
    } else if( s.n_group == ePenta ) {
      _generate_angles_5( n_angles, n_sides );
      _generate_sizes_5( n_side_size, n_sides );
    }

    {
      float c_angle = 0.0f;
      float3 c_pos = { 0.0f, 0.0f, 0.0f };
      for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
        c_angle += n_angles[i_s];
        c_pos.x += sinf( to_rad( c_angle ) ) * n_side_size[i_s];
        c_pos.z += cosf( to_rad( c_angle ) ) * n_side_size[i_s];
        center_pos = center_pos + c_pos;
      }
    }
    center_pos = center_pos / ( float ) n_sides;
    center_pos.y = 0;

    if( m_radius < float3::lenght( center_pos ) )
      m_radius = float3::lenght( center_pos );

    center_pos -= s.n_center_offset;
    center_offset = s.n_center_offset;

    uint32_t star_vertex = ( uint32_t ) n_vertices.size();

    _generate_bot_face();

    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
      _generate_floor( i_s, 0, false );
    }

    c_pos.y += n_floor_height;

    for( uint32_t i_f = 1; i_f < n_floors; ++i_f ) {
      for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
        _generate_floor( i_s, i_f, true );
      }
      for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
        _generate_floor( i_s, i_f, false );
      }
      c_pos.y += n_floor_height;
    }

    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
      _generate_floor( i_s, n_floors, true );
    }

    _generate_top_face();

    uint32_t base_vertex = 0;
    uint32_t base_bot_vertex = 0;

    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
      n_elems.push_back( n_elem_offset + base_vertex + base_bot_vertex + 1 );
      n_elems.push_back( n_elem_offset + base_vertex );
      n_elems.push_back( n_elem_offset + base_vertex + base_bot_vertex + 2 );

      base_bot_vertex += 2;
    }

    base_vertex += base_bot_vertex + 1;

    for( uint32_t i_f = 0; i_f < n_floors; ++i_f ) {
      for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
        const uint32_t offset = n_sides * 2;

        n_elems.push_back( n_elem_offset + base_vertex + 1 );
        n_elems.push_back( n_elem_offset + base_vertex + offset );
        n_elems.push_back( n_elem_offset + base_vertex );

        n_elems.push_back( n_elem_offset + base_vertex + 1 );
        n_elems.push_back( n_elem_offset + base_vertex + offset + 1 );
        n_elems.push_back( n_elem_offset + base_vertex + offset );

        base_vertex += 2;
      }
      base_vertex += n_sides * 2;
    }


    uint32_t base_top_vertex = 0;
    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
      n_elems.push_back( n_elem_offset + base_vertex + base_top_vertex + 2 );
      n_elems.push_back( n_elem_offset + base_vertex );
      n_elems.push_back( n_elem_offset + base_vertex + base_top_vertex + 1 );

      base_top_vertex += 2;
    }

    n_elem_offset += ( uint32_t ) n_vertices.size() - star_vertex;

    delete[] n_angles;
    delete[] n_side_size;

  }

  void BuildingGen::combine_buffers() {
    // vertex

    //horrible solution, but its not on the main thread so it shouldnt stall 
    if( m_vertex_buffer != nullptr ) delete[] m_vertex_buffer;
    if( m_elem_buffer != nullptr ) delete[] m_elem_buffer;
    m_vertex_buffer = nullptr;
    m_elem_buffer = nullptr;

    uint32_t num_vertices = static_cast< uint32_t >( n_vertices.size() );
    num_vertices = num_vertices * static_cast< uint32_t >( 3 );

    uint32_t num_normals = static_cast< uint32_t >( n_normals.size() );
    num_normals = num_normals * static_cast< uint32_t >( 3 );

    assert( num_vertices == num_normals && "WRONG GEOMETRY DATA" );
    assert( num_vertices != 0 && "WRONG GEOMETRY DATA" );
    assert( num_normals != 0 && "WRONG GEOMETRY DATA" );

    std::vector<float3> nc_vertices;
    std::vector<float3> nc_normals;
    std::vector<float2> nc_uvs;
    std::vector<float3> nc_tangent;
    std::vector<float3> nc_bitangent;
    std::vector<unsigned int> nc_elem;

    for( int e = 0; e < n_elems.size(); ++e ) {

      unsigned int v_i = n_elems[e];
      nc_vertices.push_back( n_vertices[v_i] );
      nc_normals.push_back( n_normals[v_i] );
      nc_uvs.push_back( n_uvs[v_i] );
      nc_elem.push_back( e );

    }

    for( int e = 0; e < nc_elem.size(); e += 3 ) {

      float3& v1 = nc_vertices[e];
      float3& v2 = nc_vertices[e + 1];
      float3& v3 = nc_vertices[e + 2];

      float2& uv1 = nc_uvs[e];
      float2& uv2 = nc_uvs[e + 1];
      float2& uv3 = nc_uvs[e + 2];

      float3 delta_pos1 = v2 - v1;
      float3 delta_pos2 = v3 - v1;

      float2 delta_uv1 = uv2 - uv1;
      float2 delta_uv2 = uv3 - uv1;

      float r = 1.0f / ( delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x );
      float3 tangent = ( delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y )*r;
      float3 bitangent = ( delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x )*r;

      //Tangent
      nc_tangent.push_back( tangent );
      nc_tangent.push_back( tangent );
      nc_tangent.push_back( tangent );

      //Bitangent
      nc_bitangent.push_back( bitangent );
      nc_bitangent.push_back( bitangent );
      nc_bitangent.push_back( bitangent );

    }

    for( int e = 0; e < nc_tangent.size(); ++e ) {
      float3& t = nc_tangent[e];
      float3& b = nc_bitangent[e];
      float3& n = nc_normals[e];

      t = t - n * float3::dot( n, t );

      if( float3::dot( float3::cross( n, t ), b ) < 0.0f )
        t = t * -1.0f;
    }

    m_indicies_count = ( uint32_t ) nc_elem.size();
    int stride = 14;
    m_vertex_length = ( uint32_t ) ( nc_vertices.size() * stride );
    m_vertex_buffer = new float[m_vertex_length];

    int count = 0;
    for( int i = 0; i < nc_vertices.size(); ++i ) {
      m_vertex_buffer[count++] = nc_vertices[i].x;
      m_vertex_buffer[count++] = nc_vertices[i].y;
      m_vertex_buffer[count++] = nc_vertices[i].z;

      m_vertex_buffer[count++] = nc_normals[i].x;
      m_vertex_buffer[count++] = nc_normals[i].y;
      m_vertex_buffer[count++] = nc_normals[i].z;

      m_vertex_buffer[count++] = nc_uvs[i].x;
      m_vertex_buffer[count++] = nc_uvs[i].y;

      m_vertex_buffer[count++] = nc_tangent[i].x;
      m_vertex_buffer[count++] = nc_tangent[i].y;
      m_vertex_buffer[count++] = nc_tangent[i].z;

      m_vertex_buffer[count++] = nc_bitangent[i].x;
      m_vertex_buffer[count++] = nc_bitangent[i].y;
      m_vertex_buffer[count++] = nc_bitangent[i].z;

    }

    // Indices's Buffer
    m_elem_buffer = new uint32_t[m_indicies_count];
    memcpy( m_elem_buffer, &nc_elem[0], sizeof( uint32_t )*m_indicies_count );

  }

  void BuildingGen::finish_and_upload() {

    k_engine->get_GPU_pool()->queue_geometry( static_cast< Geometry* >( this ),
      m_vertex_buffer, m_vertex_length, m_elem_buffer, m_indicies_count );

    n_vertices.clear();
    n_normals.clear();
    n_uvs.clear();
    n_elems.clear();

    //avoiding the delete, vector will be reused anyway.
    //n_vertices.shrink_to_fit();
    //n_normals.shrink_to_fit();
    //n_uvs.shrink_to_fit();
    //n_elems.shrink_to_fit();

    n_elem_offset = 0;
    m_radius = 0.0f;
  }

  void BuildingGen::_generate_floor( uint32_t i_s, uint32_t i_f, bool top ) {

    float uv = uv_y_min;
    if( top )
      uv = uv_y_max;

    n_vertices.push_back( c_pos - center_pos );
    n_uvs.push_back( float2( uv_x_min, 1.0f - uv ) );
    float3 A = c_pos;

    c_angle += n_angles[i_s];
    c_pos.x += sinf( to_rad( c_angle ) ) * n_side_size[i_s];
    c_pos.z += cosf( to_rad( c_angle ) ) * n_side_size[i_s];

    n_vertices.push_back( c_pos - center_pos );
    n_uvs.push_back( float2( uv_x_max, 1.0f - uv ) );

    float3 B = c_pos;
    float3 C = c_pos + float3( 0.0f, n_floor_height, 0.0f );
    c_norm = float3::cross( B - A, C - A );
    c_norm.normalize();
    n_normals.push_back( c_norm );
    n_normals.push_back( c_norm );
  }

  void BuildingGen::_generate_bot_face() {
    float3 v_center = {};
    v_center = center_offset;
    c_pos.y = 0;

    n_vertices.push_back( v_center );
    n_uvs.push_back( float2( 0.0f, 0.0f ) );
    n_normals.push_back( float3( 0.0f, 1.0f, 0.0f ) );
    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
      n_vertices.push_back( c_pos - center_pos );

      c_angle += n_angles[i_s];
      c_pos.x += sinf( to_rad( c_angle ) ) * n_side_size[i_s];
      c_pos.z += cosf( to_rad( c_angle ) ) * n_side_size[i_s];

      n_vertices.push_back( c_pos - center_pos );

      n_uvs.push_back( float2( 0.0f, 0.0f ) );
      n_uvs.push_back( float2( 0.0f, 0.0f ) );
      n_normals.push_back( float3( 0.0f, 1.0f, 0.0f ) );
      n_normals.push_back( float3( 0.0f, 1.0f, 0.0f ) );
    }
  }

  void BuildingGen::_generate_top_face() {
    float3 v_center = {};
    v_center = center_offset;
    v_center.y += n_floor_height * n_floors;
    c_pos.y = n_floor_height * n_floors;

    float2 pos_min;
    float2 pos_max;

    n_vertices.push_back( v_center );
    n_normals.push_back( float3( 0.0f, 1.0f, 0.0f ) );

    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {
      n_vertices.push_back( c_pos - center_pos );
      c_angle += n_angles[i_s];
      c_pos.x += sinf( to_rad( c_angle ) ) * n_side_size[i_s];
      c_pos.z += cosf( to_rad( c_angle ) ) * n_side_size[i_s];

      n_vertices.push_back( c_pos - center_pos );
      if( pos_min.x > c_pos.x ) { pos_min.x = c_pos.x; }
      if( pos_min.y > c_pos.z ) { pos_min.y = c_pos.z; }
      if( pos_max.x < c_pos.x ) { pos_max.x = c_pos.x; }
      if( pos_max.y < c_pos.z ) { pos_max.y = c_pos.z; }

      n_normals.push_back( float3( 0.0f, 1.0f, 0.0f ) );
      n_normals.push_back( float3( 0.0f, 1.0f, 0.0f ) );
    }

    //forced uvs
    n_uvs.push_back( float2( 0.5f, 0.0f ) );

    float uv_x = 0.0f;
    float uv_y = 0.0f;

    for( uint32_t i_s = 0; i_s < n_sides; ++i_s ) {

      uv_x = 0.75f;
      uv_y = 1.0f;

      n_uvs.push_back( float2( uv_x, uv_y ) );

      uv_x = 1.0f;
      uv_y = 0.5f;

      n_uvs.push_back( float2( uv_x, uv_y ) );
    }
  }

  void BuildingGen::_generate_angles_1( float* angles_array, uint32_t sides ) {
    float step = 360.0f / ( float ) sides;
    for( uint32_t i = 0; i < sides; ++i ) {
      angles_array[i] = step;
    }
  }

  void BuildingGen::_generate_angles_2( float* angles_array, uint32_t sides ) {
    float step = 360.0f / ( float ) sides;
    for( uint32_t i = 0; i < sides / 2; ++i ) {
      angles_array[( i * 2 ) + 0] = step * angle_set_2[a_id][0];
      angles_array[( i * 2 ) + 1] = step * angle_set_2[a_id][1];
    }
  }

  void BuildingGen::_generate_angles_3( float* angles_array, uint32_t sides ) {
    float step = 360.0f / ( float ) sides;
    for( uint32_t i = 0; i < sides / 3; ++i ) {
      angles_array[( i * 3 ) + 0] = step * angle_set_3[a_id][0];
      angles_array[( i * 3 ) + 1] = step * angle_set_3[a_id][1];
      angles_array[( i * 3 ) + 2] = step * angle_set_3[a_id][2];
    }
  }

  void BuildingGen::_generate_angles_4( float* angles_array, uint32_t sides ) {
    float step = 360.0f / ( float ) sides;
    for( uint32_t i = 0; i < sides / 4; ++i ) {
      angles_array[( i * 4 ) + 0] = step * angle_set_4[a_id][0];
      angles_array[( i * 4 ) + 1] = step * angle_set_4[a_id][1];
      angles_array[( i * 4 ) + 2] = step * angle_set_4[a_id][2];
      angles_array[( i * 4 ) + 3] = step * angle_set_4[a_id][3];
    }
  }

  void BuildingGen::_generate_angles_5( float* angles_array, uint32_t sides ) {
    float step = 360.0f / ( float ) sides;
    for( uint32_t i = 0; i < sides / 5; ++i ) {
      angles_array[( i * 5 ) + 0] = step * angle_set_5[a_id][0];
      angles_array[( i * 5 ) + 1] = step * angle_set_5[a_id][1];
      angles_array[( i * 5 ) + 2] = step * angle_set_5[a_id][2];
      angles_array[( i * 5 ) + 3] = step * angle_set_5[a_id][3];
      angles_array[( i * 5 ) + 4] = step * angle_set_5[a_id][4];
    }
  }

  void BuildingGen::_generate_sizes_1( float* sizes_array, uint32_t sides ) {
    for( uint32_t i = 0; i < sides; ++i ) {
      sizes_array[i] = n_base_size * pattern_set_1[p_id];
    }
  }

  void BuildingGen::_generate_sizes_2( float* sizes_array, uint32_t sides ) {
    for( uint32_t i = 0; i < sides / 2; ++i ) {
      sizes_array[( i * 2 )] = n_base_size *     pattern_set_2[p_id][0];
      sizes_array[( i * 2 ) + 1] = n_base_size * pattern_set_2[p_id][1];
    }
  }

  void BuildingGen::_generate_sizes_3( float* sizes_array, uint32_t sides ) {
    for( uint32_t i = 0; i < sides / 3; ++i ) {
      sizes_array[( i * 3 )] = n_base_size *     pattern_set_3[p_id][0];
      sizes_array[( i * 3 ) + 1] = n_base_size * pattern_set_3[p_id][1];
      sizes_array[( i * 3 ) + 2] = n_base_size * pattern_set_3[p_id][2];
    }
  }

  void BuildingGen::_generate_sizes_4( float* sizes_array, uint32_t sides ) {
    for( uint32_t i = 0; i < sides / 4; ++i ) {
      sizes_array[( i * 4 )] = n_base_size *     pattern_set_4[p_id][0];
      sizes_array[( i * 4 ) + 1] = n_base_size * pattern_set_4[p_id][1];
      sizes_array[( i * 4 ) + 2] = n_base_size * pattern_set_4[p_id][2];
      sizes_array[( i * 4 ) + 3] = n_base_size * pattern_set_4[p_id][3];
    }
  }

  void BuildingGen::_generate_sizes_5( float* sizes_array, uint32_t sides ) {
    for( uint32_t i = 0; i < sides / 5; ++i ) {
      sizes_array[( i * 5 )] = n_base_size *     pattern_set_5[p_id][0];
      sizes_array[( i * 5 ) + 1] = n_base_size * pattern_set_5[p_id][1];
      sizes_array[( i * 5 ) + 2] = n_base_size * pattern_set_5[p_id][2];
      sizes_array[( i * 5 ) + 3] = n_base_size * pattern_set_5[p_id][3];
      sizes_array[( i * 5 ) + 4] = n_base_size * pattern_set_5[p_id][4];
    }
  }


  BuildingGen::~BuildingGen() {
    if( m_vertex_buffer != nullptr ) delete[] m_vertex_buffer;
    if( m_elem_buffer != nullptr ) delete[] m_elem_buffer;
    m_vertex_buffer = nullptr;
    m_elem_buffer = nullptr;
  }

  const float BuildingGen::pattern_set_5[5][5] =
  {
    { 1.0f, 2.0f, -1.0f, 2.0f, 1.0f },
    { 2.0f, 1.0f, -2.0f, 1.0f, 3.0f },
    { 2.0f, 1.0f, -2.0f, 2.0f, 2.0f },
    { 3.0f, 1.0f, 1.0f, 2.0f, -1.0f },
    { 1.0f, 3.0f, -2.0f, 2.0f, 2.0f }
  };

  const float BuildingGen::pattern_set_4[5][4] =
  {
    { 1.0f, 1.5f, 2.0f, 1.0f },
    { 1.0f, 1.5f, 2.0f, 1.5f },
    { 2.0f, 1.0f, 1.5f, 2.0f },
    { 1.0f, 2.0f, 2.0f, 1.0f },
    { 2.0f, 1.5f, 2.0f, 1.5f }
  };

  const float BuildingGen::pattern_set_3[5][3] =
  {
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f }
  };

  const float BuildingGen::pattern_set_2[5][2] =
  {
    { 1.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.0f, 1.0f }
  };

  const float BuildingGen::pattern_set_1[5] =
  {
    { 1.0f },
    { 1.0f },
    { 1.0f },
    { 1.0f },
    { 1.0f }
  };

  const float BuildingGen::angle_set_5[5][5] =
  {
    { 2.0f, -1.0f, 0.0f, 2.0f, 2.0f },
    { 3.0f, -2.0f, -1.0f, 2.0f, 3.0f },
    { 1.0f, 2.0f, 3.0f, 0.0f, -1.0f },
    { -2.0f, 1.0f, 1.0f, 2.0f, 3.0f },
    { 1.0f, -1.0f, -1.0f, 2.0f, 4.0f }
  };

  const float BuildingGen::angle_set_4[5][4] =
  {
    { 3.0f, -1.0f, -1.0f, 3.0f },
    { -1.0f, 4.0f, -2.0f, 3.0f },
    { 3.0f, -1.0f, 0.0f, 2.0f },
    { 2.0f, -1.0f, 2.0f, 1.0f },
    { 2.0f, -2.0f, 1.0f, 3.0f }
  };

  const float BuildingGen::angle_set_3[5][3] =
  {
    { 1.0f, 1.0f, 1.0f },
    { 2.0f, -1.0f, 2.0f },
    { 1.0f, 2.0f, 0.0f },
    { 1.0f, -1.0f, 3.0f },
    { 2.0f, 0.0f, 1.0f }
  };

  const float BuildingGen::angle_set_2[5][2] =
  {
    { 0.0f, 2.0f },
    { 1.0f, 1.0f },
    { 2.0f, 0.0f },
    { 1.5f, 0.5f },
    { 1.0f, 1.0f }
  };
}