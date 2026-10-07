/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "LegacyCaloTagAlg.h"

#include "StoreGate/WriteHandle.h"
#include "MuonCombinedEvent/CaloTag.h"

namespace MuonCombinedR4 {
    StatusCode LegacyCaloTagAlg::initialize() {
        ATH_CHECK(m_idTrkKey.initialize());
        ATH_CHECK(m_caloTagMap.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode LegacyCaloTagAlg::execute(const EventContext& ctx) const {
        
        const MuonR4::MuonTagContainer* inDetCandidates{nullptr};
        ATH_CHECK(SG::get(inDetCandidates, m_idTrkKey, ctx));

        SG::WriteHandle outContainer{m_writeKey, ctx};
        ATH_CHECK(outContainer.record(std::make_unique<MuonR4::MuonTagContainer>()));

        const MuonCombined::InDetCandidateToTagMap* caloTags{nullptr};
        ATH_CHECK(SG::get(caloTags, m_caloTagMap, ctx));
        for (const auto& [idCandiate, tag] : *caloTags){
            /** First ensure that the track has been selected by the R4 chain */
            if (std::ranges::none_of(inDetCandidates->stdcont(),
                                    [&idCandiate](const MuonR4::MuonTag* idTag) {
                    return idTag->idTrack() == &(idCandiate->indetTrackParticle());
                })) {
                continue;
            }
            const auto* legacyTag = dynamic_cast<const MuonCombined::CaloTag*>(tag.get());
            auto* caloTag = outContainer->push_back(std::make_unique<MuonR4::MuonTag>());
            caloTag->setIdTrack(&idCandiate->indetTrackParticle());
            caloTag->setAuthor(legacyTag->author());
            using ParamDef = MuonR4::MuonTag::ParamDef;
            caloTag->setParameter(ParamDef::CaloMuonIDTag, legacyTag->caloMuonIdTag());
            caloTag->setParameter(ParamDef::CaloMuonScore, static_cast<float>(legacyTag->caloMuonScore()));
        }
        return StatusCode::SUCCESS;
    }
}