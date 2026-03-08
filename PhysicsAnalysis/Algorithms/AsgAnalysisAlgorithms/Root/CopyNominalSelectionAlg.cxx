/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgAnalysisAlgorithms/CopyNominalSelectionAlg.h>

#include <PATInterfaces/ISystematicsTool.h>

//
// method implementations
//

namespace CP
{
  CopyNominalSelectionAlg ::
  CopyNominalSelectionAlg (const std::string& name,
                     ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {}



  StatusCode CopyNominalSelectionAlg ::
  initialize ()
  {
    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_particlesHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_systematicsList.initialize());

    ANA_CHECK (makeSelectionReadAccessor (m_selectionHandle.getSelection(), m_readAccessor));
    ANA_CHECK (m_readAccessor->fillSystematics (m_systematicsList.service(), m_systematicsList.systematicsVector(), m_particlesHandle));

    return StatusCode::SUCCESS;
  }



  StatusCode CopyNominalSelectionAlg ::
  execute ()
  {
    static const SystematicSet emptySys;
    const xAOD::IParticleContainer *nominal = nullptr;

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      if (sys.empty())
      {
        ANA_CHECK (m_particlesHandle.retrieve (nominal, sys));
      } else
      {
        const xAOD::IParticleContainer *particles = nullptr;
        ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
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
    }

    return StatusCode::SUCCESS;
  }
}
