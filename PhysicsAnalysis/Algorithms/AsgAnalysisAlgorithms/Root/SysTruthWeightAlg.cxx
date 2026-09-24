/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Miha Muskinja

//
// includes
//

// Local include(s):
#include <AsgAnalysisAlgorithms/SysTruthWeightAlg.h>
#include <AsgDataHandles/ReadHandle.h>

//
// method implementations
//

namespace CP
{
StatusCode SysTruthWeightAlg::initialize()
{
  if (m_decoration.empty())
  {
      ANA_MSG_ERROR("no decoration name set");
      return StatusCode::FAILURE;
  }

  ANA_CHECK(m_truthParticleContainer.initialize());
  ANA_CHECK(m_sysTruthWeightTool.retrieve());
  ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
  ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
  ANA_CHECK(m_systematicsList.addSystematics(*m_sysTruthWeightTool));
  ANA_CHECK(m_systematicsList.initialize());

  return StatusCode::SUCCESS;
}

StatusCode SysTruthWeightAlg::execute(const EventContext& ctx)
{

  // Retreive the truth particles container
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_truthParticleContainer, ctx);
  ANA_CHECK(truthParticles.isValid());

  for (const auto &sys : m_systematicsList.systematicsVector())
  {
    const xAOD::EventInfo *eventInfo = nullptr;
    ANA_CHECK(m_eventInfoHandle.retrieve(eventInfo, sys, ctx));

    m_decoration.set(*eventInfo, m_sysTruthWeightTool->getSysWeight(truthParticles.cptr(), sys), sys);
  }

  // return
  return StatusCode::SUCCESS;
}
} // namespace CP
