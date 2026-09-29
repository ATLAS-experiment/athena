/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgAnalysisAlgorithms/OverlapRemovalAlg.h>

#include <boost/container/static_vector.hpp>

#include <utility>

//
// method implementations
//

namespace CP
{

  StatusCode OverlapRemovalAlg ::
  initialize ()
  {
    if (m_overlapRemovalDecoration.empty())
    {
      ANA_MSG_ERROR ("no overlap removal decoration name set");
      return StatusCode::FAILURE;
    }

    m_overlapRemovalAccessor = std::make_unique<SG::Accessor<char> > (m_overlapRemovalDecoration);

    ANA_CHECK (m_overlapTool.retrieve());
    ANA_CHECK (m_electronsHandle.initialize (m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_electronsSelectionHandle.initialize (m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK (m_muonsHandle.initialize (m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_muonsSelectionHandle.initialize (m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK (m_jetsHandle.initialize (m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_jetsSelectionHandle.initialize (m_systematicsList, m_jetsHandle, SG::AllowEmpty));
    ANA_CHECK (m_tausHandle.initialize (m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_tausSelectionHandle.initialize (m_systematicsList, m_tausHandle, SG::AllowEmpty));
    ANA_CHECK (m_photonsHandle.initialize (m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_photonsSelectionHandle.initialize (m_systematicsList, m_photonsHandle, SG::AllowEmpty));
    ANA_CHECK (m_fatJetsHandle.initialize (m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_fatJetsSelectionHandle.initialize (m_systematicsList, m_fatJetsHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode OverlapRemovalAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // capacity is the number of object types handled below (electrons, muons, jets, taus, photons, fat jets)
      boost::container::static_vector<std::pair<const xAOD::IParticleContainer *, const SysWriteSelectionHandle *>, 6> decorations;
      auto addDecoration = [&] (const xAOD::IParticleContainer *container, const SysWriteSelectionHandle *handle)
      {
        decorations.push_back ({container, handle});
      };

      const xAOD::ElectronContainer *electrons {nullptr};
      if (m_electronsHandle)
      {
        ANA_CHECK (m_electronsHandle.getCopy (electrons, sys, ctx));
        if (m_electronsSelectionHandle)
          addDecoration(electrons, &m_electronsSelectionHandle);
      }
      const xAOD::MuonContainer *muons {nullptr};
      if (m_muonsHandle)
      {
        ANA_CHECK (m_muonsHandle.getCopy (muons, sys, ctx));
        if (m_muonsSelectionHandle)
          addDecoration(muons, &m_muonsSelectionHandle);
      }
      const xAOD::JetContainer *jets {nullptr};
      if (m_jetsHandle)
      {
        ANA_CHECK (m_jetsHandle.getCopy (jets, sys, ctx));
        if (m_jetsSelectionHandle)
          addDecoration(jets, &m_jetsSelectionHandle);
      }
      const xAOD::TauJetContainer *taus {nullptr};
      if (m_tausHandle)
      {
        ANA_CHECK (m_tausHandle.getCopy (taus, sys, ctx));
        if (m_tausSelectionHandle)
          addDecoration(taus, &m_tausSelectionHandle);
      }
      const xAOD::PhotonContainer *photons {nullptr};
      if (m_photonsHandle)
      {
        ANA_CHECK (m_photonsHandle.getCopy (photons, sys, ctx));
        if (m_photonsSelectionHandle)
          addDecoration(photons, &m_photonsSelectionHandle);
      }
      const xAOD::JetContainer *fatJets {nullptr};
      if (m_fatJetsHandle)
      {
        ANA_CHECK (m_fatJetsHandle.getCopy (fatJets, sys, ctx));
        if (m_fatJetsSelectionHandle)
          addDecoration(fatJets, &m_fatJetsSelectionHandle);
      }

      ATH_CHECK (m_overlapTool->removeOverlaps (electrons, muons, jets, taus,
                                                photons, fatJets));

      // Re-decorate if needed
      for (const auto& [container, handle] : decorations)
      {
        for (const xAOD::IParticle *particle : *container)
        {
          handle->setBool
            (*particle, (*m_overlapRemovalAccessor) (*particle), sys);
        }
      }
    }

    return StatusCode::SUCCESS;
  }
}
