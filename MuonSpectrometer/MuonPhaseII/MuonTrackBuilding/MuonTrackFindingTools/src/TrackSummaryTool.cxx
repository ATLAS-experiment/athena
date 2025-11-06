/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackSummaryTool.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "Acts/Utilities/StringHelpers.hpp"

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

              Cat_t cat1{Cat_t::nCategories}, cat2{Cat_t::nCategories};
              Stat_t status{Stat_t::OnTrack};
              if (state.typeFlags().test(Acts::TrackStateFlag::OutlierFlag)){
                  status = Stat_t::Outlier;
              } else if (state.typeFlags().test(Acts::TrackStateFlag::HoleFlag)) {
                  status = Stat_t::Hole;
              }
              LayerIndex layer{LayerIndex::LayerUnknown};
              Identifier hitId{};
              unsigned nDim{1};
              bool small{false};
              if (state.hasUncalibratedSourceLink()) {
                  const xAOD::UncalibratedMeasurement* meas = xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
                  hitId = xAOD::identify(meas);
                  nDim = meas->numDimensions();
              } else if (state.hasReferenceSurface()) {
                  const Acts::Surface& surf{state.referenceSurface()};
                  /// Surface is not active
                  const Acts::DetectorElementBase* detEl = surf.associatedDetectorElement();
                  if (!detEl) {
                    return;
                  }
                  hitId = static_cast<const ActsTrk::IDetectorElementBase*>(detEl)->identify();
                  status = Stat_t::Hole;
              }

              using TechIdx = Muon::MuonStationIndex::TechnologyIndex;
              const TechIdx techIdx = m_idHelperSvc->technologyIndex(hitId);
              /// Measurement is not muon
              if (techIdx == Muon::MuonStationIndex::TechnologyIndex::TechnologyUnknown){
                  return;
              }
              ATH_MSG_VERBOSE("Write summary for "<<m_idHelperSvc->toString(hitId));

              const ChIndex chIdx = m_idHelperSvc->chamberIndex(hitId);
              layer = toLayerIndex(chIdx);
              small = isSmall(chIdx);
              
              if (techIdx == TechIdx::MDT || techIdx == TechIdx::MM) {
                  cat1 = Cat_t::Precision;
                  /// Mdt twin tubes
                  if (nDim == 2){
                      cat2 = Cat_t::TriggerPhi;
                  }
              } else if (techIdx == TechIdx::RPC || techIdx == TechIdx::TGC) {
                /// Combined measurement or it's a 2D BI strip
                if (nDim == 0 || nDim == 2) {
                    cat1 = Cat_t::TriggerEta;
                    cat2 = Cat_t::TriggerPhi;
                } else if (m_idHelperSvc->measuresPhi(hitId)) {
                    cat1 = Cat_t::TriggerPhi;
                } else {
                    cat1 = Cat_t::TriggerEta;
                }
              } else {
                  ATH_MSG_ALWAYS(__FILE__<<":"<<__LINE__<<" Implement sTGC!");
                  return;
              }

              if (cat1 != Cat_t::nCategories){
                ATH_MSG_VERBOSE("Increment "<<summary.toString(cat1)<<", layer: "
                  <<Muon::MuonStationIndex::layerName(layer)<<", small: "<<(small ? "yes" : "no"));
                  ++summary.value(cat1, status, layer, small);
              }
              if (cat2 != Cat_t::nCategories) {
                ATH_MSG_VERBOSE("Increment "<<summary.toString(cat2)<<", layer: "
                  <<Muon::MuonStationIndex::layerName(layer)<<", small: "<<(small ? "yes" : "no"));
                ++summary.value(cat2, status, layer, small);
              }
        });
        ATH_MSG_DEBUG("Obtained track summary from track with "<<Acts::toString(trackProxy.fourMomentum())
                      <<", q: "<<trackProxy.qOverP()
                      <<", chi2: "<<(trackProxy.chi2()/ std::max(trackProxy.nDoF(), 1u))
                      <<", nDoF: "<<trackProxy.nDoF()<<"\n"<<summary);
        return summary;                   
    }

    HitSummary TrackSummaryTool::makeSummary(const EventContext& /*ctx*/,
                                             const MsTrackSeed& seed) const {
        HitSummary summary{};
        for (const xAOD::MuonSegment* seg : seed.segments()){
           const LayerIndex lay = toLayerIndex(seg->chamberIndex());
           const bool small = isSmall(seg->chamberIndex());
           summary.value(Cat_t::Precision, Stat_t::OnTrack, lay, small) = seg->nPrecisionHits();
           summary.value(Cat_t::TriggerEta, Stat_t::OnTrack, lay, small) = seg->nTrigEtaLayers();
           summary.value(Cat_t::TriggerPhi, Stat_t::OnTrack, lay, small) = seg->nPhiLayers();
        }
        return summary;
    }
            
    void TrackSummaryTool::copySummary(const HitSummary& summary,
                                       xAOD::IParticle& track) const {
        ATH_MSG_DEBUG("Copy the summary \n "<<summary<<"\n pT: "<<track.pt()
                      <<", eta: "<<track.eta()<<", phi: "<<track.phi());
        ATH_MSG_ALWAYS(__FILE__<<":"<<__LINE__<<" Implement me");
    }     
}