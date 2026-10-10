/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGING_JETTAGVERTEXDECORATORALG_H
#define BTAGGING_JETTAGVERTEXDECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthLinks/ElementLink.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include <string>

#include "xAODJet/JetContainer.h"
#include "VxSecVertex/VxSecVertexInfo.h"
//#include "xAODBTagging/BTaggingContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"


namespace InDet {
  class ISecVertexInJetFinder;
}

namespace Trk{

  class VxSecVKalVertexInfo;

}

namespace Analysis {


}

namespace Analysis {

  class ITaggerDecorHandles {
    public:
      virtual ~ITaggerDecorHandles() = default;
  };
  
  class SV1DecorHandles : public ITaggerDecorHandles {
  public:
    SG::WriteDecorHandleKey<xAOD::JetContainer> massKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> efracKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> energyTrkInJetKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> dstToMatLayKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> n2trkKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> NGTinSvxKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> L3dKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> LxyKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> deltaRKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> isDefaultsKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> normdistKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> significance3dKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> correctSignificance3dKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> trackLinksKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> badTracksIPKey;
    SG::WriteDecorHandleKey<xAOD::JetContainer> verticesKey;

    template <class OWNER>
    SV1DecorHandles(OWNER* owner, const std::string& prefix)
      : massKey(owner, prefix + "_masssvx", prefix + "_masssvx" , "SV1: Mass decoration key"),
        efracKey(owner, prefix + "_efracsvx", prefix + "_efracsvx", "SV1: Efrac decoration key"),
        energyTrkInJetKey(owner, prefix + "_energyTrkInJet", prefix + "_energyTrkInJet", "SV1: EnergyTrkInJet decoration key"),
        dstToMatLayKey(owner, prefix + "_dstToMatLay", prefix + "_dstToMatLay", "SV1: DstToMatLay decoration key"),
        n2trkKey(owner, prefix + "_N2Tpair", prefix + "_N2Tpair", "SV1: N2Tpair decoration key"),
        NGTinSvxKey(owner, prefix + "_NGTinSvx", prefix + "_NGTinSvx", "SV1: NGTinSvx decoration key"),
        L3dKey(owner, prefix + "_L3d", prefix + "_L3d", "SV1: L3d decoration key"),
        LxyKey(owner, prefix + "_Lxy", prefix + "_Lxy", "SV1: Lxy decoration key"),
        deltaRKey(owner, prefix + "_deltaR", prefix + "_deltaR", "SV1: deltaR decoration key"),
        isDefaultsKey(owner, prefix + "_isDefaults", prefix + "_isDefaults", "SV1: isDefaults decoration key"),
        normdistKey(owner, prefix + "_normdist", prefix + "_normdist", "SV1: normdist decoration key"),
        significance3dKey(owner, prefix + "_significance3d", prefix + "_significance3d", "SV1: significance3d decoration key"),
        correctSignificance3dKey(owner, prefix + "_correctSignificance3d", prefix + "_correctSignificance3d", "SV1: correctSignificance3d decoration key"),
        trackLinksKey(owner, prefix + "_TrackParticleLinks", prefix + "_TrackParticleLinks", "SV1: TrackLinks decoration key"),
        badTracksIPKey(owner, prefix + "_badTracksIP", prefix + "_badTracksIP", "SV1: badTracksIP decoration key"),
        verticesKey(owner, prefix + "_vertices", prefix + "_vertices", "SV1: vertices decoration key")
    {}

  };


}

namespace Analysis {

  class JetTagVertexDecoratorAlg : public AthReentrantAlgorithm
  {
      public:

        /** Constructors and destructors */
        JetTagVertexDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~JetTagVertexDecoratorAlg() = default;
    
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

      private:

        std::vector<std::string> m_secVertexFinderBaseNameList;

        std::map<std::string, std::unique_ptr<ITaggerDecorHandles>> m_decorKeys;

        SG::ReadHandleKey<xAOD::JetContainer > m_JetCollectionName {this, "JetCollectionName", "", "Input jet container"};

        SG::ReadHandleKeyArray<Trk::VxSecVertexInfoContainer> m_VxSecVertexInfoNames {this, "BTagVxSecVertexInfoNames", {""}, "Input VxSecVertexInfo containers"};
        SG::ReadHandleKey<xAOD::VertexContainer> m_VertexCollectionName {this, "vxPrimaryCollectionName", "", "Input primary vertex container"};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackCollectionName {this, "TrackCollectionName", "InDetTrackParticles", "Input track container for the track links"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetSVLinkName{ this, "JetSecVtxLinkName", "", "Element Link vector form jet to SV container"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetSVFlipLinkName{ this, "JetSecVtxFlipLinkName", "", "Element Link vector form jet to SVFlip container"};


  }; 

} 

#endif 
