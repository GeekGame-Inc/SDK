#include "c++/ModIOInstance.h"

namespace modio
{
void Instance::getModStats(u32 mod_id, const std::function<void(const modio::Response &response, const modio::Stats &stats)> &callback)
{
  struct GetModStatsCall *get_mod_stats_call = new GetModStatsCall{callback};
  get_mod_stats_calls[current_call_id] = get_mod_stats_call;

  modioGetModStats((void*)((uintptr_t)current_call_id), mod_id, &onGetModStats);

  current_call_id++;
}

void Instance::getAllModStats(modio::FilterCreator &filter, const std::function<void(const modio::Response &, const std::vector<modio::Stats> &mods_stats)> &callback)
{
  struct GetAllModStatsCall *get_all_mod_stats_call = new GetAllModStatsCall{callback};
  get_all_mod_stats_calls[current_call_id] = get_all_mod_stats_call;

  modioGetAllModStats((void*)((uintptr_t)current_call_id), *filter.getFilter(), &onGetAllModStats);

  current_call_id++;
}
} // namespace modio
