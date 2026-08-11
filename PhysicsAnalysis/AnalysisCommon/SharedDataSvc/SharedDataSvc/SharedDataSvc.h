/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SHARED_DATA_SVC_SHARED_DATA_SVC_H
#define SHARED_DATA_SVC_SHARED_DATA_SVC_H

#include <AsgServices/AsgService.h>
#include <CxxUtils/checker_macros.h>
#include <SharedDataSvc/ISharedDataSvc.h>
#include <mutex>
#include <unordered_map>

namespace asg
{
  /// @brief the canonical implementation of @ref ISharedDataSvc

  class SharedDataSvc final : public extends<asg::AsgService, ISharedDataSvc>
  {
    /// Public Members
    /// ==============

  public:
    using extends::extends;  // base class constructor

    virtual StatusCode get_make_shared_void (const std::string& name, const std::type_info& type, std::shared_ptr<const void>& data, const std::function<StatusCode (std::shared_ptr<const void>&)>& generator) const override;


    /// Private Members
    /// ===============
  private:

    mutable std::mutex m_mutex; ATLAS_THREAD_SAFE

    mutable std::unordered_map<std::string,std::pair<const std::type_info*,std::weak_ptr<const void>>> m_data ATLAS_THREAD_SAFE;
  };
}

#endif
