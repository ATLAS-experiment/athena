/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//
#include "AsgAnalysisAlgorithms/EventDecoratorAlg.h"

// /// Anonymous namespace for helpers
// namespace {
//   const static SG::ConstAuxElement::ConstAccessor<unsigned int> accRRN("RandomRunNumber");
//   const static SG::ConstAuxElement::Decorator<unsigned int> decRRN("RandomRunNumber");
//   const static SG::ConstAuxElement::Decorator<unsigned int> decRLBN("RandomLumiBlockNumber");
//   const static SG::ConstAuxElement::Decorator<uint64_t> decHash("PRWHash");
// }

//
// method implementations
//

namespace CP
{

  StatusCode EventDecoratorAlg ::
  initialize ()
  {
    for (auto& [name, value] : m_uint32Decorations)
    {
      ANA_MSG_INFO ("Adding uint32_t decoration " << name << " with value " << value << " to EventInfo");
      m_decFunctions.push_back([dec = SG::Decorator<uint32_t>(name), value](const xAOD::EventInfo& ei) { dec(ei) = value; });
    }

    ANA_CHECK (m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode EventDecoratorAlg ::
  execute ()
  {
    // Take care of the weight (which is the only thing depending on systematics)
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo* systEvtInfo = nullptr;
      ANA_CHECK( m_eventInfoHandle.retrieve(systEvtInfo, sys));
      for (const auto& decFunc : m_decFunctions)
      {
        decFunc(*systEvtInfo);
      }
    };
    return StatusCode::SUCCESS;
  }
}
