/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODSegmentCnvAlg.h"

#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "MuonSpacePoint/SpacePointHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"


#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "MuonPatternEvent/MuonPatternContainer.h"

#include "ActsGeoUtils/SurfacePlacement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"

#include "Acts/Utilities/Enumerate.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include "ActsInterop/UnitConverters.h"

using namespace Acts::UnitLiterals;

namespace {
    /** @brief Returns the last parent volume that is alignable */
    const Acts::TrackingVolume* highestAlignable(const Acts::TrackingVolume* volume){
        return !volume || !volume->motherVolume() || !volume->motherVolume()->isAlignable()
            ? volume : highestAlignable(volume->motherVolume());
    }
}

namespace MuonR4{
    using namespace SegmentFit;
    /** @brief Abrivation of the link to the reco segment container  */
    using SegLink_t = ElementLink<MuonR4::SegmentContainer>;
    /** @brief Abrivation of the decorated local segment parameters */
    using SegPars_t = xAOD::PosAccessor<Acts::toUnderlying(ParamDefs::nPars)>::element_type;
    /** @brief Abrivation of the decorated local segment covariance */
    using SegCov_t = xAOD::PosAccessor<Acts::sumUpToN(Acts::toUnderlying(ParamDefs::nPars))>::element_type;
    StatusCode xAODSegmentCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_localSegParKey.initialize());
        ATH_CHECK(m_localSegCovKey.initialize());
        ATH_CHECK(m_parentSegKey.initialize());
        ATH_CHECK(m_combMeasKey.initialize());
        ATH_CHECK(m_prdStateKey.initialize());
        ATH_CHECK(m_auxMeasProv.initialize(m_writeKey.key(), m_convertBeamSpot));
        ATH_CHECK(m_trackingGeometryTool.retrieve(EnableTool{m_estimateHoles}));
        ATH_CHECK(m_extrapolationTool.retrieve(EnableTool{m_estimateHoles}));
        ATH_CHECK(m_ctxProvider.initialize(m_estimateHoles));
        return StatusCode::SUCCESS;
    }
    StatusCode xAODSegmentCnvAlg::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        PrepDataCollectorShip measDecorator{*this, ctx, gctx->context()};

        xAOD::FillContainer<xAOD::MuonSegmentContainer,
                           xAOD::MuonSegmentAuxContainer> outContainer{};
        
        
        ATH_CHECK(outContainer.record(m_writeKey, ctx));
        ATH_CHECK(measDecorator.prdCombContainer.record(m_combMeasKey, ctx));
        
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegLink_t> dec_parentLink{m_parentSegKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegPars_t> dec_locPars{m_localSegParKey, ctx};
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegCov_t> dec_locCov{m_localSegCovKey, ctx};
 
        for (const SG::ReadHandleKey<SegmentContainer>& key : m_readKeys) {
            const SegmentContainer* segmentContainer{nullptr};
            ATH_CHECK(SG::get(segmentContainer, key, ctx));
         
            /// Counter for the reco segment link
            unsigned recoSegIdx{0};
            outContainer->reserve(outContainer->size() + segmentContainer->size());
            for (const Segment* inSegment : *segmentContainer) {
                const MuonGMR4::SpectrometerSector* sector = inSegment->msSector();

                xAOD::MuonSegment* convertedSeg = outContainer->push_back(std::make_unique<xAOD::MuonSegment>());
                dec_parentLink(*convertedSeg) = SegLink_t{segmentContainer, recoSegIdx};
                ++recoSegIdx;
            
                const Amg::Vector3D& pos{inSegment->position()};
                const Amg::Vector3D& dir{inSegment->direction()};
                convertedSeg->setPosition(pos.x(), pos.y(), pos.z());
                convertedSeg->setDirection(dir.x(), dir.y(), dir.z());

                convertedSeg->setIdentifier(sector->sector(), sector->chamberIndex(),
                                           sector->side(), inSegment->technology());
                convertedSeg->setFitQuality(inSegment->chi2(), inSegment->nDoF());

                using enum ParamDefs;
                convertedSeg->setT0Error(inSegment->segementT0(), 
                                         Amg::error(inSegment->covariance(), Acts::toUnderlying(t0)));

                SegPars_t& localPars{dec_locPars(*convertedSeg)};
                const Amg::Transform3D globToLoc{sector->globalToLocalTransform(*gctx)};            
                const Amg::Vector3D locPos{globToLoc * pos};
                const Amg::Vector3D locDir{globToLoc.linear() * dir};

                localPars[Acts::toUnderlying(x0)] = locPos.x();
                localPars[Acts::toUnderlying(y0)] = locPos.y();
                localPars[Acts::toUnderlying(theta)] = locDir.theta();
                localPars[Acts::toUnderlying(phi)] = locDir.phi();
                localPars[Acts::toUnderlying(t0)] = inSegment->segementT0();

                SegCov_t& localCov{dec_locCov(*convertedSeg)};
                constexpr std::size_t n = Acts::toUnderlying(ParamDefs::nPars);
                for (std::size_t p = 1; p < n; ++p) {
                    for (std::size_t p1 = 0 ; p1 <=p;++p1) {
                        localCov[Acts::vecIdxFromSymMat<n>(p,p1)] = inSegment->covariance()(p, p1);
                    }
                }
                ATH_CHECK(linkMeasurements(*gctx, *inSegment, *convertedSeg, measDecorator));
                evaluateSummary(ctx, *convertedSeg);
            }
        }  
        return StatusCode::SUCCESS;
    }

    StatusCode xAODSegmentCnvAlg::linkMeasurements(const ActsTrk::GeometryContext& gctx,
                                                   const Segment& inSegment,
                                                   xAOD::MuonSegment& copySegment,
                                                   PrepDataCollectorShip& ship) const {
        using State = CalibratedSpacePoint::State;
        // Cache all measurements that can be combined to two measurements in a single gas gap
        std::vector< std::tuple<const xAOD::MuonMeasurement*, State, std::size_t>> combineMap{};
        std::vector< std::tuple<const xAOD::UncalibratedMeasurement*, State, std::size_t>> linkMap{};


        PrdLinkVec_t& links = ship.dec_prdLinks(copySegment);
        std::vector<char>& linkStates = ship.dec_prdStates(copySegment);
        links.reserve(2*inSegment.measurements().size());
        linkStates.reserve(2*inSegment.measurements().size());

        /** @brief Combine the two prds from the space point to a combined muonstrip and link
          *         the latter to the segment. */
        auto combine = [&](const xAOD::MuonMeasurement* m1, 
                          const xAOD::MuonMeasurement* m2) {
            auto cmbMeas = ship.prdCombContainer->push_back(std::make_unique<xAOD::CombinedMuonStrip>());

            cmbMeas->setPrimaryStrip(m1);
            cmbMeas->setSecondaryStrip(m2);
            const auto [locPos, locCov] = xAOD::positionAndCovariance(m1, m2);
            cmbMeas->localCovariance<2>() = xAOD::toStorage(locCov);
            cmbMeas->localPosition<2>() = xAOD::toStorage(locPos);
            const Identifier id1{m1->identify()}, id2{m2->identify()};
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Combine "<<m_idHelperSvc->toString(id1) 
                            <<" & "<<m_idHelperSvc->toString(id2));
            if ((m1->type() != xAOD::UncalibMeasType::sTgcStripType || 
                m_idHelperSvc->stgcIdHelper().channelType(id1) == 
                m_idHelperSvc->stgcIdHelper().channelType(id2))&&
                m_idHelperSvc->measuresPhi(id1) == m_idHelperSvc->measuresPhi(id2)) {
                THROW_EXCEPTION("Cannot combine "<<m_idHelperSvc->toString(id1)
                                <<" & "<<m_idHelperSvc->toString(id2));
            }
            return cmbMeas;
        };
        // Loop over the measurements
        for (const auto [segIdx, meas] : Acts::enumerate(inSegment.measurements())) {
            const SpacePoint* sp = meas->spacePoint();
            if (!sp) {
                if (!m_convertBeamSpot) {
                    continue;
                }
                // Up to now, there's no variety on the beamspot across the segments
                if (!ship.beamSpot) {
                    if (!ship.beamSpotMeasCreator.ok()) {
                        ATH_MSG_ERROR("Cannot create a beamspot measurement");
                        return StatusCode::FAILURE;
                    }

                    const Amg::Vector3D beamSpot = inSegment.msSector()->localToGlobalTransform(gctx) *
                                                    meas->localPosition();
                    AmgSymMatrix(2) covariance{AmgSymMatrix(2)::Identity()};
                    using CovIdx = SpacePoint::CovIdx;
                    using ProjectorType = xAOD::AuxiliaryMeasurement::ProjectorType;
                    covariance(0,0) = meas->covariance()[Acts::toUnderlying(CovIdx::etaCov)];
                    covariance(1,1) = meas->covariance()[Acts::toUnderlying(CovIdx::phiCov)];
                    /// Size of the bounds purely for visualization purposes
                    auto surf = Acts::Surface::makeShared<Acts::StrawSurface>(Amg::getTranslate3D(beamSpot),
                                                std::make_shared<Acts::LineBounds>(std::sqrt(covariance(0,0)), 20._m));

                    ship.beamSpot = ship.beamSpotMeasCreator->newMeasurement<2>(surf, ProjectorType::e2DimNoTime, covariance);
                    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Created beamspot measurement "<<(*meas)<<", "
                                    <<surf->toString(gctx.context()));
                }
                linkMap.emplace_back(ship.beamSpot, meas->fitState(), segIdx);
                continue;
            }
            switch (sp->type()) {
                using enum xAOD::UncalibMeasType;
                // Mdt &  micromegas are never combined
                case MdtDriftCircleType:
                case MMClusterType: {
                    linkMap.emplace_back(sp->primaryMeasurement(), meas->fitState(), segIdx);                        
                    break;
                } case RpcStripType:
                  case TgcStripType:
                  case sTgcStripType: {
                    if (sp->primaryMeasurement() && sp->secondaryMeasurement()) {
                        if (sp->primaryMeasurement() != sp->secondaryMeasurement()) {
                            linkMap.emplace_back(combine(sp->primaryMeasurement(), 
                                                         sp->secondaryMeasurement()),
                                                         meas->fitState(), segIdx);
                        } else {  // BI - RPC measurements
                            linkMap.emplace_back(sp->primaryMeasurement(), meas->fitState(), segIdx);
                        }
                    } else {
                        /// It might be that the segment has anoher 1D-measurement 
                        /// in the same gas gap
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Append for later combination "<<(*meas));
                        combineMap.emplace_back(sp->primaryMeasurement(), meas->fitState(), segIdx);
                    }
                    break;
                } default:
                    break;
            }
        }
    
        // Finally we need to check whether there're measurements left to combine
        for (std::size_t cmbIdx = 0; cmbIdx < combineMap.size(); ++cmbIdx){
            const xAOD::MuonMeasurement* m1{std::get<0>(combineMap[cmbIdx])};
            const State s1{std::get<1>(combineMap[cmbIdx])};
            const std::size_t segIdx1{std::get<2>(combineMap[cmbIdx])};
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Find another measurement to combine with "
                            <<m_idHelperSvc->toString(m1->identify()));
            if (cmbIdx +1 < combineMap.size()) {
                const xAOD::MuonMeasurement* m2{std::get<0>(combineMap[cmbIdx +1])};
                const State s2{std::get<1>(combineMap[cmbIdx+1])};
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check whether "<<m_idHelperSvc->toString(m2->identify())
                                <<" is a good candidate");
                // Ensure that they point to the same detector element, are within the same
                // layer and have also the same state
                if (m1->type() == m2->type() &&  m1->identifierHash() == m2->identifierHash() && 
                    m1->layerHash() == m2->layerHash() && s1 == s2) {
                    /// The first measurement should always be the eta measurement 
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - They match");
                    if (m1->measuresPhi()) {
                        linkMap.emplace_back(combine(m2, m1), s2, segIdx1);
                    } else {
                        linkMap.emplace_back(combine(m1, m2), s1, segIdx1);
                    }
                    ++cmbIdx; // skip the next measurement as it's absorbed here
                    continue;
                }
            }
            ATH_MSG_VERBOSE("No match found");
            linkMap.emplace_back(m1, s1, segIdx1);
        }

        std::ranges::sort(linkMap, [](const auto& a, const auto& b){
            return std::get<2>(a) < std::get<2>(b);
        });

        for (const auto& [prd, state, segIdx]: linkMap) {
            links.emplace_back(*static_cast<const xAOD::UncalibratedMeasurementContainer*>(prd->container()), 
                                prd->index());
            linkStates.emplace_back(Acts::toUnderlying(state));
        }
        return StatusCode::SUCCESS;
    }

    void xAODSegmentCnvAlg::evaluateSummary(const EventContext& ctx, 
                                            xAOD::MuonSegment& segment) const {


        /** Instantiate one counter for hits, outliers, holes */
        Counter hits{}, outliers{}, holes{};
        /** Also write down the geometryIdentifiers of the segment measurements */
        std::unordered_set<Identifier> crossedSurfaces{};
        const std::size_t nMeas = nMeasurements(segment);

        const Acts::Surface* startSurface{}, *lastSurface{};
        const Acts::TrackingGeometry* trackingGeo = m_estimateHoles ? m_trackingGeometryTool->trackingGeometry().get() : nullptr;

        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Loop over "<<nMeas<<" measurements.");
        for (std::size_t m = 0 ; m < nMeas; ++m) {
            const bool isOutlier = isOutlierMeasurement(segment, m);
            Counter& increment = {isOutlier ? outliers: hits};
            const xAOD::UncalibratedMeasurement* meas = getMeasurement(segment, m);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Evaluate measurement "<<m_idHelperSvc->toString(xAOD::identify(meas))
                    <<", outlier: "<<isOutlier);
            increment.precision  += xAOD::isPrecisionHit(meas);
            const auto* muonMeas = dynamic_cast<const xAOD::MuonMeasurement*>(meas);
            increment.triggerPhi += (!muonMeas|| muonMeas->measuresPhi());
            increment.triggerEta += (!muonMeas|| !muonMeas->measuresPhi()) && !xAOD::isPrecisionHit(meas);
            // Count measurement holes of the trigger hits
            if (!isOutlier && meas->numDimensions() == 1) {
                if (meas->type() == xAOD::UncalibMeasType::RpcStripType) {\
                    const auto* re = static_cast<const MuonGMR4::RpcReadoutElement*>(muonMeas->readoutElement());
                    if (!muonMeas->measuresPhi() && re->nPhiStrips()) {
                        ++holes.triggerPhi;
                    } else if (muonMeas->measuresPhi()) {
                        ++holes.triggerEta;
                    }
                } else if (meas->type() == xAOD::UncalibMeasType::TgcStripType) {
                    const auto* re = static_cast<const MuonGMR4::TgcReadoutElement*>(muonMeas->readoutElement());
                        if (!muonMeas->measuresPhi() && re->numStrips(muonMeas->layerHash())) {
                        ++holes.triggerPhi;
                    } else if (muonMeas->measuresPhi() && re->numWireGangs(muonMeas->layerHash())) {
                        ++holes.triggerEta;
                    }
                } else if (meas->type() == xAOD::UncalibMeasType::sTgcStripType)  {
                    if (!xAOD::isPrecisionHit(meas)) {
                        ++holes.precision;
                    } else {
                        ++holes.triggerPhi;
                    }
                }
            }
            
            
            if (!m_estimateHoles) {
                continue;
            }   
            const Acts::Surface& surface = xAOD::muonSurface(meas);
            if (!muonMeas){
                continue;
            }
            crossedSurfaces.insert( muonMeas->type() != xAOD::UncalibMeasType::MdtDriftCircleType ? 
                                    m_idHelperSvc->gasGapId(muonMeas->identify()) :
                                    muonMeas->identify());
            const auto* volume = highestAlignable(trackingGeo->findVolume(volumeId(surface)));
            assert(volume != nullptr);
            if (!startSurface) {
                startSurface = MuonGMR4::bottomBoundary(*volume);
            } else if (m +1 == nMeas) {
                lastSurface = MuonGMR4::topBoundary(*volume);
            }
        }
        /// Calculate the segment start parameters
        if (m_estimateHoles) {
            const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
            auto atSurface = startSurface->intersect(tgContext, segment.position(), segment.direction(),
                                                      Acts::BoundaryTolerance::Infinite()).closest();
        
            auto startPars = Acts::BoundTrackParameters::create(tgContext, startSurface->getSharedPtr(),
                                                                ActsTrk::convertPosToActs(atSurface.position()),
                                                                segment.direction(), 1./ 5._TeV, std::nullopt,
                                                                Acts::ParticleHypothesis::muon());
            if (startPars.ok()) {
                findHoles(ctx, *startPars, lastSurface, crossedSurfaces, holes);
            } else {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Start parameters not defined.");
            }
        }
        segment.setNHits(hits.precision, hits.triggerPhi, hits.triggerEta);
        segment.setNOutliers(outliers.precision, outliers.triggerPhi, outliers.triggerEta);
        segment.setNHoles(holes.precision, holes.triggerPhi, holes.triggerEta);
    }
    void xAODSegmentCnvAlg::findHoles(const EventContext& ctx,
                                      const Acts::BoundTrackParameters& startPars,
                                      const Acts::Surface* target,
                                      const std::unordered_set<Identifier>& layersWithHits,
                                      Counter& holeCounter) const {
        

        using SurfaceRecordOptions = ActsTrk::IExtrapolationTool::SurfaceRecordOptions;
        SurfaceRecordOptions propOpts{target, m_extraHolePath};
        propOpts.recordMaterial = false;
        propOpts.recordPassive = false;
        propOpts.recordSensitive = true;

        
        auto propResult = m_extrapolationTool->propagateAndRecord(ctx, startPars, propOpts);
        if (!propResult.ok()) {
            return;
        }

        /* Loop over the track record to filter out the holes*/
        for (Acts::BoundTrackParameters& record : (*propResult)) {
            const auto* placement = dynamic_cast<const ActsTrk::SurfacePlacement*>(record.referenceSurface().surfacePlacement());
            assert(placement != nullptr);
            const auto* detEl = static_cast<const MuonGMR4::MuonReadoutElement*>(placement->detectorElement());
            using enum ActsTrk::DetectorType;
            /** Surface is already part of the measurement pool */
            if (layersWithHits.count(placement->identify())) {
                continue;
            }
            /** We expect at least one hit in the same sector to avoid
             *  that hits in the sector-overlap from the adjacent chamber are
             *  marked as holes. While they are not part of the segment by
             *  construction */ 
            if(std::ranges::none_of(layersWithHits,[&](const Identifier& recorded){
                return m_idHelperSvc->chamberIndex(recorded) == detEl->chamberIndex() &&
                        m_idHelperSvc->stationPhi(recorded) == detEl->stationPhi();
            })) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Surface "
                    <<m_idHelperSvc->toStringGasGap(placement->identify())<<" is not in the same sector");
                continue;
            }
            switch (placement->detectorType()) {
                case Mm:
                case Mdt: {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Precision hole detected "
                                   <<m_idHelperSvc->toString(placement->identify()));
                    ++holeCounter.precision;
                    break;
                } case Rpc: {
                    const auto* re = static_cast<const MuonGMR4::RpcReadoutElement*>(placement->detectorElement());
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Rpc hole detected "
                                <<m_idHelperSvc->toStringGasGap(placement->identify())<<".");
                    ++holeCounter.triggerEta;
                    holeCounter.triggerPhi += (re->nPhiStrips() > 0);
                    break;
                } case Tgc: {
                    const auto* re = static_cast<const MuonGMR4::TgcReadoutElement*>(placement->detectorElement());
                    holeCounter.triggerEta += (re->numWireGangs(placement->hash()));
                    holeCounter.triggerPhi += (re->numStrips(placement->hash()));
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Tgc hole detected "
                                <<m_idHelperSvc->toStringGasGap(placement->identify())<<".");
                    break;
                } case sTgc: {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Precision hole detected "
                                   <<m_idHelperSvc->toString(placement->identify()));
                    ++holeCounter.precision;
                    ++holeCounter.triggerPhi;
                    break;
                } default:
                    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Unknown detector type "
                                    <<placement->detectorType());
                    break;
            }
        }
    }
}
