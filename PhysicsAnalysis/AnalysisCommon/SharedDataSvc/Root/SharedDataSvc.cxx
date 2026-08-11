/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SharedDataSvc/SharedDataSvc.h>

#include <boost/core/demangle.hpp>

//
// method implementations
//

namespace asg
{
  StatusCode SharedDataSvc ::
  getMakeSharedVoid (const std::string& name, const std::type_info& type, std::shared_ptr<const void>& data, const std::function<StatusCode (std::shared_ptr<const void>&)>& generator) const
  {
    std::scoped_lock lock (m_mutex);
    auto it = m_data.find(name);
    if (it != m_data.end())
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
    m_data[name] = std::pair<const std::type_info*,std::weak_ptr<const void>> (&type, data);
    return StatusCode::SUCCESS;
  }
}
