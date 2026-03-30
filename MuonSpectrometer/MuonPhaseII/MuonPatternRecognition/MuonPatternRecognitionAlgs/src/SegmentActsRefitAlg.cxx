/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentActsRefitAlg.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include "ActsCalibBase/CalibrationContext.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "StoreGate/WriteDecorHandle.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"


#include "GaudiKernel/PhysicalConstants.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandGaussZiggurat.h"


#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"

#include "xAODMuon/MuonSegmentAuxContainer.h"


#include "ActsInterop/UnitConverters.h"
#include "ActsInterop/Logger.h"

#include "Acts/Visualization/GeometryView3D.hpp"



using namespace Acts::UnitLiterals;
namespace{
    constexpr double straightQoverP = 1. / (20._TeV);
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
        ATH_CHECK(m_auxMeasProv.initialize(m_writeKey.key()));
        ATH_CHECK(m_calibTool.retrieve());

        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        auto logger = makeActsAthenaLogger(this, "Fitter");

        Acts::StraightLineStepper stepper{};
        Acts::Navigator::Config navConfig{m_trackingGeometryTool->trackingGeometry()};
        Acts::Navigator navigator(std::move(navConfig), logger->clone());
        
        Propagator_t propagator{std::move(stepper), std::move(navigator), logger->clone()};

        m_fitter = std::make_unique<Fitter_t>(std::move(propagator), logger->clone());

        m_surfAccessor = ActsTrk::detail::xAODUncalibMeasSurfAcc{m_trackingGeometryTool.get()};
        m_fitExtension.surfaceAccessor.connect<&ActsTrk::detail::xAODUncalibMeasSurfAcc::operator()>(&m_surfAccessor);
        m_fitExtension.calibrator.connect<&MuonR4::ISpacePointCalibrator::calibrateSourceLink>(m_calibTool.get());
        return StatusCode::SUCCESS;
    }
    std::tuple<Amg::Vector3D, Amg::Vector3D> 
        SegmentActsRefitAlg::smearSegment(const ActsTrk::GeometryContext& gctx,
                                          const MuonR4::Segment& segment,
                                          CLHEP::HepRandomEngine* engine) const{
        if (!m_smearSegPars) {
            return std::make_pair(segment.position(), segment.direction());
        }
        const SegmentFit::Parameters segPars = localSegmentPars(gctx, segment);
        SegmentFit::Parameters smearedPars = segPars;
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
                          <<", original: "<<segPars[idx]<<", smeared: "<<smearedPars[idx]
                          <<", deviation: "
                          <<(smearedPars[idx] - segPars[idx]) / uncert );
        }
        auto [smearLocPos, smearLocDir] = makeLine(smearedPars);
        const auto [locPos, locDir] = makeLine(segPars);
        /// Ensure that the left-right ambiguity is preserved
        if (SeedingAux::strawSigns(locPos,locDir, segment.measurements()) !=
            SeedingAux::strawSigns(smearLocPos, smearLocDir, segment.measurements())) {
            ATH_MSG_DEBUG("Parameter smearng from "<<toString(segPars)<<" -> "<<toString(smearedPars)
                        <<" changes the L/R ambiguity -> avoid for this test");
            return smearSegment(gctx, segment, engine);
        }

        const Amg::Transform3D& locToGlob{segment.msSector()->localToGlobalTransform(gctx)};
        if (smearLocDir.z() < 0) {
            smearLocDir = -smearLocDir;
        }
        return std::make_tuple(locToGlob * smearLocPos, locToGlob.linear() * smearLocDir);
    }

    const Acts::Surface* SegmentActsRefitAlg::portalSurface(const xAOD::UncalibratedMeasurement* measurement,
                                                            bool entrance) const {

        const Acts::GeometryIdentifier volId = m_surfAccessor.get(measurement)->geometryId().withSensitive(0);
        const Acts::TrackingVolume* volume = m_trackingGeometryTool->trackingGeometry()->findVolume(volId);
        return entrance ? MuonGMR4::bottomBoundary(*volume) :  MuonGMR4::topBoundary(*volume);
    }
    StatusCode SegmentActsRefitAlg::execute(const EventContext& ctx) const {
        
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_readKey, ctx));
        /// Create the context object
        const ActsTrk::GeometryContext& gctx{m_trackingGeometryTool->getGeometryContext(ctx)};
        const std::shared_ptr<const Acts::TrackingGeometry> trackingGeo = m_trackingGeometryTool->trackingGeometry();
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
        

        auto handleCreation = m_auxMeasProv.makeHandle(ctx, tgContext);
        if (!handleCreation.ok()){
            ATH_MSG_ERROR("Auxiliary measurement containers cannot be made");
            return StatusCode::FAILURE;
        }
        auto& auxMeasHandle{*handleCreation};
        using Link_t = ElementLink<xAOD::MuonSegmentContainer>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, Link_t> dec_segLink{m_linkKey, ctx};
        using ParDecor_t = SG::WriteDecorHandle<xAOD::MuonSegmentContainer, 
                                                xAOD::MeasVector<Acts::toUnderlying(ParamDefs::nPars)>>;
        
        ParDecor_t dec_locPars{m_localParsKey, ctx};
        ParDecor_t dec_seedPars{m_seedParsKey, ctx};

        SeedingAux::Config cfg{};
        cfg.parsToUse.clear();
        SeedingAux pullCalculator{cfg, makeActsAthenaLogger(this, "PullCalculator")};

        SeedingAux::Line_t line{};

        Acts::PropagatorPlainOptions propagationOption{tgContext, mfContext};
        propagationOption.maxSteps = m_maxPropSteps;
        propagationOption.maxTargetSkipping = m_maxTargetSurfSkip;
        constexpr bool doScat = false;
        constexpr bool doEloss = false;
        Gx2FitterOptions_t fitOptions{tgContext, mfContext, calContext, m_fitExtension, 
                                      std::move(propagationOption),
                                      nullptr, doScat, doEloss,
                                      Acts::FreeToBoundCorrection{m_doJacobianCorr},
                                      m_maxIter};

        ActsTrk::MutableTrackContainer tracks{ActsTrk::MutableTrackBackend{},
                                              ActsTrk::MutableTrackStateBackend{}};
        /// Loop over the segment container
        Acts::ObjVisualization3D visualHelper{};


        for (const xAOD::MuonSegment* reFitMe: *segments){
            auto saveDisplay = [&](const std::string& state) {
                if (!m_drawEvent) {
                    return;
                }
                const auto objFile = std::format("SegmentReFitTest_{:}_{:}_{:}_{:}.obj", 
                state, ctx.eventID().event_number(), reFitMe->index(), 
                MuonR4::printID(*reFitMe));
                visualHelper.write(objFile);
                ATH_MSG_DEBUG("Save event display under "<<objFile);
                visualHelper = Acts::ObjVisualization3D{};
            };

            const MuonGMR4::SpectrometerSector* msSector = 
                        m_detMgr->getSectorEnvelope(reFitMe->chamberIndex(), 
                                                    reFitMe->sector(), 
                                                    reFitMe->etaIndex());
            const Amg::Transform3D& sectorTrf{msSector->localToGlobalTransform(gctx)};
            const Amg::Vector3D planeNormal = sectorTrf.linear().col(2);

            m_calibTool->stampSignsOnMeasurements(*reFitMe);
            line.updateParameters(localSegmentPars(gctx,  *MuonR4::detailedSegment(*reFitMe)));
            /// Fetch a smeared segment position & direction
            const auto [seedPos, seedDir] = smearSegment(gctx, *MuonR4::detailedSegment(*reFitMe), randEngine);
            /// Decorate the initial seed parameters to the segment
            {
                const Amg::Transform3D invTrf = sectorTrf.inverse();
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
            std::vector<const xAOD::UncalibratedMeasurement*> hitsToFit = MuonR4::collectMeasurements(*reFitMe); 
     
            // Identify the first measurement in the chamber volume. Skip the first one if it
            // is the beamspot
            const xAOD::UncalibratedMeasurement* entrance =
                        hitsToFit.front()->type() != xAOD::UncalibMeasType::Other ?
                        hitsToFit.front() : hitsToFit.at(1);
            // Fetch the entrance & exit portals from the geometry
            const Acts::Surface* entrancePortal = portalSurface(entrance, true);
            const Acts::Surface* exitPortal = portalSurface(hitsToFit.back(), false);
            ATH_MSG_DEBUG("Entrance portal: "
                <<entrancePortal->geometryId()<<"/"<<m_idHelperSvc->toString(xAOD::identify(entrance))
                <<", exit portal: "<<exitPortal->geometryId()<<"/"<<m_idHelperSvc->toString(xAOD::identify(hitsToFit.back())));

            using namespace Acts::PlanarHelper;

            const Acts::Intersection3D isectEntrance = intersectPlane(seedPos, seedDir, planeNormal,
                                                entrancePortal->center(tgContext));
            const Acts::Intersection3D isectFirst = intersectPlane(seedPos, seedDir, planeNormal,
                                                m_surfAccessor.get(hitsToFit.front())->center(tgContext));

            if (reFitMe->nPhiLayers() < 1) {

                const Acts::Intersection3D isectExit = intersectPlane(seedPos, seedDir, planeNormal,
                                                exitPortal->center(tgContext));
                const Acts::Intersection3D isectLast = intersectPlane(seedPos, seedDir, planeNormal,
                                                m_surfAccessor.get(hitsToFit.back())->center(tgContext));

                /// We add two pseudo measurements above & beneath the segment to stabilize the  fit
                
                const Amg::Transform3D trfBeneath = GeoTrf::GeoTransformRT{sectorAngles, 0.5*isectFirst.position() +  
                                                                                         0.5*isectEntrance.position()}; 
                const Amg::Transform3D trfAbove   = GeoTrf::GeoTransformRT{sectorAngles, 0.85*isectExit.position() + 
                                                                                         0.15*isectLast.position() }; 

                auto surfBeneath = Acts::Surface::makeShared<Acts::PlaneSurface>(trfBeneath);
                auto surfAbove = Acts::Surface::makeShared<Acts::PlaneSurface>(trfAbove);
                ATH_MSG_DEBUG("Add auxiliary measurements @ "<<Amg::toString(surfBeneath->center(tgContext))
                              <<" and @"<<Amg::toString(surfAbove->center(tgContext)));

                constexpr double covVal = Acts::square(1._cm);
                hitsToFit.push_back(auxMeasHandle.newMeasurement<1>(surfBeneath, ProjectorType::e1DimNoTime, AmgSymMatrix(1){covVal}));
                if (m_drawEvent) {
                    MuonValR4::drawMeasurement(gctx, hitsToFit.back(), visualHelper, Acts::s_viewGrid);
                }
                hitsToFit.push_back(auxMeasHandle.newMeasurement<1>(surfAbove, ProjectorType::e1DimNoTime, AmgSymMatrix(1){covVal}));
                if (m_drawEvent) {
                    MuonValR4::drawMeasurement(gctx, hitsToFit.back(), visualHelper, Acts::s_viewGrid);
                }
            }

            if (m_drawEvent) {
                Acts::GeometryView3D::drawSurface(visualHelper, *entrancePortal, tgContext, 
                                                  Amg::Transform3D::Identity(), Acts::s_viewPortal);
                Acts::GeometryView3D::drawSurface(visualHelper, *exitPortal, tgContext, 
                                                  Amg::Transform3D::Identity(), Acts::s_viewPortal);
                
                /// Draw the reference segment as a red line
                MuonValR4::drawSegmentLine(gctx, *reFitMe, visualHelper,
                                Acts::ViewConfig{.color = {220, 0, 0}});
                MuonValR4::drawSegmentMeasurements(gctx, *reFitMe, visualHelper, Acts::s_viewSurface);
            }
            ATH_MSG_VERBOSE("Entrance position "<<Amg::toString(isectEntrance.position())
                        <<", path length: "<<isectEntrance.pathLength()<<", "
                        <<" - First surface position: "<<Amg::toString(isectFirst.position())
                        <<", path length: "<<isectFirst.pathLength());
            /// Place the target surface close to the entrance portal
            
            Amg::Vector3D refPos = 0.85 * isectEntrance.position()
                                 + 0.15 * isectFirst.position();
            const Amg::Transform3D trf{GeoTrf::GeoTransformRT{sectorAngles, refPos}};
            if (msgLvl(MSG::VERBOSE)) {
                const auto [locPos, locDir] = makeLine(localSegmentPars(*reFitMe));

                std::stringstream sstr{};
                sstr<<"pos: "<<Amg::toString(seedPos)<<", dir: "<<Amg::toString(seedDir)<<", chi2/nDoF: "
                    <<reFitMe->chiSquared() / reFitMe->numberDoF()<<", nDoF: "<<reFitMe->numberDoF()
                    <<", prec: "<<reFitMe->nPrecisionHits()<<", phi: "<<reFitMe->nPhiLayers()<<std::endl;
                for (const auto& meas : MuonR4::detailedSegment(*reFitMe)->measurements()) {
                    pullCalculator.updateSpatialResidual(line, *meas);
                    const Acts::Surface* surface = meas->spacePoint() ? m_surfAccessor.get(meas->spacePoint()->primaryMeasurement()) : nullptr;
                    const Acts::GeometryIdentifier geoId = surface ? surface->geometryId() : Acts::GeometryIdentifier{};
                    sstr<<"  **** "<<(*meas)<<", chi2: "<<SeedingAux::chi2Term(locPos, locDir, *meas)
                        <<", sign: "<<(meas->isStraw() ? 
                                (SeedingAux::strawSign(locPos,locDir, *meas) == 1 ? "R" : "L") : "X")
                        <<", geoId: "<<geoId;
                    if (geoId != Acts::GeometryIdentifier{}){
                        const Amg::Vector3D globPos = meas->spacePoint()->msSector()->localToGlobalTransform(gctx) * 
                                                    meas->localPosition();
                        const Acts::GeometryIdentifier volId = geoId.withSensitive(0);
                        const Acts::TrackingVolume* volume = m_trackingGeometryTool->trackingGeometry()->findVolume(volId);
                        const Acts::Vector2 lPos = (*surface->globalToLocal(tgContext,globPos, reFitMe->direction()));
                        sstr<<", inside volume: "<<volume->inside(tgContext, globPos);
                        sstr<<", inside surface: "<<surface->bounds().inside(lPos);
                        
                    }
                    sstr<<std::endl;
                }

                sstr<<" Target surf: "<<Amg::toString(trf)<<", firstSurf: "
                     << Amg::toString(trf.inverse()*isectFirst.position())
                     <<", refPoint: "<<Amg::toString(trf.inverse()*refPos)<<std::endl;
                ATH_MSG_VERBOSE("Run G2F fit on "<<msSector->identString()<<std::endl<<sstr.str());
            }
            // Specify the surface on which the segment will be expressed. 
            std::shared_ptr<const Acts::Surface> target{};
            if (hitsToFit.front()->type() == xAOD::UncalibMeasType::Other) {
                target = m_surfAccessor.get(hitsToFit.front())->getSharedPtr();
                using namespace Acts::detail::LineHelper;
                const Acts::Intersection3D lineIsect = 
                                lineIntersect<3>(target->center(tgContext),
                                                 Amg::Vector3D::UnitZ(),
                                                 seedPos, seedDir);
                // Also update the start position to be the closest point of approach
                // to the beamspot
                refPos = lineIsect.position();
            } else {
                /// Bounds are purely for visualization purposes.
                target = Acts::Surface::makeShared<Acts::PlaneSurface>(trf, 
                            std::make_unique<Acts::RectangleBounds>(1._m, 1._m));
            }
            if (m_drawEvent) {
                Acts::GeometryView3D::drawSurface(visualHelper, *target, 
                    tgContext, Amg::Transform3D::Identity(), 
                    Acts::ViewConfig{.color={0,0,220}});
            }

            Acts::Vector<4> fourPos{ActsTrk::convertPosToActs(refPos, refPos.mag() / Gaudi::Units::c_light)};
            Acts::BoundMatrix initialCov{Acts::BoundMatrix::Identity()};

            auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, seedDir, straightQoverP,
                                                                  initialCov, Acts::ParticleHypothesis::muon());
            if (!initialPars.ok()) {
                ATH_MSG_WARNING("Initial estimate of the parameters failed");
                saveDisplay("invalidPars");
                continue;
            }
            if (m_drawEvent) {
                MuonValR4::drawBoundParameters(gctx, *initialPars, visualHelper,
                                               Acts::ViewConfig{.color={0,220,0}});
            }
            ATH_MSG_DEBUG("Initial parameters --  (loc0, loc1): "<<Amg::toString((*initialPars).localPosition())
                        <<", "<<std::format("theta: {:.2f}, ", (*initialPars).theta() / 1._degree)
                              <<std::format("phi: {:.2f}", (*initialPars).phi() / 1._degree));

            std::vector<Acts::SourceLink> sourceLinks{};
            std::ranges::transform(hitsToFit, std::back_inserter(sourceLinks), 
                                   [](const xAOD::UncalibratedMeasurement* meas) {
                                        return ActsTrk::detail::xAODUncalibMeasCalibrator::pack(meas);
                                   });
            fitOptions.referenceSurface = target.get();

            auto fitTraject =  m_fitter->fit(sourceLinks.begin(), sourceLinks.end(), 
                                             *initialPars, fitOptions, tracks);
            if (!fitTraject.ok()) {
                saveDisplay("failed");
                if (m_smearSegPars) {
                    ATH_MSG_WARNING("Track fit failed.");
                    continue;
                } else {
                    ATH_MSG_ERROR("Segment refit failed. Albeit start parameters are taken from segment");
                    return StatusCode::FAILURE;
                }
            }

            auto track = *fitTraject;
            ATH_MSG_DEBUG("Track fit succeeded. ");

            Acts::BoundTrackParameters parameters = track.createParametersAtReference();
            if (m_drawEvent) {
                MuonValR4::drawBoundParameters(gctx, parameters, visualHelper,
                                               Acts::ViewConfig{.color={0,220,220}});
      
            }
            saveDisplay("goodone");

            MuonR4::Segment::HitSummary summary{};
            /// Fetch the measurements from the track state
            std::vector<const xAOD::UncalibratedMeasurement*> goodMeas{};
            unsigned int itr{0};
            tracks.trackStateContainer().visitBackwards(track.tipIndex(),[&](const auto& state){
                if (!state.hasUncalibratedSourceLink()){
                    return;
                }
                goodMeas.insert(goodMeas.begin(),
                                ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink()));
                
                ATH_MSG_VERBOSE("Loop over track state: "<<(itr++)<<", "<<m_idHelperSvc->toString(xAOD::identify(goodMeas.front()))
                                <<", id: "<<m_surfAccessor.get(goodMeas.front())->geometryId());

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

            /// Direction is always expressed in global frame -> transform to local
            const Amg::Vector3D globDir = parameters.direction();
            /// Express the parameters at the reference surface of the original segment
            const Amg::Transform3D globToLoc{sectorTrf.inverse()};
            const Amg::Vector3D refitPos = globToLoc * parameters.position(tgContext);
            const Amg::Vector3D refitDir = globToLoc.linear() * globDir;
            /// Straight line extension to plane
            const Amg::Vector3D refitSeg = refitPos + Amg::intersect<3>(refitPos, refitDir, Amg::Vector3D::UnitZ(), 0).value_or(0.) * refitDir;
            const Amg::Vector3D globPos{msSector->localToGlobalTransform(gctx) * refitSeg};

            xAOD::MuonSegment* newSegment = outHandle->push_back(std::make_unique<xAOD::MuonSegment>());
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
