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

#include <iostream>
#include <stdio.h>
#include <cassert>
#include "core/engine_settings.hh"
#include <d3dcompiler.h>

namespace kretash {

  namespace tools {

    static inline float random( float min, float max ) {
      return min + static_cast < float > ( rand() ) / ( static_cast < float > ( RAND_MAX / ( max - min ) ) );
    }

    static inline int random( int min, int max ) {
      return min + static_cast < int > ( rand() ) / ( static_cast < int > ( RAND_MAX / ( max - min ) ) );
    }

    static inline float lerp( float a, float b, float t ) {
      return a + ( b - a ) * t;
    }

    static inline float3 lerp( float3 a, float3 b, float t ) {
      return float3( lerp( a.x, b.x, t ), lerp( a.y, b.y, t ), lerp( a.z, b.z, t ) );
    }

    template<class T>
    static inline T clamp( T a, T min, T max ) {
      T r = a;

      if( a < min )
        a = min;

      if( a > max )
        a = max;

      return a;
    }

    static inline float round_down( float a, float range ) {
      float r = a;

      if( r < range && r > -range )
        r = 0.0f;

      return r;
    }

    static inline float luminance( float3 v ) {
      return v.x * 0.212f + v.y * 0.716f + v.z * 0.072f;
    }

    static void compile_vulkan_shaders( std::string shader, std::string* error ) {

      FILE *fp = nullptr;
      char path[1024];

      char buffer[MAX_PATH];
      GetModuleFileName( NULL, buffer, MAX_PATH );

      //This will break some day
      std::string exe_path = buffer;
      std::size_t found = exe_path.find( "vs2015\\..\\bin\\" );
      assert( found != std::string::npos );
      exe_path = std::string( exe_path.begin(), exe_path.begin() + found );
      exe_path += "assets\\shaders\\";

      std::string cmd = exe_path + "glslangvalidator.exe -V " +
        exe_path + shader + " -o " +
        exe_path + shader + ".spv";

      fp = _popen( cmd.c_str(), "r" );
      assert( fp != nullptr );

      std::string output = "";

      while( fgets( path, 1024, fp ) != NULL )
        output += path;

      //Not the bestestest way to do this
      if( output.size() != 222 && output.size() != 220 ) {
        std::string look_for = "ERROR";
        std::size_t s_found = output.find( look_for );
        std::size_t f_found = std::string( output.begin() + s_found + look_for.size(), output.end() ).find( look_for );
        assert( found != std::string::npos );
        *error = std::string( output.begin() + s_found, output.begin() + s_found + f_found + look_for.size() );
      } else {

        //Compiled
      }

      _pclose( fp );
    }

    static void compile_dx_shaders( std::string shader, std::string* error ) {

#ifdef _DEBUG
      UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PREFER_FLOW_CONTROL;
#else
      UINT compileFlags = 0;
#endif

      ID3DBlob* error_msg = nullptr;
      std::wstring filename = WSPATH + std::wstring( shader.begin(), shader.end() );

      ID3DBlob* temp_shader = nullptr;
      D3DCompileFromFile( filename.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, compileFlags,
        &temp_shader, &error_msg );

      if( error_msg ){
        char* compile_errors = ( char* ) ( error_msg->GetBufferPointer() );
        size_t buffer_size = error_msg->GetBufferSize();
        *error = "";
        for( size_t i = 0; i < buffer_size; i++ ) {
          *error += compile_errors[i];
        }
        error_msg->Release();
        error_msg = nullptr;
        return;
      }

      temp_shader->Release();
      temp_shader = nullptr;
      error_msg = nullptr;

      D3DCompileFromFile( filename.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, compileFlags,
        &temp_shader, &error_msg );

      if( error_msg ) {
        char* compile_errors = ( char* ) ( error_msg->GetBufferPointer() );
        size_t buffer_size = error_msg->GetBufferSize();
        *error = "";
        for( size_t i = 0; i < buffer_size; i++ ) {
          *error += compile_errors[i];
        }
        error_msg->Release();
        error_msg = nullptr;
        return;
      }

      temp_shader->Release();
      temp_shader = nullptr;
      return;
    }

  }
}