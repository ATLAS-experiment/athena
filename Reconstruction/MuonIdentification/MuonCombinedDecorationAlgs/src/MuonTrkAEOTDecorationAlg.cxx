/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrkAEOTDecorationAlg.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h" 

 
 StatusCode MuonTrkAEOTDecorationAlg::initialize() {

    ATH_CHECK(m_TrkContainer.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    
    ATH_CHECK(m_chIdKey.initialize());
    ATH_CHECK(m_deltaTransKey.initialize());
    ATH_CHECK(m_sigmaDeltaTransKey.initialize());
     
    ATH_CHECK(m_deltaAngleKey.initialize());
    ATH_CHECK(m_sigmaDeltaAngleKey.initialize());
     
     return StatusCode::SUCCESS;
 }

 StatusCode MuonTrkAEOTDecorationAlg::execute(const EventContext& ctx) const {
     SG::ReadHandle trkCont{m_TrkContainer,ctx};
     if (!trkCont.isValid()) {
         ATH_MSG_FATAL("Failed to load track collection "<<m_TrkContainer.fullKey());
         return StatusCode::FAILURE;
     }
     
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<std::vector<unsigned int>>> acc_ChId{m_chIdKey,ctx};
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> acc_DeltaTrans{m_deltaTransKey,ctx};
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> acc_SigmaDeltaTrans{m_sigmaDeltaTransKey, ctx};
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> acc_deltaAngle{m_deltaAngleKey, ctx};
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> acc_sigmaDeltaAngle{m_sigmaDeltaAngleKey, ctx};
     
    using namespace Muon::MuonStationIndex;
    for (const xAOD::TrackParticle* tp : *trkCont) {
     
        std::vector<std::vector<unsigned int>>& chId = acc_ChId(*tp);
        std::vector<float>& deltaTrans = acc_DeltaTrans(*tp);
        std::vector<float>& sigmaDeltaTrans = acc_SigmaDeltaTrans(*tp);
        std::vector<float>& deltaAngle = acc_deltaAngle(*tp);
        std::vector<float>& sigmaDeltaAngle = acc_sigmaDeltaAngle(*tp);

        const Trk::Track* trk = tp->track();
        if (!trk || !trk->trackStateOnSurfaces()){
            ATH_MSG_DEBUG("Track has no TSOS?");
            continue;
        }         
        for (const Trk::TrackStateOnSurface* tsos : *trk->trackStateOnSurfaces()) {
            if (!tsos->type(Trk::TrackStateOnSurface::Alignment)) continue;
            const Trk::AlignmentEffectsOnTrack* aeot = tsos->alignmentEffectsOnTrack();
            if (!aeot) continue;
            std::set<unsigned int> chIdSet{};
            for (const Identifier& id : aeot->vectorOfAffectedTSOS()) {
                if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) continue;
                chIdSet.insert(toInt(m_idHelperSvc->chamberIndex(id)));
            }
            std::vector<unsigned int> chIdVec{};
            chIdVec.insert(chIdVec.end(), chIdSet.begin(), chIdSet.end());
            chId.push_back(std::move(chIdVec));
            deltaTrans.push_back(aeot->deltaTranslation());
            sigmaDeltaTrans.push_back(aeot->sigmaDeltaTranslation());
            deltaAngle.push_back(aeot->deltaAngle());
            sigmaDeltaAngle.push_back(aeot->sigmaDeltaAngle());            
        }            
     }
     return StatusCode::SUCCESS;
 }
 