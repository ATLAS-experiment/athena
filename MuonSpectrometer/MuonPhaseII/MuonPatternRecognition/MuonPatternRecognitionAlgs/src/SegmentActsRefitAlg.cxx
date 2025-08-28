/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentActsRefitAlg.h"

#include "ActsCalibBase/CalibrationContext.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
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
#include "xAODTracking/TrackSurfaceAuxContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
namespace{
    constexpr double straightQoverP = 1. / (100. *Gaudi::Units::TeV);
    constexpr double pseudoSurfDist = 5.*Gaudi::Units::cm;
    using ProjectorType = ActsTrk::detail::MeasurementCalibratorBase::ProjectorType;
}

namespace MuonR4{
    using namespace SegmentFit;

    StatusCode SegmentActsRefitAlg::initialize(){
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_linkKey.initialize());
        ATH_CHECK(m_localParsKey.initialize());
        ATH_CHECK(m_seedParsKey.initialize());
        ATH_CHECK(m_surfKey.initialize());
        ATH_CHECK(m_auxMeasProv.initialize(m_writeKey.key()));

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
        
        SG::WriteHandle surfaceHandle{m_surfKey, ctx};
        ATH_CHECK(surfaceHandle.record(std::make_unique<xAOD::TrackSurfaceContainer>(),
                                       std::make_unique<xAOD::TrackStateAuxContainer>()));

        ActsTrk::detail::xAODUncalibMeasSurfAcc surfAcc{};
        auto auxMeasHandle = m_auxMeasProv.makeHandle(ctx, *surfaceHandle);

        using Link_t = ElementLink<xAOD::MuonSegmentContainer>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, Link_t> dec_segLink{m_linkKey, ctx};
        using ParDecor_t = SG::WriteDecorHandle<xAOD::MuonSegmentContainer, 
                                                xAOD::MeasVector<toInt(ParamDefs::nPars)>>;
        
        ParDecor_t dec_locPars{m_localParsKey, ctx};
        ParDecor_t dec_seedPars{m_seedParsKey, ctx};
        /// Loop over the segment container
        for (const xAOD::MuonSegment* seg: *segments){
            const MuonR4::Segment* reFitMe = MuonR4::detailedSegment(*seg);
            const Amg::Transform3D& sectorTrf{reFitMe->msSector()->localToGlobalTrans(gctx)};
            const GeoTrf::CoordEulerAngles sectorAngles = GeoTrf::getCoordRotationAngles(sectorTrf);
            /// Fetch the measurements
            std::vector<const xAOD::UncalibratedMeasurement*> startMeas = MuonR4::collectMeasurements(*reFitMe);
            
            const auto* refMeas = startMeas.front();

            const Amg::Vector3D firstSurfPos{surfAcc.get(refMeas)->transform(tgContext).translation()};
            if (!reFitMe->summary().nPhiHits) {
                const Amg::Vector3D planeNormal = sectorTrf.linear().col(2);

                const Amg::Vector3D lastSurfPos = surfAcc.get(startMeas.back())->transform(tgContext).translation();
                /// We add two pseudo measurements above & beneath the segment to stabilize the  fit
                
                const Amg::Transform3D trfBeneath = GeoTrf::GeoTransformRT{sectorAngles, firstSurfPos - pseudoSurfDist * planeNormal}; 
                const Amg::Transform3D trfAbove   = GeoTrf::GeoTransformRT{sectorAngles, lastSurfPos + pseudoSurfDist * planeNormal}; 

                auto surfBeneath = Acts::Surface::makeShared<Acts::PlaneSurface>(trfBeneath);
                auto surfAbove   = Acts::Surface::makeShared<Acts::PlaneSurface>(trfAbove);
                const double covVal = std::pow(1.*Gaudi::Units::cm, 2);
                startMeas.insert(startMeas.begin(), auxMeasHandle.newMeasurement<1>(surfBeneath, ProjectorType::e1DimNoTime, AmgSymMatrix(1){covVal}));
                startMeas.insert(startMeas.end(), auxMeasHandle.newMeasurement<1>(surfAbove, ProjectorType::e1DimNoTime, AmgSymMatrix(1){covVal}));

            } else if (const auto& firstMeas = reFitMe->measurements().front(); firstMeas->type() == xAOD::UncalibMeasType::Other) {
                auto pseudoSurf = Acts::Surface::makeShared<Acts::PlaneSurface>(
                                    GeoTrf::GeoTransformRT{sectorAngles, Amg::Vector3D::Zero()});
                startMeas.insert(startMeas.begin(), auxMeasHandle.newMeasurement<2>(pseudoSurf, ProjectorType::e2DimNoTime, AmgSymMatrix(2)::Identity()));
            }
            /// Global chi2 fitter runs only with at least 5 measurements
            if (startMeas.size() < 5){
                continue;
            }
            /// Fetch a smeared segment position & direction
            const auto [pos, dir] = smearSegment(gctx,*reFitMe, randEngine);
            ///

            /// Construct the reference surface before the first measurement
            const double extDist = dir.dot(firstSurfPos - pos) - 5.*Gaudi::Units::cm;
            /// Reference position of the surface.
            const Amg::Vector3D refPos = pos + extDist * dir;
            const Amg::Transform3D trf{GeoTrf::GeoTransformRT{sectorAngles, refPos}};
            if (msgLvl(MSG::VERBOSE)) {
                const auto [locPos, locDir] = makeLine(localSegmentPars(gctx, *reFitMe));

                std::stringstream sstr{};
                sstr<<"pos: "<<Amg::toString(pos)<<", dir: "<<Amg::toString(dir)<<", chi2/nDoF: "
                    <<reFitMe->chi2() / reFitMe->nDoF()<<", nDoF: "<<reFitMe->nDoF()<<", "
                    <<reFitMe->summary().nPrecHits<<", "<<reFitMe->summary().nPhiHits<<std::endl;
                for (const xAOD::UncalibratedMeasurement* meas : startMeas) {
                    const auto calib_sp = std::ranges::find_if(reFitMe->measurements(), [meas](const auto& sp) {
                        if (!sp->spacePoint()) return false;
                        return sp->spacePoint()->primaryMeasurement() == meas || 
                               sp->spacePoint()->secondaryMeasurement() == meas;
                    });
                    sstr<<" **** "<<m_idHelperSvc->toString(xAOD::identify(meas))<<" @ "
                         <<Amg::toString(surfAcc.get(meas)->transform(tgContext).translation())
                         <<", "<<surfAcc.get(meas)->geometryId()<<", "
                         <<(calib_sp != reFitMe->measurements().end() ? 
                                SegmentFitHelpers::chiSqTerm(locPos, locDir,0., std::nullopt, 
                                                             **calib_sp, msgStream()) : 0.)
                         <<std::endl;
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
            Acts::BoundMatrix initialCov{Acts::BoundMatrix::Identity()};

            auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, dir, straightQoverP,
                                                                  initialCov, Acts::ParticleHypothesis::muon());
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
                ATH_MSG_VERBOSE("Loop over track state: "<<(itr++)<<", "<<m_idHelperSvc->toString(xAOD::identify(goodMeas.front()))
                                <<", id: "<<surfAcc.get(goodMeas.front())->geometryId());

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
            /// Seed paramters
            const Amg::Vector3D locSeedPos = globToLoc * pos;
            const Amg::Vector3D locSeedDir = globToLoc.linear() * dir;
            dec_seedPars(*newSegment)[toInt(ParamDefs::x0)] = locSeedPos.x();
            dec_seedPars(*newSegment)[toInt(ParamDefs::y0)] = locSeedPos.y();
            dec_seedPars(*newSegment)[toInt(ParamDefs::theta)] = locSeedDir.theta();
            dec_seedPars(*newSegment)[toInt(ParamDefs::phi)] = locSeedDir.phi();

        }
        return StatusCode::SUCCESS;
    }
}
