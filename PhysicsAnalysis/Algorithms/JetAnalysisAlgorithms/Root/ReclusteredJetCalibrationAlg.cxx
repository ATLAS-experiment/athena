/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Jackson Burzynski


//
// includes
//

#include <JetAnalysisAlgorithms/ReclusteredJetCalibrationAlg.h>
#include <AthContainers/ConstDataVector.h>

//
// method implementations
//

namespace CP
{

  StatusCode ReclusteredJetCalibrationAlg ::
  initialize ()
  {
    ANA_CHECK(m_reclusteredJetHandle.initialize(m_systematicsList));
    ANA_CHECK(m_smallRJetHandle.initialize(m_systematicsList));

    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode ReclusteredJetCalibrationAlg ::
  execute (const EventContext &ctx) const
  {
    (void) ctx;
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // container we read in
      xAOD::JetContainer *reclusteredJets = nullptr;
      ATH_CHECK(m_reclusteredJetHandle.getCopy(reclusteredJets, sys));

      const xAOD::JetContainer *smallRJets = nullptr;
      ATH_CHECK(m_smallRJetHandle.retrieve(smallRJets, sys));

      // loop over jets
      for (xAOD::Jet *jet : *reclusteredJets)
      {  
        const std::vector< ElementLink< xAOD::IParticleContainer > >& element_links = jet->constituentLinks();

        xAOD::JetFourMom_t calibP4;
        calibP4.SetPxPyPzE(0,0,0,0);
        for (auto element_link: element_links){
          if (! element_link.isValid() ) {
            ATH_MSG_WARNING("Subjet element link invalid !!!!");
            continue;
          }
          const auto subjet = *element_link;
          int index = subjet->index();
          const xAOD::Jet* subjetCal = smallRJets->at(index);
          calibP4 += subjetCal->jetP4();
        }
        // Set calibrated P4
        jet->setAttribute<xAOD::JetFourMom_t>("JetManualCalibScaleMomentum",calibP4);
        jet->setJetP4( calibP4 );
      }
    }

    return StatusCode::SUCCESS;
  }
}
