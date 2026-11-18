/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentSelectionTool.h"

#include "MuonTrackEvent/ExpandedSector.h"
#include "Acts/Definitions/Units.hpp"

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
            case MM:
            case STGC: {
                return segment.nPrecisionHits() >= m_nNswSeedMinHitCut;
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
                return segment.nPrecisionHits() >= m_nNswMinHitCut;
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
        if (segA.nPhiLayers() || segB.nPhiLayers()) {
            /** If one segment has phi information and the other doesn't then just check
            *  whether it's possible that the segment with phi is also in the same sector as the other */
            if (!segB.nPhiLayers()) {
                if (!ExpandedSector{segA.position().phi()}.isNeighbour(
                    ExpandedSector{static_cast<unsigned>(segB.sector()), ExpandedSector::SectorProjector::center})){
                    return false;
                }
            } else if (!segA.nPhiLayers()) {
                if (!ExpandedSector{segB.position().phi()}.isNeighbour(
                    ExpandedSector{static_cast<unsigned>(segA.sector()), ExpandedSector::SectorProjector::center})){
                    return false;
                }
            } else {        
                /** Both segments have phi information. Ensure that their phi is within 5 degrees */
                const double dPhi = std::abs(segA.position().deltaPhi(segB.position()));
                if (dPhi > 5. * Gaudi::Units::deg) {
                    return false;
                }
            }
        }
        using namespace Acts::UnitLiterals;
        /** One toroid kick between the two segments: in the bending plane the chord
         *  between the segments must lie between their directions. If it doesn't, 
         *  the smaller of the two deviations is the violation */
        const xAOD::MuonSegment& a = segA.position().mag() < segB.position().mag() ? segA : segB;
        const xAOD::MuonSegment& b = &a == &segA ? segB : segA;
        const Amg::Vector3D eR{Acts::makeDirectionFromPhiTheta(a.position().phi(), 90._degree)};
        const Amg::Vector3D chord{b.position() - a.position()};
        auto angle = [&eR](const Amg::Vector3D& v) { 
            return std::atan2(v.dot(eR), v.z()); 
        };
        const double thetaA {angle(a.direction())};
        const double thetaB {angle(b.direction())};
        const double thetaC {angle(chord)};
        const double dA {std::remainder(thetaC - thetaA, 360._degree)};
        const double dB {std::remainder(thetaB - thetaC, 360._degree)};
        if (dA * dB >= 0.) {
            return true;
        }
        const double violation {std::min(std::abs(dA), std::abs(dB))};
        return violation < m_maxKickViolation;
    }

}