/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGING_BTAGLIGHTSECVERTEXING_H
#define BTAGGING_BTAGLIGHTSECVERTEXING_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "BTagging/IBTagLightSecVertexing.h"

#include <string>

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "JetTagTools/IJetFitterVariablesFactory.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

namespace Trk{

  class VxSecVKalVertexInfo;
  class VxJetFitterVertexInfo;

}


/** The namespace of all packages in PhysicsAnalysis/JetTagging */
namespace Analysis
{
  class BTagLightSecVertexing : public AthAlgTool, virtual public IBTagLightSecVertexing
  {
      public:
          /** Constructors and destructors */
          BTagLightSecVertexing(const std::string&,const std::string&,const IInterface*);
          virtual ~BTagLightSecVertexing() = default;

          virtual StatusCode initialize() override;
          virtual StatusCode BTagSecVertexing_exec(const xAOD::JetContainer * jetContainer, xAOD::BTaggingContainer * btaggingContainer) const override;

      private:
        
         StatusCode fillVkalVariables(const xAOD::Jet&, xAOD::BTagging*, const Trk::VxSecVKalVertexInfo*, const xAOD::TrackParticleContainer*,  const std::string&) const;
         StatusCode fillJFVariables(const xAOD::Jet&, xAOD::BTagging*, const Trk::VxJetFitterVertexInfo*, const xAOD::TrackParticleContainer*, const std::string&) const;

         ToolHandle<IJetFitterVariablesFactory> m_JFvarFactory;

         std::vector<std::string> m_secVertexFinderTrackNameList;
         std::vector<std::string> m_secVertexFinderBaseNameList;

         SG::ReadHandleKey<xAOD::VertexContainer> m_VertexCollectionName {this, "vxPrimaryCollectionName", "", "Input primary vertex container"};
         SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetSVLinkName{ this, "JetSecVtxLinkName", "", "Element Link vector form jet to SV container"};
	 SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetSVFlipLinkName{ this, "JetSecVtxFlipLinkName", "", "Element Link vector form jet to SVFlip container"};
         SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetJFVtxLinkName{ this, "JetJFVtxLinkName", "", "Element Link vector form jet to JF vertex"};
	 SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetJFFlipVtxLinkName{ this, "JetJFFlipVtxLinkName", "", "Element Link vector form jet to JF vertexFlip"};
         SG::ReadHandleKeyArray<Trk::VxSecVertexInfoContainer> m_VxSecVertexInfoNames {this, "BTagVxSecVertexInfoNames", {""}, "Input VxSecVertexInfo containers"};

  }; // End class

} // End namespace

#endif // BTAGLIGHTSECVERTEXING_H
