/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "SpacePointMakerAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include <xAODMuonViews/ChamberViewer.h>
#include <thread>

namespace {
    inline std::vector<std::shared_ptr<unsigned>> matchCountVec(unsigned int n) {
        std::vector<std::shared_ptr<unsigned>> out{};
        out.reserve(n);
        for (unsigned int p = 0; p < n ;++p) {
            out.emplace_back(std::make_shared<unsigned>(0));
        }
        return out;
    }
}

namespace MuonR4 {
bool SpacePointMakerAlg::SpacePointStatistics::FieldKey::operator<(const FieldKey& other) const{
    if (techIdx != other.techIdx) {
        return static_cast<int>(techIdx) < static_cast<int>(other.techIdx);
    }
    if (stIdx != other.stIdx) {
        return static_cast<int>(stIdx) < static_cast<int>(other.stIdx);
    }
    return eta < other.eta;
}
unsigned int SpacePointMakerAlg::SpacePointStatistics::StatField::allHits() const {
    return measEta + measPhi + measEtaPhi;
}
SpacePointMakerAlg::SpacePointStatistics::SpacePointStatistics(const Muon::IMuonIdHelperSvc* idHelperSvc):
    m_idHelperSvc{idHelperSvc}{}

void SpacePointMakerAlg::SpacePointStatistics::addToStat(const std::vector<SpacePoint>& spacePoints){
    std::lock_guard guard{m_mutex};
    for (const SpacePoint& sp : spacePoints){
        FieldKey key{};
        key.stIdx = m_idHelperSvc->stationIndex(sp.identify());
        key.techIdx = m_idHelperSvc->technologyIndex(sp.identify());
        key.eta = m_idHelperSvc->stationEta(sp.identify());
        StatField & stats = m_map[key];
        if (sp.measuresEta() && sp.measuresPhi()) {
            ++stats.measEtaPhi;
        } else {
            stats.measEta += sp.measuresEta();
            stats.measPhi += sp.measuresPhi();
        }               
    }
}
void SpacePointMakerAlg::SpacePointStatistics::dumpStatisics(MsgStream& msg) const {
    using KeyVal = std::pair<FieldKey, StatField>; 
    std::vector<KeyVal> sortedstats{};
    sortedstats.reserve(m_map.size());
    /// Sort statistics from largest to smallest
    for (const auto & [key, stats] : m_map){
        sortedstats.emplace_back(std::make_pair(key, stats));
    }
    std::stable_sort(sortedstats.begin(), sortedstats.end(), [](const KeyVal& a, const KeyVal&b) {
        return a.second.allHits() > b.second.allHits();
    });
    msg<<MSG::ALWAYS<<"###########################################################################"<<endmsg;
    for (const auto & [key, stats] : sortedstats) {
        msg<<MSG::ALWAYS<<" "<<Muon::MuonStationIndex::technologyName(key.techIdx)
                        <<" "<<Muon::MuonStationIndex::stName(key.stIdx)
                        <<" "<<std::abs(key.eta)<<(key.eta < 0 ? "A" : "C")
                        <<" "<<std::setw(8)<<stats.measEtaPhi
                        <<" "<<std::setw(8)<<stats.measEta
                        <<" "<<std::setw(8)<<stats.measPhi<<endmsg;
    }
    msg<<MSG::ALWAYS<<"###########################################################################"<<endmsg;
    
}


SpacePointMakerAlg::SpacePointMakerAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator}{}


StatusCode SpacePointMakerAlg::finalize() {
    if (m_statCounter) {
        m_statCounter->dumpStatisics(msgStream());
    }
    return StatusCode::SUCCESS;
}
StatusCode SpacePointMakerAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_mdtKey.initialize(!m_mdtKey.empty()));
    ATH_CHECK(m_rpcKey.initialize(!m_rpcKey.empty()));
    ATH_CHECK(m_tgcKey.initialize(!m_tgcKey.empty()));
    ATH_CHECK(m_mmKey.initialize(!m_mmKey.empty()));
    ATH_CHECK(m_stgcKey.initialize(!m_stgcKey.empty()));
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    if (m_doStat) m_statCounter = std::make_unique<SpacePointStatistics>(m_idHelperSvc.get());
    return StatusCode::SUCCESS;
}

template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::TgcStrip*>& etaHits,
                                             const std::vector<const xAOD::TgcStrip*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::TgcReadoutElement* re = etaHits[0]->readoutElement();
        ATH_MSG_VERBOSE("Collected "<<etaHits.size()<<"/"<<phiHits.size()<<" hits in "<<m_idHelperSvc->toStringGasGap(etaHits[0]->identify()));
        return ((1.*etaHits.size()) / ((1.*re->numChannels(etaHits[0]->measurementHash())))) < m_maxOccTgcEta &&
               ((1.*phiHits.size()) / ((1.*re->numChannels(phiHits[0]->measurementHash())))) < m_maxOccTgcPhi;
    }
template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::RpcMeasurement*>& etaHits,
                                             const std::vector<const xAOD::RpcMeasurement*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::RpcReadoutElement* re = etaHits[0]->readoutElement();
        ATH_MSG_VERBOSE("Collected "<<etaHits.size()<<"/"<<phiHits.size()<<" hits in "<<m_idHelperSvc->toStringGasGap(etaHits[0]->identify()));
        return ((1.*etaHits.size()) / (1.*re->nEtaStrips())) < m_maxOccRpcEta &&
               ((1.*phiHits.size()) / (1.*re->nPhiStrips())) < m_maxOccRpcPhi;
    }

template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::sTgcMeasurement*>& etaHits,
                                             const std::vector<const xAOD::sTgcMeasurement*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::sTgcReadoutElement* re = etaHits[0]->readoutElement();
        return ((1.*etaHits.size()) / (1.*re->numStrips(etaHits[0]->gasGap()))) < m_maxOccStgcEta &&
               ((1.*phiHits.size()) / (1.*re->numWireGroups(phiHits[0]->gasGap()))) < m_maxOccStgcPhi;
    }

template <class ContType>
    StatusCode SpacePointMakerAlg::loadContainerAndSort(const EventContext& ctx,
                                                        const SG::ReadHandleKey<ContType>& key,
                                                        PreSortedSpacePointMap& fillContainer) const {
    if (key.empty()) {
        ATH_MSG_DEBUG("Key "<<typeid(ContType).name()<<" not set. Do not fill anything");
        return StatusCode::SUCCESS;
    }                          
    SG::ReadHandle readHandle{key, ctx};
    ATH_CHECK(readHandle.isPresent());
    if (readHandle->empty()){
        ATH_MSG_DEBUG("nothing to do"); 
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle gctx{m_geoCtxKey, ctx};
    ATH_CHECK(gctx.isPresent());
    
    using PrdType = typename ContType::const_value_type;
    using PrdVec = std::vector<PrdType>;
    xAOD::ChamberViewer viewer{*readHandle};
    do {

      SpacePointsPerChamber& pointsInChamb = fillContainer[viewer.at(0)->readoutElement()->msSector()];
      ATH_MSG_DEBUG("Fill space points for chamber "<<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
      if constexpr( std::is_same_v<ContType, xAOD::MdtDriftCircleContainer> ||
                    std::is_same_v<ContType, xAOD::MMClusterContainer>) {
 
            pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.capacity() + viewer.size());
            for (const PrdType prd: viewer) {
                ATH_MSG_VERBOSE("Create space point from "<<m_idHelperSvc->toString(prd->identify())
                              <<", hash: "<<prd->identifierHash());
                pointsInChamb.etaHits.emplace_back(*gctx, prd, nullptr);           
            }
       } else {
            /// Loop over the chamber hits to split the hits per gasGap
            using EtaPhiHits = std::array<PrdVec, 2>;
            std::vector<EtaPhiHits> hitsPerGasGap{};
            for (const PrdType prd : viewer) {
                ATH_MSG_VERBOSE("Create space point from "<<m_idHelperSvc->toString(prd->identify())<<", hash: "<<prd->identifierHash());
                
                unsigned int gapIdx = prd->gasGap() -1;
                if constexpr (std::is_same_v<ContType, xAOD::RpcMeasurementContainer>) {
                    gapIdx = prd->readoutElement()->createHash(0, prd->gasGap(), prd->doubletPhi(), false);
                }

                bool measPhi{false};
                if constexpr(std::is_same_v<ContType, xAOD::sTgcMeasContainer>) {
                    /// Make directly to a space point
                    if (prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad) {
                        pointsInChamb.etaHits.emplace_back(*gctx, prd, nullptr);
                        continue;
                    }
                    /// Wires measure the phi coordinate
                    measPhi = prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire;
                } else {
                    /// Tgc & Rpcs have the measuresPhi property
                    measPhi = prd->measuresPhi();
                }

                if (hitsPerGasGap.size() <= gapIdx) {
                    hitsPerGasGap.resize(gapIdx + 1);
                }
                /// Sort in the hit
                PrdVec& toPush = hitsPerGasGap[gapIdx][measPhi];
                if (toPush.capacity() == toPush.size()) {
                    toPush.reserve(toPush.size() + m_capacityBucket);
                }
                toPush.push_back(prd);
            }
            /// Create the space points
            for (auto& [etaHits, phiHits] : hitsPerGasGap) {
                if (!passOccupancy2D(etaHits, phiHits)) {
                    ATH_MSG_VERBOSE("Occupancy cut not passed "<<etaHits.size()<<", "<<phiHits.size());
                    pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.size() + etaHits.size());
                    pointsInChamb.phiHits.reserve(pointsInChamb.phiHits.size() + phiHits.size());
                    for (const PrdType etaPrd : etaHits) {
                        pointsInChamb.etaHits.emplace_back(*gctx, etaPrd);
                        ATH_MSG_VERBOSE("Add new eta hit "<<m_idHelperSvc->toString(pointsInChamb.etaHits.back().identify())
                                <<" "<<Amg::toString(pointsInChamb.etaHits.back().positionInChamber()));

                    }
                    for (const PrdType phiPrd : phiHits) {
                        pointsInChamb.phiHits.emplace_back(*gctx, phiPrd);
                        ATH_MSG_VERBOSE("Add new phi hit "<<m_idHelperSvc->toString(pointsInChamb.phiHits.back().identify())
                                <<" "<<Amg::toString(pointsInChamb.phiHits.back().positionInChamber()));
                    }
                    continue;
                }
                std::vector<std::shared_ptr<unsigned>> etaCounts{matchCountVec(etaHits.size())}, 
                                                       phiCounts{matchCountVec(phiHits.size())};
                pointsInChamb.etaHits.reserve(etaHits.size()*phiHits.size());
                /// Simple combination by taking the cross-product
                for (unsigned int etaP = 0; etaP < etaHits.size(); ++etaP) {
                    /// There's no valid combination with another phi hit
                    for (unsigned int phiP = 0; phiP < phiHits.size(); ++ phiP) {
                        /** Tgc measurements with different bunch crossing tags cannot be combined */
                        if constexpr(std::is_same<xAOD::TgcStripContainer, ContType>::value) {
                            if (!(etaHits[etaP]->bcBitMap() & phiHits[phiP]->bcBitMap())){
                                continue;
                            }
                        }
                        SpacePoint& spacePoint{pointsInChamb.etaHits.emplace_back(*gctx, etaHits[etaP], phiHits[phiP])};
                        ATH_MSG_VERBOSE("Create new spacepoint from "<<m_idHelperSvc->toString(etaHits[etaP]->identify())
                        <<" & "<<m_idHelperSvc->toString(phiHits[phiP]->identify())<<" at "<<Amg::toString(spacePoint.positionInChamber()));
                        spacePoint.setInstanceCounts(etaCounts[etaP], phiCounts[phiP]);
                    }
                    if (!(*etaCounts[etaP])) {
                        pointsInChamb.etaHits.emplace_back(*gctx, etaHits[etaP]);
                        continue;
                    }
                }
                /// If there's a phi measuremnt which cannot be combined with the others 
                /// or no eta measurement is suitable, then manually push_back the phi hits
                for (unsigned int phiP = 0; phiP < phiHits.size(); ++ phiP){
                    if (!(*phiCounts[phiP])) {
                        pointsInChamb.phiHits.emplace_back(*gctx, phiHits[phiP]);
                    }
                }
            }
       }
    } while (viewer.next());
    return StatusCode::SUCCESS;
}


StatusCode SpacePointMakerAlg::execute(const EventContext& ctx) const {
    PreSortedSpacePointMap preSortedContainer{};
    ATH_CHECK(loadContainerAndSort(ctx, m_mdtKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_rpcKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_tgcKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_mmKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_stgcKey, preSortedContainer));
    std::unique_ptr<SpacePointContainer> outContainer = std::make_unique<SpacePointContainer>();
    
    for (auto &[chamber, hitsPerChamber] : preSortedContainer){
        ATH_MSG_VERBOSE("Fill space points for chamber "<<chamber);
        distributePointsAndStore(ctx, std::move(hitsPerChamber), *outContainer);
    }
    SG::WriteHandle<SpacePointContainer> writeHandle{m_writeKey, ctx};
    ATH_CHECK(writeHandle.record(std::move(outContainer)));
    return StatusCode::SUCCESS;
}

void SpacePointMakerAlg::distributePointsAndStore(const EventContext& ctx,
                                                  SpacePointsPerChamber&& hitsPerChamber,
                                                  SpacePointContainer& finalContainer) const {
    SpacePointBucketVec splittedHits{};
    splittedHits.emplace_back();
    if (m_statCounter){
        m_statCounter->addToStat(hitsPerChamber.etaHits);
        m_statCounter->addToStat(hitsPerChamber.phiHits);

    }
    distributePointsAndStore(ctx, std::move(hitsPerChamber.etaHits), splittedHits);
    distributePointsAndStore(ctx, std::move(hitsPerChamber.phiHits), splittedHits);
    
    for (SpacePointBucket& bucket : splittedHits) {
        if (bucket.size() > 1)
            finalContainer.push_back(std::make_unique<SpacePointBucket>(std::move(bucket)));
    }

}
void SpacePointMakerAlg::distributePointsAndStore(const EventContext& ctx,
                                                  std::vector<SpacePoint>&& spacePoints,
                                                  SpacePointBucketVec& splittedHits) const {
    
    if (spacePoints.empty()) return;

    const bool defineBuckets = splittedHits[0].empty();
    const bool hasEtaMeas{spacePoints[0].measuresEta()};    
    
    auto pointPos = [hasEtaMeas, defineBuckets] (const SpacePoint& p) {
        return hasEtaMeas || !defineBuckets ?  p.positionInChamber().y() : p.positionInChamber().x();
    };
    SG::ReadHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};

    auto channelDir = [hasEtaMeas, defineBuckets, &gctx](const SpacePoint & p) {
        const Amg::Vector3D d = xAOD::channelDirInChamber(*gctx, p.primaryMeasurement());
        return std::abs(hasEtaMeas || !defineBuckets ? d.y() : d.z());
    };

    std::sort(spacePoints.begin(), spacePoints.end(), 
              [&pointPos] (const SpacePoint& a, const SpacePoint& b) {
                    return pointPos(a) < pointPos(b);
              });
    

    double lastPoint = pointPos(spacePoints[0]);

    auto newBucket = [this, &lastPoint, &splittedHits, &pointPos, &channelDir] (const double currPos) {
        splittedHits.emplace_back();
        splittedHits.back().setBucketId(splittedHits.size() -1);
        SpacePointBucket& overlap{splittedHits[splittedHits.size() - 2]};
        SpacePointBucket& newContainer{splittedHits[splittedHits.size() - 1]};
     
        for (const std::shared_ptr<SpacePoint>& pointInBucket : overlap) {
            const double overlapPos = pointPos(*pointInBucket) + pointInBucket->uncertainty()[1] * channelDir(*pointInBucket);
            if (std::abs(overlapPos - currPos) < m_spacePointOverlap) {
                newContainer.push_back(pointInBucket);
            }
        }
        lastPoint = newContainer.empty() ? currPos : pointPos(**newContainer.begin());
        overlap.setCoveredRange(pointPos(**overlap.begin()), pointPos(**overlap.rbegin()));
    };

    for (SpacePoint& toSort : spacePoints) {        
        const double currPoint = pointPos(toSort);
        /// Phi modules
        if (!defineBuckets) {
           std::shared_ptr<SpacePoint> madePoint = std::make_shared<SpacePoint>(std::move(toSort));
           for (SpacePointBucket& bucket : splittedHits) {
                const double measDir = channelDir(toSort);
                const double posMin = currPoint - toSort.uncertainty()[1] * measDir;
                const double posMax = currPoint + toSort.uncertainty()[1] * measDir;
                
                if (posMax >= bucket.coveredMin() && bucket.coveredMax() >= posMin) {
                    bucket.push_back(madePoint);
                }
           }
           continue;
        }
        /// The current measurement is too far away from the first one. Make a new bucket
        if (currPoint - lastPoint > m_maxBucketLength || (!splittedHits.empty() && !splittedHits.back().empty()  && currPoint - pointPos(*splittedHits.back().back()) > m_spacePointWindow )) {
            newBucket(currPoint);            
        }
        std::shared_ptr<SpacePoint> spacePoint = std::make_shared<SpacePoint>(std::move(toSort));
        splittedHits.back().emplace_back(spacePoint);
        if (splittedHits.size() > 1) {
            SpacePointBucket& overlap{splittedHits[splittedHits.size() - 2]};
            const double overlapPos = currPoint - spacePoint->uncertainty()[1] * channelDir(*spacePoint);
            if (overlapPos - overlap.coveredMax() < m_spacePointOverlap) {
                overlap.push_back(spacePoint);
            }
        }
    }
    if (defineBuckets){
        SpacePointBucket& lastBucket{splittedHits[splittedHits.size() - 1]};
        newBucket(pointPos(*lastBucket.back()));
        /// Remove the probably empty bucket again.
        splittedHits.pop_back();
    }

}

}
