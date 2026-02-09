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
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "ActsInterop/UnitConverters.h"

using namespace Acts::UnitLiterals;
namespace{
    constexpr double straightQoverP = 1. / (100._TeV);
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
        ATH_CHECK(m_calibTool.retrieve());

        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    std::tuple<Amg::Vector3D, Amg::Vector3D> 
        SegmentActsRefitAlg::smearSegment(const ActsTrk::GeometryContext& gctx,
                                          const MuonR4::Segment& segment,
                                          CLHEP::HepRandomEngine* engine) const{
        // return std::make_pair(segment.position(), segment.direction()* (segment.direction().z() < 0 ? -1. : 1.));
        const auto segPars = localSegmentPars(gctx, segment);
        auto smearedPars = segPars;
        /// Smear the parameters
        for (ParamDefs precPar : {ParamDefs::y0, ParamDefs::theta,
                                  ParamDefs::x0, ParamDefs::phi}) { 
            if (precPar == ParamDefs::phi && !segment.summary().nPhiHits) {
                break;
            }
            const unsigned idx = Acts::toUnderlying(precPar);
            const double uncert =  Amg::error(segment.covariance(), idx) * m_smearRange;
            smearedPars[idx] = CLHEP::RandGaussZiggurat::shoot(engine, segPars[idx], uncert);
            ATH_MSG_VERBOSE("Apply smearing to "<<SeedingAux::parName(precPar)
        <<" parameter -- cov: "<<uncert
    <<", original: "<<segPars[idx]<<", smeared: "<<smearedPars[idx]<<", deviation: "
            <<(smearedPars[idx] - segPars[idx]) / uncert );

        }
        auto [smearLocPos, smearLocDir] = makeLine(smearedPars);
        const auto [locPos, locDir] = makeLine(segPars);
        /// Ensure that the left-right ambiguity is preserved
        if (SeedingAux::strawSigns(locPos,locDir, segment.measurements()) !=
            SeedingAux::strawSigns(smearLocPos, smearLocDir, segment.measurements())) {
            ATH_MSG_ALWAYS("Parameter smearng from "<<toString(segPars)<<" -> "<<toString(smearedPars)
                        <<" changes the L/R ambiguity -> avoid for this test");
            return smearSegment(gctx, segment, engine);
        }

        const Amg::Transform3D& locToGlob{segment.msSector()->localToGlobalTransform(gctx)};
        if (smearLocDir.z() < 0) {
            smearLocDir = -smearLocDir;
        }
        return std::make_tuple(locToGlob * smearLocPos, locToGlob.linear() * smearLocDir);
    }
    StatusCode SegmentActsRefitAlg::execute(const EventContext& ctx) const {
        
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_readKey, ctx));
        /// Create the context object
        const ActsTrk::GeometryContext& gctx{m_trackingGeometryTool->getGeometryContext(ctx)};
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
                                                xAOD::MeasVector<Acts::toUnderlying(ParamDefs::nPars)>>;
        
        ParDecor_t dec_locPars{m_localParsKey, ctx};
        ParDecor_t dec_seedPars{m_seedParsKey, ctx};

        Acts::ObjVisualization3D visualHelper{};

        /// Loop over the segment container
        for (const xAOD::MuonSegment* reFitMe: *segments){
            const auto msSector = m_detMgr->getSectorEnvelope(reFitMe->chamberIndex(), 
                                                              reFitMe->sector(), 
                                                              reFitMe->etaIndex());
            const Amg::Transform3D& sectorTrf{msSector->localToGlobalTransform(gctx)};

            m_calibTool->stampSignsOnMeasurements(*reFitMe);

            /// Fetch a smeared segment position & direction
            const auto [seedPos, seedDir] = smearSegment(gctx, *MuonR4::detailedSegment(*reFitMe), randEngine);
            /// Decorate the initial seed parameters to the segment
            {
                auto invTrf = sectorTrf.inverse();
                const Amg::Vector3D locSeedPos = invTrf * seedPos;
                const Amg::Vector3D locSeedDir = invTrf.linear() *  seedDir;
                auto& seedPars = dec_seedPars(*reFitMe);
                seedPars[Acts::toUnderlying(ParamDefs::x0)] = locSeedPos.x();
                seedPars[Acts::toUnderlying(ParamDefs::y0)] = locSeedPos.y();
                seedPars[Acts::toUnderlying(ParamDefs::theta)] = locSeedDir.theta();
                seedPars[Acts::toUnderlying(ParamDefs::phi)] = locSeedDir.phi();
            }

            const GeoTrf::CoordEulerAngles sectorAngles = GeoTrf::getCoordRotationAngles(sectorTrf);
            /// Fetch the measurements
            std::vector<const xAOD::UncalibratedMeasurement*> startMeas = MuonR4::collectMeasurements(*reFitMe);

            const auto* refMeas = startMeas.front();

            const Amg::Vector3D firstSurfPos{surfAcc.get(refMeas)->localToGlobalTransform(tgContext).translation()};

            if (reFitMe->nPhiLayers() < 1) {
                const Amg::Vector3D planeNormal = sectorTrf.linear().col(2);

                const Amg::Vector3D lastSurfPos = surfAcc.get(startMeas.back())->localToGlobalTransform(tgContext).translation();
                /// We add two pseudo measurements above & beneath the segment to stabilize the  fit
                
                const Amg::Transform3D trfBeneath = GeoTrf::GeoTransformRT{sectorAngles, firstSurfPos - pseudoSurfDist * planeNormal}; 
                const Amg::Transform3D trfAbove   = GeoTrf::GeoTransformRT{sectorAngles, lastSurfPos + pseudoSurfDist * planeNormal}; 

                auto surfBeneath = Acts::Surface::makeShared<Acts::PlaneSurface>(trfBeneath);
                auto surfAbove   = Acts::Surface::makeShared<Acts::PlaneSurface>(trfAbove);
                const double covVal = std::pow(10.*Gaudi::Units::cm, 2);
                startMeas.insert(startMeas.begin(), auxMeasHandle.newMeasurement<1>(surfBeneath, ProjectorType::e1DimNoTime, AmgSymMatrix(1){covVal}));
                startMeas.insert(startMeas.end(), auxMeasHandle.newMeasurement<1>(surfAbove, ProjectorType::e1DimNoTime, AmgSymMatrix(1){covVal}));

            }

            if (m_drawEvent) {
                /// Draw the reference segment as a red line
                MuonValR4::drawSegmentLine(gctx, *reFitMe, visualHelper,
                                Acts::ViewConfig{.color = {220, 0, 0}});
                MuonValR4::drawSegmentMeasurements(gctx, *reFitMe, visualHelper, Acts::s_viewSurface);
            }
            /// Construct the reference surface before the first measurement
            const Amg::Vector3D trfZ = sectorTrf.linear().col(2);
            const double extDist = Amg::intersect<3>(seedPos, seedDir, trfZ, 
                                                     firstSurfPos.dot(trfZ) - 10.*Gaudi::Units::cm).value_or(0.);
            /// Reference position of the surface.
            const Amg::Vector3D refPos = seedPos + extDist * seedDir;
            const Amg::Transform3D trf{GeoTrf::GeoTransformRT{sectorAngles, refPos}};
            if (msgLvl(MSG::VERBOSE)) {
                const auto [locPos, locDir] = makeLine(localSegmentPars(*reFitMe));

                std::stringstream sstr{};
                sstr<<"pos: "<<Amg::toString(seedPos)<<", dir: "<<Amg::toString(seedDir)<<", chi2/nDoF: "
                    <<reFitMe->chiSquared() / reFitMe->numberDoF()<<", nDoF: "<<reFitMe->numberDoF()<<", "
                    <<reFitMe->nPrecisionHits()<<", "<<reFitMe->nPhiLayers()<<std::endl;
                for (const auto& meas : MuonR4::detailedSegment(*reFitMe)->measurements()) {
                    sstr<<"  **** "<<(*meas)<<", chi2: "<<SeedingAux::chi2Term(locPos, locDir, *meas)
                        <<", sign: "<<(meas->isStraw() ? 
                                (SeedingAux::strawSign(locPos,locDir, *meas) == 1 ? "R" : "L") : "-")
                        <<", geoId: "<<(meas->spacePoint() ? surfAcc.get(meas->spacePoint()->primaryMeasurement())->geometryId()
                                                 : Acts::GeometryIdentifier{})<<std::endl;
                }

                sstr<<" Target surf: "<<Amg::toString(trf)<<", firstSurf: "<< Amg::toString(trf.inverse()*firstSurfPos)
                     <<", refPoint: "<<Amg::toString(trf.inverse()*refPos)<<std::endl;
                ATH_MSG_VERBOSE("Run G2F fit on "<<msSector->identString()<<std::endl<<sstr.str());
            }
            /// Plane surface
            auto target = Acts::Surface::makeShared<Acts::PlaneSurface>(trf);

            auto fourPos{ActsTrk::convertPosToActs(refPos, refPos.mag() / Gaudi::Units::c_light)};
            Acts::BoundMatrix initialCov{Acts::BoundMatrix::Identity()};

            auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, seedDir, straightQoverP,
                                                                  initialCov, Acts::ParticleHypothesis::muon());
            if (!initialPars.ok()) {
                ATH_MSG_WARNING("Initial estimate of the parameters failed");
                continue;
            }
            if (m_drawEvent) {
                MuonValR4::drawBoundParameters(gctx, *initialPars, visualHelper,
                                               Acts::ViewConfig{.color={0,220,0}});
            }
            ATH_MSG_DEBUG("Initial parameters "<<Amg::toString((*initialPars).parameters()));
            auto fitTraject = m_trackFitTool->fit(startMeas, *initialPars, 
                                                  tgContext, mfContext, calContext, target.get());
            if (!fitTraject) {
                ATH_MSG_WARNING("Track fit failed.");
                if (m_drawEvent) {
                    visualHelper.write(std::format("SegmentReFitTest_failed_{:}_{:}_{:}.obj", 
                                          ctx.eventID().event_number(), reFitMe->index(), 
                                          MuonR4::printID(*reFitMe)));
                }
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

                const xAOD::UncalibratedMeasurement* m = goodMeas.front();
                const bool isPrecHit = (m->type() == xAOD::UncalibMeasType::MdtDriftCircleType ||
                                        m->type() == xAOD::UncalibMeasType::MMClusterType ||
                                       (m->type() == xAOD::UncalibMeasType::sTgcStripType && 
                                         !m_idHelperSvc->measuresPhi(xAOD::identify(m))));

                summary.nPrecHits += isPrecHit;
                if (m->type() == xAOD::UncalibMeasType::Other || 
                    m_idHelperSvc->measuresPhi(xAOD::identify(m))){
                    ++summary.nPhiHits;
                } else {
                    summary.nEtaTrigHits += !isPrecHit;
                }
            });

            Acts::BoundTrackParameters parameters = track.createParametersAtReference();
            if (m_drawEvent) {
                MuonValR4::drawBoundParameters(gctx, parameters, visualHelper,
                                               Acts::ViewConfig{.color={0, 0, 220}});

                visualHelper.write(std::format("SegmentReFitTest_goodone_{:}_{:}_{:}.obj", 
                                          ctx.eventID().event_number(), reFitMe->index(), 
                                          MuonR4::printID(*reFitMe)));
            }
            /// Direction is always expressed in global frame -> transform to local
            const Amg::Vector3D globDir = parameters.direction();
            /// Express the parameters at the reference surface of the original segment
            const Amg::Transform3D globToLoc{sectorTrf.inverse()};
            const Amg::Vector3D refitPos = globToLoc * parameters.position(tgContext);
            const Amg::Vector3D refitDir = globToLoc.linear() * globDir;
            /// Straight line extension to plane
            const Amg::Vector3D refitSeg = refitPos + Amg::intersect<3>(refitPos, refitDir, Amg::Vector3D::UnitZ(), 0).value_or(0.) * refitDir;
            const Amg::Vector3D globPos{msSector->localToGlobalTransform(gctx) * refitSeg};

            auto newSegment = outHandle->push_back(std::make_unique<xAOD::MuonSegment>());
            dec_segLink(*newSegment) = Link_t{*segments, reFitMe->index(), ctx};

            newSegment->setDirection(globDir.x(), globDir.y(), globDir.z());
            newSegment->setPosition(globPos.x(), globPos.y(), globPos.z());
            
            newSegment->setFitQuality(track.chi2(), track.nDoF());
            newSegment->setNHits(summary.nPrecHits, summary.nPhiHits, summary.nEtaTrigHits);
            auto& locFitPars = dec_locPars(*newSegment);
            locFitPars[Acts::toUnderlying(ParamDefs::x0)] = refitSeg.x();
            locFitPars[Acts::toUnderlying(ParamDefs::y0)] = refitSeg.y();
            locFitPars[Acts::toUnderlying(ParamDefs::theta)] = refitDir.theta();
            locFitPars[Acts::toUnderlying(ParamDefs::phi)] = refitDir.phi();
        }
        return StatusCode::SUCCESS;
    }
}
