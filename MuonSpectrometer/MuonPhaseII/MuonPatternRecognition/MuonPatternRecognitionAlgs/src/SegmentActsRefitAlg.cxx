/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentActsRefitAlg.h"

#include "ActsCalibration/CalibrationContext.h"
#include "ActsCalibration/xAODUncalibMeasCalibrator.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "StoreGate/WriteDecorHandle.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "GaudiKernel/PhysicalConstants.h"
#include <AthenaKernel/RNGWrapper.h>
#include "CLHEP/Random/RandGaussZiggurat.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
namespace{
    constexpr double straightQoverP = 1. / (100. *Gaudi::Units::TeV);
}

namespace MuonR4{
    using namespace SegmentFit;

    StatusCode SegmentActsRefitAlg::initialize(){
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_linkKey.initialize());
        ATH_CHECK(m_localParsKey.initialize());

        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_segSelector.retrieve());

        return StatusCode::SUCCESS;
    }
    std::tuple<Amg::Vector3D, Amg::Vector3D> 
        SegmentActsRefitAlg::smearSegment(const ActsGeometryContext& gctx,
                                          const MuonR4::Segment& segment,
                                          CLHEP::HepRandomEngine* engine) const{
        auto segPars = localSegmentPars(gctx, segment);
        /// Smear the parameters
        for (ParamDefs precPar : {ParamDefs::y0, ParamDefs::theta,
                                  ParamDefs::x0, ParamDefs::phi}) { 
            if (precPar == ParamDefs::x0 && !segment.summary().nPhiHits) {
                break;
            }
            const unsigned idx = toInt(precPar);           
            segPars[idx] = CLHEP::RandGaussZiggurat::shoot(engine, segPars[idx], 
                                                           m_smearRange*Amg::error(segment.covariance(), idx));
        }
        const auto [locPos, locDir] = makeLine(segPars);
        const Amg::Transform3D& locToGlob{segment.msSector()->localToGlobalTrans(gctx)};
        return std::make_tuple(locToGlob*locPos, locToGlob.linear() * locDir);
    }
    StatusCode SegmentActsRefitAlg::execute(const EventContext& ctx) const {
        
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_readKey, ctx));
        /// Create the context object
        const ActsGeometryContext& gctx{m_trackingGeometryTool->getGeometryContext(ctx)};
        const Acts::GeometryContext tgContext = gctx.context();
        const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
        const Acts::CalibrationContext calContext = ActsTrk::getCalibrationContext(ctx);
        /// Random engine to smear the segment parameters
        ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, name());
        rngWrapper->setSeed(name(), ctx);
        CLHEP::HepRandomEngine* randEngine = rngWrapper->getEngine(ctx);

        SG::WriteHandle outHandle{m_writeKey, ctx};
        ATH_CHECK(outHandle.record(std::make_unique<xAOD::MuonSegmentContainer>(),
                                   std::make_unique<xAOD::MuonSegmentAuxContainer>()));

        using Link_t = ElementLink<xAOD::MuonSegmentContainer>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, Link_t> dec_segLink{m_linkKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, 
                             xAOD::MeasVector<toInt(ParamDefs::nPars)>> dec_locPars{m_localParsKey, ctx};
        /// Loop over the segment container
        for (const xAOD::MuonSegment* seg: *segments){
            const MuonR4::Segment* reFitMe = MuonR4::detailedSegment(*seg);

            /// Fetch the measurements
            const std::vector<const xAOD::UncalibratedMeasurement*> startMeas = MuonR4::collectMeasurements(*reFitMe);
            /// Global chi2 fitter runs only with at least 5 measurements
            if (startMeas.size() < 5){
                continue;
            }
            /// Fetch a smeared segment position & direction
            const auto [pos, dir] = smearSegment(gctx,*reFitMe, randEngine);
            ///
            const Amg::Vector3D firstSurfPos{xAOD::muonSurface(startMeas.front()).transform(tgContext).translation()};
            /// Construct the reference surface before the first measurement
            const double extDist = dir.dot(firstSurfPos - pos) - 5.*Gaudi::Units::cm;
            /// Reference position of the surface.
            const Amg::Vector3D refPos = pos + extDist * dir;
            const Amg::Vector3D locRefPos = reFitMe->msSector()->globalToLocalTrans(gctx)*refPos;
            const Amg::Transform3D trf{reFitMe->msSector()->localToGlobalTrans(gctx) * Amg::getTranslate3D(locRefPos)};
            if (msgLvl(MSG::VERBOSE)) {
                std::stringstream sstr{};
                sstr<<"pos: "<<Amg::toString(pos)<<", dir: "<<Amg::toString(dir)<<", chi2/nDoF: "
                    <<reFitMe->chi2() / reFitMe->nDoF()<<", nDoF: "<<reFitMe->nDoF()<<", "
                    <<reFitMe->summary().nPrecHits<<", "<<reFitMe->summary().nPhiHits<<std::endl;
                for (const xAOD::UncalibratedMeasurement* meas : startMeas) {
                    sstr<<" **** "<<m_idHelperSvc->toString(xAOD::identify(meas))<<" @ "
                         <<Amg::toString(xAOD::muonSurface(meas).transform(tgContext).translation())<<std::endl;
                }
                sstr<<" Target surf: "<<Amg::toString(trf)<<", firstSurf: "<< Amg::toString(trf.inverse()*firstSurfPos)
                     <<", refPoint: "<<Amg::toString(trf.inverse()*refPos)<<std::endl;
                ATH_MSG_VERBOSE("Run G2F fit on "<<reFitMe->msSector()->identString()<<std::endl<<sstr.str());
            }
            /// Plane surface
            auto target = Acts::Surface::makeShared<Acts::PlaneSurface>(trf);

            Acts::ActsVector<4> fourPos{};
            fourPos.block<3,1>(Acts::ePos0, 0) = refPos;
            fourPos[Acts::eTime] = refPos.mag() / Gaudi::Units::c_light;
            auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, dir, straightQoverP,
                                                                  std::nullopt, Acts::ParticleHypothesis::muon());
            if (!initialPars.ok()) {
                ATH_MSG_WARNING("Initial estimate of the parameters failed");
                continue;
            }
            auto fitTraject = m_trackFitTool->fit(startMeas, *initialPars, tgContext, mfContext, calContext, target.get());
            if (!fitTraject) {
                ATH_MSG_WARNING("Track fit failed.");
                continue;
            }
            auto track = fitTraject->getTrack(0);
            ATH_MSG_DEBUG("Track fit succeeded. ");

            MuonR4::Segment::HitSummary summary{};
            /// Fetch the measurements from the track state
            std::vector<const xAOD::UncalibratedMeasurement*> goodMeas{};
            unsigned int itr{0};
            fitTraject->trackStateContainer().visitBackwards(track.tipIndex(),[&](const auto& state){
                if (!state.hasUncalibratedSourceLink()){
                    return;
                }
                goodMeas.insert(goodMeas.begin(),
                                ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink()));
                ATH_MSG_VERBOSE("Loop over track state: "<<(itr++)<<", "<<m_idHelperSvc->toString(xAOD::identify(goodMeas.front())));

                summary.nPrecHits += (goodMeas.front()->type() == xAOD::UncalibMeasType::MdtDriftCircleType);
                if (m_idHelperSvc->measuresPhi(xAOD::identify(goodMeas.front()))){
                    ++summary.nPhiHits;
                } else {
                    summary.nEtaTrigHits += (goodMeas.front()->type() != xAOD::UncalibMeasType::MdtDriftCircleType);
                }
            });

            Acts::BoundTrackParameters parameters = track.createParametersAtReference();

            /// Direction is always expressed in global frame -> transform to local
            const Amg::Vector3D globDir = parameters.direction();
            /// Express the parameters at the reference surface of the original segment
            const Amg::Transform3D globToLoc{reFitMe->msSector()->globalToLocalTrans(gctx)};
            const Amg::Vector3D refitPos = globToLoc * parameters.position(tgContext);
            const Amg::Vector3D refitDir = globToLoc.linear() * globDir;
            /// Straight line extension to plane
            const Amg::Vector3D refitSeg = refitPos + Amg::intersect<3>(refitPos, refitDir, Amg::Vector3D::UnitZ(), 0).value_or(0.) * refitDir;
            const Amg::Vector3D globPos{reFitMe->msSector()->localToGlobalTrans(gctx) * refitSeg};

            auto newSegment = outHandle->push_back(std::make_unique<xAOD::MuonSegment>());
            dec_segLink(*newSegment) = Link_t{*segments, seg->index(), ctx};

            newSegment->setDirection(globDir.x(), globDir.y(), globDir.z());
            newSegment->setPosition(globPos.x(), globPos.y(), globPos.z());
            
            newSegment->setFitQuality(track.chi2(), track.nDoF());
            newSegment->setNHits(summary.nPrecHits, summary.nPhiHits, summary.nEtaTrigHits);
            dec_locPars(*newSegment)[toInt(ParamDefs::x0)] = refitSeg.x();
            dec_locPars(*newSegment)[toInt(ParamDefs::y0)] = refitSeg.y();
            dec_locPars(*newSegment)[toInt(ParamDefs::theta)] = refitDir.theta();
            dec_locPars(*newSegment)[toInt(ParamDefs::phi)] = refitDir.phi();
            
            
        }
        return StatusCode::SUCCESS;
    }
}