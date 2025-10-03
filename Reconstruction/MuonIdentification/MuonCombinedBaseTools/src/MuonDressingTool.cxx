/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonDressingTool.h"

#include "TrkTrack/Track.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
#include "TrkTrackSummary/TrackSummary.h"

using namespace Muon::MuonStationIndex;

namespace MuonCombined {
   StatusCode MuonDressingTool::initialize() {
        ATH_CHECK(m_hitSummaryTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }

    void MuonDressingTool::addMuonHitSummary(xAOD::Muon& muon, const Trk::TrackSummary* trackSummary) const {
  
        
        if (!trackSummary && (muon.muonType() != xAOD::Muon::SegmentTagged && muon.muonType() != xAOD::Muon::CaloTagged ) ) {
            // get link to track particle
            const xAOD::TrackParticle* primTrk = muon.primaryTrackParticle();
            if (primTrk->track()) {
                trackSummary = primTrk->track()->trackSummary();
            }
        }

        uint8_t innerSmallHits{0}, innerLargeHits{0}, middleSmallHits{0}, middleLargeHits{0}, 
                outerSmallHits{0}, outerLargeHits{0}, extendedSmallHits{0}, extendedLargeHits{0};

        uint8_t innerSmallHoles{0}, innerLargeHoles{0}, middleSmallHoles{0}, middleLargeHoles{0}, 
                outerSmallHoles{0}, outerLargeHoles{0}, extendedSmallHoles{0}, extendedLargeHoles{0};

        auto incrementPrecSummary = [&](const Identifier chId, const uint8_t hits, const uint8_t holes) {
            const ChIndex chIdx = m_idHelperSvc->chamberIndex(chId);
            switch (toLayerIndex(chIdx)) {
                case LayerIndex::Inner:
                    (isSmall(chIdx) ? innerSmallHits  : innerLargeHits)  += hits;
                    (isSmall(chIdx) ? innerSmallHoles : innerLargeHoles) += holes;
                    break;
                case LayerIndex::Middle:
                    (isSmall(chIdx) ? middleSmallHits  : middleLargeHits)  += hits;
                    (isSmall(chIdx) ? middleSmallHoles : middleLargeHoles) += holes;
                    break;
                case LayerIndex::Outer:
                    (isSmall(chIdx) ? outerSmallHits  : outerLargeHits)  += hits;
                    (isSmall(chIdx) ? outerSmallHoles : outerLargeHoles) += holes;
                    break;
                case LayerIndex::Extended:
                    (isSmall(chIdx) ? extendedSmallHits  : extendedLargeHits)  += hits;
                    (isSmall(chIdx) ? extendedSmallHoles : extendedLargeHoles) += holes;
                    break;
                case LayerIndex::BarrelExtended:
                   extendedSmallHits += hits;
                   extendedSmallHoles += holes;
                   break;
                case LayerIndex::LayerIndexMax:
                case LayerIndex::LayerUnknown:
                    ATH_MSG_WARNING("Unknown ChamberIndex " << chName(chIdx));
                    break;
            }
        };



        uint8_t phiLayer1Hits{0}, phiLayer2Hits{0}, phiLayer3Hits{0}, phiLayer4Hits{0};
        uint8_t etaLayer1Hits{0}, etaLayer2Hits{0}, etaLayer3Hits{0}, etaLayer4Hits{0};

        uint8_t phiLayer1Holes{0}, phiLayer2Holes{0}, phiLayer3Holes{0}, phiLayer4Holes{0};
        uint8_t etaLayer1Holes{0}, etaLayer2Holes{0}, etaLayer3Holes{0}, etaLayer4Holes{0};

        uint8_t phiLayer1RPCHits{0}, phiLayer2RPCHits{0}, phiLayer3RPCHits{0}, phiLayer4RPCHits{0};
        uint8_t etaLayer1RPCHits{0}, etaLayer2RPCHits{0}, etaLayer3RPCHits{0}, etaLayer4RPCHits{0};

        uint8_t phiLayer1RPCHoles{0}, phiLayer2RPCHoles{0}, phiLayer3RPCHoles{0}, phiLayer4RPCHoles{0};
        uint8_t etaLayer1RPCHoles{0}, etaLayer2RPCHoles{0}, etaLayer3RPCHoles{0}, etaLayer4RPCHoles{0};

        uint8_t phiLayer1TGCHits{0}, phiLayer2TGCHits{0}, phiLayer3TGCHits{0}, phiLayer4TGCHits{0};
        uint8_t etaLayer1TGCHits{0}, etaLayer2TGCHits{0}, etaLayer3TGCHits{0}, etaLayer4TGCHits{0};

        uint8_t phiLayer1TGCHoles{0}, phiLayer2TGCHoles{0}, phiLayer3TGCHoles{0}, phiLayer4TGCHoles{0};
        uint8_t etaLayer1TGCHoles{0}, etaLayer2TGCHoles{0}, etaLayer3TGCHoles{0}, etaLayer4TGCHoles{0};

        uint8_t phiLayer1STGCHits{0}, phiLayer2STGCHits{0};
        uint8_t etaLayer1STGCHits{0}, etaLayer2STGCHits{0};

        uint8_t phiLayer1STGCHoles{0}, phiLayer2STGCHoles{0};
        uint8_t etaLayer1STGCHoles{0}, etaLayer2STGCHoles{0};

        auto incrementTrigSummary = [&](const Identifier& chId,
                                        const uint8_t etaHits, const uint8_t etaHoles,
                                        const uint8_t phiHits, const uint8_t phiHoles) {
            const PhiIndex phiIndex = m_idHelperSvc->phiIndex(chId);
            switch (phiIndex) {                
                case PhiIndex::BI1:
                case PhiIndex::BM1:
                case PhiIndex::T4:
                case PhiIndex::CSC:
                case PhiIndex::STGC1:
                    phiLayer1Hits += phiHits; phiLayer1Holes += phiHoles;
                    etaLayer1Hits += etaHits; etaLayer1Holes += etaHoles;
                    break;
                case PhiIndex::BI2:
                case PhiIndex::BM2:
                case PhiIndex::T1:
                case PhiIndex::STGC2: 
                    phiLayer2Hits += phiHits; phiLayer2Holes += phiHoles;
                    etaLayer2Hits += etaHits; etaLayer2Holes += etaHoles;
                    break;
                case PhiIndex::BO1:
                case PhiIndex::T2: 
                    phiLayer3Hits += phiHits; phiLayer3Holes += phiHoles;
                    etaLayer3Hits += etaHits; etaLayer3Holes += etaHoles;
                    break;
                case PhiIndex::BO2:
                case PhiIndex::T3: 
                    phiLayer4Hits += phiHits; phiLayer4Holes += phiHoles;
                    etaLayer4Hits += etaHits; etaLayer4Holes += etaHoles;
                    break;
                
                case PhiIndex::PhiIndexMax:
                case PhiIndex::PhiUnknown:
                    ATH_MSG_WARNING("Unknown ChamberIndex" << Muon::MuonStationIndex::phiName(phiIndex));
                    break;
            }
            /// Now fill the hit summary per technology
            switch (phiIndex) {
                case PhiIndex::PhiIndexMax:
                case PhiIndex::PhiUnknown:
                case PhiIndex::CSC:
                    ATH_MSG_WARNING("Unknown ChamberIndex" << Muon::MuonStationIndex::phiName(phiIndex));
                    break;
                case PhiIndex::BI1:
                case PhiIndex::BI2:
                    ATH_MSG_VERBOSE("BI chambers "<<m_idHelperSvc->toString(chId)<<" are not part of the summary");
                    break;
                case PhiIndex::BM1:
                    phiLayer1RPCHits += phiHits; etaLayer1RPCHits += etaHits;
                    phiLayer1RPCHoles += phiHoles; etaLayer1RPCHoles += etaHoles;
                    break;
                case PhiIndex::BM2:
                    phiLayer2RPCHits += phiHits; etaLayer2RPCHits += etaHits;
                    phiLayer2RPCHoles += phiHoles; etaLayer2RPCHoles += etaHoles;
                    break; 
                case PhiIndex::BO1:
                    phiLayer3RPCHits += phiHits; etaLayer3RPCHits += etaHits;
                    phiLayer3RPCHoles += phiHoles; etaLayer3RPCHoles += etaHoles;
                    break; 
                case PhiIndex::BO2:
                    phiLayer4RPCHits += phiHits; etaLayer4RPCHits += etaHits;
                    phiLayer4RPCHoles += phiHoles; etaLayer4RPCHoles += etaHoles;
                    break;
                case PhiIndex::T4:
                    phiLayer1TGCHits += phiHits; etaLayer1TGCHits += etaHits;
                    phiLayer1TGCHoles += phiHoles; etaLayer1TGCHoles += etaHoles;
                    break;
                case PhiIndex::T1:
                    phiLayer2TGCHits += phiHits; etaLayer2TGCHits += etaHits;
                    phiLayer2TGCHoles += phiHoles; etaLayer2TGCHoles += etaHoles;
                    break;
                case PhiIndex::T2:
                    phiLayer3TGCHits += phiHits; etaLayer3TGCHits += etaHits;
                    phiLayer3TGCHoles += phiHoles; etaLayer3TGCHoles += etaHoles;
                    break;
                case PhiIndex::T3:
                    phiLayer4TGCHits += phiHits; etaLayer4TGCHits += etaHits;
                    phiLayer4TGCHoles += phiHoles; etaLayer4TGCHoles += etaHoles;
                    break;
                case PhiIndex::STGC1:
                    phiLayer1STGCHits += phiHits; etaLayer1STGCHits += etaHits;
                    phiLayer1STGCHoles += phiHoles; etaLayer1STGCHoles += etaHoles;
                    break;
                case PhiIndex::STGC2:
                    phiLayer2STGCHits += phiHits; etaLayer2STGCHits += etaHits;
                    phiLayer2STGCHoles += phiHoles; etaLayer2STGCHoles += etaHoles;
                    break;
            }
        };
    
        uint8_t innerCloseHits{0}, middleCloseHits{0}, outerCloseHits{0}, extendedCloseHits{0};
        uint8_t innerOutBoundsHits{0}, middleOutBoundsHits{0}, outerOutBoundsHits{0}, extendedOutBoundsHits{0};

        uint8_t combinedTrackOutBoundsHits{0};

        uint8_t numberOfPrecisionLayers{0}, numberOfPrecisionHoleLayers{0}, numberOfGoodPrecisionLayers{0};
        uint8_t numberOfPhiLayers{0}, numberOfPhiHoleLayers{0};

        uint8_t numberOfTriggerEtaLayers{0}, numberOfTriggerEtaHoleLayers{0};


        uint8_t mainSector{0}, secondSector{0}, isEndcap{0}, isSmall{0};

        uint8_t cscEtaHits{0}, cscUnspoiledEtaHits{0}, MMHits{0}, MMHoles{0};



        if (trackSummary) {
            Muon::IMuonHitSummaryTool::CompactSummary summary = m_hitSummaryTool->summary(*trackSummary);
            mainSector = secondSector = summary.mainSector;

            for (auto sec : summary.sectors) {
                if (sec != summary.mainSector) {
                    secondSector = sec;
                }
            }

            numberOfPrecisionLayers = summary.nprecisionLayers;
            numberOfPrecisionHoleLayers = summary.nprecisionHoleLayers;
            numberOfPhiLayers = summary.nphiLayers;
            numberOfPhiHoleLayers = summary.nphiHoleLayers;
            numberOfTriggerEtaLayers = summary.ntrigEtaLayers;
            numberOfTriggerEtaHoleLayers = summary.ntrigEtaHoleLayers;
            numberOfGoodPrecisionLayers = summary.nprecisionGoodLayers;

            isEndcap = (summary.isEndcap);
            isSmall = (summary.isSmall);

            for (auto layer : summary.stationLayers) {
                combinedTrackOutBoundsHits += layer.second.noutBoundsHits;
                switch (layer.first) {
                    case StIndex::BI:
                    case StIndex::EI:
                        innerCloseHits += layer.second.nprecisionCloseHits;
                        break;
                    case StIndex::BM:
                    case StIndex::EM:
                        middleCloseHits += layer.second.nprecisionCloseHits;
                        break;
                    case StIndex::BO:
                    case StIndex::EO:
                        outerCloseHits += layer.second.nprecisionCloseHits;
                        break;
                    case StIndex::BE:
                    case StIndex::EE:
                        extendedCloseHits += layer.second.nprecisionCloseHits;
                        break;
                    case StIndex::StUnknown:
                    case StIndex::StIndexMax:
                        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Unexpected st index");
                        break;
                }
            }

            // get out-of-bounds from ME track, shouldn't be any on combined track anyway
            const xAOD::TrackParticle* MStrk = muon.trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
            if (MStrk && MStrk->track() && MStrk->track()->trackSummary()) {
                for (const auto& layer : m_hitSummaryTool->summary(*MStrk->track()->trackSummary()).stationLayers) {
                    switch (layer.first) {
                        case StIndex::BI:
                        case StIndex::EI:
                            innerOutBoundsHits += layer.second.noutBoundsHits;
                            break;
                        case StIndex::EM:
                        case StIndex::BM:
                            middleOutBoundsHits += layer.second.noutBoundsHits;
                            break;
                        case StIndex::EO:
                        case StIndex::BO:
                            outerOutBoundsHits += layer.second.noutBoundsHits;
                            break;
                        case StIndex::BE:
                        case StIndex::EE: 
                            extendedOutBoundsHits += layer.second.noutBoundsHits;
                            break;
                        case StIndex::StUnknown:
                        case StIndex::StIndexMax:
                            ATH_MSG_WARNING("Station index should not be unknown or max");
                            break;
                    }
                }
            }

            if (trackSummary->muonTrackSummary()) {
                const Trk::MuonTrackSummary& mts = *trackSummary->muonTrackSummary();
                // loop over chambers
                for (const Trk::MuonTrackSummary::ChamberHitSummary&  summary : mts.chamberHitSummary()) {
                    switch (m_idHelperSvc->technologyIndex(summary.chamberId())) {
                        case TechnologyIndex::TechnologyUnknown:
                        case TechnologyIndex::TechnologyIndexMax:
                            ATH_MSG_WARNING("Unkown technology");
                            break;
                        case TechnologyIndex::MDT:{
                            incrementPrecSummary(summary.chamberId(), summary.nhits(), summary.nholes());
                            break;
                        } case TechnologyIndex::CSC:{
                            incrementPrecSummary(summary.chamberId(), summary.netaHits(), summary.etaProjection().nholes);
                            cscEtaHits += summary.etaProjection().nhits;
                            cscUnspoiledEtaHits += summary.etaProjection().ngoodHits;
                            break;
                        } case TechnologyIndex::MM:{
                            incrementPrecSummary(summary.chamberId(), summary.nhits(), summary.nholes());
                            MMHits += summary.nhits();
                            MMHoles += summary.nholes();
                            break;
                        } case TechnologyIndex::STGC:{
                            incrementPrecSummary(summary.chamberId(), summary.netaHits(), summary.etaProjection().nholes);
                            incrementTrigSummary(summary.chamberId(),
                                                 summary.netaHits(), summary.etaProjection().nholes,
                                                 summary.nphiHits(), summary.phiProjection().nholes);
                            break;
                        } case TechnologyIndex::RPC:
                          case TechnologyIndex::TGC: {
                            incrementTrigSummary(summary.chamberId(),
                                                 summary.netaHits(), summary.etaProjection().nholes,
                                                 summary.nphiHits(), summary.phiProjection().nholes);
                            break;
                        }
                    }
                }
            }
        }

        muon.setSummaryValue(numberOfPrecisionLayers, xAOD::numberOfPrecisionLayers);
        muon.setSummaryValue(numberOfPrecisionHoleLayers, xAOD::numberOfPrecisionHoleLayers);
        muon.setSummaryValue(numberOfPhiLayers, xAOD::numberOfPhiLayers);
        muon.setSummaryValue(numberOfPhiHoleLayers, xAOD::numberOfPhiHoleLayers);
        muon.setSummaryValue(numberOfTriggerEtaLayers, xAOD::numberOfTriggerEtaLayers);
        muon.setSummaryValue(numberOfTriggerEtaHoleLayers, xAOD::numberOfTriggerEtaHoleLayers);
        muon.setSummaryValue(numberOfGoodPrecisionLayers, xAOD::numberOfGoodPrecisionLayers);

        muon.setSummaryValue(mainSector, xAOD::primarySector);
        muon.setSummaryValue(secondSector, xAOD::secondarySector);
        muon.setSummaryValue(innerSmallHits, xAOD::innerSmallHits);
        muon.setSummaryValue(innerLargeHits, xAOD::innerLargeHits);
        muon.setSummaryValue(middleSmallHits, xAOD::middleSmallHits);
        muon.setSummaryValue(middleLargeHits, xAOD::middleLargeHits);
        muon.setSummaryValue(outerSmallHits, xAOD::outerSmallHits);
        muon.setSummaryValue(outerLargeHits, xAOD::outerLargeHits);
        muon.setSummaryValue(extendedSmallHits, xAOD::extendedSmallHits);
        muon.setSummaryValue(extendedLargeHits, xAOD::extendedLargeHits);

        muon.setSummaryValue(innerSmallHoles, xAOD::innerSmallHoles);
        muon.setSummaryValue(innerLargeHoles, xAOD::innerLargeHoles);
        muon.setSummaryValue(middleSmallHoles, xAOD::middleSmallHoles);
        muon.setSummaryValue(middleLargeHoles, xAOD::middleLargeHoles);
        muon.setSummaryValue(outerSmallHoles, xAOD::outerSmallHoles);
        muon.setSummaryValue(outerLargeHoles, xAOD::outerLargeHoles);
        muon.setSummaryValue(extendedSmallHoles, xAOD::extendedSmallHoles);
        muon.setSummaryValue(extendedLargeHoles, xAOD::extendedLargeHoles);

        muon.setSummaryValue(phiLayer1Hits, xAOD::phiLayer1Hits);
        muon.setSummaryValue(phiLayer2Hits, xAOD::phiLayer2Hits);
        muon.setSummaryValue(phiLayer3Hits, xAOD::phiLayer3Hits);
        muon.setSummaryValue(phiLayer4Hits, xAOD::phiLayer4Hits);

        muon.setSummaryValue(etaLayer1Hits, xAOD::etaLayer1Hits);
        muon.setSummaryValue(etaLayer2Hits, xAOD::etaLayer2Hits);
        muon.setSummaryValue(etaLayer3Hits, xAOD::etaLayer3Hits);
        muon.setSummaryValue(etaLayer4Hits, xAOD::etaLayer4Hits);

        muon.setSummaryValue(phiLayer1Holes, xAOD::phiLayer1Holes);
        muon.setSummaryValue(phiLayer2Holes, xAOD::phiLayer2Holes);
        muon.setSummaryValue(phiLayer3Holes, xAOD::phiLayer3Holes);
        muon.setSummaryValue(phiLayer4Holes, xAOD::phiLayer4Holes);

        muon.setSummaryValue(etaLayer1Holes, xAOD::etaLayer1Holes);
        muon.setSummaryValue(etaLayer2Holes, xAOD::etaLayer2Holes);
        muon.setSummaryValue(etaLayer3Holes, xAOD::etaLayer3Holes);
        muon.setSummaryValue(etaLayer4Holes, xAOD::etaLayer4Holes);

        muon.setSummaryValue(phiLayer1RPCHits, xAOD::phiLayer1RPCHits);
        muon.setSummaryValue(phiLayer2RPCHits, xAOD::phiLayer2RPCHits);
        muon.setSummaryValue(phiLayer3RPCHits, xAOD::phiLayer3RPCHits);
        muon.setSummaryValue(phiLayer4RPCHits, xAOD::phiLayer4RPCHits);

        muon.setSummaryValue(etaLayer1RPCHits, xAOD::etaLayer1RPCHits);
        muon.setSummaryValue(etaLayer2RPCHits, xAOD::etaLayer2RPCHits);
        muon.setSummaryValue(etaLayer3RPCHits, xAOD::etaLayer3RPCHits);
        muon.setSummaryValue(etaLayer4RPCHits, xAOD::etaLayer4RPCHits);

        muon.setSummaryValue(phiLayer1RPCHoles, xAOD::phiLayer1RPCHoles);
        muon.setSummaryValue(phiLayer2RPCHoles, xAOD::phiLayer2RPCHoles);
        muon.setSummaryValue(phiLayer3RPCHoles, xAOD::phiLayer3RPCHoles);
        muon.setSummaryValue(phiLayer4RPCHoles, xAOD::phiLayer4RPCHoles);

        muon.setSummaryValue(etaLayer1RPCHoles, xAOD::etaLayer1RPCHoles);
        muon.setSummaryValue(etaLayer2RPCHoles, xAOD::etaLayer2RPCHoles);
        muon.setSummaryValue(etaLayer3RPCHoles, xAOD::etaLayer3RPCHoles);
        muon.setSummaryValue(etaLayer4RPCHoles, xAOD::etaLayer4RPCHoles);

        muon.setSummaryValue(phiLayer1TGCHits, xAOD::phiLayer1TGCHits);
        muon.setSummaryValue(phiLayer2TGCHits, xAOD::phiLayer2TGCHits);
        muon.setSummaryValue(phiLayer3TGCHits, xAOD::phiLayer3TGCHits);
        muon.setSummaryValue(phiLayer4TGCHits, xAOD::phiLayer4TGCHits);

        muon.setSummaryValue(etaLayer1TGCHits, xAOD::etaLayer1TGCHits);
        muon.setSummaryValue(etaLayer2TGCHits, xAOD::etaLayer2TGCHits);
        muon.setSummaryValue(etaLayer3TGCHits, xAOD::etaLayer3TGCHits);
        muon.setSummaryValue(etaLayer4TGCHits, xAOD::etaLayer4TGCHits);

        muon.setSummaryValue(phiLayer1TGCHoles, xAOD::phiLayer1TGCHoles);
        muon.setSummaryValue(phiLayer2TGCHoles, xAOD::phiLayer2TGCHoles);
        muon.setSummaryValue(phiLayer3TGCHoles, xAOD::phiLayer3TGCHoles);
        muon.setSummaryValue(phiLayer4TGCHoles, xAOD::phiLayer4TGCHoles);

        muon.setSummaryValue(etaLayer1TGCHoles, xAOD::etaLayer1TGCHoles);
        muon.setSummaryValue(etaLayer2TGCHoles, xAOD::etaLayer2TGCHoles);
        muon.setSummaryValue(etaLayer3TGCHoles, xAOD::etaLayer3TGCHoles);
        muon.setSummaryValue(etaLayer4TGCHoles, xAOD::etaLayer4TGCHoles);

        // New Small Wheel
        // STGC
        muon.setSummaryValue(phiLayer1STGCHits, xAOD::phiLayer1STGCHits);
        muon.setSummaryValue(phiLayer2STGCHits, xAOD::phiLayer2STGCHits);

        muon.setSummaryValue(etaLayer1STGCHits, xAOD::etaLayer1STGCHits);
        muon.setSummaryValue(etaLayer2STGCHits, xAOD::etaLayer2STGCHits);

        muon.setSummaryValue(phiLayer1STGCHoles, xAOD::phiLayer1STGCHoles);
        muon.setSummaryValue(phiLayer2STGCHoles, xAOD::phiLayer2STGCHoles);

        muon.setSummaryValue(etaLayer1STGCHoles, xAOD::etaLayer1STGCHoles);
        muon.setSummaryValue(etaLayer2STGCHoles, xAOD::etaLayer2STGCHoles);

        // MM
        muon.setSummaryValue(MMHits, xAOD::MMHits);
        muon.setSummaryValue(MMHoles, xAOD::MMHoles);

        muon.setSummaryValue(innerCloseHits, xAOD::innerClosePrecisionHits);
        muon.setSummaryValue(middleCloseHits, xAOD::middleClosePrecisionHits);
        muon.setSummaryValue(outerCloseHits, xAOD::outerClosePrecisionHits);
        muon.setSummaryValue(extendedCloseHits, xAOD::extendedClosePrecisionHits);

        muon.setSummaryValue(innerOutBoundsHits, xAOD::innerOutBoundsPrecisionHits);
        muon.setSummaryValue(middleOutBoundsHits, xAOD::middleOutBoundsPrecisionHits);
        muon.setSummaryValue(outerOutBoundsHits, xAOD::outerOutBoundsPrecisionHits);
        muon.setSummaryValue(extendedOutBoundsHits, xAOD::extendedOutBoundsPrecisionHits);

        muon.setSummaryValue(combinedTrackOutBoundsHits, xAOD::combinedTrackOutBoundsPrecisionHits);

        muon.setSummaryValue(isEndcap, xAOD::isEndcapGoodLayers);
        muon.setSummaryValue(isSmall, xAOD::isSmallGoodSectors);

        muon.setSummaryValue(cscEtaHits, xAOD::cscEtaHits);
        muon.setSummaryValue(cscUnspoiledEtaHits, xAOD::cscUnspoiledEtaHits);
    }

}  // namespace MuonCombined
