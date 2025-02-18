/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackToTruthPartAssocAlg.h"

#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include <unordered_set>
namespace {
    using IdSet_t = std::unordered_set<Identifier>;
    using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
    using TruthPartWithIds_t = std::tuple<const xAOD::TruthParticle*, IdSet_t>;
    using IdDecorHandle_t = SG::ReadDecorHandle<xAOD::TruthParticleContainer, std::vector<unsigned long long>>;
}

namespace MuonR4{
    StatusCode TrackToTruthPartAssocAlg::initialize() {
        
        
        ATH_CHECK(m_trkKey.initialize());
        ATH_CHECK(m_originWriteKey.initialize());
        ATH_CHECK(m_typeWriteKey.initialize());
        ATH_CHECK(m_linkWriteKey.initialize());
        ATH_CHECK(m_truthMuonKey.initialize());

        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());

        for (const std::string& hitIds  : m_simHitIds) {
            m_simHitKeys.emplace_back(m_truthMuonKey, hitIds);
        }
        ATH_CHECK(m_simHitKeys.initialize());
        ATH_CHECK(m_truMuOriginKey.initialize());
        ATH_CHECK(m_truMuTypeKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode TrackToTruthPartAssocAlg::execute(const EventContext& ctx) const {
        SG::ReadHandle tracks{m_trkKey, ctx};
        ATH_CHECK(tracks.isPresent());


        SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> acc_truthOrigin{m_originWriteKey, ctx};
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> acc_truthType{m_typeWriteKey, ctx};
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, TruthLink_t> acc_truthLink{m_linkWriteKey, ctx};
        ///
        /// Initialize the Identifier decorators
        std::vector<IdDecorHandle_t> idDecorHandles{};
        for (const SG::ReadDecorHandleKey<xAOD::TruthParticleContainer>& hitKey : m_simHitKeys) {
            idDecorHandles.emplace_back(hitKey, ctx);
        }
        
        std::vector<TruthPartWithIds_t> truthWithIds{};
        ///
        SG::ReadHandle truthMuonCont{m_truthMuonKey, ctx};
        ATH_CHECK(truthMuonCont.isPresent());
        for (const xAOD::TruthParticle* truthMuon : *truthMuonCont) {
            IdSet_t assocIds{};
            ATH_MSG_DEBUG("Truth muon: pT:"<<truthMuon->pt()<<" [GeV], eta: "<<truthMuon->eta()<<", phi: "
                        <<truthMuon->phi()<<", q: "<<truthMuon->charge()<<", truthType: "<<xAOD::TruthHelpers::getParticleTruthType(*truthMuon)
                        <<", origin: "<<xAOD::TruthHelpers::getParticleTruthOrigin(*truthMuon));
            for (const IdDecorHandle_t& hitDecor : idDecorHandles) {
                std::ranges::transform(hitDecor(*truthMuon), std::inserter(assocIds, assocIds.begin()),
                                [this](unsigned long long rawId) {
                                    const Identifier id{rawId};
                                    ATH_MSG_VERBOSE(" --- associated hit id: "<<m_idHelperSvc->toString(id));
                                    return id; 
                                });
            }
            truthWithIds.emplace_back(std::make_tuple(truthMuon, std::move(assocIds)));
        }

        for (const xAOD::TrackParticle* trackPart : *tracks){
            const Trk::Track* track = trackPart->track();
            if (!track) {
                ATH_MSG_ERROR("Associated reconstructed track is not available for "<<m_trkKey);
                return StatusCode::FAILURE;
            }
            /// Assemble the track identifiers
            IdSet_t trackIds{};
            for (const Trk::TrackStateOnSurface* tsos : *track->trackStateOnSurfaces()) {
                const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
                /// Probably scatterer
                if (!meas) {
                    continue;
                }
                const Identifier measId = m_edmHelperSvc->getIdentifier(*meas);
                if (!measId.is_valid() || !m_idHelperSvc->isMuon(measId)){
                    ATH_MSG_VERBOSE("Measurement is not a muon one");
                    continue;
                }
                trackIds.insert(measId);
            }
            /// Start matching by finding the particle with the largest fraction
            int bestMatchFrac{-1};
            const xAOD::TruthParticle* bestMatch{nullptr};
            for (const auto& [truthPart, truthIds]: truthWithIds) {
                const int matchedReco = std::ranges::count_if(trackIds,[&truthIds](const Identifier& recoId){
                                                                return truthIds.count(recoId);
                                                            });
                ///  If the particle has no matched hits or it's worse than the best matched
                if (!matchedReco || matchedReco < bestMatchFrac){
                    continue;
                }
                bestMatchFrac = matchedReco;
                bestMatch = truthPart;
            }
            /// Track could not be matched
            if (!bestMatch) {
                continue;
            }
            acc_truthOrigin(*trackPart) = xAOD::TruthHelpers::getParticleTruthOrigin(*bestMatch);
            acc_truthType(*trackPart) = xAOD::TruthHelpers::getParticleTruthType(*bestMatch);
            const auto bestMatchCont = static_cast<const xAOD::TruthParticleContainer*>(bestMatch->container());
            acc_truthLink(*trackPart) = TruthLink_t{bestMatchCont, bestMatch->index()};
        }
        return StatusCode::SUCCESS;
    }
}