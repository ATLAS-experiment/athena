/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


//
// includes
//

#include <AsgAnalysisAlgorithms/AsgOriginalObjectLinkAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode AsgOriginalObjectLinkAlg ::
  initialize ()
  {
    if (m_baseContainerName.empty())
    {
      ANA_MSG_ERROR ("Base particle container name should not be empty.");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_particleHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_baseContainerName.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode AsgOriginalObjectLinkAlg ::
  execute (const EventContext &ctx) const
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particleHandle.getCopy (particles, sys));

      SG::ReadHandle<xAOD::IParticleContainer> baseParticles(m_baseContainerName, ctx);

      if (!xAOD::setOriginalObjectLink (*baseParticles, *particles))
      {
        ATH_MSG_ERROR ("Cannot set original object links for container named " << m_baseContainerName);
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }
}
