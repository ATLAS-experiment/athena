/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentSelectionTool.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

namespace {
    using LayIdx_t = Muon::MuonStationIndex::LayerIndex;
}


namespace MuonR4{

    StatusCode SegmentSelectionTool::initialize(){
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    } 
    bool SegmentSelectionTool::passSeedingQuality(const EventContext& /* ctx*/,
                                                  const Segment& segment) const {
        const HitSummary& summary = segment.summary();

        switch (summary.tech) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                if (summary.nPrecHits < m_nMdtSeedHitCut || summary.nPrecOutlier > m_nMdtSeedOutlierCut){
                    return false;
                }
                const LayIdx_t layIdx {Muon::MuonStationIndex::toLayerIndex(segment.msSector()->chamberIndex())};
                /** @brief Apply a minimal threshold on the rpc phi trigger hits */
                if (segment.msSector()->barrel()) {
                    switch (layIdx) {
                        case LayIdx_t::Inner:
                        case LayIdx_t::BarrelExtended:
                            return  summary.nPhiHits >= m_nRpcPhiSeedHitCutBI;
                        case LayIdx_t::Middle:
                            return summary.nPhiHits >= m_nRpcPhiSeedHitCutBM;
                        case LayIdx_t::Outer:
                            return summary.nPhiHits >= m_nRpcPhiSeedHitCutBO;
                        default:
                            break;
                    }
                } else {
                    /** Apply another threshold on the TGC trigger hits */
                    switch (layIdx) {
                        case LayIdx_t::Inner:
                        case LayIdx_t::Extended:
                            return summary.nPhiHits >= m_nTgcPhiSeedHitCutEI;
                        case LayIdx_t::Middle:
                            return summary.nPhiHits >= m_nTgcPhiSeedHitCutEM;
                        default:
                            break;
                    }
                }
                break;
            }
            case xAOD::UncalibMeasType::MMClusterType:
            case xAOD::UncalibMeasType::sTgcStripType:{
                ATH_MSG_ALWAYS(__FILE__<<":"<<__LINE__<<" Implement me");
                break;
            }
            default:
                break;
        }
        return false;
    }

    bool SegmentSelectionTool::passTrackQuality(const EventContext& /*ctx*/,
                                                const Segment& segment) const {
        const HitSummary& summary = segment.summary();
        switch (summary.tech) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                return summary.nPrecHits >= m_nMdtMinHitCut;
            }
            case xAOD::UncalibMeasType::MMClusterType:
            case xAOD::UncalibMeasType::sTgcStripType:{
                ATH_MSG_ALWAYS(__FILE__<<":"<<__LINE__<<" Implement me");
                break;
            }
            default:
                break;
        }
        return false;
    }
    bool SegmentSelectionTool::compatibleForTrack(const EventContext& /*ctx*/,
                                                 const Segment& segA,
                                                 const Segment& segB) const {
        /** Segment is on the same spectrometer layer */
        if(segA.msSector() == segB.msSector()) {
            return false;
        }
        /** Segment sector deviates too much */
        const unsigned secMax = Muon::MuonStationIndex::numberOfSectors();
        const unsigned deltaSec = std::abs(segA.msSector()->sector() - segB.msSector()->sector()) % secMax;
        if (deltaSec > 1) {
            return false;
        } 
        const HitSummary& sumA = segA.summary();
        const HitSummary& sumB = segB.summary();
        /** If both segments don't have phi information, then they may be compatible */
        if (!sumA.nPhiHits && !sumB.nPhiHits) {
            return true;
        } 
        /** If one segment has phi information and the other doesn't then just check
         *  whether it's possible that the segment with phi is also in the same sector as the other */
        else if (sumA.nPhiHits && !sumB.nPhiHits) {
            if (!m_sectorMap.insideSector(segB.msSector()->sector(), segA.position().phi())){
                return false;
            }
        } else if (!sumA.nPhiHits && sumB.nPhiHits) {
            if (!m_sectorMap.insideSector(segA.msSector()->sector(), segB.position().phi())) {
                return false;
            }
        } 
        /** Both segments have phi information. Ensure that their phi is within 5 degrees */
        else {        
            /// Accept only segments that are 5 degree apart
            const double dPhi = std::abs(segA.position().deltaPhi(segB.position()));
            if (dPhi > 5. * Gaudi::Units::deg) {
                return false;
            }
        }
        return true;
    }

}