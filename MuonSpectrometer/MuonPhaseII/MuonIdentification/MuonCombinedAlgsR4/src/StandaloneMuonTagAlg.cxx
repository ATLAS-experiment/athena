/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StandaloneMuonTagAlg.h"

#include "ActsEvent/Decoration.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

#include "xAODMuonViews/ContainerDecorator.h"
namespace MuonCombinedR4{
    using namespace Acts::UnitLiterals;

    StatusCode StandaloneMuonTagAlg::initialize(){
        ATH_CHECK(m_msTrackKey.initialize());

        if (m_refitWithBS) {
            m_extrapolateToIP = true;
        }
        ATH_CHECK(m_combinedTagKey.initialize(m_extrapolateToIP && !m_combinedTagKey.empty()));
        ATH_CHECK(m_trackPartAtIpKey.initialize(m_extrapolateToIP));
        ATH_CHECK(m_trackAtIpKey.initialize(m_refitWithBS));
        ATH_CHECK(m_trackAtIpActsLinkKey.initialize(m_refitWithBS));


        ATH_CHECK(m_trackFitTool.retrieve(EnableTool{m_refitWithBS}));
        ATH_CHECK(m_extrapolationTool.retrieve(EnableTool{m_extrapolateToIP}));
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_cnvTool.retrieve(EnableTool{m_refitWithBS}));
        ATH_CHECK(m_beamSpotKey.initialize(m_extrapolateToIP));
        ATH_CHECK(m_ctxProvider.initialize(m_extrapolateToIP));
        
        ATH_CHECK(m_tagKey.initialize());
        ATH_CHECK(m_summaryTool.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode StandaloneMuonTagAlg::prepareContainers(const EventContext& ctx, DataShip& ship) const {
        const xAOD::TrackParticleContainer* msTracks{nullptr};
        ATH_CHECK(SG::get(msTracks, m_msTrackKey, ctx));

        const MuonR4::MuonTagContainer* combinedTags{};
        ATH_CHECK(SG::get(combinedTags, m_combinedTagKey, ctx));
        /** Fill the MS tags to be processed by the algorithm */
        ship.msTracks.reserve(msTracks->size());
        std::copy_if(msTracks->begin(), msTracks->end(), std::back_inserter(ship.msTracks),
                    [combinedTags, this](const xAOD::TrackParticle* msTrack) {
                        // No combined tag container passed -> process them all
                        if (!combinedTags) {
                            return true;
                        }
                        if (std::any_of(combinedTags->begin(), combinedTags->end(), 
                            [msTrack](const MuonR4::MuonTag* cmbTag){
                                return cmbTag->msTrack() == msTrack;
                        })) {
                            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" -  Skip track particle "<<msTrack->pt()<<", "
                                        <<", eta: "<<msTrack->eta()<<", phi: "<<msTrack->phi()
                                        <<" as it's part of a combined muon already.");
                            return false;
                        }
                        return true;
                    });
        
        const xAOD::UncalibratedMeasurementContainer* beamSpotCont{};
        ATH_CHECK(SG::get(beamSpotCont, m_beamSpotKey, ctx));
        if (!beamSpotCont) {
            return StatusCode::SUCCESS;
        }
        ship.tgContext = m_ctxProvider.getGeometryContext(ctx);
        ship.mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
        ship.calContext = m_ctxProvider.getCalibrationContext(ctx);
        if (beamSpotCont->empty()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No beam spot measurement available");
            return StatusCode::SUCCESS;
        }
        ship.beamSpot = beamSpotCont->front();

        return StatusCode::SUCCESS;
    }
    StatusCode StandaloneMuonTagAlg::execute(const EventContext& ctx) const{
 
        DataShip dataShip{};
        ATH_CHECK(prepareContainers(ctx, dataShip));
        
        for (const xAOD::TrackParticle* msTrack : dataShip.msTracks) {
            auto msTag = std::make_unique<MuonR4::MuonTag>(); 
            msTag->setAuthor(xAOD::Muon::Author::MuidSA);
            msTag->setMsTrack(msTrack);
          
            auto track = ActsTrk::getActsTrack(*msTrack);
            if (!track) {
                ATH_MSG_ERROR("No Acts track object is associated");
                return StatusCode::FAILURE;
            }
            if (m_extrapolateToIP) {
                const xAOD::TrackParticle* meTrack = expressAtIP(ctx, *msTrack, dataShip);
                if (meTrack) {
                    msTag->setMeTrack(meTrack);
                } else if (m_rejectFailedIP){
                    continue;
                }
            }
            msTag->setSummary(m_summaryTool->makeSummary(ctx, *track));
            msTag->setSegments((*track).component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks"));
            dataShip.outMuonTags->push_back(std::move(msTag));

        }
        ATH_CHECK(dataShip.outMuonTags.record(m_tagKey, ctx));
        ATH_CHECK(dataShip.msTracksAtIP.record(m_trackPartAtIpKey, ctx));
        if (m_refitWithBS) {
            // Constant declination
            Acts::ConstVectorTrackContainer ctrackBackend{std::move(dataShip.actsTracksAtIP.container())};
            Acts::ConstVectorMultiTrajectory ctrackStateBackend{std::move(dataShip.actsTracksAtIP.trackStateContainer())};
            auto ctc = std::make_unique<ActsTrk::TrackContainer>(std::move(ctrackBackend),
                                                                 std::move(ctrackStateBackend));
  
            SG::WriteHandle writeHandle{m_trackAtIpKey, ctx};
            ATH_CHECK(writeHandle.record(std::move(ctc)));
            xAOD::ContainerDecorator dec_trackLink{m_trackAtIpActsLinkKey, ctx, 
                                                   ElementLink<ActsTrk::TrackContainer>{}};
            for (std::size_t trk = 0 ; trk < dataShip.msTracksAtIP->size(); ++trk) {
                const ActsTrk::TrackContainer::ConstTrackProxy convMe = writeHandle->getTrack(trk);
                xAOD::TrackParticle* convTo = dataShip.msTracksAtIP->at(trk);
                ATH_CHECK(m_cnvTool->convert(*convTo, ctx, convMe, convMe.referenceSurface()));
                m_summaryTool->copySummary(m_summaryTool->makeSummary(ctx, convMe), *convTo);
                dec_trackLink(*convTo) = ElementLink<ActsTrk::TrackContainer>(*writeHandle, convMe.index(), ctx);
            }
        }
        return StatusCode::SUCCESS;
    }
    xAOD::TrackParticle* StandaloneMuonTagAlg::expressAtIP(const EventContext& ctx,
                                                           const xAOD::TrackParticle& msTrack,
                                                           DataShip& ship) const {

        auto startPars = ActsTrk::getActsTrack(msTrack)->createParametersAtReference();
        std::shared_ptr<const Acts::Surface> targetSurf{};
        if (ship.beamSpot) {
            ActsTrk::detail::xAODUncalibMeasSurfAcc surfAcc{m_trackingGeometrySvc.get()};
            targetSurf = surfAcc.get(ship.beamSpot)->getSharedPtr();
        } else {
            targetSurf = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Transform3::Identity());
        }
        // Do not refit if there is no beam spot or if there is no
        // beam spot surface
       
        auto extPars = m_extrapolationTool->propagate(ctx, startPars, *targetSurf, Acts::Direction::Backward());
        if(!extPars.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to back extrapolate the parameters "
                <<startPars<<"\n to beam spot.");
            return nullptr;
        }
        const Amg::Vector3D extPos = extPars->position(ship.tgContext);
        ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__<<" Extrapolated MS track to "<<Amg::toString(extPos)<<".");
        if (!m_refitWithBS || !ship.beamSpot) {
            auto meTrack = ship.msTracksAtIP->push_back(std::make_unique<xAOD::TrackParticle>());
            (*meTrack) = msTrack;

           
            const Acts::BoundVector boundParams = extPars->parameters();
            meTrack->setDefiningParameters(extPos.perp(), extPos.z(),
                                           boundParams[Acts::eBoundPhi],
                                           boundParams[Acts::eBoundTheta],
                                           boundParams[Acts::eBoundQOverP] * 1_MeV);
            return meTrack;
        }
        ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__
            <<" Attempt to refit the MS track with beamspot constraint.");
        std::vector<const xAOD::UncalibratedMeasurement*> measurements{ship.beamSpot};
        for (const auto state : ActsTrk::getActsTrack(msTrack)->trackStates()) {
            if (state.hasUncalibratedSourceLink()) {
                measurements.push_back(ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink()));
            }
        }
        auto fitResult = m_trackFitTool->fit(measurements, *extPars,
                                             ship.tgContext, ship.mfContext, 
                                             ship.calContext, targetSurf.get());

        if (!fitResult) {
            return nullptr;
        }
        ship.actsTracksAtIP.ensureDynamicColumns(*fitResult);
        auto destProxy = ship.actsTracksAtIP.getTrack(ship.actsTracksAtIP.addTrack());
        destProxy.copyFrom(fitResult->getTrack(0));
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Refitted track @"<<
            Amg::toString(destProxy.createParametersAtReference().position(ship.tgContext)));
        /** Fit succeeded. Return a new track particle, which is empty.
          * The filling of the parameters happens at the end of the loop */
        return ship.msTracksAtIP->push_back(std::make_unique<xAOD::TrackParticle>());
    }
}
