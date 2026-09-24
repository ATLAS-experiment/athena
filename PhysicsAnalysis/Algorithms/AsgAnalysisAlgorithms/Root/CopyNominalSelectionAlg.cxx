/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgAnalysisAlgorithms/CopyNominalSelectionAlg.h>

//
// method implementations
//

namespace CP
{
  StatusCode CopyNominalSelectionAlg ::
  initialize ()
  {
    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_particlesHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_systematicsList.initialize());

    bool hasNominal = false;
    for (const auto& sys : m_systematicsList.systematicsVector())
      hasNominal |= sys.empty();
    if (!hasNominal) {
      ANA_MSG_ERROR ("nominal systematic not in systematics list, cannot copy nominal selection");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (makeSelectionReadAccessor (m_selectionHandle.getSelection(), m_readAccessor));
    ANA_CHECK (m_readAccessor->fillSystematics (m_systematicsList.service(), m_systematicsList.systematicsVector(), m_particlesHandle));

    return StatusCode::SUCCESS;
  }



  StatusCode CopyNominalSelectionAlg ::
  execute (const EventContext& ctx)
  {
    static const SystematicSet emptySys;
    const xAOD::IParticleContainer *nominal = nullptr;
    ANA_CHECK (m_particlesHandle.retrieve (nominal, emptySys, ctx));

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      if (sys.empty()) continue;

      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys, ctx));
      if (particles->size() != nominal->size())
        {
          ANA_MSG_ERROR ("systematic container " << sys.name() << " has a different number "
                         "of objects than nominal, can't copy selections!");
          return StatusCode::FAILURE;
        }

      for (std::size_t index = 0u; index != particles->size(); ++ index)
        {
          const xAOD::IParticle *particle = particles->at (index);
          if (m_preselection.getBool (*particle, sys))
            {
              bool passedNominal = m_readAccessor->getBool (*nominal->at(index), &emptySys);
              m_selectionHandle.setBool (*particle, passedNominal, sys);
            }
          else
            {
              m_selectionHandle.setBool (*particle, false, sys);
            }
        }
    }

    return StatusCode::SUCCESS;
  }
}
