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
#include <Windows.h>
#include <string>
#include "types.hh"

#define MAX_TIMERS 16
#define FPS_SMOOTH 10
#define k_engine_settings (EngineSettings::get_instance())

#define k_start_count_timer( str ) k_engine_settings->start_timer( str );

#define k_end_count_timer( str )  static double all_times = 0.0f; \
                                  static int32_t samples = 0; \
                                  if( samples > 200 && samples < 300 )  \
                                    all_times += k_engine_settings->get_time( str );\
                                  else if( samples > 300 ) \
                                    std::cout << "----------------------------------------  " << all_times << std::endl; \
                                  ++samples; 

#define PATH                    "../assets/"
#define TPATH                   "../assets/texture/"
#define SPATH                   L"../assets/shaders/"
#define SOUNDPATH               "../assets/sound/"
#define OPATH                   "../assets/obj/"

class                           EngineSettings {
public:
  static EngineSettings* get_instance() {
    if( m_instance == nullptr )
      m_instance = new EngineSettings();
    return m_instance;
  }

  static void shutdown(){
    delete m_instance;
    m_instance = nullptr;
  }

  void                          start_timer( std::string timer_id );
  double                        get_time( std::string timer_id );

  void                          start_frame();
  void                          start_render();
  void                          end_frame();

  float                         get_delta_time() { return m_delta_time; }
  float                         get_time_elapsed() { return m_time_elapsed; }

  engine_settings               get_settings();
  void                          log( std::string l );

private:
  EngineSettings();
  ~EngineSettings();

  static EngineSettings*        m_instance;

  int32_t                       frame_counter;
  float                         m_delta_time;
  float                         m_time_elapsed;
  float                         m_fps_smooth;
  float                         m_update_smooth;
  float                         m_render_smooth;
  engine_settings               m_engine_settings;

  std::string                   ids_[MAX_TIMERS];
  LARGE_INTEGER                 starting_time[MAX_TIMERS];
  LARGE_INTEGER                 ending_time[MAX_TIMERS];
  LARGE_INTEGER                 elapsed_microseconds[MAX_TIMERS];
  LARGE_INTEGER                 frequency[MAX_TIMERS];
};