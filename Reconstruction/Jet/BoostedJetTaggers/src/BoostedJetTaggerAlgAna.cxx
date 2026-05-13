/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "BoostedJetTaggerAlgAna.h"

#include "xAODJet/JetContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCore/ShallowAuxContainer.h"

namespace BJT{

    BoostedJetTaggerAlgAna::BoostedJetTaggerAlgAna(const std::string &name,
                                                ISvcLocator *pSvcLocator)
      : AnaAlgorithm(name, pSvcLocator){

    }

    StatusCode BoostedJetTaggerAlgAna::initialize(){

      ATH_MSG_INFO("Initializing BoostedJetTaggerAlgAna");

      // Read input handler
      ATH_CHECK(m_jets.initialize(m_systematicsList));
      
      // Intialise syst list
      ANA_CHECK(m_systematicsList.initialize());

      // jet tagger tool
      ATH_CHECK(m_tagger.retrieve());
      if(!m_scalefactor.empty())
        ATH_CHECK(m_scalefactor.retrieve());

      return StatusCode::SUCCESS;
    }

    StatusCode BoostedJetTaggerAlgAna::execute(){

      for (const auto& sys : m_systematicsList.systematicsVector()){

        // Retrieve inputs
        const xAOD::JetContainer *jets = nullptr;
        ANA_CHECK(m_jets.retrieve(jets, sys));

        // jet tagger WP tool
        ATH_CHECK(m_tagger -> decorate(*jets));

        // scale factors tool
        if(!m_scalefactor.empty())
          ATH_CHECK(m_scalefactor -> decorate(*jets));

      }

      return StatusCode::SUCCESS;

    }

}
