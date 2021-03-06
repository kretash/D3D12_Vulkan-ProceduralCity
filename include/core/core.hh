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

#define PI 3.14159265358979323846264338f

#include <stdint.h>

#if CHECK_MEM
#ifdef _DEBUG
#define __KRETASH_MOD__ 1
#define _CRTDBG_MAP_ALLOC
#include<iostream>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif