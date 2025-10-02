/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDALGS_MUONAEOTDECORATIONALG_H
#define MUONCOMBINEDALGS_MUONAEOTDECORATIONALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
/*
    Algorithm to decorate the alignment effect on track information on a muon track particle
*/

class MuonTrkAEOTDecorationAlg: public AthReentrantAlgorithm {
   public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        ~MuonTrkAEOTDecorationAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;
    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrkContainer{this, "TrackContainer",
                                                                        "Trks", "Track particles to use for the decoration"};

        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_chIdKey{this, "ChambedIds", m_TrkContainer, 
                                                                        "alignEffectChId", "alignEffectChId decoration key"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_deltaTransKey{this, "deltaTrans", m_TrkContainer,
                                                                              "alignEffectDeltaTrans", "alignEffectDeltaTrans decoration"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_sigmaDeltaTransKey{this, "sigmaDeltaTrans",m_TrkContainer,
                                                                                   "alignEffectSigmaDeltaTrans", "alignEffectSigmaDeltaTrans decoration key"};
        
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_deltaAngleKey{this, "deltaAngle", m_TrkContainer,
                                                                              "alignEffectDeltaAngle" , "alignEffectDeltaAngle decoration"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_sigmaDeltaAngleKey{this, "sigmaDeltaAngle", m_TrkContainer, 
                                                                                   "alignEffectSigmaDeltaAngle" , "alignEffectSigmaDeltaAngle decoration"};

};
#endif