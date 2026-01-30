/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "BoostedJetTaggerAlg.h"

#include "xAODJet/JetContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCore/ShallowAuxContainer.h"

namespace BJT{

    BoostedJetTaggerAlg::BoostedJetTaggerAlg(const std::string &name,
                                                ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator){

    }

    StatusCode BoostedJetTaggerAlg::initialize(){

      ATH_MSG_INFO("Initializing BoostedJetTaggerAlg");

      // Read input handles
      ATH_CHECK(m_jets.initialize());
      
      // jet tagger
      ATH_CHECK(m_tagger.retrieve());

      return StatusCode::SUCCESS;
    }

    StatusCode BoostedJetTaggerAlg::execute(){
              
        // Retrieve inputs
        SG::ReadHandle<xAOD::JetContainer> jets(m_jets);

        // jet tagger
        ATH_CHECK(m_tagger -> decorate(*jets));

        return StatusCode::SUCCESS;
    }

}
