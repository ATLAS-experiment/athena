/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//
// CTTDecorCheck.h - Description
//
/*
   Test algorithm to check that Jet decorations regarding the CTT are correctly saved
   Before running CTTDecorCheck the decorations have to be applied through the JetTagTools/ClassifiedTrackTaggerDecorator algorithm

    Author: Katharina Voss
    e-mail: katharina.voss@cern.ch
*/
#ifndef ANALYSISEXAMPLES_CTTDecorCheckInTool_H
#define ANALYSISEXAMPLES_CTTDecorCheckInTool_H

#include <vector>
#include "AthenaBaseComps/AthAlgorithm.h"
//#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"

 
#include "xAODJet/JetContainer.h" 
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "JetTagTools/ClassifiedTrackTaggerTool.h"



class TLorentzVector;



  class CTTDecorCheckInTool : public AthAlgorithm
  {
   public:
       /* Constructor */
      CTTDecorCheckInTool(const std::string& type, ISvcLocator* pSvcLocator);
       /* Destructor */
      virtual ~CTTDecorCheckInTool();


      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;
      virtual StatusCode finalize() override;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

   private:

      // get handle to new CTT Tool
      ToolHandle<Analysis::IClassifiedTrackTaggerTool>    m_classifiedTrackTagger;
      std::string m_jetCollection;

      // ReadHandle for the jets
      SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{this,"JetContainer","AntiKt4EMPFlowJets","ReadHandleKey for Jet Container"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_particlesKey{this, "trackCollection", "InDetTrackParticles"};
      SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey { this, "VertexContainer", "PrimaryVertices"};  
    
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetReadDecorKey{this,"JetDecorKey","AntiKt4EMPFlowJets.CTTScore","ReadDecorHandleKey for adding CTT score to Jets"};
    

 };




#endif
