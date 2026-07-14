/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentSelectionTool.h"

#include "MuonTrackEvent/ExpandedSector.h"

namespace {
    using namespace Muon::MuonStationIndex;
}


namespace MuonR4{

    StatusCode SegmentSelectionTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    } 
    bool SegmentSelectionTool::passSeedingQuality(const EventContext& /* ctx*/,
                                                  const xAOD::MuonSegment& segment) const {

        switch (segment.technology()) {
            using enum TechnologyIndex;
            case MDT: {
                if (segment.nPrecisionHits() < m_nMdtSeedHitCut || 
                    segment.nPrecisionOutliers() > m_nMdtSeedOutlierCut){
                    return false;
                }
                const LayerIndex layIdx {toLayerIndex(segment.chamberIndex())};
                /** @brief Apply a minimal threshold on the rpc phi trigger hits */
                if (isBarrel(segment.chamberIndex())) {
                    switch (toLayerIndex(segment.chamberIndex())) {
                        using enum LayerIndex;
                        case Inner:
                        case BarrelExtended:
                            return segment.nPhiLayers() >= m_nRpcPhiSeedHitCutBI;
                        case Middle:
                            return segment.nPhiLayers() >= m_nRpcPhiSeedHitCutBM;
                        case Outer:
                            return segment.nPhiLayers() >= m_nRpcPhiSeedHitCutBO;
                        default:
                            break;
                    }
                } else {
                    /** Apply another threshold on the TGC trigger hits */
                    switch (layIdx) {
                        using enum LayerIndex;
                        case Extended:
                            return true;
                        case Inner:
                            return segment.nPhiLayers() >= m_nTgcPhiSeedHitCutEI;
                        case Middle:
                            return segment.nPhiLayers() >= m_nTgcPhiSeedHitCutEM;
                        default:
                            break;
                    }
                }
                break;
            }
            case MM: {
                return segment.nPrecisionHits() >= m_nMmSeedMinHitCut;
            }
            case STGC: {
                return segment.nPrecisionHits() >= m_nStgcSeedMinHitCut;
            }
            default:
                break;
        }
        return false;
    }

    bool SegmentSelectionTool::passTrackQuality(const EventContext& /*ctx*/,
                                                const xAOD::MuonSegment& segment) const {
        switch (segment.technology()) {
            using enum TechnologyIndex;
            case MDT: {
                return segment.nPrecisionHits() >= m_nMdtMinHitCut;
            }
            case MM:
            case STGC:{
                return segment.nPrecisionHits() >= m_nMdtMinHitCut;
            }
            default:
                break;
        }
        return false;
    }
    bool SegmentSelectionTool::compatibleForTrack(const EventContext& /*ctx*/,
                                                  const xAOD::MuonSegment& segA,
                                                  const xAOD::MuonSegment& segB) const {
        /** Segment is on the same spectrometer layer */
        if(segA.chamberIndex() == segB.chamberIndex() ||
           segA.etaIndex() * segB.etaIndex() < 0) {
            return false;
        }
        /** If both segments don't have phi information, then they may be compatible */
        if (!segA.nPhiLayers() && !segB.nPhiLayers()) {
            return true;
        } 
        /** If one segment has phi information and the other doesn't then just check
         *  whether it's possible that the segment with phi is also in the same sector as the other */
        else if (segA.nPhiLayers() && !segB.nPhiLayers()) {
            if (!ExpandedSector{segA.position().phi()}.isNeighbour(
                 ExpandedSector{static_cast<unsigned>(segB.sector()), ExpandedSector::SectorProjector::center})){
                return false;
            }
        } else if (!segA.nPhiLayers() && segB.nPhiLayers()) {
            if (!ExpandedSector{segB.position().phi()}.isNeighbour(
                 ExpandedSector{static_cast<unsigned>(segA.sector()), ExpandedSector::SectorProjector::center})){
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