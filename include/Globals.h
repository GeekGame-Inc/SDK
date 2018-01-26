#ifndef MODIO_GLOBALS_H
#define MODIO_GLOBALS_H

#include <iostream>

#include "Utility.h"

namespace modio
{
  struct CurrentDownloadInfo
  {
    std::string url;
    double download_total;
	double download_progress;
  };

  extern std::string API_KEY;
  extern std::string ACCESS_TOKEN;
  extern int GAME_ID;
  extern std::string ROOT_PATH;
  extern unsigned int DEBUG_LEVEL;
  extern std::string MODIO_URL;
  extern std::string MODIO_VERSION_PATH;
  extern u32 MAX_CALL_CACHE;
  extern u32 LAST_EVENT_POLL;
  extern void (*callback)(ModioResponse response, ModioModEvent* mod_events_array, u32 mod_events_array_size);
}

#endif
