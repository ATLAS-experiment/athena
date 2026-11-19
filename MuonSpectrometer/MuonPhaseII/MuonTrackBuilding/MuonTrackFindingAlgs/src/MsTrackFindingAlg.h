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

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsEvent/AuxiliaryMeasurementHandler.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"

#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
#include "MuonRecToolInterfacesR4/ITrackVisualizationTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingTool.h"
#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"


namespace MuonR4{
    /**  @brief The MSTrackFindingAlg combines the built MS segments to track seeds and then attempts to fit the hits on a seed. 
     *          In the standard case, the track seed is formed by expressing all segments on a common virtual cylinder and 
     *          to search for accumulation points on that surface. The SeedingTool is also repsonsible to provide an initial
     *          estimate on the track parameters in terms of direction and trajectory curvature. The measurements are then 
     *          extracted from the segments associated with the seed an a track fit is attempted. Failed seeds are retried 
     *          by selecting any combination of N-1 segments and then to fit each of them. The best result is then
     *          taken for the next iteration.
     * 
     *          Track candidates are then extrapolated to the Calorimeter exit volume to express the parameters there
     *          Failed extrapolation cases may be discarded depending on the configuration. */
    class MsTrackFindingAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
        
            virtual ~MsTrackFindingAlg();
            /** @brief Standard algorithm hook to setup the extrapolator, retrieve the
             *         tools and declare algorithm's data dependencies */
            virtual StatusCode initialize() override final;
            /** @brief Standard algorithm execution hook */
            virtual StatusCode execute(const EventContext& ctx) const override final;
            /** @brief Abrivation for the result of bound track parameters */
            using OptBoundPars_t = Acts::Result<Acts::BoundTrackParameters>;
            /** @brief Abrivation for a vector of measurements */
            using MeasVec_t = std::vector<const xAOD::UncalibratedMeasurement*>;
            /** @brief Auxiliary object to create the pseudo measurements */
            using PseudoMeasHandle_t = ActsTrk::AuxiliaryMeasurementHandler::MeasurementProvider;
        private:
            /** @brief Attempts to fit the track seed candidate to a full track and returns whether the
             *         fit succeeded.
             *  @param tgContext: Geometry context to access the alignment of the surfaces
             *  @param mfContextc: Magnetic field context to access the field map during the fit
             *  @param calContext: Calibration context to access the calibration constants from Store gate
             *               during the track state filling
             *  @param seed: The seed of interest to fit
             *  @param outContainer: Mutable track container to which the output track is written
             *  @param auxMeasContainer: Write handle to create auxiliary measurements for each fit if
             *                           needed.  */
            bool fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                  const Acts::MagneticFieldContext& mfContext,
                                  const Acts::CalibrationContext& calContext,
                                  const MsTrackSeed& seed,
                                  ActsTrk::MutableTrackContainer& outContainer,
                                  PseudoMeasHandle_t& auxMeasContainer) const;
            
            template <std::size_t N> 
            std::unique_ptr<ActsTrk::MutableTrackContainer> 
                fitBestCombination(const Acts::GeometryContext& tgContext,
                                   const Acts::MagneticFieldContext& mfContext,
                                   const Acts::CalibrationContext& calContext,
                                   const MsTrackSeed& seed,
                                   std::optional<Acts::BoundTrackParameters> startPars,
                                   PseudoMeasHandle_t& auxMeasContainer) const;
            /** @brief Collect all measurements from the segments associated with the seed
             *         If there are less than 2 phi measurements auxiliary measurements are
             *         inserted to stabilize the later fit result
             *  @param tgContext: The geometry context to place later the auxiliary surface
             *  @param startPars: The track start parameters in use
             *  @param seed: The seed in use from which associated segments the measurments are taken
             *  @param auxMeasContainer: Mutable refernce to the auxiliary container */
            MeasVec_t collectMeasurements(const Acts::GeometryContext& tgContext,
                                          const Acts::BoundTrackParameters& startPars,
                                          const MsTrackSeed& seed,
                                          PseudoMeasHandle_t& auxMeasContainer) const;
            /** @brief Evaluate whether the fit result is acceptable in terms of chi2   
             *         and whether the fitted track contains precision hits on N stations
             *  @param ctx: EventContext to access the conditions data in the track summary
             *              evaluation
             * @param fitResult: The fit container to test
             * @param reqStation: The threshold on the number of stations that need to contain
             *                     a precision hit.*/
            bool goodFit(const EventContext& ctx,
                         std::unique_ptr<ActsTrk::MutableTrackContainer>& fitResult,
                         const std::size_t reqStations) const;
            /** @brief Extrapolate the tracks back to the calo exit and update the track parameters
             *          accoringly. If the extrapolation fails, a false is returned
             *  @param ctx: EventContext to access the GeometryContext and the magnetic field
             *  @param track: The track to be expressed at the calorimeter entrance */            
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
            /** @brief Utility to impose external constraints in the fit  */
            ActsTrk::AuxiliaryMeasurementHandler m_pseudoMeasuremntCreator{this};
            /** @brief Visualization tool to debug the track finding */
            ToolHandle<MuonValR4::ITrackVisualizationTool> m_visualizationTool{this, "VisualizationTool", ""};
            /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" , ""};
            /** @brief Key to the output track container */
            SG::WriteHandleKey<ActsTrk::TrackContainer> m_writeKey{this, "TrackWriteKey", "MuonMsTracks"};

    };      
}

#endif