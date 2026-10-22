/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CombinedFitAlg.h"

#include "ActsEvent/Decoration.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonViews/ContainerDecorator.h"
#include "xAODMuonPrepData/UtilFunctions.h"
namespace {
    /** @brief Print debug information about the track particle
     *  @param p: The reference to the track particle of interest */
    std::string print(const xAOD::TrackParticle& p) {
        std::stringstream sstr{};
        sstr<<"particle with " <<(p.pt()*1.e-3)<<" [GeV], eta: "<<p.eta()<<", phi: "
             <<p.phi()<<",  q:"<<p.charge()<<", author: "<<p.trackFitter()<<std::endl;
        return sstr.str();
    }

}


namespace MuonCombinedR4 {
    StatusCode CombinedFitAlg::initialize() {
        ATH_CHECK(m_cnvTool.retrieve());
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());

        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_stacoKey.initialize());
        ATH_CHECK(m_outTagKey.initialize());
        ATH_CHECK(m_beamSpotKey.initialize(false));
        
        ATH_CHECK(m_CmbParticleKey.initialize());
        ATH_CHECK(m_MeParticleKey.initialize(m_fitMEtrack));

        ATH_CHECK(m_cmbTrackKey.initialize());
        ATH_CHECK(m_meTrackKey.initialize(m_fitMEtrack));
        
        ATH_CHECK(m_meTrackLinkKey.initialize(m_fitMEtrack));
        ATH_CHECK(m_cmbTrackLinkKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode CombinedFitAlg::prepareDataShip(const EventContext& ctx, DataShip& ship) const {
        const MuonR4::MuonTagContainer* stacoTags{nullptr};
        ATH_CHECK(SG::get(stacoTags, m_stacoKey, ctx));
        /** Sort the tags by their MS track particle  */
        for (const MuonR4::MuonTag* tag : *stacoTags) {
            ship.preMatchedTags[tag->msTrack()].push_back(tag);
        }
        for (auto & [_, assocTags] : ship.preMatchedTags) {
            /// Sort all candidates per MS track according to their matching chi2
            std::ranges::sort(assocTags, [](const MuonR4::MuonTag* a,
                                            const MuonR4::MuonTag* b) {
                using enum MuonR4::MuonTag::ParamDef;
                constexpr float max = std::numeric_limits<float>::max();
                return a->parameter<float>(msInnerMatchChi2).value_or(max) <
                       b->parameter<float>(msInnerMatchChi2).value_or(max);
            });
        }
        ATH_CHECK(ship.outputTags.record(m_outTagKey, ctx));
        ATH_CHECK(ship.cmbParticles.record(m_CmbParticleKey, ctx));
        ATH_CHECK(ship.meParticles.record(m_MeParticleKey, ctx));
        
        ship.calContext = m_ctxProvider.getCalibrationContext(ctx);
        ship.mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
        ship.tgContext = m_ctxProvider.getGeometryContext(ctx);
        return StatusCode::SUCCESS;
    }
    StatusCode CombinedFitAlg::execute(const EventContext& ctx) const  {
        DataShip ship{};
        ATH_CHECK(prepareDataShip(ctx, ship));

        for (const auto& [msTrack, tags] : ship.preMatchedTags) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Process MS track "<<print(*msTrack));
            for (const MuonR4::MuonTag* candiate: tags) {
                if (!fitMuonTag(*candiate, ship)) {
                    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fit failed.");
                    continue;
                }
            }
        }

        /** Fill the track particles into the output containers */
        ATH_CHECK(writeTracksToStoreGate(ctx, m_cmbTrackKey, m_cmbTrackLinkKey, 
                                         *ship.cmbParticles, std::move(ship.actsCmbTracks)));
        ATH_CHECK(writeTracksToStoreGate(ctx, m_meTrackKey, m_meTrackLinkKey, 
                                         *ship.meParticles, std::move(ship.actsMeTracks)));
        return StatusCode::SUCCESS;
    }

    MuonR4::MuonTag* CombinedFitAlg::fitMuonTag(const MuonR4::MuonTag& tag,
                                                DataShip& ship) const {
        std::vector<Acts::SourceLink> measurements = extractMeasurements(tag);
        if (measurements.empty()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No associated measurmeents");
            return nullptr;
        }
        auto idTrack = ActsTrk::getActsTrack(*tag.idTrack());
        /** Use the Id tack parameters as start parameters */
        Acts::BoundTrackParameters startPars = idTrack->createParametersAtReference();

        auto fitResult = m_trackFitTool->fit(measurements, startPars,
                                             ship.tgContext, ship.mfContext,
                                             ship.calContext, &startPars.referenceSurface());
        if (!fitResult) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fit failed");
            return nullptr;
        }
        /** Copy the fit result to the output container */
        ship.actsCmbTracks.ensureDynamicColumns(*fitResult);
        auto destProxy = ship.actsCmbTracks.getTrack(ship.actsCmbTracks.addTrack());
        destProxy.copyFrom(fitResult->getTrack(0));
        /** Create the output track particle */
        auto* combParticle = ship.cmbParticles->push_back(std::make_unique<xAOD::TrackParticle>());
        auto* combTag = ship.outputTags->push_back(std::make_unique<MuonR4::MuonTag>());
        combTag->setAuthor(MuonR4::MuonTag::Author::MuidCo);
        combTag->setCbTrack(combParticle);
        combTag->setIdTrack(tag.idTrack());
        combTag->setMsTrack(tag.msTrack());
        return combTag;
    }
    void CombinedFitAlg::buildMETrack(MuonR4::MuonTag& combinedTag,
                                      DataShip& ship) const {
        if (!m_fitMEtrack) {
            return;
        }
        /// Fetch the combined track from the data ship
        auto combinedTrk = ship.actsCmbTracks.at(combinedTag.cbTrack()->index());
        /// Collect the muon measurements from the combined track        
        std::vector<Acts::SourceLink> measurements{};
        for (const auto state : combinedTrk.trackStates()) {
            if (!state.hasUncalibratedSourceLink()) {
                continue;
            }
            Acts::SourceLink sl = state.getUncalibratedSourceLink();
            if (xAOD::isMuon(ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(sl))) {
                measurements.push_back(std::move(sl));
            }
        }
        /// @todo: Add the beamspot if neccessary
        Acts::BoundTrackParameters startPars = combinedTrk.createParametersAtReference();

        auto fitResult = m_trackFitTool->fit(measurements, startPars,
                                             ship.tgContext, ship.mfContext,
                                             ship.calContext, &startPars.referenceSurface());
        if (!fitResult) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fit failed");
            return;
        }
        ship.actsMeTracks.ensureDynamicColumns(*fitResult);
        auto destProxy = ship.actsMeTracks.getTrack(ship.actsMeTracks.addTrack());
        destProxy.copyFrom(fitResult->getTrack(0));
        /** Create the output track particle */
        auto* meParticle = ship.meParticles->push_back(std::make_unique<xAOD::TrackParticle>());
        combinedTag.setMeTrack(meParticle);
    }
 
    StatusCode 
        CombinedFitAlg::writeTracksToStoreGate(const EventContext& ctx,
                                               const SG::WriteHandleKey<ActsTrk::TrackContainer>& key,
                                               const SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>& decorKey,
                                               xAOD::TrackParticleContainer& particleContainer,
                                               ActsTrk::MutableTrackContainer&& fittedTracks) const {
        if (key.empty()) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Empty key do nothing");
            return StatusCode::SUCCESS;
        }
        Acts::ConstVectorTrackContainer ctrackBackend{std::move(fittedTracks.container())};
        Acts::ConstVectorMultiTrajectory ctrackStateBackend{std::move(fittedTracks.trackStateContainer())};
        auto ctc = std::make_unique<ActsTrk::TrackContainer>(std::move(ctrackBackend),
                                                             std::move(ctrackStateBackend));

        SG::WriteHandle writeHandle{key, ctx};
        ATH_CHECK(writeHandle.record(std::move(ctc)));

        xAOD::ContainerDecorator dec_trackLink{decorKey, ctx, 
                                               ElementLink<ActsTrk::TrackContainer>{}};
        for (std::size_t trk = 0 ; trk < particleContainer.size(); ++trk) {
            const ActsTrk::TrackContainer::ConstTrackProxy convMe = writeHandle->getTrack(trk);
            xAOD::TrackParticle* convTo = particleContainer.at(trk);
            ATH_CHECK(m_cnvTool->convert(*convTo, ctx, convMe, convMe.referenceSurface()));
            dec_trackLink(*convTo) = ElementLink<ActsTrk::TrackContainer>(*writeHandle, convMe.index(), ctx);
        }
        return StatusCode::SUCCESS;
    }

    std::vector<Acts::SourceLink> 
        CombinedFitAlg::extractMeasurements(const MuonR4::MuonTag& muonTag) const {
        std::vector<Acts::SourceLink> measurements{};
        measurements.append_range(extractMeasurements(*muonTag.idTrack()));
        measurements.append_range(extractMeasurements(*muonTag.msTrack()));
        return measurements;
    }
    std::vector<Acts::SourceLink> 
        CombinedFitAlg::extractMeasurements(const xAOD::TrackParticle& trackPart) const {
        std::vector<Acts::SourceLink> measurements{};
        auto track = ActsTrk::getActsTrack(trackPart);
        if (!track) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to retrieve the associated ACTS track from "
                <<print(trackPart));
             return measurements;
        }
        for (const auto state : track->trackStates()) {
            if (!state.hasUncalibratedSourceLink()) {
                continue;
            }
            Acts::SourceLink sl = state.getUncalibratedSourceLink();
            /** Filter out the auxiliary external constraints */
            if (ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(sl)->type() == 
                xAOD::UncalibMeasType::Other) {
                continue;
            }
            measurements.push_back(std::move(sl));
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extracted "<<measurements.size()<<" measurments from "
                <<print(trackPart));
        return measurements;
    }
}