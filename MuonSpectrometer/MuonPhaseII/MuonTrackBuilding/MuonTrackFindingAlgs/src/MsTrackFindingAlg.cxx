/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MsTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"


#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"

#include "ActsInterop/UnitConverters.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "TruthUtils/HepMCHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include <system_error>

using namespace Acts::UnitLiterals;
using namespace Acts::PlanarHelper;

namespace {
    bool isNswSegment(const xAOD::MuonSegment& seg) {
        return seg.technology() == Muon::MuonStationIndex::TechnologyIndex::STGC || 
               seg.technology() == Muon::MuonStationIndex::TechnologyIndex::MM;
    }
}

namespace MuonR4{
    StatusCode MsTrackFindingAlg::initialize() {
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_msTrkSeedKey.initialize());

        ATH_CHECK(m_visualizationTool.retrieve(EnableTool{!m_visualizationTool.empty()}));
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_calibTool.retrieve());
        ATH_CHECK(m_writeKey.initialize());

        if (m_trackingGeometryTool->trackingGeometry()->geometryVersion() !=
            Acts::TrackingGeometry::GeometryVersion::Gen3){
            ATH_MSG_ERROR("The MS track fit requires the Gen 3 geometry format");
            return StatusCode::FAILURE;
        }

        MsTrackSeeder::Config seederCfg{};
        seederCfg.seedHalfLength = m_seedHalfLength;
        seederCfg.selector = m_segSelector.get();
        seederCfg.detMgr = m_detMgr;

        m_seeder = std::make_unique<MsTrackSeeder>(name(), std::move(seederCfg));
        return StatusCode::SUCCESS;
    }

    MsTrackFindingAlg::~MsTrackFindingAlg() = default;

    StatusCode MsTrackFindingAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("Run track finding in event "<<ctx.eventID().event_number());
        
        const xAOD::MuonSegmentContainer* allEventSegs{nullptr};
        ATH_CHECK(SG::get(allEventSegs, m_segmentKey, ctx));

        auto seedContainer = findTrackSeeds(ctx, *allEventSegs);

        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
        const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
        const Acts::CalibrationContext calContext{ActsTrk::getCalibrationContext(ctx)};
        
        
        Acts::VectorTrackContainer trackBackend{};
        Acts::VectorMultiTrajectory trackStateBackend{};
        ActsTrk::MutableTrackContainer cacheTrkContainer{std::move(trackBackend), 
                                                         std::move(trackStateBackend)};
        /// Attach the number of the parent seed to the output container
        cacheTrkContainer.addColumn<std::size_t>("parentSeed");
        unsigned seedIdx{0};
        for (const MsTrackSeed& seed : *seedContainer) {
            if (!fitSeedCandidate(tgContext, mfContext, calContext, seed, 
                                  cacheTrkContainer)) {
                ++seedIdx;
                continue;
            }
            auto lastTrack = cacheTrkContainer.getTrack(cacheTrkContainer.size() -1);
            lastTrack.component<std::size_t, Acts::hashString("parentSeed")>() = seedIdx;
            ++seedIdx;
        }
        SG::WriteHandle writeHandleSeed{m_msTrkSeedKey, ctx};
        ATH_CHECK(writeHandleSeed.record(std::move(seedContainer)));
    
        // Constant declination
        Acts::ConstVectorTrackContainer ctrackBackend{std::move(cacheTrkContainer.container())};
        Acts::ConstVectorMultiTrajectory ctrackStateBackend{std::move(cacheTrkContainer.trackStateContainer())};
        auto ctc = std::make_unique<ActsTrk::TrackContainer>(std::move(ctrackBackend),
                                                             std::move(ctrackStateBackend));
  
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(ctc)));
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<MsTrackSeedContainer>  
        MsTrackFindingAlg::findTrackSeeds(const EventContext& ctx,
                                          const xAOD::MuonSegmentContainer& segments) const {

        auto seedContainer = m_seeder->findTrackSeeds(ctx, m_trackingGeometryTool->getGeometryContext(ctx), segments);

        if (!m_visualizationTool.empty()) {
            m_visualizationTool->displaySeeds(ctx, *m_seeder, segments, *seedContainer);
        }
        return seedContainer;
    }
std::pair<MsTrackFindingAlg::OptBoundPars_t, 
          MsTrackFindingAlg::MeasVec_t>
        MsTrackFindingAlg::prepareFit(const Acts::GeometryContext& tgContext,
                                      const Acts::MagneticFieldContext& mfContext,
                                      const Acts::CalibrationContext& calContext,
                                      const MsTrackSeed& seed) const {
        const EventContext& ctx{*calContext.get<const EventContext*>()};        
        MeasVec_t measurements{};
        measurements.reserve(100);
        ///Fetch the measurements from the segment & find the first segment which has phi hits
        const xAOD::MuonSegment* refSeg{nullptr};
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            /// Ensure that the drift signs from the fit are stamped onto the Uncalibrated measurements
            m_calibTool->stampSignsOnMeasurements(*segment);
            MeasVec_t segMeasurements = collectMeasurements(*segment, /*skipOutlier:*/ true);
            if (msgLvl(MSG::VERBOSE)) {
                std::stringstream sstr{};
                for (const xAOD::UncalibratedMeasurement* m : segMeasurements) {
                    const Acts::Surface& surf{xAOD::muonSurface(m)};
                    sstr<<" ***  "<<m_idHelperSvc->toString(xAOD::identify(m))
                        <<", "<<m->numDimensions()<<", "
                        <<", "<<surf.geometryId()<<" @ "<<Amg::toString(surf.localToGlobalTransform(tgContext))<<std::endl;
                }
                ATH_MSG_VERBOSE("Fetch measurements from segment: "<<Amg::toString(segment->position())
                         <<", direction: "<<Amg::toString(segment->direction()) << " eta " << segment->direction().eta() << " phi " << segment->direction().phi() <<"\n"<<sstr.str());
            }
            measurements.insert(measurements.end(), 
                                std::make_move_iterator(segMeasurements.begin()),
                                std::make_move_iterator(segMeasurements.end()));

            // Ususally we would like to take the first segment with a sufficient amount of phi hits to set the initial position and direction of the track fit. However in some cases the segment from the NSW has a missreconstructed phi direction which causes the track fit to loose all BW and OW hits in the first iteration. Therefore if the first segment is a NSW segment we first try use a non-NSW segments with enough phi hits. If we don't find any segment with enough phi hits we will use the NSW segment as reference as long as it passes the seeding quality criteria.   
            if (!refSeg &&  !isNswSegment(*segment) &&  m_segSelector->passSeedingQuality(ctx, *detailedSegment(*segment))) {
                refSeg = segment;
                ATH_MSG_VERBOSE("Set reference segment");
            }
        }
        //if we did not find a reference segment let's try the NSW one before we give up on the track
        if(!refSeg){
            for (const xAOD::MuonSegment* segment : seed.segments()) {
                if (isNswSegment(*segment) && m_segSelector->passSeedingQuality(ctx, *detailedSegment(*segment))) {
                    refSeg = segment;
                    ATH_MSG_VERBOSE("Set reference segment from NSW");
                    break;
                }
            }
        }

        if (!refSeg || measurements.empty()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__
                            <<" - No reference segment passing seeding quality "<<
                            (refSeg != nullptr)<<" was found. #"<<measurements.size()<<" measurements. ");
            return std::make_pair(Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument)),
                                  std::vector<const xAOD::UncalibratedMeasurement_v1*>{});
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<measurements.size()<<" measurements");
        Amg::Vector3D seedPos{refSeg->position()};
        Amg::Vector3D seedDir{refSeg->direction()};
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Initial seed pos: "<<Amg::toString(seedPos)
                        <<", dir: "<<Amg::toString(seedDir) << " eta " << seedDir.eta() << " phi " << seedDir.phi() /Gaudi::Units::degree );
        /// The middle or outer segment provide the phi information. Not so easy becasue we want to
        /// Take the y0 & precision direction from the inner segment but the phi & x0 from a straight
        /// line extrapolation onto the plane
        if (refSeg != seed.segments().front()) {
            const MuonGMR4::SpectrometerSector* innerPlane = m_seeder->envelope(*seed.segments().front());
            const Acts::PlaneSurface& surf = innerPlane->surface();
            const Amg::Transform3D toInnerPlane = surf.localToGlobalTransform(tgContext).inverse();
            const Amg::Vector3D locSeedPos = toInnerPlane * seedPos;
            const Amg::Vector3D locSeedDir = toInnerPlane.linear() * seedDir;

            auto seedOnInner = Acts::PlanarHelper::intersectPlane(locSeedPos, locSeedDir,
                                                                  Amg::Vector3D::UnitZ(), 0.);

            using enum SegmentFit::ParamDefs;
            SegmentFit::Parameters innerPars = SegmentFit::localSegmentPars(*seed.segments().front());
            innerPars[Acts::toUnderlying(x0)] = seedOnInner.position().x();
            const Amg::Vector3D innerSegDir = 
                    Acts::makeDirectionFromPhiTheta(innerPars[Acts::toUnderlying(phi)],
                                                    innerPars[Acts::toUnderlying(theta)]);
            const Amg::Vector3D combSegDir = 
                    Acts::makeDirectionFromAxisTangents(houghTanAlpha(locSeedDir),
                                                        houghTanBeta(innerSegDir));
            seedPos = surf.localToGlobalTransform(tgContext) * Amg::Vector3D{innerPars[Acts::toUnderlying(x0)],
                                                                             innerPars[Acts::toUnderlying(y0)], 0};
            seedDir = surf.localToGlobalTransform(tgContext).linear() * combSegDir;
        }
        /// Create a surface shortly before the first measurement that belongs to a valid volume.
        /// Trigger and phi layer hits may have surface geometry IDs that don't map to a named
        /// tracking volume (e.g., they belong to a gap region), even though the measurements
        /// are valid and used in the fit. We search for the first precision measurement whose
        /// geometry ID successfully resolves to a named Acts::TrackingVolume so we can extract
        /// a boundary surface to seed the track parameters.
        const xAOD::UncalibratedMeasurement* firstVolumeMeas{nullptr};
        const Acts::TrackingVolume* volume{nullptr};
        for (const xAOD::UncalibratedMeasurement* meas : measurements) {
            const Acts::GeometryIdentifier volId = volumeId(xAOD::muonSurface(meas));
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check measurement "
                <<m_idHelperSvc->toString(xAOD::identify(meas))<<", "<<xAOD::muonSurface(meas).geometryId());
            volume = m_trackingGeometryTool->trackingGeometry()->findVolume(volId);
            if (volume) {
                firstVolumeMeas = meas;
                break;
            }
        }

        if (!volume) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__
                            <<" - Failed to find tracking volume for any seed measurement");
            return std::make_pair(Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument)),
                                  std::vector<const xAOD::UncalibratedMeasurement_v1*>{});
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Using seed measurement " 
                        << m_idHelperSvc->toString(xAOD::identify(firstVolumeMeas))
                        << " with volume id " << volumeId(xAOD::muonSurface(firstVolumeMeas)));

        auto boundSurf = MuonGMR4::bottomBoundary(*volume);
        if (!boundSurf) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to find boundary surface for tracking volume");
            return std::make_pair(Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument)),
                                  std::vector<const xAOD::UncalibratedMeasurement_v1*>{});
        }
        auto targetSurf = boundSurf->getSharedPtr();
        using namespace Acts::PlanarHelper;

        auto pIsect = intersectPlane(seedPos, seedDir, 
                                     targetSurf->localToGlobalTransform(tgContext).linear().col(2), 
                                     targetSurf->center(tgContext));
        
        auto fourPos = ActsTrk::convertPosToActs(pIsect.position(), pIsect.position().mag() / Gaudi::Units::c_light);
        const double qOverP = 1./ m_seeder->estimateQtimesP(*tgContext.get<const ActsTrk::GeometryContext*>(),
                                                            *mfContext.get<const AtlasFieldCacheCondObj*>(), seed);
        auto initialPars = Acts::BoundTrackParameters::create(tgContext, targetSurf, fourPos, 
                                                              seedDir, ActsTrk::energyToActs(qOverP),
                                                              Acts::BoundMatrix::Identity(), 
                                                              Acts::ParticleHypothesis::muon());
        return std::make_pair(std::move(initialPars),  std::move(measurements));

    }
    bool MsTrackFindingAlg::fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                             const Acts::MagneticFieldContext& mfContext,
                                             const Acts::CalibrationContext& calContext,
                                             const MsTrackSeed& seed,
                                             ActsTrk::MutableTrackContainer& outContainer) const {
        
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Attempt to fit a new track seed \n"<<seed);
        const EventContext& ctx{*calContext.get<const EventContext*>()};
        const auto [initialPars, measurements] = prepareFit(tgContext, mfContext, calContext, seed);
        
        if (!initialPars.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to construct valid parameters for seed \n"<<seed);
            if (m_visualizationTool.isEnabled()) {
                m_visualizationTool->displayTrackSeedObj(ctx, seed, initialPars, "FailedStartPars");
            }
            return false;
        }
        auto fitTraject = m_trackFitTool->fit(measurements, *initialPars, 
                                              tgContext, mfContext, calContext, 
                                              &(*initialPars).referenceSurface());
        if (!fitTraject || fitTraject->size() == 0) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fit failed ");
            if (m_visualizationTool.isEnabled()) {
                m_visualizationTool->displayTrackSeedObj(ctx, seed, initialPars, "FailedFit");
            }
            return false;
        }

        auto track = fitTraject->getTrack(0);
        const Amg::Vector3D trkP4 = ActsTrk::convertMomFromActs(track.fourMomentum()).first;
        double pt = trkP4.perp() / 1000; //in GeV
        if(pt < 2 ) {
            double chi2PerDoF = track.chi2() / (std::max(track.nDoF(), 1u));
            ATH_MSG_DEBUG(" ===cat dog: found low pt track candidate with pt "<<pt<<" GeV chi2/ndof "<< chi2PerDoF <<  " chi2 "<< track.chi2() << " nDOF "<< track.nDoF() <<"eta: "<<trkP4.eta());

        track.container().trackStateContainer().visitBackwards(track.tipIndex(), [&](const auto& state) {
            if(state.hasUncalibratedSourceLink()){
                const auto* uncalib = dynamic_cast<const xAOD::MuonMeasurement*>(ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink()));
                if(uncalib){
                    ATH_MSG_DEBUG("    has meas: "<<m_idHelperSvc->toString(xAOD::identify(uncalib)) << " state " << state.typeFlags());
                }
            }
        }
        );

        }
        /** Add the links to the segments making up this track as an extra
         *  column. Use the indices of the segment objects which can later
         *  be transformed into a full ElementLink as there is only one
         *  SegmentContainer from which the seeds are built */
        {
            fitTraject->addColumn<std::vector<const xAOD::MuonSegment*>>("muonSegLinks");
            fitTraject->getTrack(0).component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks") = seed.segments();
        }
        outContainer.ensureDynamicColumns(*fitTraject);
        auto destProxy = outContainer.getTrack(outContainer.addTrack());
        destProxy.copyFrom(fitTraject->getTrack(0));
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Good track fit...");
        if (m_visualizationTool.isEnabled()) {
            m_visualizationTool->displayTrackSeedObj(ctx, seed, 
                destProxy.createParametersAtReference(), "GoodFit");
        }
        return true;
    }
    
}
