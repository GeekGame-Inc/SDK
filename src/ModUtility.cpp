#include "ModUtility.h"

namespace modio
{
void addCallToCache(std::string url, nlohmann::json response_json)
{
  u32 current_time_seconds = modio::getCurrentTimeSeconds();
  std::string current_time_string = modio::toString(current_time_seconds);
  std::string filename = current_time_string + ".json";
  modio::writeJson(modio::getModIODirectory() + "cache/" + filename, response_json);
  nlohmann::json cache_file_json = modio::openJson(modio::getModIODirectory() + "cache.json");
  // TODO: Restore automatic cache removal?
  for (nlohmann::json::iterator it = cache_file_json.begin(); it != cache_file_json.end(); ++it)
  {
    if (modio::hasKey((*it), "url") && modio::hasKey((*it), "file") && (*it)["url"] == url)
    {
      std::string cache_filename = (*it)["file"];
      cache_file_json.erase(it);
      modio::removeFile(modio::getModIODirectory() + "cache/" + cache_filename);
      break;
    }
  }

  nlohmann::json cache_object;
  cache_object["datetime"] = current_time_seconds;
  cache_object["file"] = filename;
  cache_object["url"] = url;
  cache_file_json.push_back(cache_object);

  modio::writeJson(modio::getModIODirectory() + "cache.json", cache_file_json);
}

std::string getCallFileFromCache(std::string url, u32 max_age_seconds)
{
  nlohmann::json cache_file_json = modio::openJson(modio::getModIODirectory() + "cache.json");
  u32 current_time = modio::getCurrentTimeSeconds();
  for (nlohmann::json::iterator it = cache_file_json.begin(); it != cache_file_json.end(); ++it)
  {
    if (modio::hasKey((*it), "url") && modio::hasKey((*it), "datetime") && modio::hasKey((*it), "file") && (*it)["url"] == url)
    {
      u32 file_datetime = (*it)["datetime"];
      u32 difference = current_time - file_datetime;

      if (difference <= max_age_seconds)
      {
        return (*it)["file"];
      }
    }
  }
  return "";
}

void installDownloadedMods()
{
  modio::writeLogLine("Installing downloaded mods...", MODIO_DEBUGLEVEL_LOG);

  nlohmann::json downloaded_mods_json = modio::openJson(modio::getModIODirectory() + "downloaded_mods.json");

  for (auto &downloaded_mod_json : downloaded_mods_json)
  {
    std::string installation_path = downloaded_mod_json["installation_path"];
    std::string downloaded_zip_path = downloaded_mod_json["downloaded_zip_path"];
    nlohmann::json mod_json = downloaded_mod_json["mod"];

    if (modio::hasKey(mod_json, "id") &&
        modio::hasKey(mod_json, "modfile") &&
        modio::hasKey(mod_json["modfile"], "id") &&
        modio::hasKey(mod_json, "date_updated"))
    {
      u32 mod_id = mod_json["id"];
      u32 modfile_id = mod_json["modfile"]["id"];
      u32 date_updated = mod_json["date_updated"];

      modio::writeLogLine("Installing mod " + modio::toString(mod_id), MODIO_DEBUGLEVEL_LOG);

      modio::createDirectory(installation_path);
      modio::writeLogLine("Extracting...", MODIO_DEBUGLEVEL_LOG);
      modio::minizipwrapper::extract(downloaded_zip_path, installation_path);
      modio::writeLogLine("Removing temporary file...", MODIO_DEBUGLEVEL_LOG);
      modio::removeFile(downloaded_zip_path);
      modio::writeJson(installation_path + std::string("modio.json"), mod_json);

      modio::writeLogLine("Finished installing mod", MODIO_DEBUGLEVEL_LOG);
    }
    else
    {
      modio::writeLogLine("Mod data is missing, could not install mod.", MODIO_DEBUGLEVEL_ERROR);
    }
  }
  nlohmann::json empty_json;
  modio::writeJson(modio::getModIODirectory() + "downloaded_mods.json", empty_json);
  modio::downloaded_mods.clear();
  updateInstalledModsJson();
  modio::writeLogLine("Finished installing downloaded mods", MODIO_DEBUGLEVEL_LOG);
}

static nlohmann::json createDownloadedModJson(std::string installation_path, std::string downloaded_zip_path, nlohmann::json mod_json)
{
  nlohmann::json downloaded_mod;
  downloaded_mod["installation_path"] = installation_path;
  downloaded_mod["downloaded_zip_path"] = downloaded_zip_path;
  downloaded_mod["mod"] = mod_json;
  return downloaded_mod;
}

void addToDownloadedModsJson(std::string installation_path, std::string downloaded_zip_path, nlohmann::json mod_json)
{
  nlohmann::json downloaded_mods_json = modio::openJson(modio::getModIODirectory() + "downloaded_mods.json");

  bool already_downloaded = false;

  for (auto &downloaded_mod : downloaded_mods)
  {
    if (downloaded_mod == mod_json["id"])
    {
      already_downloaded = true;
      break;
    }
  }

  if (!already_downloaded)
  {
    downloaded_mods.push_back(mod_json["id"]);
    downloaded_mods_json.push_back(createDownloadedModJson(installation_path, downloaded_zip_path, mod_json));
    modio::writeJson(modio::getModIODirectory() + "downloaded_mods.json", downloaded_mods_json);
  }
}

bool checkIfModIsStillInstalled(std::string path, u32 mod_id)
{
  nlohmann::json installed_mod_json = modio::openJson(path + "modio.json");
  if (modio::hasKey(installed_mod_json, "id"))
    return mod_id == installed_mod_json["id"];
  return false;
}

bool checkIfModfileIsStillInstalled(std::string path, u32 modfile_id)
{
  nlohmann::json installed_mod_json = modio::openJson(path + "modio.json");
  if (modio::hasKey(installed_mod_json, "modfile") && modio::hasKey(installed_mod_json["modfile"], "id"))
    return modfile_id == installed_mod_json["modfile"]["id"];
  return false;
}

void updateInstalledModsJson()
{
  modio::writeLogLine("Checking installed mod cache data...", MODIO_DEBUGLEVEL_LOG);

  std::string mods_directory_path = modio::getModIODirectory() + "mods/";
  std::vector<std::string> directory_names = getDirectoryNames(mods_directory_path);

  modio::installed_mods.clear();
  
  for(auto mod_directory : directory_names)
  {
    std::string installed_mod_directory_path = mods_directory_path + mod_directory;
    nlohmann::json installed_mod_json = modio::openJson(installed_mod_directory_path + "/modio.json");

    if(modio::hasKey(installed_mod_json, "date_updated")
        && modio::hasKey(installed_mod_json, "id")
        && modio::hasKey(installed_mod_json, "modfile")
        && modio::hasKey(installed_mod_json["modfile"], "id")
        )
    {
      modio::writeLogLine(modio::toString((u32)installed_mod_json["id"]) + " detected at " + installed_mod_directory_path + "/", MODIO_DEBUGLEVEL_LOG);

      nlohmann::json installed_mod_json_element;
      installed_mod_json_element["date_updated"] = installed_mod_json["date_updated"];
      installed_mod_json_element["mod_id"] = installed_mod_json["id"];
      installed_mod_json_element["modfile_id"] = installed_mod_json["modfile"]["id"];
      installed_mod_json_element["path"] = installed_mod_directory_path + "/";

      modio::installed_mods.push_back(installed_mod_json_element);
    }
  }


  modio::writeJson(modio::getModIODirectory() + "installed_mods.json", modio::installed_mods);
  modio::writeLogLine("Finished checking installed mod cache data...", MODIO_DEBUGLEVEL_LOG);
}

void clearOldCache()
{
  modio::writeLogLine("Clearing old cache files...", MODIO_DEBUGLEVEL_LOG);
  u32 current_time = modio::getCurrentTimeSeconds();
  nlohmann::json cache_json = modio::openJson(modio::getModIODirectory() + "cache.json");
  nlohmann::json resulting_cache_json;
  for (auto cache_file_json : cache_json)
  {
    if (modio::hasKey(cache_file_json, "datetime") && modio::hasKey(cache_file_json, "file"))
    {
      u32 cache_time = cache_file_json["datetime"];
      u32 time_difference = current_time - cache_time;

      if (time_difference > MAX_CACHE_TIME_SECONDS)
      {
        std::string cache_file_to_delete_path = modio::getModIODirectory() + "cache/";
        std::string cache_filename = cache_file_json["file"];
        cache_file_to_delete_path += cache_filename;
        modio::removeFile(cache_file_to_delete_path);
      }
      else
      {
        resulting_cache_json.push_back(cache_file_json);
      }
    }
  }
  modio::writeJson(modio::getModIODirectory() + "cache.json", resulting_cache_json);
  modio::writeLogLine("Finished clearing old cache files.", MODIO_DEBUGLEVEL_LOG);
}

std::string getInstalledModPath(u32 mod_id)
{
  for (auto installed_mod_json : modio::installed_mods)
  {
    if (modio::hasKey(installed_mod_json, "mod_id") && modio::hasKey(installed_mod_json, "path") && installed_mod_json["mod_id"] == mod_id)
    {
      return installed_mod_json["path"];
    }
  }
  return "";
}
} // namespace modio
