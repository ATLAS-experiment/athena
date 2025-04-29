/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// JetMatchingTool.cxx                                                  //
// Implementation file for class JetMatchingTool                        //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
////////////////////////////////////////////////////////////////////////// 

#include "DerivationFrameworkMCTruth/JetMatchingTool.h"

namespace DerivationFramework{

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------- Constructor/Destructor --------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  JetMatchingTool::JetMatchingTool(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t,n,p){
  }

  JetMatchingTool::~JetMatchingTool(){
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  --------------------------------------------------------- Initialize/Finalize ---------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */
  
  StatusCode JetMatchingTool::initialize() {

    ATH_MSG_INFO("Initialize");
    
    // Print the cuts.

    ATH_MSG_INFO("Cut on the pt of the jets: "           << m_jetPtCut);
    ATH_MSG_INFO("Cut on the eta of the jets: "          << m_jetEtaCut);
    ATH_MSG_INFO("Cut for deltaR to consider a match: "  << m_drCut);

    return StatusCode::SUCCESS;
  }

  StatusCode JetMatchingTool::finalize(){
    return StatusCode::SUCCESS;
  }
  
  /*
  --------------------------------------------------------------------------------------------------------------------------------------
  ---------------------------------------------------------- Hadron Matching  ----------------------------------------------------------
  --------------------------------------------------------------------------------------------------------------------------------------
  */

  std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>> JetMatchingTool::matchHadronsToJets(const xAOD::TruthParticleContainer* hadrons, const xAOD::JetContainer* jets) const{

    // NOTE (not from Adria): The matching is unique for hadrons but not for jets
    // Need to cut on jets before matching, not for hadrons where we can do it later.
    // However, the current HFClassification is matching hadrons to the closest jet before the cuts and then cut on jets later - was this studied
    
    // Declare a map variable to store the list of truth particles that are matched to each jet.

    std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>> particleMatch;

    // Match each truth particle to a jet.
    // Use a for to go through the truth particles.

    for(xAOD::TruthParticleContainer::const_iterator hadron = hadrons->begin(); hadron!=hadrons->end(); ++hadron){
          
      // Create a jet object to store the closest one to the truth particle that is being considered.

      const xAOD::Jet* holder = nullptr;

      // Define a variable to store the smallest deltaR between the jets and the hadron.

      float drmin=999999;

      // Use a for to go through the jets.

      for(const xAOD::Jet* jet : *jets){

        // Check if cuts are applied on jets.

        if(m_jetPtCut>0 || m_jetEtaCut>=0){

          // In this case, cuts should be applied.
          // The jet is not considered if the cuts are not satisfied.

          if(jet->p4().Pt()<m_jetPtCut) continue;
          if(fabs(jet->p4().Eta())>m_jetEtaCut) continue;
        }
        
        // Compute deltaR between the jet and the hadron and check if it is smaller than drmin.
        // If it is smaller, save the jet in holder and dr in drmin.

        float dr = jet->p4().DeltaR((*hadron)->p4());

        if(dr<drmin){
          drmin=dr;
          holder=jet;
        }
        
      }

      // If the smallest drmin is smaller than drcut, then the truth particle is matched to the jet holder.
      // Hence, save truth particle in particleMatch to the position that corresponds to jet holder.

      if(drmin<m_drCut){        
        particleMatch[holder].push_back(hadron);
      }

    }

    // Return the map particleMatch

    return particleMatch;
  }
  
}
