/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgTools/AsgComponentFactories.h>

#include <AsgTools/IAsgTool.h>
#include <AsgTools/MessageCheckAsgTools.h>
#include <TSystem.h>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <tbb/concurrent_unordered_map.h>
#include <unordered_set>

//
// method implementations
//

namespace asg
{
#ifdef XAOD_STANDALONE

  namespace
  {
    tbb::concurrent_unordered_map<std::string, std::function<std::unique_ptr<AsgComponent>(const std::string& name)>> s_factories;
    tbb::concurrent_unordered_map<std::string,std::string> s_typeModuleMap;

    struct ModuleData final
    {
      std::atomic<bool> loaded{false};
      std::mutex mutex;
    };
    tbb::concurrent_unordered_map<std::string,ModuleData> s_modules;
  }



  StatusCode registerComponentFactory (const std::string& type, const std::function<std::unique_ptr<AsgComponent>(const std::string& name)>& factory)
  {
    using namespace msgComponentConfig;
    if (!s_factories.emplace (type, factory).second)
    {
      ATH_MSG_ERROR ("attempt to register a factory for type " << type << " that already has a factory");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }



  const std::function<std::unique_ptr<AsgComponent>(const std::string& name)> *getComponentFactory (const std::string& type)
  {
    using namespace msgComponentConfig;
    if (auto iter = s_factories.find (type); iter != s_factories.end())
      return &iter->second;
    static std::once_flag flag;
    std::call_once (flag, [&] () {
      const char *path = gSystem->Getenv ("LD_LIBRARY_PATH");
      if (!path) return;
      std::vector<std::string> paths;
      boost::split (paths, path, boost::is_any_of (":"));
      std::regex pathRegex ("\\.asgcomponents$");
      for (const auto& p : paths)
      {
        if (p.empty()) continue;
        std::error_code ec;
        if (!std::filesystem::exists(p, ec))
        {
          ANA_MSG_DEBUG ("skipping non-existent directory for component factory maps: " << p);
          continue;
        }
        ANA_MSG_DEBUG ("checking for component factory map in " << p);
        // find all files that end in ".asgcomponents"
        for (auto const& entry : std::filesystem::directory_iterator(p))
        {
          if (std::regex_search (entry.path().filename().string(), pathRegex))
            loadComponentFactoryMap (entry.path());
        }
      }
    });
    if (auto iter = s_typeModuleMap.find (type); iter != s_typeModuleMap.end())
    {
      ATH_MSG_DEBUG ("loading component factory module for type " << type << " from " << iter->second);
      loadComponentFactoryModule (iter->second);
    } else
    {
      ATH_MSG_DEBUG ("no component factory module known for type " << type);
      return nullptr;
    }
    if (auto iter = s_factories.find (type); iter != s_factories.end())
      return &iter->second;
    else
    {
      ATH_MSG_WARNING ("no component factory found for type " << type << " even after loading its module");
      return nullptr;
    }
  }



  std::vector<std::string> getLoadedComponentFactoryTypes ()
  {
    using namespace msgComponentConfig;
    ATH_MSG_DEBUG ("getting component factory types");
    std::vector<std::string> result;
    result.reserve (s_factories.size());
    for (const auto& pair : s_factories)
      result.push_back (pair.first);
    std::sort (result.begin(), result.end());
    return result;
  }



  void loadComponentFactoryModule (const std::string& moduleName, const std::string& modulePath)
  {
    using namespace msgComponentConfig;
    ATH_MSG_DEBUG ("loading component factory module " << moduleName);

    auto [iter, inserted] = s_modules.emplace(std::piecewise_construct, std::forward_as_tuple(moduleName), std::forward_as_tuple());
    if (iter->second.loaded == true)
    {
      ATH_MSG_DEBUG ("component factory module " << moduleName << " already loaded");
      return;
    }
    std::scoped_lock lock(iter->second.mutex);
    if (iter->second.loaded == true)
    {
      ATH_MSG_DEBUG ("component factory module " << moduleName << " already loaded");
      return;
    }

    // Load the module and register its factories
    std::string path;
    if (!modulePath.empty())
      path = modulePath + "/" + moduleName;
    else
      path = gSystem->DynamicPathName (moduleName.c_str());
    ANA_MSG_DEBUG ("loading component factory module from " << path);
    if (gSystem->Load (path.c_str()) < 0)
    {
      ATH_MSG_FATAL ("failed to preload component factory module " << moduleName);
      std::terminate();
    }
    iter->second.loaded = true;
  }



  void loadComponentFactoryMap (const std::string& path)
  {
    using namespace msgComponentConfig;
    ATH_MSG_DEBUG ("loading component factory map from " << path);

    std::ifstream inputStream (path);
    if (!inputStream)
    {
      ATH_MSG_FATAL ("failed to open component factory map file " << path);
      std::abort();
    }
    std::unordered_set<std::string> foundModules;
    std::unordered_set<std::string> skippedModules;
    std::string moduleName, typeName;
    while (inputStream >> moduleName >> typeName)
    {
      // If the module is already known from another component factory
      // map, we skip it. This is for a niche case in which we drop a
      // component factory map in our local version that exists in the
      // release.
      if (auto iter = s_modules.find(moduleName); iter != s_modules.end())
      {
        if (skippedModules.emplace(moduleName).second)
          ANA_MSG_DEBUG ("skipping information on " << moduleName << " from " << path << " because it has already been described in another file");
        continue;
      }
      foundModules.insert(moduleName);
      ATH_MSG_DEBUG ("registering component factory type " << typeName << " from module " << moduleName);
      auto [iter, success] = s_typeModuleMap.emplace (typeName, moduleName);
      if (!success)
      {
        if (iter->second != moduleName)
        {
          ATH_MSG_FATAL ("conflicting component factory module for type " << typeName
                         << ": already registered from " << iter->second
                         << ", but trying to register from " << moduleName);
          std::abort();
        } else
        {
          ATH_MSG_DEBUG ("component factory type " << typeName << " registered twice for module " << moduleName);
        }
      }
    }
    for (const auto& moduleName : foundModules)
      s_modules.emplace(std::piecewise_construct, std::forward_as_tuple(moduleName), std::forward_as_tuple());
  }

#endif
}
