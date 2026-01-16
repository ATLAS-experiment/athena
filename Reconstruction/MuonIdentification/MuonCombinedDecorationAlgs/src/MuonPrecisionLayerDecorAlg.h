/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDDECORATIONALGS_MONPRECISIONLAYERDECORALG_H
#define MUONCOMBINEDDECORATIONALGS_MONPRECISIONLAYERDECORALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODMuon/MuonContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"

/*
    Algorithm to decorate the muon precision layers to the muon
*/

class MuonPrecisionLayerDecorAlg: public AthReentrantAlgorithm {
   public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        ~MuonPrecisionLayerDecorAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;
    private:
        
        SG::ReadHandleKey<xAOD::MuonContainer> m_MuonContainer{this, "MuonContainer",
                                                                "Muons", "Muon container for AEOT decoration"};

        SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_TrackContainer{this, "TrackContainer",
                                                                              {"ExtrapolatedMuonTrackParticles", 
                                                                               "CombinedMuonTrackParticles", 
                                                                               "MSOnlyExtrapolatedMuonTrackParticles"},
                                                              "List of track containers serving as primary tracks"};
        SG::ReadDecorHandleKeyArray<xAOD::TrackParticleContainer> m_trkAlignReadKey{this, "ReadChambedIds", {},
                                        "Use the alignEffectChId decoration as hook to schedule this alg correctly." };

        SG::WriteDecorHandleKey<xAOD::MuonContainer> m_goodPrecLayerKey{this, "GoodPrecisionLayers", m_MuonContainer,
                                                                        "numberOfGoodPrecisionLayers" , "numberOfGoodPrecisionLayers decoration"};
        SG::WriteDecorHandleKey<xAOD::MuonContainer> m_isGoodSmallKey{this, "SmallGoodSectors", m_MuonContainer, 
                                                                      "isSmallGoodSectors" , "isSmallGoodSectors decoration"};
        SG::WriteDecorHandleKey<xAOD::MuonContainer> m_isEndcapGoodLayersKey{this, "EndCapGoodLayers", m_MuonContainer,
                                                                            "isEndcapGoodLayers" , "isEndcapGoodLayers decoration"};
   
};
#endif