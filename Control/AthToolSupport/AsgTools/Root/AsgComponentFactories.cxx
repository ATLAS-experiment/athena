/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgTools/AsgComponentFactories.h>

#include <AsgTools/IAsgTool.h>
#include <AsgTools/MessageCheckAsgTools.h>
#include <tbb/concurrent_unordered_map.h>

//
// method implementations
//

namespace asg
{
#ifdef XAOD_STANDALONE

  namespace
  {
    tbb::concurrent_unordered_map<std::string, std::function<std::unique_ptr<AsgComponent>(const std::string& name)>> s_factories;
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
    auto iter = s_factories.find (type);
    if (iter == s_factories.end())
      return nullptr;
    return &iter->second;
  }

#endif
}
