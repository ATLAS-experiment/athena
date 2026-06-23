/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthSegConnectionAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
using SegLinkVec_t = std::vector<SegLink_t>;

namespace{
    inline std::string print(const xAOD::MuonSegment& segment) {
        static const SG::ConstAccessor<float> acc_pt{"pt"};
        ServiceHandle<Muon::IMuonIdHelperSvc> idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonReadoutElement"};


        std::stringstream sstr{};
        sstr<<segment.chamberIndex()<<(segment.etaIndex() > 0 ? 'A' : 'C') << segment.sector()
            <<" @"<<Amg::toString(segment.position())<<",dir: "<<Amg::toString(segment.direction())
            <<", pT: "<<acc_pt(segment);
        auto hitSet = MuonR4::getMatchingSimHits(segment);
        sstr<<", hits: "<<hitSet.size()<<std::endl;
        std::vector<const xAOD::MuonSimHit*> hits{hitSet.begin(), hitSet.end()};
        std::ranges::sort(hits, [](const auto* a, const auto* b){ return a->identify() < b->identify(); });
        for (const xAOD::MuonSimHit* hit : hits){
            sstr<<"       --- "<<idHelperSvc->toString(hit->identify())<<", pdgId:"<<hit->pdgId()
                <<", e: "<<hit->kineticEnergy()<<", link: "<<hit->genParticleLink()<<std::endl;
        }
        return sstr.str();
    }
    inline std::string print(const std::vector<const xAOD::MuonSegment*>& segments) {
        std::stringstream sstr{};
        for (const xAOD::MuonSegment* seg : segments) {
            sstr<<" **** "<<print(*seg)<<std::endl;
        }
        return sstr.str();
    }
}

namespace MuonR4 {
    StatusCode TruthSegConnectionAlg::initialize() {
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_truthPartLinkKey.initialize());
        ATH_CHECK(m_connectKey.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode TruthSegConnectionAlg::execute(const EventContext& ctx) const {

        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_segmentKey, ctx));

        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegLinkVec_t> connectHandle{m_connectKey, ctx};

        std::map<int, std::vector<const xAOD::MuonSegment*>> bkgParticles{};
        for (const xAOD::MuonSegment* segment : *segments) {
            SegLinkVec_t& links{connectHandle(*segment)};

            const xAOD::TruthParticle* truth = getTruthMatchedParticle(*segment);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Try to associate other truth segments to "<<
                           print(*segment));
            if (truth == nullptr) {
                const auto simHits = getMatchingSimHits(*segment);
                ATH_CHECK(!simHits.empty());
                const int trackId = (*simHits.begin())->genParticleLink().id();
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - No associated truth particle found. Rely on track id:"<<trackId);
                bkgParticles[trackId].push_back(segment);
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Found truth particle with "<<truth->pt()<<", eta: "<<truth->eta()
                            <<", phi: "<<truth->phi()<<", charge: "<<truth->charge());
            for (const xAOD::MuonSegment* matched : getTruthSegments(*truth)) {
                if(matched == segment) {
                    continue;
                }
                ATH_CHECK(matched->container() == segments);
                links.emplace_back(*segments, matched->index());
            }
            ATH_MSG_VERBOSE("Associated "<<links.size()<<" other truth segments.");
        }
        /// background segment association
        for (const auto& [trackId, assocSegs] : bkgParticles){
            ATH_MSG_VERBOSE("Associate to id: "<<trackId<<" corresponding to "
                    <<assocSegs.size()<<" segments\n"<<print(assocSegs));
            for (const xAOD::MuonSegment* segment : assocSegs) {
                SegLinkVec_t& links{connectHandle(*segment)};
                for (const xAOD::MuonSegment* matched : assocSegs) {
                    if (matched == segment) {
                        continue;
                    }
                    ATH_CHECK(matched->container() == segments);
                    links.emplace_back(*segments, matched->index());
                }
            }
           
        }
        return StatusCode::SUCCESS;
    }
        
}
