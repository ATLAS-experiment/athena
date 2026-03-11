/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//
#include "AsgAnalysisAlgorithms/EventDecoratorAlg.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AthContainers/AuxVectorData.h"
#include "CxxUtils/checker_macros.h"

//
// method implementations
//

namespace CP
{

  StatusCode EventDecoratorAlg ::
  initialize ()
  {
    ANA_CHECK(m_eventInfoKey.initialize());

    for (const auto& [name, value] : m_uint32Decorations)
    {
      ANA_MSG_INFO ("Adding uint32_t decoration " << name << " with value " << value << " to EventInfo");
      SG::WriteDecorHandleKey<xAOD::EventInfo> decorKey{m_eventInfoKey, name};
      ANA_CHECK(decorKey.initialize());
#ifndef XAOD_STANDALONE
      // This adds an output dependency for MT scheduling. I have to
      // manually add the dependency, since the dependency is only
      // auto-declared if the key is also a property, which doesn't seem
      // ideal here.
      addDependency(decorKey.fullKey(), decorKey.mode());
#endif
      //probably safer to use decorKey by value here, despite coverity warning
      //coverity[PASS_BY_VALUE]
      m_decFunctions.push_back([decorKey, value](const xAOD::EventInfo& ei) {
        SG::WriteDecorHandle<xAOD::EventInfo,uint32_t> dec(std::move(decorKey));
        dec(ei) = value;
      });
    }

    return StatusCode::SUCCESS;
  }



  StatusCode EventDecoratorAlg ::
  execute (const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    if (!eventInfo.isValid())
    {
      ANA_MSG_ERROR("Failed to retrieve EventInfo");
      return StatusCode::FAILURE;
    }

    for (const auto& decFunc : m_decFunctions)
    {
      decFunc(*eventInfo);
    }

    return StatusCode::SUCCESS;
  }
}
