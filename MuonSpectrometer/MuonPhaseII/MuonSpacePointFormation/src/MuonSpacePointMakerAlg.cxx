/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePointMakerAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"


namespace MuonR4 {


MuonSpacePointMakerAlg::MuonSpacePointMakerAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator}{}


StatusCode MuonSpacePointMakerAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_mdtKey.initialize(!m_mdtKey.empty()));
    ATH_CHECK(m_rpcKey.initialize(!m_rpcKey.empty()));
    ATH_CHECK(m_tgcKey.initialize(!m_tgcKey.empty()));
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    return StatusCode::SUCCESS;
}

template <class ContType>StatusCode MuonSpacePointMakerAlg::loadContainerAndSort(const EventContext& ctx,
                                                                                 const SG::ReadHandleKey<ContType>& key,
                                                                                 PreSortedSpacePointMap& fillContainer) const {
    if (key.empty()) {
        ATH_MSG_DEBUG("Key "<<typeid(ContType).name()<<" not set. Do not fill anything");
        return StatusCode::SUCCESS;
    }                          
    SG::ReadHandle<ContType> readHandle{key, ctx};
    ATH_CHECK(readHandle.isPresent());

    SG::ReadCondHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
    ATH_CHECK(gctx.isValid());
    
    using PrdType = typename ContType::const_value_type;
    using PrdVec = std::vector<PrdType>;
    /// Fill the Mdt hits directly into their respective pre sorted container
    if constexpr (std::is_same<ContType, xAOD::MdtDriftCircleContainer>::value) {
        for (const PrdType prd : *readHandle) {
            fillContainer[prd->readoutElement()->getChamber()].etaHits.emplace_back(**gctx, prd, nullptr);
        }
    } else {
        /// Helper pair to separate eta & phi hits
        using EtaPhiHits = std::pair<PrdVec, PrdVec>;
        /// To reduce the combinatorics store the eta & phi hits per gas gap. 
        /// All strip detectors have at maximum 4 gasGaps, 
        //  But the Rpcs are additionally split accoring to their doubletPhi
        using EtaPhiHitsPerChamber = std::array<EtaPhiHits, 6>;
        std::map<const MuonGMR4::MuonReadoutElement*, EtaPhiHitsPerChamber> collectedPrds{};
        for (const PrdType prd : *readHandle) {
            EtaPhiHitsPerChamber& hitsPerChamb = collectedPrds[prd->readoutElement()];
            /// Sort the hit into a GasGap
            unsigned int gapIdx = prd->gasGap() -1;
            /// Split the Rpcs additionally according to their doubletPhi. 
            if constexpr (std::is_same<ContType, xAOD::RpcStripContainer>::value) {
                gapIdx = 2*gapIdx + (prd->doubletPhi() - 1);
            }
            EtaPhiHits& hitsPerLayer = hitsPerChamb[gapIdx];
            if (!prd->measuresPhi()) {
                hitsPerLayer.first.push_back(prd);
            } else {
                hitsPerLayer.second.push_back(prd);
            }
        }
        /// Loop over the splitted hits and form the space points
        for (auto& [reEle, hitsPerChamb] : collectedPrds) {
           spacePointsPerChamber& fillInto {fillContainer[reEle->getChamber()]};           
           ATH_MSG_VERBOSE("Fill collected measurements for "<<m_idHelperSvc->toStringDetEl(reEle->identify()));
           for (auto& [etaHits, phiHits]: hitsPerChamb) {
                /// If one of the two is empty no chance to combine them
                if (etaHits.empty() || phiHits.empty()) {
                    fillInto.etaHits.reserve(fillInto.etaHits.size() + etaHits.size());
                    fillInto.phiHits.reserve(fillInto.phiHits.size() + phiHits.size());
                    for (const PrdType etaPrd : etaHits) {
                        fillInto.etaHits.emplace_back(**gctx, etaPrd);
                    }
                    for (const PrdType phiPrd : phiHits) {
                        fillInto.phiHits.emplace_back(**gctx, phiPrd);
                    }
                    continue;
                }
                /// Simple combination by taking the cross-product
                fillInto.etaHits.reserve(fillInto.etaHits.size() + etaHits.size() * phiHits.size());
                for (const PrdType etaPrd : etaHits) {
                    for (const PrdType phiPrd: phiHits) {
                        fillInto.etaHits.emplace_back(**gctx, etaPrd, phiPrd);
                    }
                }
            }
        }
    }
    return StatusCode::SUCCESS;                                      
}


StatusCode MuonSpacePointMakerAlg::execute(const EventContext& ctx) const {
    PreSortedSpacePointMap preSortedContainer{};
    ATH_CHECK(loadContainerAndSort(ctx, m_mdtKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_rpcKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_tgcKey, preSortedContainer));   
    std::unique_ptr<MuonSpacePointContainer> outContainer = std::make_unique<MuonSpacePointContainer>();
    
    for (auto &[chamber, hitsPerChamber] : preSortedContainer){
        ATH_MSG_VERBOSE("Fill space points for chamber "<<chamber);
        distributePointsAndStore(ctx, std::move(hitsPerChamber), *outContainer);
    }
    SG::WriteHandle<MuonSpacePointContainer> writeHandle{m_writeKey, ctx};
    ATH_CHECK(writeHandle.record(std::move(outContainer)));
    return StatusCode::SUCCESS;
}

void MuonSpacePointMakerAlg::distributePointsAndStore(const EventContext& ctx,
                                                      spacePointsPerChamber&& hitsPerChamber,
                                                      MuonSpacePointContainer& finalContainer) const {
    SpacePointBucketVec splittedHits{};
    splittedHits.emplace_back();
    distributePointsAndStore(ctx, std::move(hitsPerChamber.etaHits), splittedHits);
    distributePointsAndStore(ctx, std::move(hitsPerChamber.phiHits), splittedHits);
    
    for (MuonSpacePointBucket& bucket : splittedHits) {
        if (bucket.size() > 1)
            finalContainer.push_back(std::make_unique<MuonSpacePointBucket>(std::move(bucket)));
    }

}
void MuonSpacePointMakerAlg::distributePointsAndStore(const EventContext& ctx,
                                                      std::vector<MuonSpacePoint>&& spacePoints,
                                                      SpacePointBucketVec& splittedHits) const {
    
    if (spacePoints.empty()) return;

    const bool defineBuckets = splittedHits[0].empty();
    const bool hasEtaMeas{spacePoints[0].measuresEta()};    
    
    auto pointPos = [hasEtaMeas, defineBuckets] (const MuonSpacePoint& p) {
        return hasEtaMeas || !defineBuckets ?  p.positionInChamber().y() : p.positionInChamber().x();
    };
    SG::ReadCondHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};

    auto channelDir = [hasEtaMeas, defineBuckets, &gctx](const MuonSpacePoint & p) {
        const Amg::Vector3D d = xAOD::channelDirInChamber(**gctx, p.primaryMeasurement());
        return std::abs(hasEtaMeas || !defineBuckets ? d.y() : d.z());
    };

    std::sort(spacePoints.begin(), spacePoints.end(), 
              [&pointPos] (const MuonSpacePoint& a, const MuonSpacePoint& b) {
                    return pointPos(a) < pointPos(b);
              });
    


    constexpr double maxWindowSize = 2. * Gaudi::Units::m;
    constexpr double overlapSize = 25. * Gaudi::Units::cm;
    
    double lastPoint = pointPos(spacePoints[0]);

    auto newBucket = [&lastPoint, &splittedHits, &pointPos, &channelDir] (const double currPos) {
        splittedHits.emplace_back();
        splittedHits.back().setBucketId(splittedHits.size() -1);
        MuonSpacePointBucket& overlap{splittedHits[splittedHits.size() - 2]};
        MuonSpacePointBucket& newContainer{splittedHits[splittedHits.size() - 1]};
     
        for (const std::shared_ptr<MuonSpacePoint>& pointInBucket : overlap) {
            const double overlapPos = pointPos(*pointInBucket) + pointInBucket->uncertainty()[1] * channelDir(*pointInBucket);
            if (std::abs(overlapPos - currPos) < overlapSize) {
                newContainer.push_back(pointInBucket);
            }
        }
        lastPoint = newContainer.empty() ? currPos : pointPos(**newContainer.begin());
        overlap.setCoveredRange(pointPos(**overlap.begin()), pointPos(**overlap.rbegin()));
    };

    for (MuonSpacePoint& toSort : spacePoints) {        
        const double currPoint = pointPos(toSort);
        /// Phi modules
        if (!defineBuckets) {
           std::shared_ptr<MuonSpacePoint> madePoint = std::make_shared<MuonSpacePoint>(std::move(toSort));
           for (MuonSpacePointBucket& bucket : splittedHits) {
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
        if (currPoint - lastPoint > maxWindowSize) {
            newBucket(currPoint);            
        }
        std::shared_ptr<MuonSpacePoint> spacePoint = std::make_shared<MuonSpacePoint>(std::move(toSort));
        splittedHits.back().emplace_back(spacePoint);
        if (splittedHits.size() > 1) {
            MuonSpacePointBucket& overlap{splittedHits[splittedHits.size() - 2]};
            const double overlapPos = currPoint - spacePoint->uncertainty()[1] * channelDir(*spacePoint);
            if (overlapPos - overlap.coveredMax() < overlapSize) {
                overlap.push_back(spacePoint);
            }
        }
    }
    MuonSpacePointBucket& lastBucket{splittedHits[splittedHits.size() - 1]};
    newBucket(pointPos(*lastBucket.back()));
    /// Remove the probably empty bucket again.
    splittedHits.pop_back();

}

}