/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

//
// includes
//

#include <AsgAnalysisAlgorithms/AsgUnionPreselectionAlg.h>

#include <xAODBase/IParticle.h>

//
// method implementations
//

namespace CP
{

  StatusCode AsgUnionPreselectionAlg ::
  initialize ()
  {
    if (m_selectionDecoration.empty())
    {
      ANA_MSG_ERROR("Selection decoration can not be empty.");
      return StatusCode::FAILURE;
    }
    if (m_preselection.empty())
    {
      ANA_MSG_ERROR("Preselection can not be empty.");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_particlesHandle.initialize(m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_systematicsList.initialize());

    m_decorator.emplace (m_selectionDecoration);

    return StatusCode::SUCCESS;
  }



  StatusCode AsgUnionPreselectionAlg ::
  execute ()
  {
    // first loop through systematics and set the default selection
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
      for (auto *particle : *particles)
      {
        (*m_decorator) (*particle) = false;
      }
    }

    // second loop through systematics, and set the selection to true if
    // particle passes the selection
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
      for (auto *particle : *particles)
      {
        if (!(*m_decorator) (*particle) && m_preselection.getBool (*particle, sys))
          (*m_decorator) (*particle) = true;
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace CP
