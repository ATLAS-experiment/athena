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
             *  @param cCtx: Calibration context to access the calibration constants from Store gate
             *               during the track state filling
             *  @param seed: The seed of interest to fit */
            std::pair<OptBoundPars_t, MeasVec_t> prepareFit(const Acts::GeometryContext& tgContext,
                                                            const Acts::CalibrationContext& calContext,
                                                            const MsTrackSeed& seed) const;

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
            /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Visualization tool to debug the track finding */
            ToolHandle<MuonValR4::ITrackVisualizationTool> m_visualizationTool{this, "VisualizationTool", ""};
            /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" , ""};
            /** @brief Maximum search window to search segments for */
            Gaudi::Property<double> m_seedHalfLength{this, "SeedHalfLength", 50.*Gaudi::Units::cm};
            /** @brief Key to the output track container */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_writeKey{this, "TrackWriteKey", "MsTracks"};
            /** @brief Use ML-guided segment grouping before baseline seeding */
            Gaudi::Property<bool> m_useMlSeeder{this, "UseMlSeeder", false, "Use segment-edge ML candidate ids to split seeding"};
            /** @brief Segment decoration containing vector<unsigned> candidate IDs */
            Gaudi::Property<std::string> m_mlCandidateDecoration{this, "MlCandidateDecoration", "trackCandidateIds", "Segment vector<unsigned> decoration with ML track-candidate ids"};
            Gaudi::Property<unsigned> m_mlMinSegmentsPerCandidate{this, "MlMinSegmentsPerCandidate", 2};
            Gaudi::Property<bool> m_mlFallbackToBaselineIfUndecorated{
                this, "MlFallbackToBaselineIfUndecorated", true,
                "Run baseline seeder if the input segment container has no ML decoration"};
            Gaudi::Property<bool> m_mlFallbackToBaselineIfNoCandidates{
                this, "MlFallbackToBaselineIfNoCandidates", false,
                "Run baseline seeder if ML grouping produced no seed candidates"};
            Gaudi::Property<bool> m_mlRunCandidatesInParallel{
                this, "MlRunCandidatesInParallel", true,
                "Run baseline seeding independently for ML candidate groups in parallel"};
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_mlCandidateDecorKey{
                this, "MlCandidateDecorationKey", "", "Scheduler dependency on ML candidate decoration"};
    };      
}

#endif