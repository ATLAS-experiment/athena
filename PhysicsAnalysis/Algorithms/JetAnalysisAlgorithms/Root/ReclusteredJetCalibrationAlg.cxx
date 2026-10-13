/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Jackson Burzynski


//
// includes
//

#include <JetAnalysisAlgorithms/ReclusteredJetCalibrationAlg.h>
#include <vector>
#include <string>
//
// method implementations
//

namespace
{
  const std::string manualCalibScaleStr{"JetManualCalibScaleMomentum"};
}

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
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // container we read in
      xAOD::JetContainer *reclusteredJets = nullptr;
      ATH_CHECK(m_reclusteredJetHandle.getCopy(reclusteredJets, sys, ctx));

      const xAOD::JetContainer *smallRJets = nullptr;
      ATH_CHECK(m_smallRJetHandle.retrieve(smallRJets, sys, ctx));
     
      // loop over jets
      
      for (xAOD::Jet *jet : *reclusteredJets)
      {  
        const std::vector< ElementLink< xAOD::IParticleContainer > >& element_links = jet->constituentLinks();
        if (element_links.empty()) {
          ATH_MSG_ERROR("Reclustered jet has no constituent links, cannot recalibrate it from the small-R jets");
          return StatusCode::FAILURE;
        }

        xAOD::JetFourMom_t calibP4;
        calibP4.SetPxPyPzE(0,0,0,0);
        std::size_t nValid = 0;
        for (const auto& element_link: element_links){
          if (! element_link.isValid() ) {
            ATH_MSG_WARNING("Subjet element link invalid !!!!");
            continue;
          }
          const auto subjet = *element_link;
          const SG::AuxVectorData* subjetContainer = subjet->container();
          if (subjetContainer == nullptr || subjetContainer->size_v() != smallRJets->size()) {
            ATH_MSG_ERROR("Size of the reclustered jet constituents' parent container does not match "
                          "the size of the small-R jet container " << m_smallRJetHandle.getName(sys)
                          << ", check the smallRJets configuration");
            return StatusCode::FAILURE;
          }
          int index = subjet->index();
          const xAOD::Jet* subjetCal = smallRJets->at(index);
          calibP4 += subjetCal->jetP4();
          ++nValid;
        }
        if (nValid == 0) {
          ATH_MSG_WARNING("Reclustered jet has no valid constituent links, its four-momentum is set to zero");
        }
        // Set calibrated P4
        jet->setAttribute<xAOD::JetFourMom_t>(manualCalibScaleStr,calibP4);
        jet->setJetP4( calibP4 );
      }
    }

    return StatusCode::SUCCESS;
  }
}
