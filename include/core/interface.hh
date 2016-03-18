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
#include <memory>
#include <Windows.h>
#include "core/core.hh"
#include "core/xx/interface.hh"

#define SHADERS_COUNT 8

namespace                           kretash {

  enum                              API;

  class                             Interface {
  public:

    Interface();
    ~Interface();

    void                            init();
    void                            new_frame();
    void                            render();
    bool                            handle_events( UINT msg, WPARAM wParam, LPARAM lParam );

  private:
    void                            _menu_bar();

    void                            _options();

    void                            _performance();

    void                            _shader_menu();
    void                            _shader_editor();
    void                            _make_shader_active( int32_t i );
    void                            _set_shader_filenames();
    void                            _shader_selectables( std::string d3d12, std::string vk_vs, std::string vk_fs, int32_t id );
    void                            _compile_shaders( std::string filename, int32_t base_id );
    void                            _save_shader( std::string filename, int32_t id );

    std::shared_ptr<xxInterface>    m_interface;

    bool                            m_options = false;

    bool                            m_performance = false;
    bool                            m_performance_allways = false;
    std::vector<float>              m_delta_times = {};
    std::vector<float>              m_update_times = {};
    std::vector<float>              m_render_times = {};

    bool                            m_shader_editor = false;
    bool                            m_shaders_open[SHADERS_COUNT] = {};
    std::string                     m_shader_files[SHADERS_COUNT] = {};
    char*                           m_text_buffer[SHADERS_COUNT] = {};
    std::string                     m_shader_error = {};
    int32_t                         m_selected_API;
  };
}