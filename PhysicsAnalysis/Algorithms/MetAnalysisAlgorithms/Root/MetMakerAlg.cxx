/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MetAnalysisAlgorithms/MetMakerAlg.h>

#include <xAODMissingET/MissingETAuxContainer.h>
#include <xAODMissingET/MissingETAssociationHelper.h>

#include "AthContainers/ConstDataVector.h"

//
// method implementations
//

namespace CP
{

  StatusCode MetMakerAlg ::
  initialize ()
  {
    ANA_CHECK (m_makerTool.retrieve());

    for (auto* handle : {&m_electronsHandle, &m_photonsHandle,
                         &m_muonsHandle, &m_tausHandle}) {
      ANA_CHECK (handle->initialize (m_systematicsList, SG::AllowEmpty));
    }
    ANA_CHECK (m_invisHandles.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK (m_electronsSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK (m_muonsSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK (m_photonsSelection.initialize(m_systematicsList, m_photonsHandle, SG::AllowEmpty));
    ANA_CHECK (m_tausSelection.initialize(m_systematicsList, m_tausHandle, SG::AllowEmpty));
    //ANA_CHECK (m_invisSelections.initialize(m_systematicsList, m_invisHandles));
     // Initialize invisible selections - need to pair each selection with its corresponding handle
    if (m_invisSelections.size() != m_invisHandles.size()) {
      ATH_MSG_ERROR("Number of invisible selections (" << m_invisSelections.size()
                    << ") doesn't match number of invisible handles (" << m_invisHandles.size() << ")");
      return StatusCode::FAILURE;
    }
    for (size_t i = 0; i < m_invisHandles.size(); ++i) {
      ANA_CHECK (m_invisSelections.at(i).initialize(m_systematicsList, m_invisHandles.at(i), SG::AllowEmpty));
    }
    ANA_CHECK (m_jetsHandle.initialize (m_systematicsList));
    ANA_CHECK (m_metHandle.initialize (m_systematicsList));

    if (!m_systematicsTool.empty())
    {
      ANA_CHECK (m_systematicsTool.retrieve());
      ANA_CHECK (m_systematicsList.addSystematics (*m_systematicsTool));
    }

    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode MetMakerAlg ::
  execute ()
  {
    const xAOD::MissingETContainer* metcore {nullptr};
    ANA_CHECK (evtStore()->retrieve(metcore, m_metCoreName));

    const xAOD::MissingETAssociationMap* metMap {nullptr};
    ANA_CHECK (evtStore()->retrieve(metMap, m_metAssociationName));

    // Helper keeps track of object selection flags for this map
    xAOD::MissingETAssociationHelper metHelper(&(*metMap));

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      auto met = std::make_unique<xAOD::MissingETContainer> ();
      auto aux = std::make_unique<xAOD::MissingETAuxContainer> ();
      met->setStore (aux.get());

      metHelper.resetObjSelectionFlags();

      for (size_t i = 0; i < m_invisHandles.size(); ++i) {
        const xAOD::IParticleContainer* invisible = nullptr;
        ATH_CHECK( m_invisHandles.at(i).retrieve(invisible, sys) );
        ConstDataVector<xAOD::IParticleContainer> invisSelected(SG::VIEW_ELEMENTS);
        for (const xAOD::IParticle *invisParticle : *invisible)
          if (m_invisSelections.at(i).getBool(*invisParticle, sys))
            invisSelected.push_back(invisParticle);
        ANA_CHECK (m_makerTool->markInvisible (invisSelected.asDataVector(), metHelper, met.get() ) );
      }

      // Lambda helping with calculating the MET terms coming from the leptons
      // (and photons).
      auto processParticles =
        [&] (SysReadHandle<xAOD::IParticleContainer>& handle,
             SysReadSelectionHandle &selection,
             xAOD::Type::ObjectType type,
             const std::string& term) -> StatusCode {
          if (!handle) {
            return StatusCode::SUCCESS;
          }
          const xAOD::IParticleContainer* particles = nullptr;
          ANA_CHECK (handle.retrieve (particles, sys));
          ConstDataVector<xAOD::IParticleContainer> selected(SG::VIEW_ELEMENTS);
          for (const xAOD::IParticle *particle : *particles)
              if (selection.getBool(*particle, sys))
                  selected.push_back(particle);
          ANA_CHECK (m_makerTool->rebuildMET (term, type, met.get(),
                                              selected.asDataVector(), metHelper));
          return StatusCode::SUCCESS;
        };

      // Calculate the terms coming from the user's selected objects.
      ANA_CHECK (processParticles (m_electronsHandle, m_electronsSelection,
                                   xAOD::Type::Electron, m_electronsKey));
      ANA_CHECK (processParticles (m_photonsHandle, m_photonsSelection,
                                   xAOD::Type::Photon, m_photonsKey));
      ANA_CHECK (processParticles (m_tausHandle, m_tausSelection,
                                   xAOD::Type::Tau, m_tausKey));
      ANA_CHECK (processParticles (m_muonsHandle, m_muonsSelection,
                                   xAOD::Type::Muon, m_muonsKey));

      const xAOD::JetContainer *jets {nullptr};
      ANA_CHECK (m_jetsHandle.retrieve (jets, sys));

      if (m_doTrackMet)
      {
        ANA_CHECK (m_makerTool->rebuildTrackMET (m_jetsKey, m_softTermKey, met.get(), jets, metcore, metHelper, m_doJetJVT));
      } else
      {
        ANA_CHECK (m_makerTool->rebuildJetMET (m_jetsKey, m_softTermKey, met.get(), jets, metcore, metHelper, m_doJetJVT));
      }

      // Systematics
      if (!m_systematicsTool.empty())
      {
        ANA_CHECK (m_systematicsTool->applySystematicVariation (sys));

        xAOD::MissingET *softTerm = (*met)[m_softTermKey];
        if (softTerm == nullptr)
        {
          ANA_MSG_ERROR ("failed to find MET soft-term \"" << m_softTermKey << "\"");
          return StatusCode::FAILURE;
        }

        // Set the seed for reproducibility
        m_systematicsTool->setRandomSeed(static_cast<int>(1e6 * softTerm->phi()));

        // This returns a `CorrectionCode`, so in principle this could
        // return an `OutOfValidity` result, but I have no idea what
        // that would mean or how to handle it, so I'm implicitly
        // converting it into a `FAILURE` instead.
        ANA_CHECK (m_systematicsTool->applyCorrection (*softTerm, metHelper));
      }

      ANA_CHECK (m_metHandle.record (std::move (met), std::move (aux), sys));
    }

    return StatusCode::SUCCESS;
  }
}
