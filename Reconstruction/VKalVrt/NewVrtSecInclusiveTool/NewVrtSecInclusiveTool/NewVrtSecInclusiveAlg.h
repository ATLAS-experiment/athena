/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NewVrtSecInclusiveAlg.h, (c) ATLAS Detector software
// author: Vadim Kostyukhin (vadim.kostyukhin@cern.ch)
///////////////////////////////////////////////////////////////////

/**
* Example algorithm to run the NewVrtSecInclusive algorithm and save results to StoreGate
*/

#ifndef VKalVrt_NewVrtSecInclusiveAlg_H
#define VKalVrt_NewVrtSecInclusiveAlg_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "NewVrtSecInclusiveTool/IVrtInclusive.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODJet/JetContainer.h"


namespace Rec {

   class NewVrtSecInclusiveAlg : public AthReentrantAlgorithm {
     public: 

       NewVrtSecInclusiveAlg( const std::string& name, ISvcLocator* pSvcLocator );

       StatusCode initialize() override;
       StatusCode execute(const EventContext &ctx) const override;
       StatusCode finalize() override;

    private:

      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tpContainerKey{this,"TrackParticleContainer","InDetTrackParticles","Read TrackParticle container"};
      SG::ReadHandleKey<xAOD::VertexContainer>        m_pvContainerKey{this,"PrimaryVertexContainer","PrimaryVertices","Read PrimaryVertices container"};
      SG::ReadHandleKey<xAOD::JetContainer>           m_jetContainerKey{this,"JetContainer","AntiKt4EMPFlowJets","Read Jets container"};
      SG::ReadHandleKey<xAOD::VertexContainer>        m_btsvContainerKey{this,"BTagSVContainer","BTagging_AntiKt4EMPFlowSecVtx","Read BTagiingSV container"};

      SG::WriteHandleKey<xAOD::VertexContainer>  m_foundVerticesKey{this,"BVertexContainerName","AllBVertices","Found vertices container"};
      ToolHandle < Rec::IVrtInclusive >          m_bvertextool;
  };
}

#endif
