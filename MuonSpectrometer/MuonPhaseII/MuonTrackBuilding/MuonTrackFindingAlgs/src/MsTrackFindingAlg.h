/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"


#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ContextUtility.h"

#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
#include "MuonRecToolInterfacesR4/ITrackVisualizationTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingTool.h"

#include "ActsEvent/TrackContainerHandlesHelper.h"
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
            /** @brief Attempts to fit the track seed candidate to a full track and returns whether the
             *         fit succeeded.
             *  @param tgContext: Geometry context to access the alignment of the surfaces
             *  @param mfContextc: Magnetic field context to access the field map during the fit
             *  @param calContext: Calibration context to access the calibration constants from Store gate
             *               during the track state filling
             *  @param seed: The seed of interest to fit
             *  @param outContainer: Mutable track container to which the output track is written */
            bool fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                  const Acts::MagneticFieldContext& mfContext,
                                  const Acts::CalibrationContext& calContext,
                                  const MsTrackSeed& seed,
                                  ActsTrk::MutableTrackContainer& outContainer) const;

            /** @brief Prepares the input to the fit by collecting the measurements on the segment & 
             *  @param tgContext: Geometry context to access the alignment of the surfaces
             *  @param calContext: Calibration context to access the calibration constants from Store gate
             *               during the track state filling
             *  @param seed: The seed of interest to fit */
            std::pair<OptBoundPars_t, MeasVec_t> prepareFit(const Acts::GeometryContext& tgContext,
                                                            const Acts::CalibrationContext& calContext,
                                                            const MsTrackSeed& seed) const;

            bool expressAtCaloExit(const EventContext& ctx,
                                   ActsTrk::MutableTrackContainer::TrackProxy track) const;
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Pointer to the MuonDetectorManager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Temporary container write handle to push the seeds to store gate for later efficiency analysis */
            SG::WriteHandleKey<MsTrackSeedContainer> m_msTrkSeedKey{this, "MsTrkSeedKey", "MsTrackSeeds"};
            /** @brief The track seeding tool to construct the seed candidates and to estimate the initial parameters */
            ToolHandle<ITrackSeedingTool> m_seedingTool{this, "SeedingTool", ""};
            /** @brief Track fitting tool */
            ToolHandle<ActsTrk::IFitterTool> m_trackFitTool{this, "FittingTool", ""};
            /** @brief Calibration tool to fill the track states */
            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Service handle to the tracking geometry service */
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
            /** @brief Propagate the track at the MS entry and express its parameters */
            Gaudi::Property<bool> m_expressAtMsEntrance{this, "expressAtMsEntrance", true};
            /** @brief Ignore failed track extrapolations to the entrance */
            Gaudi::Property<bool> m_ignoreFailedMsEntrance{this, "ignoreFailedExtpMsEntrance", true};
            /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Visualization tool to debug the track finding */
            ToolHandle<MuonValR4::ITrackVisualizationTool> m_visualizationTool{this, "VisualizationTool", ""};
            /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" , ""};
            /** @brief Key to the output track container */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_writeKey{this, "TrackWriteKey", "MsTracks"};
    };      
}

#endif