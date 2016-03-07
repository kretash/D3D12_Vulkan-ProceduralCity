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

#include "core.hh"

#ifdef __VULKAN__

#include <vector>

struct                              mem_block;

enum pool_type{
  kDEVICE_MEMORY = 0,
  kHOST_VISIBLE = 1,
};

class VKPool{
public:
  VKPool();
  ~VKPool();

  void                              init( uint64_t size, pool_type t );
  mem_block                         get_mem( uint64_t size );
  void                              release( mem_block m );
  void                              defrag();

private:
  pool_type                         m_pool_type;
  std::vector<mem_block>            m_free_memory;
  std::vector<mem_block>            m_used_memory;
};

#endif