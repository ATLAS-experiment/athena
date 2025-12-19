/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthSegToTruthPartAssocAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthLinks/ElementLink.h"
#include "Identifier/Identifier.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthVertex.h"
#include "ActsInterop/UnitConverters.h"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include <unordered_set>


using namespace Acts::UnitLiterals;
using namespace MuonR4::SegmentFit;
namespace {
    using IdSet_t = std::unordered_set<Identifier>;
    unsigned int countMatched(const std::unordered_set<const xAOD::MuonSimHit*>& simHits,
                              const IdSet_t& matchIds) {
        return std::ranges::count_if(simHits, [&matchIds](const xAOD::MuonSimHit* hit) {
                                return matchIds.count(hit->identify());
                            });
    }

    Acts::Vector4 vertexPos(const xAOD::TruthParticle& truthPart) {
        if (truthPart.hasProdVtx()){
            const xAOD::TruthVertex* vtx = truthPart.prodVtx();
            return ActsTrk::convertPosToActs(Amg::Vector3D{vtx->x(), vtx->y(), vtx->z()}, vtx->t());
        }
        return Acts::Vector4::Zero();
    }

    static const SG::ConstAccessor<float> acc_q{"charge"};
    static const SG::ConstAccessor<float> acc_pt{"pt"};
}
namespace MuonR4{
    StatusCode TruthSegToTruthPartAssocAlg::initialize() {
        ATH_CHECK(m_truthKey.initialize());
        for (const std::string& hitIds  : m_simHitIds) {
            m_simHitKeys.emplace_back(m_truthKey, hitIds);
        }
        ATH_CHECK(m_simHitKeys.initialize());
        ATH_CHECK(m_segLinkKey.initialize());
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackingGeometryTool.retrieve(EnableTool{m_includePileUpObjs}));
        ATH_CHECK(m_extrapolationTool.retrieve(EnableTool{m_includePileUpObjs}));
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegToTruthPartAssocAlg::execute(const EventContext& ctx) const {

        const xAOD::TruthParticleContainer* truthParticles{nullptr};
        ATH_CHECK(SG::get(truthParticles, m_truthKey, ctx));

        using IdDecorHandle_t = SG::ReadDecorHandle<xAOD::TruthParticleContainer, std::vector<unsigned long long>>;
        using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
        using SegLinkVec_t = std::vector<SegLink_t>;
        SG::WriteDecorHandle<xAOD::TruthParticleContainer, SegLinkVec_t> segLinkDecor{m_segLinkKey ,ctx};

        /// Initialize the Identifier decorators
        std::vector<IdDecorHandle_t> idDecorHandles{};
        for (const SG::ReadDecorHandleKey<xAOD::TruthParticleContainer>& hitKey : m_simHitKeys) {
            idDecorHandles.emplace_back(hitKey, ctx);
        }
        /// Setup a vector with the truth particles and the associated hits
        using IdSet_t = std::unordered_set<Identifier>;
        using TruthTuple_t = std::tuple<const xAOD::TruthParticle*, IdSet_t>;
        std::vector<TruthTuple_t> truthPartWithIds{};
        /// List of muons from the pile-up overlay
        std::vector<const xAOD::TruthParticle*> bkgMuons{};
        truthPartWithIds.reserve(truthParticles->size());
        for (const xAOD::TruthParticle* truthMuon : *truthParticles){
            segLinkDecor(*truthMuon).clear();
            IdSet_t assocIds{};
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Truth muon "<<truthMuon->pt()<<", eta: "<<truthMuon->eta()
                        <<", phi: "<<(truthMuon->phi() / 1._degree)<<", id: "<< HepMC::uniqueID(truthMuon));
            for (const IdDecorHandle_t& hitDecor : idDecorHandles) {
                std::ranges::transform(hitDecor(*truthMuon), std::inserter(assocIds, assocIds.begin()),
                                       [this](unsigned long long rawId){
                                           const Identifier id{rawId};
                                           ATH_MSG_VERBOSE(" --- associated hit id: "<<m_idHelperSvc->toString(id));
                                           return id; 
                                        });
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Truth muon with pt: "<<(truthMuon->pt() / 1_GeV)
                         <<", eta: "<<truthMuon->eta()<<", phi: "<<truthMuon->phi() 
                         <<", uniqueID: "<<HepMC::uniqueID(truthMuon)<<", associated hits: "<<assocIds.size());
            if (assocIds.empty()) {
                bkgMuons.push_back(truthMuon);
            } else {
                truthPartWithIds.emplace_back(std::make_tuple(truthMuon, std::move(assocIds)));
            }
        }
        /// Fetch the segment container
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
        
        /// Setup the write decorators
        TruthPartDecor_t truthLinkDecor{m_truthLinkKey, ctx};
        /// List of segments from the pile-up overlay.
        std::vector<const xAOD::MuonSegment*> bkgSegments{};

        for (const xAOD::MuonSegment* segment : *segments){
            std::unordered_set<const xAOD::MuonSimHit*> simHits = getMatchingSimHits(*segment);
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Reconstructed truth segment "
                          <<SegmentFit::toString(SegmentFit::localSegmentPars(*segment))
                          <<", chamberId: "<<Muon::MuonStationIndex::chName(segment->chamberIndex())
                          <<", phi: "<<segment->sector()<<", nPrecHits: "<<segment->nPrecisionHits()
                          <<", nDoF: "<<segment->numberDoF()<<" sim hits: "<<simHits.size());
            if (msgLvl(MSG::VERBOSE)){
                std::vector<const xAOD::MuonSimHit*> sortedHits{simHits.begin(), simHits.end()};
                std::ranges::sort(sortedHits, [](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                    return a->identify() < b->identify();
                });
                for (const xAOD::MuonSimHit* hit: sortedHits) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Associated sim hit: "<<m_idHelperSvc->toString(hit->identify())
                           <<", locPos: "<<Amg::toString(xAOD::toEigen(hit->localPosition()))
                           <<", locDir: "<<Amg::toString(xAOD::toEigen(hit->localDirection())) 
                           <<", "<<hit->genParticleLink());
                }
            }
            /// Check if the segment does not have a valid gen particle link
            if (!(*simHits.begin())->genParticleLink().isValid()) {
                bkgSegments.push_back(segment);
                truthLinkDecor(*segment) = TruthPartLink_t{};
                continue;
            }
            /* now find the truth particle with all associated hits */
            const auto best_itr = std::ranges::max_element(truthPartWithIds, 
                                                           [&simHits](const TruthTuple_t& truthTupleA,
                                                                      const TruthTuple_t& truthTupleB) {
                return countMatched(simHits, std::get<1>(truthTupleA)) < 
                       countMatched(simHits, std::get<1>(truthTupleB));
            });
            if (best_itr == truthPartWithIds.end()) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No truth particle matched the truth hits of the segment");
                continue;
            }
            if (1.*countMatched(simHits, std::get<1>(*best_itr)) < 0.5* simHits.size()) {
                if (msgLvl(MSG::VERBOSE)) {
                    for (const auto& [truthMuon, assocIds]: truthPartWithIds){
                        std::stringstream unMatchedStr{};
                        unsigned int counts{0};
                        for (const xAOD::MuonSimHit* hit: simHits) {
                            if (!assocIds.count(hit->identify())){
                                unMatchedStr<<" *** "<<m_idHelperSvc->toString(hit->identify())<<std::endl;
                            } else {
                                ++counts;
                            }
                        }
                        if (!counts) {
                            continue;
                        }
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Truth muon "<<truthMuon->pt()<<", eta: "<<truthMuon->eta()
                            <<", "<<truthMuon->phi()<<", barcode: "<<HepMC::uniqueID(truthMuon)<<", matched hits: "
                            <<counts<<", unmatched: "<<std::endl<<unMatchedStr.str());
                    }
                }
                continue;
            }
            const xAOD::TruthParticle* truthPart{std::get<0>(*best_itr)};
            segLinkDecor(*truthPart).emplace_back(segments, segment->index());
            truthLinkDecor(*segment) = TruthPartLink_t{truthParticles, truthPart->index()};
        
        }
        /// Finally sort the segments along the trajectory
        for (const xAOD::TruthParticle* truthMuon : *truthParticles){
            const Amg::Vector3D dir = Amg::Vector3D{truthMuon->px(), truthMuon->py(), truthMuon->pz()}.normalized();
            const xAOD::TruthVertex* vtx{truthMuon->prodVtx()};
            const Amg::Vector3D pos = (vtx? Amg::Vector3D{vtx->x(), vtx->y(), vtx->z()} : Amg::Vector3D::Zero());
            SegLinkVec_t& linkedSegs{segLinkDecor(*truthMuon)};
            std::ranges::sort(linkedSegs,[&pos, &dir](const SegLink_t& linkA, const SegLink_t& linkB){
                                            return dir.dot((*linkA)->position() - pos) <
                                                   dir.dot((*linkB)->position() - pos);
                                        });
        }
        return StatusCode::SUCCESS;
    }
    void TruthSegToTruthPartAssocAlg::matchPileupSegments(const EventContext& ctx,
                                                          const std::vector<const xAOD::TruthParticle*>& pileUpMuons,
                                                          const std::vector<const xAOD::MuonSegment*>& pileUpSegments,
                                                          TruthPartDecor_t& truthPartDecor,
                                                          TruthSegLinkDecor_t& truthSegDecor) const {
        
        if (!m_includePileUpObjs) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Matching between pile-up segments & muons is disabled");
            return;
        }
        if (pileUpMuons.empty() || pileUpSegments.empty()) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Segments ("<<pileUpSegments.size()<<") or muons ("
                         <<pileUpMuons.size()<<") are empty -> nothing to do");
            return;
        }
        if (msgLvl(MSG::DEBUG)) {
            std::stringstream sstr{};
            for (const xAOD::TruthParticle* bkgMuon : pileUpMuons) {
                sstr<<" *** pT: "<<(bkgMuon->pt() / 1_GeV)<<", eta: "<<bkgMuon->eta()
                    <<", phi: "<<(bkgMuon->phi() / 1_degree)<<", pdgId: "<<bkgMuon->pdgId()
                    <<", pos: "<<Amg::toString(vertexPos(*bkgMuon))
                    <<", uid: "<<bkgMuon->uid() <<std::endl;
            }
            sstr<<"\n To these segments: "<<std::endl;
            unsigned int counter{0};
            for (const xAOD::MuonSegment* bkgSeg : pileUpSegments) {
                sstr<<"   "<<(counter++)<<") "<<Amg::toString(bkgSeg->position())
                    <<", phi: "<<(bkgSeg->position().phi() / 1_degree)<<", eta: "<<(bkgSeg->position().eta())
                    <<", pT: "<< (acc_pt(*bkgSeg) / 1_GeV)
                    <<" chamberId: "<<Muon::MuonStationIndex::chName(bkgSeg->chamberIndex())
                    <<", sector: "<<bkgSeg->sector()<<", nPrecHits: "<<bkgSeg->nPrecisionHits()
                    <<", chi2: "<<(bkgSeg->chiSquared() / std::max(bkgSeg->numberDoF(), 1.f))
                    <<", nDoF: "<<bkgSeg->numberDoF()<<std::endl;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - \nMatch the following muons "<<std::endl<<sstr.str());
        }
        std::vector<char> segmentMatched(pileUpSegments.size(), 0);

        const ActsTrk::GeometryContext& gctx{m_trackingGeometryTool->getGeometryContext(ctx)};
        const Acts::GeometryContext tgContext = gctx.context();

        for (const xAOD::TruthParticle* bkgMuon : pileUpMuons) {
            const Acts::Vector4 fourPos = vertexPos(*bkgMuon);
            const Acts::Vector4 fourMom = ActsTrk::convertMomToActs(Amg::Vector3D{bkgMuon->px(), bkgMuon->py(), bkgMuon->pz()},
                                                                    bkgMuon->m());
            const Amg::Vector3D threeMom = fourMom.block<3,1>(0,0);
            const Amg::Vector3D start = ActsTrk::convertPosFromActs(fourPos).first;
            auto startSurf = Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::getTranslate3D(start));

        
            Acts::BoundMatrix initialCov{Acts::BoundMatrix::Identity()};

            auto initialPars = Acts::BoundTrackParameters::create(tgContext, startSurf, fourPos, threeMom.unit(), 
                                                                   bkgMuon->charge() / threeMom.mag(),
                                                                   initialCov, Acts::ParticleHypothesis::muon());
            if(!initialPars.ok()) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to create start parameters");
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Created new start parameters "<<Amg::toString(fourPos)<<", "
                        <<Amg::toString(threeMom) <<", pT: "<<(threeMom.perp() / 1_GeV)
                        <<" GeV, eta: "<<threeMom.eta()<<", phi: "<<(threeMom.phi() / 1_degree)<<", q: "<<bkgMuon->charge());
            
            for (std::size_t sIdx =0 ; sIdx < pileUpSegments.size(); ++sIdx) {
                // Skip already matched segments
                if (segmentMatched[sIdx]) {
                    continue;
                }
                const xAOD::MuonSegment* bkgSeg = pileUpSegments[sIdx];
                if (acc_q(*bkgSeg) != bkgMuon->charge()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment charge does not match");
                    continue;
                }
                const Amg::Vector3D segPos = bkgSeg->position();
                const Amg::Vector3D segDir = bkgSeg->direction();
                /// First check on the distance of closest approach
                const double lDist = std::abs(Amg::lineDistance(segPos, segDir, start, threeMom.unit()));
                const double dPhi = std::abs(segPos.deltaPhi(threeMom));
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment "<<sIdx<<", "<<Amg::toString(segPos)
                            <<", "<<Amg::toString(segDir) <<", lDist: "<<lDist<<", dPhi: "<<(dPhi / 1_degree));
                if (dPhi > m_pileUpObjDPhiCut) {
                    continue;
                }
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Start extrapolation");
                const auto* sector = m_detMgr->getSectorEnvelope(bkgSeg->chamberIndex(), bkgSeg->sector(), bkgSeg->etaIndex());
                auto propPars = m_extrapolationTool->propagate(ctx, *initialPars, sector->surface());
                if (!propPars) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolation failed.");
                    continue;
                }
                ///
                const Amg::Vector2D dPosExtp = propPars->localPosition() - (sector->globalToLocalTrans(gctx) * segPos).segment<2>(0);
                const double dThetaExtp = std::abs(segDir.theta() - propPars->theta());
                const double dPhiExtp  = std::abs(segDir.phi() - propPars->phi());
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Parameter difference: "<<Amg::toString(dPosExtp)
                            <<", dTheta:"<<(dThetaExtp / 1._degree)<<", dPhi: "<<(dPhiExtp / 1._degree));      
                

                if (std::abs(dPosExtp.x()) > m_pileUpObjExtpDxCut || std::abs(dPosExtp.y()) >  m_pileUpObjExtpDyCut ||
                    dThetaExtp > m_pileUpObjExtpDthetaCut || dPhiExtp >  m_pileUpObjExtpDphiCut) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Parameters differ too much. Cut values"
                        <<m_pileUpObjExtpDxCut<<", "<<m_pileUpObjExtpDyCut<<", "<<m_pileUpObjExtpDthetaCut<<", "<<m_pileUpObjExtpDphiCut);
                    continue;
                }
                truthPartDecor(*bkgSeg) = TruthPartLink_t{truthSegDecor.cptr(), bkgMuon->index()};
                truthSegDecor(*bkgMuon).emplace_back(truthPartDecor.cptr(), bkgSeg->index());
                segmentMatched[sIdx] = true;
            }
        }
   }
}
