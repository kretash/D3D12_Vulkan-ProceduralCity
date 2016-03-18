#include "core/engine_settings.hh"
#include <cassert>
#include <iostream>
#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define FPS_SMOOTH 30

namespace kretash {

  EngineSettings* EngineSettings::m_instance = nullptr;

  EngineSettings::EngineSettings() :
    frame_counter( 0 ),
    m_time_elapsed( 0.0f ),
    m_delta_time( 0.0f ),
    m_update_time( 0.0f ),
    m_render_time( 0.0f ),
    m_smooth_delta_time( 0.0f ),
    m_smooth_update_time( 0.0f ),
    m_smooth_render_time( 0.0f ),
    m_smooth_delta_time_additve( 0.0f ),
    m_smooth_update_time_additve( 0.0f ),
    m_smooth_render_time_additve( 0.0f )
  {
    for( int32_t i = 0; i < MAX_TIMERS; ++i ) {
      ids_[i] = "NULL";
    }

    std::ifstream json;
    std::string json_line;
    std::string json_buffer;
    json.open( PATH"engine_settings.json", std::ios::out );

    if( json.is_open() ) {
      while( std::getline( json, json_line ) ) {
        json_buffer += json_line;
      }
      json.close();
    }

    rapidjson::Document doc;
    doc.Parse( json_buffer.c_str() );

    m_engine_settings.m_api = ( API ) doc["API"].GetInt();
    m_engine_settings.resolution_width = doc["resolution_width"].GetInt();
    m_engine_settings.resolution_height = doc["resolution_height"].GetInt();
    m_engine_settings.fullscreen = doc["fullscreen"].GetBool();
    m_engine_settings.grid = doc["grid"].GetInt();
    m_engine_settings.animated_camera = doc["animated_camera"].GetBool();
    m_engine_settings.play_sound = doc["play_sound"].GetBool();
    m_engine_settings.sound_file = doc["sound_file"].GetString();
    m_engine_settings.msaa_enabled = doc["MSAA_enabled"].GetBool();
    m_engine_settings.msaa_count = doc["MSAA_count"].GetInt();
    m_engine_settings.upscale_render = doc["upscale_render"].GetDouble();

  }

  void EngineSettings::start_timer( std::string timer_id ) {
    for( int32_t i = 0; i <= MAX_TIMERS; ++i ) {
      assert( i != MAX_TIMERS && "RAN OUT OF TIMERS " );

      if( ids_[i] == timer_id || ids_[i] == "NULL" ) {
        ids_[i] = timer_id;
        starting_time[i] = std::chrono::high_resolution_clock::now();
        break;
      }
    }
  }

  double EngineSettings::get_time( std::string timer_id ) {

    using namespace std::chrono;
    high_resolution_clock::time_point end = high_resolution_clock::now();

    for( int32_t i = 0; i <= MAX_TIMERS; ++i ) {
      assert( i != MAX_TIMERS && "RAN OUT OF TIMERS " );

      if( ids_[i] == timer_id ) {

        duration<double> time_span = duration_cast< duration<double> >( end - starting_time[i] );

        return time_span.count()*1000.0f;
      }
    }

    assert( false && "TIMER NOT FOUND" );
    return -1.0;
  }

  void EngineSettings::save_settings() {

    std::string json_buffer;
    {
      std::ifstream json;
      std::string json_line;
      json.open( PATH"engine_settings.json", std::ios::out );

      if( json.is_open() ) {
        while( std::getline( json, json_line ) ) {
          json_buffer += json_line;
        }
        json.close();
      }
    }
    rapidjson::Document doc;
    doc.Parse( json_buffer.c_str() );

    doc["API"].SetInt( m_engine_settings.m_api );
    doc["resolution_width"].SetInt( m_engine_settings.resolution_width );
    doc["resolution_height"].SetInt( m_engine_settings.resolution_height );
    doc["fullscreen"].SetBool( m_engine_settings.fullscreen );
    doc["grid"].SetInt( m_engine_settings.grid );
    doc["animated_camera"].SetBool( m_engine_settings.animated_camera );
    doc["play_sound"].SetBool( m_engine_settings.play_sound );
    doc["sound_file"].SetString( m_engine_settings.sound_file.c_str(), ( rapidjson::SizeType )m_engine_settings.sound_file.size() );
    doc["MSAA_enabled"].SetBool( m_engine_settings.msaa_enabled );
    doc["MSAA_count"].SetInt( m_engine_settings.msaa_count );
    doc["upscale_render"].SetDouble( m_engine_settings.upscale_render );

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );
    doc.Accept( writer );

    {
      std::ofstream json;
      std::string json_line;
      json.open( PATH"engine_settings.json", std::ofstream::out | std::ofstream::trunc );

      if( json.is_open() ) {
        json << buffer.GetString();
        json.close();
      }
    }

  }

  void EngineSettings::start_frame() {
    start_timer( "frame" );
  }

  void EngineSettings::start_render() {
    start_timer( "render" );
  }

  void EngineSettings::end_frame() {
    ++frame_counter;

    m_delta_time = ( float ) get_time( "frame" );
    m_render_time = ( float ) get_time( "render" );
    m_update_time = m_delta_time - m_render_time;

    m_time_elapsed += m_delta_time;
    m_smooth_delta_time_additve += m_delta_time;
    m_smooth_update_time_additve += m_update_time;
    m_smooth_render_time_additve += m_render_time;

    if( frame_counter > FPS_SMOOTH ) {
      m_smooth_delta_time = m_smooth_delta_time_additve / ( float ) frame_counter;
      m_smooth_update_time = m_smooth_update_time_additve / ( float ) frame_counter;
      m_smooth_render_time = m_smooth_render_time_additve / ( float ) frame_counter;

      m_smooth_delta_time_additve = 0.0f;
      m_smooth_update_time_additve = 0.0f;
      m_smooth_render_time_additve = 0.0f;
      frame_counter = 0;
    }
  }

  engine_settings EngineSettings::get_settings() {
    return m_engine_settings;
  }

  engine_settings* EngineSettings::get_psettings() {
    return &m_engine_settings;
  }

  void EngineSettings::log( std::string l ) {
    std::cout << l << std::endl;
  }

  EngineSettings::~EngineSettings() {

  }
}