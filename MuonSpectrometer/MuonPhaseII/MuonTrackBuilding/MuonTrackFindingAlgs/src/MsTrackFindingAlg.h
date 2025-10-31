/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINIDNGALGS_MSTRACKFINIDNGALG_H
#define MUONTRACKFINIDNGALGS_MSTRACKFINIDNGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "MuonPatternEvent/MuonPatternContainer.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IFitterTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
#include "MuonRecToolInterfacesR4/ITrackVisualizationTool.h"


#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "GaudiKernel/SystemOfUnits.h"


namespace MuonR4{
    class MsTrackFindingAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
        
            virtual ~MsTrackFindingAlg();
            /** @brief Standard algorithm hook to setup the extrapolator, retrieve the
             *         tools and declare algorithm's data dependencies */
            virtual StatusCode initialize() override final;
            /** @brief Standard algorithm execution hook */
            virtual StatusCode execute(const EventContext& ctx) const override final;

            using OptBoundPars_t = Acts::Result<Acts::BoundTrackParameters>;
            using MeasVec_t = std::vector<const xAOD::UncalibratedMeasurement*>;
        private:
            /** @brief Iterates over the search tree and combines close-by segments to a track seed.
             *         Seeds with the same segments as other seeds are deduplicated
             *  @param ctx: The event's context to access StoreGate & Conditions
             *  @param segments: Full segment container */
            std::unique_ptr<MsTrackSeedContainer> findTrackSeeds(const EventContext& ctx,
                                                                 const xAOD::MuonSegmentContainer& segments) const;

            /** @brief Attempts to fit the track seed candidate to a full track and returns whether the
             *         fit succeeded.
             *  @param gCtx: Geometry context to access the alignment of the surfaces
             *  @param mCtxc: Magnetic field context to access the field map during the fit
             *  @param cCtx: Calibration context to access the calibration constants from Store gate
             *               during the track state filling
             *  @param seed: The seed of interest to fit
             *  @param outContainer: Mutable track container to which the output track is written */
            bool fitSeedCandidate(const Acts::GeometryContext& gCtx,
                                  const Acts::MagneticFieldContext& mCtx,
                                  const Acts::CalibrationContext& cCtx,
                                  const MsTrackSeed& seed,
                                  ActsTrk::MutableTrackContainer& outContainer) const;

            /** @brief Prepares the input by the fit by collecting the measurements on the segment & 
             *  @param gCtx: Geometry context to access the alignment of the surfaces
             *  @param mCtxc: Magnetic field context to access the field map during the fit
             *  @param cCtx: Calibration context to access the calibration constants from Store gate
             *               during the track state filling
             *  @param seed: The seed of interest to fit */
            std::pair<OptBoundPars_t, MeasVec_t> prepareFit(const Acts::GeometryContext& tgContext,
                                                            const Acts::MagneticFieldContext& mfContext,
                                                            const Acts::CalibrationContext& calContext,
                                                            const MsTrackSeed& seed) const;
            
            void visualizeObj(const Acts::GeometryContext& tgContext,
                              const Acts::CalibrationContext& calContext,
                              const MsTrackSeed& seed,
                              const OptBoundPars_t& parsToExt) const;

            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container
             *         & on the NSW segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentContainer", "MuonSegmentsFromR4" };
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Pointer to the MuonDetectorManager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Temporary container write handle to push the seeds to store gate for later efficiency analysis */
            SG::WriteHandleKey<MsTrackSeedContainer> m_msTrkSeedKey{this, "MsTrkSeedKey", "MsTrackSeeds"};
            /** @brief Segment selection tool to pick the good quality segments */
            ToolHandle<ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Track fitting tool */
            ToolHandle<ActsTrk::IFitterTool> m_trackFitTool{this, "FittingTool", ""};

            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", ""};
            /** @brief Tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Visualization tool to debug the track finding */
            ToolHandle<MuonValR4::ITrackVisualizationTool> m_visualizationTool{this, "VisualizationTool", ""};
            /** @brief Maximum search window to search segments for */
            Gaudi::Property<double> m_seedHalfLength{this, "SeedHalfLength", 50.*Gaudi::Units::cm};
            /** @brief Key to the output track container */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_writeKey{this, "TrackWriteKey", "MsTracks"};
            /** @brief Dump the segments & the pre estimated track parameters */
            Gaudi::Property<bool> m_drawEvent{this , "drawEvent", false };
 
            /** @brief Pointer to the actual seeder implementation */
            std::unique_ptr<MsTrackSeeder> m_seeder{};
    };      
}

#endif