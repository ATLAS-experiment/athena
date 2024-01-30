/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGING_JETSECVTXFINDINGALG_H
#define BTAGGING_JETSECVTXFINDINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include <string>

//general interface for secondary vertex finders
#include "InDetRecToolInterfaces/ISecVertexInJetFinder.h"

#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "VxSecVertex/VxSecVertexInfo.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

/** The namespace of all packages in PhysicsAnalysis/JetTagging */
namespace Analysis
{

  class JetSecVtxFindingAlg : public AthReentrantAlgorithm
  {
      public:
        /** Constructors and destructors */
        JetSecVtxFindingAlg(const std::string& name, ISvcLocator *pSvcLocator);
        virtual ~JetSecVtxFindingAlg() = default;
    
        /** Main routines specific to an ATHENA algorithm */
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext& ctx) const override final;

      private:
        
        ToolHandle< InDet::ISecVertexInJetFinder > m_secVertexFinderToolHandle;

        SG::ReadHandleKey<xAOD::JetContainer > m_JetCollectionName {this, "JetCollectionName", "", "Input jet container"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_TracksToTag { this, "TracksToTag", "", "Element Link vector from jet to IParticleContainer"};
        SG::ReadHandleKey<xAOD::VertexContainer> m_VertexCollectionName {this, "vxPrimaryCollectionName", "", "Input primary vertex container"};
        SG::WriteHandleKey<Trk::VxSecVertexInfoContainer> m_VxSecVertexInfoName {this, "BTagVxSecVertexInfoName", "", "Output VxSecVertexInfo container"};

  }; // End class

} // End namespace

#endif // JETSECVTXFINDINGALG_H
