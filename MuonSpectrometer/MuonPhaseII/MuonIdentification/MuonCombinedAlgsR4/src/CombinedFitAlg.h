/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_COMBINEDFITALG_H
#define MUONCOMBINEDALGSR4_COMBINEDFITALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"



#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/ITrackToTrackParticleCnvTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"

#include "ActsEvent/ContextUtility.h"
#include "ActsEvent/TrackContainer.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"


#include "MuonTrackEvent/MuonTag.h"
#include "MuonTrackEvent/TrackingHelpers.h"

#include "xAODMuonViews/FillContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

namespace MuonCombinedR4 {

     class CombinedFitAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;

        private:
            /** @brief Helper struct to ship the input output data
             *         during the combined track fit */
            struct DataShip{
                /** @brief The prematched combined muon tags */
                std::map<const xAOD::TrackParticle*,
                        std::vector<const MuonR4::MuonTag*>,
                        MuonR4::ParticleSorter> preMatchedTags{};
                /** @brief The output combined tag collection */
                xAOD::FillContainer<MuonR4::MuonTagContainer, void*> outputTags{};
                /** @brief The output combined particle container  */
                xAOD::FillContainer<xAOD::TrackParticleContainer,
                                    xAOD::TrackParticleAuxContainer> cmbParticles{};
                /** @brief The output ME particle container */
                xAOD::FillContainer<xAOD::TrackParticleContainer,
                                    xAOD::TrackParticleAuxContainer> meParticles{};
                /** @brief The geometry context to align the surfaces during the fit */
                Acts::GeometryContext tgContext{Acts::GeometryContext::dangerouslyDefaultConstruct()};
                /** @brief The magnetic field context */
                Acts::MagneticFieldContext mfContext{};
                /** @brief The calibration context */
                Acts::CalibrationContext calContext{};
                /** @brief The combined acts track container */
                ActsTrk::MutableTrackContainer actsCmbTracks{Acts::VectorTrackContainer{}, 
                                                                  Acts::VectorMultiTrajectory{}};
                /** @brief The ME track container */
                ActsTrk::MutableTrackContainer actsMeTracks{Acts::VectorTrackContainer{}, 
                                                            Acts::VectorMultiTrajectory{}};
            };
            /** @brief Load the input containers from store gate and setup the write Handles 
             *         for the output
             *  @param ctx: EventContext for StoreGate access
             *  @param ship: DataShip carrying the needed data containers */
            StatusCode prepareDataShip(const EventContext& ctx, DataShip& ship) const;
            /** @brief Attempt to fit the muon tag to a combined track particle
             *  @param tag: The STACO tag to use
             *  @param ship: The dataship to save the output track and track particles */
            MuonR4::MuonTag* fitMuonTag(const MuonR4::MuonTag& tag,
                                        DataShip& ship) const;
            
            void buildMETrack(MuonR4::MuonTag& combinedTag,
                               DataShip& ship) const;
            /** @brief Convert the ActsTracks to track particles and write the 
             *         Acts track container to store gate. Also establish the link 
             *         to the track from the filled track particle
             *  @param ctx: Event Context to access StoreGate to write the container to 
             *              create the element links
             *  @param key: The key to write the Acts::Track container to StoreGate
             *  @param decorKey: DecorHandleKey to establish the link to the Acts::Tracks
             *  @param particleContainer: Mutable reference to the track particle which 
             *                            particles are filled with the track information
             * @param fittedTracks: The fitted track to write to store gate */
            StatusCode writeTracksToStoreGate(const EventContext& ctx,
                                              const SG::WriteHandleKey<ActsTrk::TrackContainer>& key,
                                              const SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>& decorKey,
                                              xAOD::TrackParticleContainer& particleContainer,
                                              ActsTrk::MutableTrackContainer&& fittedTracks) const;

            /** @brief Retrieve the ID & Muon measurements from the associated ID & MS track particles
             *  @param muonTag: The STACO tag carrying the pointers to the MS & IDtracks to be combined */
            std::vector<Acts::SourceLink> extractMeasurements(const MuonR4::MuonTag& muonTag) const;
            /** @brief Retrieve the uncalibrated measurements from the associated Acts::Track
             *  @param trackPart: Reference to the track particle of interest */
            std::vector<Acts::SourceLink> extractMeasurements(const xAOD::TrackParticle& trackPart) const;
            /** @brief Conversion tool from Acts -> xAOD:TrackParticle */
            ToolHandle<ActsTrk::ITrackToTrackParticleCnvTool> m_cnvTool{this, "TrackToTrackParticleCnvTool", ""};
            /** @brief Track fitting tool */
            ToolHandle<ActsTrk::IFitterTool> m_trackFitTool{this, "FittingTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Use STACO as input to the MuidCo chain as the track pair is already done upstream */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_stacoKey{this, "StacoKey", "MuonTagsSTACO"};
            /** @brief Write key of the created combined tags */
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_outTagKey{this, "MuonTagKey" , "MuonTagsMuidCo"};
             /** @brief Data dependency on the beam spot */
            SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotMeasurements"};
            /** @brief Write key of the created combined track particles */
            SG::WriteHandleKey<xAOD::TrackParticleContainer> m_CmbParticleKey{this, "CombinedParticleKey", 
                                                                                 "MuonCmbTrackParticles"};
            /** @brief Write key of the created ME track particles */
            SG::WriteHandleKey<xAOD::TrackParticleContainer> m_MeParticleKey{this, "MeParticleKey",
                                                                                "MuonMeTrackParticles"};
            /** @brief Write key to store the fitted Acts combined tracks */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_cmbTrackKey{this, "CombinedTrackKey", "MuonCmbTracks"};
            /** @brief Write key to store the fitted Acts ME tracks */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_meTrackKey{this, "MeTrackKey", "MuonMeTracks"};

            /** @brief Element link to the produced extrapolated Acts track */
            SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_cmbTrackLinkKey{this, "CombinedTrackLinkKey", 
                                                                                    m_CmbParticleKey, "actsTrack"};

            SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_meTrackLinkKey{this, "MeTrackLinKey", 
                                                                                  m_MeParticleKey, "actsTrack"};
            /** @brief Toggle whether the ME track is formed after the combined fit */
            Gaudi::Property<bool> m_fitMEtrack{this, "fitMETracks" , false};
    };
}


#endif