/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackSummaryTool.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "ActsGeometryInterfaces/ISurfacePlacement.h"

#include "xAODMuon/versions/MuonTrackSummaryAccessors_v1.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "Acts/Utilities/StringHelpers.hpp"

#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"

#include "xAODMuonPrepData/UtilFunctions.h"
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
    void TrackSummaryTool::complementaryHole(const Identifier& gasGapId,
                                           const MuonGMR4::MuonReadoutElement* reEle,
                                           HitSummary& summary) const {
        if (!reEle) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No readout element associated "
                            <<m_idHelperSvc->toString(gasGapId));
            return;
        }
        const bool measPhi = m_idHelperSvc->measuresPhi(gasGapId);
        switch(reEle->detectorType()) {
            using enum ActsTrk::DetectorType;
            case Rpc:{
                const auto* castRE = static_cast<const MuonGMR4::RpcReadoutElement*>(reEle);
                if (castRE->nPhiStrips() || measPhi) {
                    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                    const Identifier holeId = idHelper.panelID(gasGapId, idHelper.gasGap(gasGapId), !measPhi);
                    incrementSummary(holeId, Stat_t::Hole, 1, summary);
                }
                break;
            } case Tgc: {
                const auto* castRE = static_cast<const MuonGMR4::TgcReadoutElement*>(reEle);
                const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
                const Identifier holeId = idHelper.channelID(gasGapId, 
                                                             idHelper.gasGap(gasGapId), !measPhi, 1);

                if (castRE->numChannels(castRE->measurementHash(holeId))){
                    incrementSummary(holeId, Stat_t::Hole, 1, summary);
                }
                break;
            } case sTgc: {
                const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                switch (idHelper.channelType(gasGapId)) {
                    using enum sTgcIdHelper::sTgcChannelTypes;
                    case Strip:
                        incrementSummary(idHelper.channelID(gasGapId, 
                                                            idHelper.multilayer(gasGapId),
                                                            idHelper.gasGap(gasGapId), Wire, 1), 
                                         Stat_t::Hole, 1, summary);
                        break;
                    case Wire:
                        incrementSummary(idHelper.channelID(gasGapId, 
                                                            idHelper.multilayer(gasGapId),
                                                            idHelper.gasGap(gasGapId), Strip, 1), 
                                         Stat_t::Hole, 1, summary);
                    default:
                        break;
                }
            }
            default: 
                break;
        }
    }
    HitSummary TrackSummaryTool::makeSummary(const EventContext& ctx,
                                             const ConstTrack_t trackProxy) const {
        return makeSummaryImpl(ctx, trackProxy);
    }
    HitSummary TrackSummaryTool::makeSummary(const EventContext& ctx,
                                             const Track_t trackProxy) const {
        return makeSummaryImpl(ctx, trackProxy);
    }


    template <Acts::TrackProxyConcept T>
    HitSummary TrackSummaryTool::makeSummaryImpl(const EventContext& /*ctx*/,
                                             const T& trackProxy) const  {
        HitSummary summary{};
        trackProxy.container().trackStateContainer().visitBackwards(trackProxy.tipIndex(), 
            [&](const auto& state){
                Stat_t status{Stat_t::OnTrack};
                if (state.typeFlags().isOutlier()){
                    status = Stat_t::Outlier;
                } else if (state.typeFlags().isHole()) {
                    status = Stat_t::Hole;
                }
                if (state.hasUncalibratedSourceLink()) {
                    const auto* uncalib = dynamic_cast<const xAOD::MuonMeasurement*>(xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink()));
                    if (!uncalib) {
                        return;
                    }
                    // for the combined sTgc space point we have to fill the primary and secondary measuremment seperately to resolve the strip/pad/wire combinations
                    if(uncalib->numDimensions() == 0) {
                        const auto* combinedMeas = dynamic_cast<const xAOD::CombinedMuonStrip*>(uncalib);
                        incrementSummary(combinedMeas->primaryStrip()->identify(), status, combinedMeas->primaryStrip()->numDimensions(), summary);
                        incrementSummary(combinedMeas->secondaryStrip()->identify(), status, combinedMeas->secondaryStrip()->numDimensions(), summary);
                    } else {
                        incrementSummary(uncalib->identify(), status, uncalib->numDimensions(), summary);
                        complementaryHole(uncalib->identify(), uncalib->readoutElement(), summary);
                    }
                } else if (state.hasReferenceSurface()) {
                    const Acts::Surface& surf{state.referenceSurface()};
                    /// Surface is not active
                    if (!surf.isSensitive() || !surf.isAlignable()) {
                        return;
                    }
                    const auto* detEl = dynamic_cast<const ActsTrk::ISurfacePlacement*>(surf.surfacePlacement());
                    if (!detEl) {
                        return;
                    }
                    incrementSummary(detEl->identify(), status, 1, summary);
                    complementaryHole(detEl->identify(), 
                        dynamic_cast<const MuonGMR4::MuonReadoutElement*>(detEl->detectorElement()), 
                        summary);
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
            switch(m_idHelperSvc->stgcIdHelper().channelType(hitId)) {
                case sTgcIdHelper::sTgcChannelTypes::Pad:{
                    cat1 = Cat_t::sTgcPad;
                    break;
                } case sTgcIdHelper::sTgcChannelTypes::Wire: {
                    cat1 = Cat_t::TriggerPhi;
                    break;
                }  case sTgcIdHelper::sTgcChannelTypes::Strip:{
                    cat1 = Cat_t::Precision;
                    break;
                } default: {
                    ATH_MSG_ERROR(__FILE__ << ":" << __LINE__ << " Unknown stgc channel type");
                    break;
                }
            }            
        } else {
            ATH_MSG_ERROR(__FILE__ << ":" << __LINE__ << "  Unkown technology index "<<techIdx);
            return;
        }

        if (cat1 != Cat_t::nCategories){
            ATH_MSG_VERBOSE("Increment "<<cat1<<", "<<status<<", layer: "
                <<layer<<", small: "<<(small ? "yes" : "no"));
            ++summary.value(cat1, status, layer, small);
        }
        if (cat2 != Cat_t::nCategories) {
            ATH_MSG_VERBOSE("Increment "<<cat2<<", "<<status<<", layer: "
                    <<layer<<", small: "<<(small ? "yes" : "no"));
            ++summary.value(cat2, status, layer, small);
        }
    }

    HitSummary TrackSummaryTool::makeSummary(const EventContext& /*ctx*/,
                                             std::span<const xAOD::MuonSegment* const> segments) const {
        HitSummary summary{};
        for (const xAOD::MuonSegment* seg : segments) {
            const LayerIndex lay = toLayerIndex(seg->chamberIndex());
            const bool small = isSmall(seg->chamberIndex());
            if (!m_reDoSegments) {
                summary.value(Cat_t::Precision,  Stat_t::OnTrack, lay, small) = seg->nPrecisionHits();
                summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, lay, small) = seg->nTrigEtaLayers();
                summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, lay, small) = seg->nPhiLayers();

                summary.value(Cat_t::Precision,  Stat_t::Outlier, lay, small) = seg->nPrecisionOutliers();
                summary.value(Cat_t::TriggerEta, Stat_t::Outlier, lay, small) = seg->nTriggerEtaOutliers();
                summary.value(Cat_t::TriggerPhi, Stat_t::Outlier, lay, small) = seg->nTriggerPhiOutliers();

                summary.value(Cat_t::Precision,  Stat_t::Hole, lay, small) = seg->nPrecisionHoles();
                summary.value(Cat_t::TriggerEta, Stat_t::Hole, lay, small) = seg->nTriggerEtaHoles();
                summary.value(Cat_t::TriggerPhi, Stat_t::Hole, lay, small) = seg->nTriggerPhiHoles();

            } else {
                const std::size_t nHits = nMeasurements(*seg);
                for (std::size_t hit = 0; hit < nHits; ++hit) {
                    Stat_t state = isOutlierMeasurement(*seg, hit) ? Stat_t::Outlier : Stat_t::OnTrack;
                    const auto* uncalibMeas = dynamic_cast<const xAOD::MuonMeasurement*>(getMeasurement(*seg, hit));
                    // for the combined sTgc space point we have to fill the primary and secodnray measuremment seperately to resolve the strip/pad/wire combinations
                    if(uncalibMeas->type() == xAOD::UncalibMeasType::sTgcStripType && uncalibMeas->numDimensions() == 0){
                        const auto* combinedMeas = static_cast<const xAOD::CombinedMuonStrip*>(uncalibMeas);
                        incrementSummary(combinedMeas->primaryStrip()->identify(), state, combinedMeas->primaryStrip()->numDimensions(), summary);
                        incrementSummary(combinedMeas->secondaryStrip()->identify(), state, combinedMeas->secondaryStrip()->numDimensions(), summary);
                        
                    } else {
                        incrementSummary(uncalibMeas->identify(), state, uncalibMeas->numDimensions(), summary);
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
            acc(innerClosePrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Inner, false)
                                         + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Inner, true);

            acc(middleClosePrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Middle, false)
                                          + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Middle, true);

            acc(outerClosePrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Outer, false)
                                         + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Outer, true);

            acc(extendedClosePrecisionHits) = summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Extended, false)
                                            + summary.value(Cat_t::Precision, Stat_t::Outlier, LayerIndex::Extended, true);
        }

        /// Trigger hits
        acc(innerTriggerEtaHits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Inner, true)
                                 + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Inner, false);

        acc(middleTriggerEtaHits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Middle, true)
                                  + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Middle, false);

        acc(outerTriggerEtaHits) = summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Outer, true)
                                 + summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, LayerIndex::Outer, false);


         if (m_fillHoles) {
            acc(innerTriggerEtaHoles) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Inner, true)
                                      + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Inner, false);

            acc(middleTriggerEtaHoles) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Middle, true)
                                       + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Middle, false);

            acc(outerTriggerEtaHoles) = summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Outer, true)
                                      + summary.value(Cat_t::TriggerEta, Stat_t::Hole, LayerIndex::Outer, false);
        }
        acc(innerTriggerPhiHits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Inner, true)
                                 + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Inner, false);

        acc(middleTriggerPhiHits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Middle, true)
                                  + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Middle, false);

        acc(outerTriggerPhiHits) = summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Outer, true)
                                 + summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, LayerIndex::Outer, false);

        if (m_fillHoles) {
            acc(innerTriggerPhiHoles) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Inner, true)
                                      + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Inner, false);

            acc(middleTriggerPhiHoles) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Middle, true)
                                       + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Middle, false);

            acc(outerTriggerPhiHoles) = summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Outer, true)
                                      + summary.value(Cat_t::TriggerPhi, Stat_t::Hole, LayerIndex::Outer, false);
        }
    }     
}