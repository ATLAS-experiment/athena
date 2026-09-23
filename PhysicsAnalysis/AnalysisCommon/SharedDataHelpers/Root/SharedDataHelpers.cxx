/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SharedDataHelpers/SharedDataHelpers.h>

#include <SharedDataHelpers/MessageCheck.h>

#include <CxxUtils/checker_macros.h>

#include <boost/core/demangle.hpp>

#include <mutex>
#include <unordered_map>

//
// method implementations
//

namespace asg
{
  namespace detail
  {
    StatusCode getMakeSharedDataVoid (const std::string& name, const std::type_info& type, std::shared_ptr<const void>& data, const std::function<StatusCode (std::shared_ptr<const void>&)>& generator)
    {
      using namespace msgSharedDataHelpers;

      static std::recursive_mutex s_mutex ATLAS_THREAD_SAFE;
      static std::unordered_map<std::string,std::pair<const std::type_info*,std::weak_ptr<const void>>> s_data ATLAS_THREAD_SAFE;

      std::scoped_lock lock (s_mutex);
      auto it = s_data.find(name);
      if (it != s_data.end())
      {
        if (*it->second.first != type)
        {
          ANA_MSG_ERROR ("type mismatch for shared data " << name << " " << boost::core::demangle (type.name()) << " vs " << boost::core::demangle (it->second.first->name()));
          return StatusCode::FAILURE;
        }
        if ((data = it->second.second.lock()))
          return StatusCode::SUCCESS;
      }

      if (generator(data).isFailure())
      {
        ANA_MSG_ERROR ("failed to generate shared data " << name);
        return StatusCode::FAILURE;
      }
      if (data == nullptr)
      {
        ANA_MSG_ERROR ("generated shared data " << name << " is nullptr");
        return StatusCode::FAILURE;
      }
      s_data[name] = std::pair<const std::type_info*,std::weak_ptr<const void>> (&type, data);
      return StatusCode::SUCCESS;
    }
  }
}
