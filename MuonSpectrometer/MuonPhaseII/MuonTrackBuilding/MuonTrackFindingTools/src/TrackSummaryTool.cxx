/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackSummaryTool.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "xAODMuon/versions/MuonTrackSummaryAccessors_v1.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "Acts/Utilities/StringHelpers.hpp"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "xAODMuonPrepData/CombinedMuonStrip.h"

using namespace ActsTrk::detail;
using namespace Muon::MuonStationIndex;
namespace MuonR4 {

    using Cat_t = HitSummary::HitCategory;
    using Stat_t = HitSummary::Status;
    using LayerIndex = HitSummary::LayerIndex;

    StatusCode TrackSummaryTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }
    HitSummary TrackSummaryTool::makeSummary(const EventContext& /*ctx*/,
                                             const ConstTrack_t trackProxy) const  {
        HitSummary summary{};
        trackProxy.container().trackStateContainer().visitBackwards(trackProxy.tipIndex(), 
            [&](const auto& state){
                Stat_t status{Stat_t::OnTrack};
                if (state.typeFlags().test(Acts::TrackStateFlag::OutlierFlag)){
                    status = Stat_t::Outlier;
                } else if (state.typeFlags().test(Acts::TrackStateFlag::HoleFlag)) {
                    status = Stat_t::Hole;
                }
                if (state.hasUncalibratedSourceLink()) {
                    const auto* meas = xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
                    // for the combined sTgc space point we have to fill the primary and secodnray measuremment seperately to resolve the strip/pad/wire combinations
                    if(meas->type() == xAOD::UncalibMeasType::sTgcStripType && meas->numDimensions() == 0){
                        const auto* combinedMeas = static_cast<const xAOD::CombinedMuonStrip*>(meas);
                        incrementSummary(xAOD::identify(combinedMeas->primaryStrip()), status, combinedMeas->primaryStrip()->numDimensions(), summary);
                        incrementSummary(xAOD::identify(combinedMeas->secondaryStrip()), status, combinedMeas->secondaryStrip()->numDimensions(), summary);
                        
                    } else {
                        incrementSummary(xAOD::identify(meas), status, meas->numDimensions(), summary);
                    }
                } else if (state.hasReferenceSurface()) {
                    const Acts::Surface& surf{state.referenceSurface()};
                    /// Surface is not active
                    const Acts::DetectorElementBase* detEl = surf.associatedDetectorElement();
                    if (!detEl) {
                        return;
                    }
                    incrementSummary(static_cast<const ActsTrk::IDetectorElementBase*>(detEl)->identify(),
                                     status, 1, summary);
                }
        });
        ATH_MSG_DEBUG("Obtained track summary from track with "<<Acts::toString(trackProxy.fourMomentum())
                      <<", q: "<<trackProxy.qOverP()
                      <<", chi2: "<<(trackProxy.chi2()/ std::max(trackProxy.nDoF(), 1u))
                      <<", nDoF: "<<trackProxy.nDoF()<<"\n"<<summary);
        return summary;                   
    }
    void TrackSummaryTool::incrementSummary(const Identifier& hitId,
                                            const Stat_t status,
                                            const unsigned prdDim,
                                            HitSummary& summary) const {
        using TechIdx = Muon::MuonStationIndex::TechnologyIndex;
        const TechIdx techIdx = m_idHelperSvc->technologyIndex(hitId);

        /// Hit is not a muon hit
        if (techIdx == TechIdx::TechnologyUnknown) {
            return;
        }
        const ChIndex chIdx = m_idHelperSvc->chamberIndex(hitId);
        const LayerIndex layer{toLayerIndex(chIdx)};
        const bool small{isSmall(chIdx)};

        Cat_t cat1{Cat_t::nCategories}, cat2{Cat_t::nCategories};

        if (techIdx == TechIdx::MDT || techIdx == TechIdx::MM) {
            cat1 = Cat_t::Precision;
            /// Mdt twin tubes
            if (prdDim == 2){
                cat2 = Cat_t::TriggerPhi;
            }
        } else if (techIdx == TechIdx::RPC || techIdx == TechIdx::TGC) {
            /// Combined measurement or it's a 2D BI strip
            if (prdDim == 0 || prdDim == 2) {
                cat1 = Cat_t::TriggerEta;
                cat2 = Cat_t::TriggerPhi;
            } else if (m_idHelperSvc->measuresPhi(hitId)) {
                cat1 = Cat_t::TriggerPhi;
            } else {
                cat1 = Cat_t::TriggerEta;
            }
        } else if (techIdx == TechIdx::STGC) {
            if(m_idHelperSvc->stgcIdHelper().channelType(hitId) == sTgcIdHelper::sTgcChannelTypes::Pad){
                cat1 = Cat_t::sTgcPad;
            } else if (m_idHelperSvc->stgcIdHelper().channelType(hitId) == sTgcIdHelper::sTgcChannelTypes::Wire){
                cat1 = Cat_t::TriggerPhi;
            } else if (m_idHelperSvc->stgcIdHelper().channelType(hitId) == sTgcIdHelper::sTgcChannelTypes::Strip){
                cat1 = Cat_t::Precision;
            }
        } else {
            ATH_MSG_ERROR(__FILE__ << ":" << __LINE__ << "  Unkown technology index "<<static_cast<int>(techIdx));
            return;
        }

        if (cat1 != Cat_t::nCategories){
            ATH_MSG_VERBOSE("Increment "<<summary.toString(cat1)<<", "<<summary.toString(status)<<", layer: "
                <<Muon::MuonStationIndex::layerName(layer)<<", small: "<<(small ? "yes" : "no"));
            ++summary.value(cat1, status, layer, small);
        }
        if (cat2 != Cat_t::nCategories) {
            ATH_MSG_VERBOSE("Increment "<<summary.toString(cat2)<<", "<<summary.toString(status)<<", layer: "
                    <<Muon::MuonStationIndex::layerName(layer)<<", small: "<<(small ? "yes" : "no"));
            ++summary.value(cat2, status, layer, small);
        }
    }

    HitSummary TrackSummaryTool::makeSummary(const EventContext& /*ctx*/,
                                             const std::vector<const xAOD::MuonSegment*> & segments) const {
        HitSummary summary{};
        for (const xAOD::MuonSegment* seg : segments) {
            const LayerIndex lay = toLayerIndex(seg->chamberIndex());
            const bool small = isSmall(seg->chamberIndex());
            if (!m_reDoSegments) {
                summary.value(Cat_t::Precision, Stat_t::OnTrack, lay, small) = seg->nPrecisionHits();
                summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, lay, small) = seg->nTrigEtaLayers();
                summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, lay, small) = seg->nPhiLayers();
            } else {
                const std::size_t nHits = nMeasurements(*seg);
                for (std::size_t hit = 0; hit < nHits; ++hit) {
                    Stat_t state = isOutlierMeasurement(*seg, hit) ? Stat_t::Outlier : Stat_t::OnTrack;
                    const auto * meas = getMeasurement(*seg, hit);
                    // for the combined sTgc space point we have to fill the primary and secodnray measuremment seperately to resolve the strip/pad/wire combinations
                    if(meas->type() == xAOD::UncalibMeasType::sTgcStripType && meas->numDimensions() == 0){
                        const auto* combinedMeas = static_cast<const xAOD::CombinedMuonStrip*>(meas);
                        incrementSummary(xAOD::identify(combinedMeas->primaryStrip()), state, combinedMeas->primaryStrip()->numDimensions(), summary);
                        incrementSummary(xAOD::identify(combinedMeas->secondaryStrip()), state, combinedMeas->secondaryStrip()->numDimensions(), summary);
                        
                    } else {
                        incrementSummary(xAOD::identify(meas), state, meas->numDimensions(), summary);
                    }
                }
            }
        }
        return summary;
    }
    HitSummary TrackSummaryTool::makeSummary(const EventContext& /*ctx*/,
                                             const Trk::Track& track) const {
        HitSummary summary{};
        for (const Trk::TrackStateOnSurface* tsos : *track.trackStateOnSurfaces()) {
            using enum Trk::TrackStateOnSurface::TrackStateOnSurfaceType;
            if (tsos->type(Hole)){
                incrementSummary(tsos->surface().associatedDetectorElementIdentifier(),
                                 Stat_t::Hole, 1, summary);
                continue;
            } 
            const auto* meas = tsos->measurementOnTrack();
            if (!meas) {
                continue;
            }
            Stat_t state = tsos->type(Outlier) ? Stat_t::Outlier : Stat_t::OnTrack;
            if (const auto* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas); rot != nullptr) {
                incrementSummary(rot->identify(), state, 1, summary);
            } else if (const auto* rot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(meas); rot != nullptr) {
                for (unsigned n = 0; n <rot->numberOfContainedROTs(); ++n) {
                    incrementSummary(rot->rioOnTrack(n).identify(), state, 1, summary);
                }
            }
        }
        return summary;
    }
            
    void TrackSummaryTool::copySummary(const HitSummary& summary,
                                       const xAOD::IParticle& track) const {
        ATH_MSG_DEBUG("Copy the summary \n "<<summary<<"\n pT: "<<(track.pt() * 1.e-3)
                      <<", eta: "<<track.eta()<<", phi: "<<track.phi());
        
        auto acc = [&track](const xAOD::MuonSummaryType type) -> std::uint8_t& {
            const std::string accName = SG::AuxTypeRegistry::instance().getName(
                xAOD::muonTrackSummaryAccessorV1(type).auxid());
            const SG::Decorator<std::uint8_t> dec{accName};
            return dec(track);
        };
        using enum xAOD::MuonSummaryType;
        /// Precision hits
        acc(innerSmallHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Inner, true);
        acc(innerLargeHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Inner, false);
        acc(middleSmallHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Middle, true);
        acc(middleLargeHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Middle, false);
        acc(outerSmallHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Outer, true);
        acc(outerLargeHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Outer, false);
        acc(extendedSmallHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Extended, true);
        acc(extendedLargeHits) = summary.value(Cat_t::Precision, Stat_t::OnTrack, LayerIndex::Extended, false);
        if (m_fillHoles) {
            acc(innerSmallHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Inner, true);
            acc(innerLargeHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Inner, false);
            acc(middleSmallHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Middle, true);
            acc(middleLargeHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Middle, false);
            acc(outerSmallHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Outer, true);
            acc(outerLargeHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Outer, false);
            acc(extendedSmallHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Extended, true);
            acc(extendedLargeHoles) = summary.value(Cat_t::Precision, Stat_t::Hole, LayerIndex::Extended, false);
        }
        if (m_fillOutliers) {
            acc(innerOutBoundsPrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Inner, false)
                                             + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Inner, true);

            acc(middleOutBoundsPrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Middle, false)
                                              + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Middle, true);

            acc(outerOutBoundsPrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Outer, false)
                                             + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Outer, true);

            acc(extendedOutBoundsPrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Extended, false)
                                                + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Extended, true);
        }

        /// Trigger hits
        acc(etaLayer1Hits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Inner, true)
                           + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Inner, false);

        acc(etaLayer2Hits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Middle, true)
                           + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Middle, false);

        acc(etaLayer3Hits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Outer, true)
                          + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Outer, false);

        acc(etaLayer4Hits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Extended, true)
                           + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Extended, false);

         if (m_fillHoles) {
            acc(etaLayer1Holes) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Inner, true)
                                + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Inner, false);

            acc(etaLayer2Holes) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Middle, true)
                                + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Middle, false);

            acc(etaLayer3Holes) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Outer, true)
                                + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Outer, false);

            acc(etaLayer4Holes) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Extended, true)
                                + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Extended, false);
        }
        acc(phiLayer1Hits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Inner, true)
                           + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Inner, false);

        acc(phiLayer2Hits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Middle, true)
                           + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Middle, false);

        acc(phiLayer3Hits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Outer, true)
                           + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Outer, false);

        acc(phiLayer4Hits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Extended, true)
                           + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Extended, false);
        if (m_fillHoles) {
            acc(phiLayer1Holes) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Inner, true)
                                + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Inner, false);

            acc(phiLayer2Holes) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Middle, true)
                                + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Middle, false);

            acc(phiLayer3Holes) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Outer, true)
                                + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Outer, false);

            acc(phiLayer4Holes) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Extended, true)
                                + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Extended, false);
        }
    }     
}