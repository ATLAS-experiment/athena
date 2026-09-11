/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthTrackSeederTool.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "StoreGate/ReadHandle.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Definitions/Units.hpp"


#include "TruthUtils/AtlasPID.h"

using namespace Acts::UnitLiterals;

namespace {
    /** @brief Count the hits on segment including outliers */
    std::uint8_t countHits(const xAOD::MuonSegment& s) {
        return s.nPrecisionHits() + s.nTrigEtaLayers() + s.nPhiLayers() +
               s.nPrecisionOutliers() + s.nTriggerEtaOutliers() + s.nTriggerPhiOutliers();
    }
}

namespace MuonR4 {
    StatusCode TruthTrackSeederTool::initialize() {
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_ctxProvider.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode TruthTrackSeederTool::findTrackSeeds(const EventContext& ctx,
                                                     std::vector<MsTrackSeed>& outSeeds) const {
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
        std::unordered_map<const xAOD::TruthParticle*, std::vector<const xAOD::MuonSegment*>> truthSeeds{};
        /** Fill the truth seed map*/
        for (const xAOD::MuonSegment* seg : *segments) {
            const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*seg);
            if (truthPart && truthPart->isMuon()) {
                truthSeeds[truthPart].push_back(seg);
            }    
        }
        for (auto& [truthPart, assocSegs] : truthSeeds) {
            std::ranges::sort(assocSegs, [](const xAOD::MuonSegment* a, const xAOD::MuonSegment* b){
                if (a->chamberIndex() != b->chamberIndex()) {
                    return a->chamberIndex() < b->chamberIndex();
                }
                return countHits(*a) > countHits(*b);
            });
            /// Remove duplicate segments on the same station
            auto [begin, end] = std::ranges::unique(assocSegs, [](const xAOD::MuonSegment* a, const xAOD::MuonSegment* b){
                return a->chamberIndex() == b->chamberIndex();
            });
            assocSegs.erase(begin,end);
            if (assocSegs.size() < 2) {
                continue;
            }
            MsTrackSeed& seed = outSeeds.emplace_back(MsTrackSeed::Location::Barrel,
                                                      ExpandedSector{truthPart->phi()});
            std::ranges::for_each(assocSegs, [&seed](const xAOD::MuonSegment* seg) {
                seed.addSegment(seg);
            });
        }

        return StatusCode::SUCCESS;
    }
            
    Acts::Result<Acts::BoundTrackParameters> 
        TruthTrackSeederTool::estimateStartParameters(const EventContext& ctx,
                                                      const MsTrackSeed& seed) const {

        const Acts::GeometryContext tgContext{m_ctxProvider.getGeometryContext(ctx)};
        /// Get the first two segments
        const xAOD::MuonSegment* firstSeg = seed.segments().front();
        const xAOD::MuonSegment* secondSeg = seed.segments()[1];
        // If both segments are on the same station (BI) check whether the order needs to be swapped
        if (Muon::MuonStationIndex::toStationIndex(firstSeg->chamberIndex()) ==
            Muon::MuonStationIndex::toStationIndex(secondSeg->chamberIndex())) {
            const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(*firstSeg);
            if (!truthSeg) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No truth segment found");
                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            const Acts::Surface& firstSurf{xAOD::muonSurface(firstMeasurement(*firstSeg, false))};
            const Acts::Surface& secondSurf{xAOD::muonSurface(firstMeasurement(*secondSeg, false))}; 
            const double distA = firstSurf.intersect(tgContext, truthSeg->position(), truthSeg->direction()).closest().pathLength();
            const double distB = secondSurf.intersect(tgContext, truthSeg->position(), truthSeg->direction()).closest().pathLength();
            ATH_MSG_VERBOSE(__func__<<" "<<__LINE__<<" - Detected segments in sector overlap "
                    <<printID(*firstSeg)<<" & "<<printID(*secondSeg)
                    <<". Check whether they need  to be swapped "<<distA<<" vs. "<<distB);
            if (distA> distB) {
                std::swap(firstSeg, secondSeg);
            }
        }
        const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(*firstSeg);
        if (!truthSeg) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No truth segment found");
            return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
        }
        const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*truthSeg);
        if (!truthPart) {
             ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No truth particle found");
            return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
        }
        const MuonGMR4::SpectrometerSector* msSector{m_detMgr->getSectorEnvelope(truthSeg->chamberIndex(),
                                                                    truthSeg->sector(),
                                                                    truthSeg->etaIndex())};
        const Acts::Surface& sectorSurf{msSector->surface()};
        const Amg::Vector3D firstPos{atFirstSurface(tgContext, *firstSeg, false)};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - First measurement "
            <<msSector->idHelperSvc()->toString(xAOD::identify(firstMeasurement(*firstSeg, false)))
            <<", position: "<<Amg::toString(firstPos));

        const Amg::Transform3D& oldTrf{sectorSurf.localToGlobalTransform(tgContext)};
        const Amg::Vector3D segDir = truthSeg->direction();
        const Amg::Vector3D locDir = oldTrf.inverse().linear() * segDir;
        const double pathLength = std::abs((truthSeg->position() - firstPos).dot(segDir)) + 10._cm;

        const Amg::Transform3D newTrf{oldTrf * Amg::getTranslate3D(-pathLength * locDir)};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Create new surface in front of "<<msSector->identString()
                                <<", "<<Amg::toString(newTrf));
        auto shiftedSurf = Acts::Surface::makeShared<Acts::PlaneSurface>(newTrf);

        Acts::BoundVector locPars = SegmentFit::boundSegmentPars(tgContext, *m_detMgr, *truthSeg).parameters();
        static const SG::ConstAccessor<float> acc_pt{"pt"};
        locPars[Acts::eBoundQOverP] = truthPart->charge() / ActsTrk::energyToActs(acc_pt(*truthSeg) * std::cosh(truthPart->eta()));
        Acts::BoundTrackParameters result{shiftedSurf, locPars,
                                          Acts::BoundMatrix::Identity(), 
                                          Acts::ParticleHypothesis::muon()};
        using namespace Acts::detail::LineHelper;
        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream outStream{};
            outStream<<"Start parameters:\n"<<result<<",\n"
                    <<result.referenceSurface().toString(tgContext)
                    <<"absolute momentum: "
                    <<result.absoluteMomentum()<<"/"<<ActsTrk::energyToActs(truthPart->pt()* std::cosh(truthPart->eta()))
                    <<", start point closure: "<<(result.position(tgContext) + pathLength * segDir - truthSeg->position()).mag()
                    <<", angle closure: "<<Amg::angle(result.direction(), segDir) / 1._degree
                    <<"\n\ndump measurements:\n";

            for (const xAOD::MuonSegment* segment : seed.segments()) {
                outStream<<" - Segment "<<printID(*segment)<<" @ "<<Amg::toString(segment->position())<<"+"
                         <<Amg::toString(segment->direction())<<std::endl;
                for (unsigned int m = 0; m < nMeasurements(*segment); ++m) {
                    const xAOD::UncalibratedMeasurement* meas = getMeasurement(*segment, m);
                    if (meas->type() == xAOD::UncalibMeasType::Other) {
                        continue;
                    }
                    const auto* mMeas = dynamic_cast<const xAOD::MuonMeasurement*>(meas);
                    const Acts::Surface& surf{xAOD::muonSurface(meas)};
                    const double dist{surf.intersect(tgContext, result.position(tgContext), segDir, 
                                                      Acts::BoundaryTolerance::Infinite()).closest().pathLength()};
                    outStream<<" *** "<<m_detMgr->idHelperSvc()->toString(mMeas->identify())
                             <<" @ "<<Amg::toString(surf.localToGlobalTransform(tgContext) * mMeas->localMeasurementPos())
                             <<", center: "<<Amg::toString(surf.center(tgContext))
                             <<", travelled distance: "<<dist<<std::endl;
                }
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Created start parameters:\n"<<outStream.str());
        }
        return result;                                             
    }

            
    double TruthTrackSeederTool::estimateQtimesP(const EventContext&/* ctx*/,
                                              const Amg::Vector3D& /*planeNorm*/,
                                           std::span<const PosMomPair_t> /*circlePoints*/) const {
        ATH_MSG_INFO(__func__<<"() Will be removed soon");
        return 0.;
    } 
}
