/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//
// TCTDecorCheck.h - Description
//
/*
   Test algorithm to check that TrackParticle decorations regarding the TCT are correctly saved
   Before running TCTDecorCheck the decorations have to be applied through the InDetVKalVxInJetTool/TrackClassificationDecorator algorithm

    Author: Katharina Voss
    e-mail: katharina.voss@cern.ch
*/
#ifndef ANALYSISEXAMPLES_TCTDecorCheckInTool_H
#define ANALYSISEXAMPLES_TCTDecorCheckInTool_H

#include <vector>
#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
//#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/ReadDecorHandle.h"
 
#include "xAODJet/JetContainer.h" 
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "InDetVKalVxInJetTool/InDetTrkInJetType.h"



class TLorentzVector;


  class TCTDecorCheckInTool : public AthAlgorithm
  {
   public:
       /* Constructor */
      TCTDecorCheckInTool(const std::string& type, ISvcLocator* pSvcLocator);
       /* Destructor */
      virtual ~TCTDecorCheckInTool();


      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;
      virtual StatusCode finalize() override;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

   private:

      // get handle to TCT Tool
    ToolHandle<InDet::IInDetTrkInJetType> m_trackClassificationTool
    {this, "TrackClassificationTool", "InDet::InDetTrkInJetType"};

      // ReadHandle for the jets
      SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{this,"JetContainer","AntiKt4EMPFlowJets","ReadHandleKey for Jet Container"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_particlesKey{this, "trackCollection", "InDetTrackParticles"};
      SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey { this, "VertexContainer", "PrimaryVertices"};  
      
      /** The read key for adding TCT score as decoration to TrackParticle objects */
      //from https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Event/xAOD/xAODTrackingCnv/src/TrackParticleCnvAlg.cxx
      SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackReadDecorKeyTCTScore{this,"trackDecorKeyTCTScore",
      "InDetTrackParticles.TCTScore_AntiKt4EMPFlowJets","ReadDecorHandleKey for adding TCT score to TrackParticles"};
      SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackReadDecorKeyJetLink{this,"trackDecorKeyJetLink",
      "InDetTrackParticles.TCTJetLink_AntiKt4EMPFlowJets","ReadDecorHandleKey for adding JetLink to TrackParticles"};

      /** The write key for adding TCT score as decoration to Jet objects */
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetReadDecorKeyTCTScore{this,"jetDecorKeyTCTScore",
      "AntiKt4EMPFlowJets.TCTScore","ReadDecorHandleKey for adding TCT score to Jets"};
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetReadDecorKeyTrackLink{this,"jetDecorKeyJetLink",
      "AntiKt4EMPFlowJets.TCTTrackLink","ReadDecorHandleKey for adding TrackParticleLink to Jets"};

      StringProperty m_decoratorMethod{this, "decoratorMethod", "decorateTrack"};

 };


#endif
