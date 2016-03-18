/*
----------------------------------------------------------------------------------------------------
------                  _   _____ _  __                     ------------ /_/\  ---------------------
------              |/ |_) |_  | |_|(_ |_|                  ----------- / /\ \  --------------------
------              |\ | \ |__ | | |__)| |                  ---------- / / /\ \  -------------------
------   CARLOS MARTINEZ ROMERO - kretash.wordpress.com     --------- / / /\ \ \  ------------------
------                                                      -------- / /_/__\ \ \  -----------------
------       PROCEDURAL CITY RENDERING WITH THE NEW         ------  /_/______\_\/\  ----------------
------            GENERATION GRAPHICS APIS                  ------- \_\_________\/ -----------------
----------------------------------------------------------------------------------------------------

Licensed under the MIT License (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the License at
http://opensource.org/licenses/MIT
*/

#pragma once
#include <string>
#include <chrono>
#include "types.hh"

#define MAX_TIMERS 16
#define k_engine_settings (EngineSettings::get_instance())

#define k_start_count_timer( str ) k_engine_settings->start_timer( str );

#define k_end_count_timer( str )  {static double all_times = 0.0f; \
                                  static int32_t samples = 0; \
                                  if( samples > 200 && samples < 300 )  \
                                    all_times += k_engine_settings->get_time( str );\
                                  else if( samples > 300 ) \
                                    std::cout << str << "count --------------------------------  " << all_times << std::endl; \
                                  ++samples; }

#define k_start_min_timer( str ) k_engine_settings->start_timer( str );

#define k_end_min_timer( str )  {static double min_time = 100000.0f; \
                                  static int32_t samples = 0; \
                                  if( samples > 200 && samples < 300 ){  \
                                    double time = k_engine_settings->get_time( str );\
                                    if( time < min_time ) min_time = time; \
                                  }else if( samples > 300 ){ \
                                    std::cout << str << " min ----------------------------------  " << min_time << std::endl;} \
                                  ++samples;}


#define k_start_max_timer( str ) k_engine_settings->start_timer( str );

#define k_end_max_timer( str )  {static double max_time = 0.0f; \
                                  static int32_t samples = 0; \
                                  if( samples > 200 && samples < 300 ){  \
                                    double time = k_engine_settings->get_time( str );\
                                    if( time > max_time ) max_time = time; \
                                  }else if( samples > 300 ){ \
                                    std::cout << str << " max ----------------------------------  " << max_time << std::endl;} \
                                  ++samples;}

#define k_start_print_timer( str ) k_engine_settings->start_timer( str );

#define k_end_print_timer( str )  std::cout << str << " --- " << k_engine_settings->get_time( str ) << std::endl;

#define PATH                    "../assets/"
#define TPATH                   "../assets/texture/"
#define WSPATH                   L"../assets/shaders/"
#define SPATH                  "../assets/shaders/"
#define SOUNDPATH               "../assets/sound/"
#define OPATH                   "../assets/obj/"

namespace kretash {
  class                           EngineSettings {
  public:
    static EngineSettings* get_instance() {
      if( m_instance == nullptr )
        m_instance = new EngineSettings();
      return m_instance;
    }

    static void shutdown() {
      delete m_instance;
      m_instance = nullptr;
    }

    void                          start_timer( std::string timer_id );
    double                        get_time( std::string timer_id );

    void                          start_frame();
    void                          start_render();
    void                          end_frame();

    float                         get_time_elapsed() { return m_time_elapsed; }
    float                         get_delta_time() { return m_delta_time; }
    float                         get_update_time(){ return m_update_time; }
    float                         get_render_time(){ return m_render_time; }
    float                         get_smooth_delta_time() { return m_smooth_delta_time; }
    float                         get_smooth_update_time() { return m_smooth_update_time; }
    float                         get_smooth_render_time() { return m_smooth_render_time; }

    engine_settings               get_settings();
    engine_settings*              get_psettings();
    void                          save_settings();
    void                          log( std::string l );

  private:
    EngineSettings();
    ~EngineSettings();

    static EngineSettings*        m_instance;

    int32_t                       frame_counter;
    float                         m_time_elapsed;

    float                         m_delta_time;
    float                         m_update_time;
    float                         m_render_time;

    float                         m_smooth_delta_time;
    float                         m_smooth_update_time;
    float                         m_smooth_render_time;
    float                         m_smooth_delta_time_additve;
    float                         m_smooth_update_time_additve;
    float                         m_smooth_render_time_additve;

    engine_settings               m_engine_settings;

    std::string                   ids_[MAX_TIMERS];

    std::chrono::high_resolution_clock::time_point
      starting_time[MAX_TIMERS];
  };
}