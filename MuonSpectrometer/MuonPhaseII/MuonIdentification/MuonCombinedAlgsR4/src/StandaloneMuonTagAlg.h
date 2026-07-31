/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_STANDALONEMUONTAGALG_H
#define MUONCOMBINEDALGSR4_STANDALONEMUONTAGALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h" 

#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"

#include "xAODMuonViews/FillContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/ITrackToTrackParticleCnvTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "MuonTrackEvent/MuonTag.h"

namespace MuonCombinedR4{
    /** @brief Algorithm to transform the produced MS tracks into a muon tag container
     *         and to associate the segments with the MuonTag */
    class StandaloneMuonTagAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;

        private:
            struct DataShip{
                /** @brief Collection of MS track particles to be processed */
                std::vector<const xAOD::TrackParticle*> msTracks{};
                /** @brief Collection of ms track particles expressed at the beamspot or
                           the primary vertex */
                xAOD::FillContainer<xAOD::TrackParticleContainer,
                                    xAOD::TrackParticleAuxContainer> msTracksAtIP{};
                
                /** @brief Refitted track particle container   */
                ActsTrk::MutableTrackContainer actsTracksAtIP{Acts::VectorTrackContainer{}, 
                                                              Acts::VectorMultiTrajectory{}};
                /** @brief The output container where the standalone tags are appended to  */
                xAOD::FillContainer<MuonR4::MuonTagContainer, void*> outMuonTags{};
                /** @brief Pointer to the beam spot measurement */
                const xAOD::UncalibratedMeasurement* beamSpot{nullptr};
                /** @brief The Acts::Geometry context needed for the refit */
                Acts::GeometryContext tgContext{Acts::GeometryContext::dangerouslyDefaultConstruct()};
                /** @brief The magnetic field context needed for the refit */
                Acts::MagneticFieldContext mfContext{};
                /** @brief The calibration context neeeded for the refit */
                Acts::CalibrationContext calContext{};
            };

            StatusCode prepareContainers(const EventContext& ctx, DataShip& ship) const;

            xAOD::TrackParticle* expressAtIP(const EventContext& ctx,
                                             const xAOD::TrackParticle& msTrack,
                                             DataShip& ship) const;
            /** @brief Key of the input muon track container */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_msTrackKey{this, "MsTracks", "MsTrackParticlesR4"};
            /** @brief Key of the combined muon tag container. MS tracks that were formed to a combined
             *         track are excluded from the back extrapolation to the IP. */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_combinedTagKey{this, "CombinedTags", ""};
            /** @brief Data dependency on the beam spot */
            SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotMeasurements"};
            /** @brief Tracking geometry tool */
           ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
            /** @brief Conversion tool from ACts -> xAOD:TrackParticle */
            ToolHandle<ActsTrk::ITrackToTrackParticleCnvTool> m_cnvTool{this, "TrackToTrackParticleCnvTool", ""};
            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Key to write the tag output container */
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_tagKey{this, "MsTags" , "MuonTagsSA"};
            /** @brief Key to store the extrapolated Acts track container */
            SG::WriteHandleKey<xAOD::TrackParticleContainer> m_trackPartAtIpKey{this, "TrackPartAtIpKey", 
                                                                                "MsTrksAtIpTrackParticles"};
            /** @brief Element link to the produced extrapolated Acts track */
            SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackAtIpActsLinkKey{this, "TrkAtIpLinkToActsKey", 
                                                                                          m_trackPartAtIpKey, "actsTrack"};
            /** @brief Key to the output track container */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackAtIpKey{this, "TrackAtIpKey", "MsTrksAtIp"};
            /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "TrackSummaryTool" , ""};
            /** @brief Track fitting tool */
            ToolHandle<ActsTrk::IFitterTool> m_trackFitTool{this, "FittingTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Flag toggling whether the algorithm shall express the MS tracks at 
             *         the beam spot */
            Gaudi::Property<bool> m_extrapolateToIP{this, "ExtrapolateToIP", true};
            /** @brief Flag toggling whether the track fit shall be re executed with the
             *         beamspot as an exta constraint */
            Gaudi::Property<bool> m_refitWithBS{this, "RefitWithBeamSpot" , true};
            /** @brief */
            Gaudi::Property<bool> m_rejectFailedIP{this, "rejectFailedIpExtrap", false};
            
   };
}


#endif