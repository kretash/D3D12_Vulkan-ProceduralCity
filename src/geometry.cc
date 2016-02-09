#include <cassert>

#include "tinyobj/tiny_obj_loader.h"
#include "core/engine_settings.hh"
#include "core/geometry.hh"
#include "core/GPU_pool.hh"
#include "core/engine.hh"
#include "core/window.hh"

Geometry::Geometry() :
  m_indicies_count( 0 ),
  m_indicies_offset( 0 ),
  m_vertex_offset( 0 ) {

}

Geometry::Geometry( const Geometry* c ) :
  m_indicies_count( c->m_indicies_count ),
  m_indicies_offset( c->m_indicies_offset ),
  m_vertex_offset( c->m_vertex_offset ) {
}

void Geometry::load( std::string filename ) {
  std::string filename_ = OPATH + filename;

  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string tiny_error = tinyobj::LoadObj( shapes, materials, filename_.c_str() );

  if( tiny_error.empty() == false ) {

    std::wstring msg( tiny_error.begin(), tiny_error.end() );
    std::wstring file( filename_.begin(), filename_.end() );

    if( tiny_error[0] == 'W' ) {
      MessageBoxW( k_engine->get_window()->get_window_handle(), msg.c_str(), file.c_str(), MB_OK );
    } else {
      MessageBoxW( k_engine->get_window()->get_window_handle(), msg.c_str(), file.c_str(), MB_OK );
      assert( false && "WE HAVE A TINY ERROR" );
    }
  }

  assert( shapes.size() == 1 && "NO NEED FOR MORE" );

  std::vector<float3> n_vertices;
  std::vector<float3> n_normals;
  std::vector<float2> n_uvs;
  std::vector<float3> n_tangent;
  std::vector<float3> n_bitangent;
  std::vector<unsigned int> n_elem;

  for( int e = 0; e < shapes[0].mesh.indices.size(); ++e ) {

    unsigned int v_i = shapes[0].mesh.indices[e] * 3;
    unsigned int uv_i = shapes[0].mesh.indices[e] * 2;

    n_vertices.push_back(
      float3( shapes[0].mesh.positions[v_i], shapes[0].mesh.positions[v_i + 1], shapes[0].mesh.positions[v_i + 2] ) );
    n_normals.push_back(
      float3( shapes[0].mesh.normals[v_i], shapes[0].mesh.normals[v_i + 1], shapes[0].mesh.normals[v_i + 2] ) );


    n_uvs.push_back( float2( shapes[0].mesh.texcoords[uv_i], shapes[0].mesh.texcoords[uv_i + 1] ) );

    n_elem.push_back( e );
  }

  for( int e = 0; e < n_elem.size(); e += 3 ) {

    float3& v1 = n_vertices[e];
    float3& v2 = n_vertices[e+1];
    float3& v3 = n_vertices[e+2];

    float2& uv1 = n_uvs[e];
    float2& uv2 = n_uvs[e + 1];
    float2& uv3 = n_uvs[e + 2];

    float3 delta_pos1 = v2 - v1;
    float3 delta_pos2 = v3 - v1;

    float2 delta_uv1 = uv2 - uv1;
    float2 delta_uv2 = uv3 - uv1;

    float r = 1.0f / ( delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x );
    float3 tangent = ( delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y )*r;
    float3 bitangent = ( delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x )*r;

    //Tangent
    n_tangent.push_back( tangent );
    n_tangent.push_back( tangent );
    n_tangent.push_back( tangent );

    //Bitangent
    n_bitangent.push_back( bitangent );
    n_bitangent.push_back( bitangent );
    n_bitangent.push_back( bitangent );

  }

  for( int e = 0; e < n_tangent.size(); ++e ){
    float3& t = n_tangent[e];
    float3& b = n_bitangent[e];
    float3& n = n_normals[e];

    t = t - n * float3::dot( n, t );

    if( float3::dot( float3::cross( n, t ), b ) < 0.0f )
      t = t * -1.0f;
  }

  m_indicies_count = ( uint32_t ) n_elem.size();
  int stride = 14;
  uint32_t array_lenght = ( uint32_t ) ( n_vertices.size() * stride );
  float* array_data = new float[array_lenght];

  int count = 0;
  for( int i = 0; i < n_vertices.size(); ++i ) {
    array_data[count++] = n_vertices[i].x;
    array_data[count++] = n_vertices[i].y;
    array_data[count++] = n_vertices[i].z;

    array_data[count++] = n_normals[i].x;
    array_data[count++] = n_normals[i].y;
    array_data[count++] = n_normals[i].z;

    array_data[count++] = n_uvs[i].x;
    array_data[count++] = n_uvs[i].y;

    array_data[count++] = n_tangent[i].x;
    array_data[count++] = n_tangent[i].y;
    array_data[count++] = n_tangent[i].z;

    array_data[count++] = n_bitangent[i].x;
    array_data[count++] = n_bitangent[i].y;
    array_data[count++] = n_bitangent[i].z;

  }

  // Indicies Buffer
  uint32_t* elements_data = new uint32_t[m_indicies_count];
  for( uint32_t e = 0; e < n_elem.size(); ++e )
    elements_data[e] = n_elem[e];


  k_engine->get_GPU_pool()->queue_geometry( static_cast< Geometry* >( this ),
    array_data, array_lenght, elements_data, m_indicies_count );

  //GPU pool responsible for deleting this
  //delete[] array_data;
  //delete[] elements_data;
}

Geometry::~Geometry() {

}