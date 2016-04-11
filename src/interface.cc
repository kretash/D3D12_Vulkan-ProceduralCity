#include "core/interface.hh"
#include "imgui/imgui.h"
#include "core/engine.hh"
#include "core/window.hh"
#include "core/factory.h"
#include "core/input.hh"
#include "core/renderer.hh"
#include "core/xx/renderer.hh"
#include "core/engine_settings.hh"
#include "core/world.hh"
#include "core/tools.hh"
#include <fstream>
#include <iostream>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

namespace kretash {

#define SHADER_SIZE 8192
  
  Interface::Interface() {
    for( int i = 0; i < SHADERS_COUNT; ++i ) {
      m_text_buffer[i] = new char[SHADER_SIZE];
      memset( m_text_buffer[i], 0, SHADER_SIZE );
    }
  }

  void Interface::init() {
    Factory* m_factory = k_engine->get_factory();
    m_factory->make_interface( &m_interface );

    m_interface->init( k_engine->get_window() );

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    engine_settings* es = k_engine_settings->get_psettings();
    m_selected_API = es->m_api - 1;
  }

  bool Interface::handle_events( UINT msg, WPARAM wParam, LPARAM lParam ) {

    ImGuiIO& io = ImGui::GetIO();
    switch( msg ) {
    case WM_LBUTTONDOWN:
      io.MouseDown[0] = true;
      return true;
    case WM_LBUTTONUP:
      io.MouseDown[0] = false;
      return true;
    case WM_RBUTTONDOWN:
      io.MouseDown[1] = true;
      return true;
    case WM_RBUTTONUP:
      io.MouseDown[1] = false;
      return true;
    case WM_MOUSEWHEEL:
      io.MouseWheel += GET_WHEEL_DELTA_WPARAM( wParam ) > 0 ? +1.0f : -1.0f;
      return true;
    case WM_MOUSEMOVE:
      io.MousePos.x = ( signed short ) ( lParam );
      io.MousePos.y = ( signed short ) ( lParam >> 16 );
      return true;
    case WM_KEYDOWN:
      if( wParam < 256 )
        io.KeysDown[wParam] = 1;
      return true;
    case WM_KEYUP:
      if( wParam < 256 )
        io.KeysDown[wParam] = 0;
      return true;
    case WM_CHAR:
      // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
      if( wParam > 0 && wParam < 0x10000 )
        io.AddInputCharacter( ( unsigned short ) wParam );
      return true;
    }
    return false;
  }

  void Interface::new_frame() {

    ImGuiIO& io = ImGui::GetIO();

    Window* window = k_engine->get_window();
    Input* input = k_engine->get_input();

    io.DisplaySize.x = static_cast< float >( k_engine->get_window()->get_width() );
    io.DisplaySize.y = static_cast< float >( k_engine->get_window()->get_height() );
    io.DeltaTime = k_engine_settings->get_delta_time() / 1000.0f;//change it to seconds
    io.KeyCtrl = ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
    io.KeyShift = ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
    io.KeyAlt = ( GetKeyState( VK_MENU ) & 0x8000 ) != 0;
    io.MousePos = ImVec2( ( float ) input->get_cursor().x, ( float ) input->get_cursor().y );
    io.MouseDown[0] = input->stealth_get_key( k_LEFT_MOUSE_BTN );
    io.MouseDown[1] = input->stealth_get_key( k_MID_MOUSE_BTN );
    io.MouseDown[2] = input->stealth_get_key( k_RIGHT_MOUSE_BTN );
    SetCursor( io.MouseDrawCursor ? nullptr : LoadCursor( nullptr, IDC_ARROW ) );

    ImGui::NewFrame();

  }

  void Interface::render() {

    m_delta_times.push_back( k_engine_settings->get_delta_time() );
    while( m_delta_times.size() > 100 ) { m_delta_times.erase( m_delta_times.begin() ); }

    m_update_times.push_back( k_engine_settings->get_delta_time() );
    while( m_update_times.size() > 100 ) { m_update_times.erase( m_update_times.begin() ); }

    m_render_times.push_back( k_engine_settings->get_delta_time() );
    while( m_render_times.size() > 100 ) { m_render_times.erase( m_render_times.begin() ); }

    if( false == k_engine->get_input()->has_focus() ) {

      _menu_bar();

      if( m_options ) {
        _options();
      }

      if( m_shader_editor ) {
        _shader_menu();
        _shader_editor();
      }


      //bool open = true;
      //static ExampleAppConsole console;
      //console.Draw( "Example: Console", &open );
    }

      if( ( m_performance_allways || false == k_engine->get_input()->has_focus()) && m_performance ) {
        _performance();
      }

    //bool t = true;
    //ImGui::ShowTestWindow( &t );


    {

      bool open = true;
      int32_t width = k_engine->get_window()->get_width();
      int32_t heihgt = k_engine->get_window()->get_height();

      float delta_t = k_engine_settings->get_smooth_delta_time();
      float update_t = k_engine_settings->get_smooth_update_time();
      float render_t = k_engine_settings->get_smooth_render_time();

      ImGui::SetNextWindowPos( ImVec2( ( float ) width - 250.0f, 25.0f ) );

      ImGui::Begin( "Fixed Overlay", &open, ImVec2( 0, 0 ),
        0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings );

      ImGui::Text( "Carlos Martinez Romero" );
      ImGui::Text( "Procedural City Rendering With" );
      ImGui::Text( "The New Generation Graphics APIs" );
      ImGui::Separator();
      ImGui::Text( "FPS        : %.1f", 1000.0f / delta_t );
      ImGui::Text( "Delta  Time: %.3f", delta_t );
      ImGui::Text( "Update Time: %.3f", update_t );
      ImGui::Text( "Render Time: %.3f", render_t );
      ImGui::Separator();
      ImGui::Text( "Press F1 to show the menu. " );
      ImGui::End();

    }

    ImGui::Render();

  }

  void Interface::_menu_bar() {

    if( ImGui::BeginMainMenuBar() ) {
      if( ImGui::BeginMenu( "File" ) ) {
        if( ImGui::MenuItem( "Exit", "ESC" ) ) {
          k_engine->quit();
        }
        ImGui::EndMenu();
      }
      if( ImGui::BeginMenu( "View" ) ) {
        if( ImGui::MenuItem( "Options", nullptr, &m_options ) ) {
          m_shader_editor = false;
          m_performance = false;
        }
        if( ImGui::MenuItem( "Shader Editor", nullptr, &m_shader_editor ) ) {
          m_options = false;
          m_performance = false;
        }
        if( ImGui::MenuItem( "Performance Stats", nullptr, &m_performance ) ) {
          m_options = false;
          m_shader_editor = false;
        }

        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
  }

  void Interface::_options() {
    float alpha = -0.01f;
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_ShowBorders;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoScrollbar;

    float h = ( float ) k_engine->get_window()->get_height() - 32.0f;

    ImGui::Begin( "Options", &m_options, ImVec2( 320.0f, h ), alpha, window_flags );
    ImGui::SetWindowPos( "Options", ImVec2( 4, 24 ) );

    if( ImGui::CollapsingHeader( "Engine" ) ) {

      engine_settings* es = k_engine_settings->get_psettings();
      bool clicked = false;

      ImGui::Combo( "API", &m_selected_API, " Vulkan \0 D3D12 \0\0" );

      //static int32_t resolution = 1;
      //ImGui::Combo( "Resolution", &resolution, " 1920x1080 \0 1600x900 \0 1280x720 \0\0" );

      //static bool fullscreen = false;
      //ImGui::Checkbox( "Fullscreen", &fullscreen );

      //static bool msaa = false;
      //ImGui::Checkbox( "MSAA", &msaa );

      //static int32_t msaa_samples = 0;
      //ImGui::SliderInt( "MSAA Samples", &msaa_samples, 0, 8 );

      if( ImGui::Button( "Apply" ) ) {

        engine_settings* es = k_engine_settings->get_psettings();
        API selected = ( API ) ( m_selected_API + 1 );

          es->m_api = selected;
          //k_engine_settings->save_settings();
          k_engine->reload();
        
      }
    }

    if( ImGui::CollapsingHeader( "World" ) ) {

      World* w = k_engine->get_world();
      engine_settings* es = k_engine_settings->get_psettings();

      ImGui::SliderFloat( "Time", w->get_time(), 0.0f, 24.0f );
      ImGui::Checkbox( "Stop time", w->get_stop_time() );
      ImGui::Checkbox( "Update city", &es->update_city );
      
      bool cinematic_camera = k_engine->get_camera()->get_cinematic_camera();
      ImGui::Checkbox( "Cinematic Camera", &cinematic_camera );
      k_engine->get_camera()->set_cinematic_camera( cinematic_camera );

    }

    ImGui::End();
  }

  void Interface::_performance() {
    float alpha = -0.01f;
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_ShowBorders;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoScrollbar;

    float h = ( float ) k_engine->get_window()->get_height() - 32.0f;

    ImGui::Begin( "Performance", &m_options, ImVec2( 320.0f, h ), alpha, window_flags );
    ImGui::SetWindowPos( "Performance", ImVec2( 4, 24 ) );

    ImGui::Checkbox( "Always show performance. ", &m_performance_allways );

    ImGui::PlotHistogram( "Delta Time", m_delta_times.data(), ( int ) m_delta_times.size(), 0, nullptr, 
      0.0f, 60.0f, ImVec2( 0, 80 ) );

    ImGui::PlotHistogram( "Update Time", m_update_times.data(), ( int ) m_update_times.size(), 0, nullptr, 
      0.0f, 60.0f, ImVec2( 0, 80 ) );

    ImGui::PlotHistogram( "Render Time", m_render_times.data(), ( int ) m_render_times.size(), 0, nullptr, 
      0.0f, 60.0f, ImVec2( 0, 80 ) );

    ImGui::End();
  }

  void Interface::_shader_menu() {
    float alpha = -0.01f;
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_ShowBorders;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoScrollbar;

    float h = ( float ) k_engine->get_window()->get_height() - 32.0f;
    _set_shader_filenames();

    ImGui::Begin( "Shader Editor", &m_shader_editor, ImVec2( 320.0f, h ), alpha, window_flags );
    ImGui::SetWindowPos( "Shader Editor", ImVec2( 4, 24 ) );

    if( m_shader_error.size() != 0 ) {

      ImGui::OpenPopup( "Shader Error" );
      if( ImGui::BeginPopupModal( "Shader Error" ) ) {

        ImGui::Text( m_shader_error.c_str() );

        if( ImGui::Button( "Close" ) ) {
          ImGui::CloseCurrentPopup();
          m_shader_error = "";
        }

        ImGui::EndPopup();
      }
    }

    if( ImGui::CollapsingHeader( "Basic" ) ) {

      _shader_selectables( "basic.hlsl", "basic.vert", "basic.frag", 0 );

      if( ImGui::Button( "Compile" ) && m_shader_error.size() == 0 ) {
        _compile_shaders( "basic", 0 );
      }
    }

    if( ImGui::CollapsingHeader( "Texture" ) ) {

      _shader_selectables( "texture.hlsl", "texture.vert", "texture.frag", 2 );

      if( ImGui::Button( "Compile" ) && m_shader_error.size() == 0 ) {
        _compile_shaders( "texture", 2 );
      }
    }

    if( ImGui::CollapsingHeader( "Skydome" ) ) {

      _shader_selectables( "skydome.hlsl", "skydome.vert", "skydome.frag", 4 );

      if( ImGui::Button( "Compile" ) && m_shader_error.size() == 0 ) {
        _compile_shaders( "skydome", 4 );

      }
    }

    if( ImGui::CollapsingHeader( "Post" ) ) {

      _shader_selectables( "post.hlsl", "post.vert", "post.frag", 6 );

      if( ImGui::Button( "Compile" ) && m_shader_error.size() == 0 ) {
        _compile_shaders( "post", 6 );

      }
    }

    ImGui::End();
  }

  void Interface::_shader_editor() {
    for( int i = 0; i < SHADERS_COUNT; ++i ) {
      if( m_shaders_open[i] ) {

        float alpha = -0.01f;
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_ShowBorders;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoScrollbar;

        float h = ( float ) k_engine->get_window()->get_height() - 32.0f;
        float w = ( float ) k_engine->get_window()->get_width() - 340.0f;

        ImGui::Begin( "Shader Text Editor", &m_shaders_open[i], ImVec2( w, h ), alpha, window_flags );
        ImGui::SetWindowPos( "Shader Text Editor", ImVec2( 330, 24 ) );

        if( m_text_buffer[i][0] == 0 ) {

          std::string line;
          std::ifstream file( SPATH + m_shader_files[i] );
          int32_t p = 0;

          if( file.is_open() ) {
            while( std::getline( file, line ) ) {

              for( int e = 0; e < line.size(); e++ ) {
                m_text_buffer[i][p++] = line[e];
              }
              m_text_buffer[i][p++] = '\n';
            }
            file.close();
          } else {
            std::cout << "Error opening file " << std::endl;
          }
        }

        ImGui::InputTextMultiline( "##source", &m_text_buffer[i][0], SHADER_SIZE, ImVec2( -1.0f, -1.0f ), 0 );

        ImGui::End();
      }
    }
  }

  void Interface::_make_shader_active( int32_t i ) {

    for( int e = 0; e < i; e++ ) m_shaders_open[e] = false;
    m_shaders_open[i] = true;
    for( int e = i + 1; e < SHADERS_COUNT; e++ ) m_shaders_open[e] = false;

  }

  void Interface::_compile_shaders( std::string filename, int32_t base_id ) {

    API m_api = k_engine_settings->get_settings().m_api;

    if( m_api == kVulkan ) {
      std::string error = "";

      _save_shader( filename + ".vert", base_id );
      _save_shader( filename + ".frag", base_id + 1 );

      tools::compile_vulkan_shaders( filename + ".vert", &error );
      if( error.size() != 0 ) {
        m_shader_error = error;
      } else {
        tools::compile_vulkan_shaders( filename + ".frag", &error );
        if( error.size() != 0 ) m_shader_error = error;
      }
    } else if( m_api == kD3D12 ) {

      _save_shader( filename + ".hlsl", base_id );

      std::string error = "";
      tools::compile_dx_shaders( filename + ".hlsl", &error );
      if( error.size() != 0 ) m_shader_error = error;

    }

    if( m_shader_error.size() == 0 )
      k_engine->get_renderer( rTEXTURE )->get_renderer()->create_graphics_pipeline( rTEXTURE );
  }

  void Interface::_save_shader( std::string filename, int32_t id ) {

    std::ofstream vs;
    vs.open( SPATH + filename, std::ofstream::out | std::ofstream::trunc );

    if( vs.is_open() && m_text_buffer[id][0] != 0 ) {
      vs << m_text_buffer[id];
      vs.close();
    } else {
      std::cout << " Error opening" << filename << "\n";
    }

  }

  void Interface::_set_shader_filenames() {
    API m_api = k_engine_settings->get_settings().m_api;

    if( m_api == kD3D12 ) {
      m_shader_files[0] = "basic.hlsl";
      m_shader_files[1] = "basic.hlsl";
      m_shader_files[2] = "texture.hlsl";
      m_shader_files[3] = "texture.hlsl";
      m_shader_files[4] = "skydome.hlsl";
      m_shader_files[5] = "skydome.hlsl";
      m_shader_files[6] = "post.hlsl";
      m_shader_files[7] = "post.hlsl";
    } else if( m_api == kVulkan ) {
      m_shader_files[0] = "basic.vert";
      m_shader_files[1] = "basic.frag";
      m_shader_files[2] = "texture.vert";
      m_shader_files[3] = "texture.frag";
      m_shader_files[4] = "skydome.vert";
      m_shader_files[5] = "skydome.frag";
      m_shader_files[6] = "post.vert";
      m_shader_files[7] = "post.frag";
    }

  }

  void Interface::_shader_selectables( std::string d3d12, std::string vk_vs, std::string vk_fs, int32_t id ) {

    API m_api = k_engine_settings->get_settings().m_api;
    bool clicked = false;

    if( m_api == kD3D12 ) {
      if( ImGui::Selectable( d3d12.c_str(), &clicked ) ) {
        if( clicked ) {
          _make_shader_active( id );
          clicked = false;
        }
      }
    } else if( m_api == kVulkan ) {
      if( ImGui::Selectable( vk_vs.c_str(), &clicked ) ) {
        if( clicked ) {
          _make_shader_active( id );
          clicked = false;
        }
      }
      if( ImGui::Selectable( vk_fs.c_str(), &clicked ) ) {
        if( clicked ) {
          _make_shader_active( id + 1 );
          clicked = false;
        }
      }
    }
  }

  Interface::~Interface() {

  }
}
