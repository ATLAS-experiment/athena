/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGBASEALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGBASEALG_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "src/TrackStatePrinterTool.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

// ACTS
#include "Acts/EventData/ProxyAccessor.hpp"

// ActsTrk
#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "IMeasurementSelector.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

// Handle Keys
#include "src/detail/Definitions.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/TrackContainer.h"

namespace ActsTrk {
  namespace detail {
    class TrackFindingMeasurements;
    class SharedHitCounter;
  }

  class TrackFindingBaseAlg : public AthReentrantAlgorithm {
  public:
    TrackFindingBaseAlg(const std::string &name, ISvcLocator *pSvcLocator);
    ~TrackFindingBaseAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  protected:
    using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<detail::RecoTrackContainer>;

    struct MeasurementSelectorConfig {
       std::vector<std::pair<float, float>> m_chi2CutOffOutlier;
       std::vector<float>                   m_etaBins;
    } m_measurementSelectorConfig;

    struct DetectorContextHolder {
      Acts::GeometryContext geometry;
      Acts::MagneticFieldContext magField;
      Acts::CalibrationContext calib;
    };

    struct TrackFindingDefaultOptions {
      TrackFinderOptions options;
      TrackFinderOptions secondOptions;
      std::unique_ptr<ActsTrk::IMeasurementSelector> measurementSelector;
    };

    // Access Acts::CombinatorialKalmanFilter etc using "pointer to implementation"
    // so we don't have to instantiate the heavily templated classes in the header.
    // To maintain const-correctness, only use this via the accessor functions.
    struct CKF_pimpl;

    CKF_pimpl &trackFinder();
    const CKF_pimpl &trackFinder() const;

    std::unique_ptr<CKF_pimpl> m_trackFinder;

    detail::xAODUncalibMeasSurfAcc m_unalibMeasSurfAcc {};


    // Tool Handles
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
    ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
    ToolHandle<ActsTrk::TrackStatePrinterTool> m_trackStatePrinter{this, "TrackStatePrinter", "", "optional track state printer"};
    ToolHandle<ActsTrk::IActsToTrkConverterTool > m_ATLASConverterTool{this, "ATLASConverterTool", ""};
    ToolHandle<ActsTrk::IFitterTool> m_fitterTool{this, "FitterTool", "", "Fitter Tool for Seeds"};
    ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>> m_pixelCalibTool{this, "PixelCalibrator", "", "Opt. pixel measurement calibrator"};
    ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>> m_stripCalibTool{this, "StripCalibrator", "", "Opt. strip measurement calibrator"};
    ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>> m_hgtdCalibTool{this, "HGTDCalibrator", "", "Opt. HGTD measurement calibrator"};

    SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};
    ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};

    // Configuration
    Gaudi::Property<unsigned int> m_maxPropagationStep{this, "maxPropagationStep", 1000, "Maximum number of steps for one propagate call"};
    Gaudi::Property<std::vector<double>> m_etaBins{this, "etaBins", {}, "bins in |eta| to specify variable selections"};
    // Acts::MeasurementSelector selection cuts for associating measurements with predicted track parameters on a surface.
    Gaudi::Property<std::vector<double>> m_chi2CutOff{this, "chi2CutOff", {}, "MeasurementSelector: maximum local chi2 contribution"};
    Gaudi::Property<std::vector<double>> m_chi2OutlierCutOff{this, "chi2OutlierCutOff", {}, "MeasurementSelector: maximum local chi2 contribution for outlier"};
    Gaudi::Property<std::vector<size_t>> m_numMeasurementsCutOff{this, "numMeasurementsCutOff", {}, "MeasurementSelector: maximum number of associated measurements on a single surface"};
    Gaudi::Property<std::vector<std::size_t>> m_ptMinMeasurements{this, "ptMinMeasurements", {}, "if specified for the given seed collection, applies ptMin cut in branch stopper once ptMinMinMeasurements have been encountered"};
    Gaudi::Property<std::vector<std::size_t>> m_absEtaMaxMeasurements{this, "absEtaMaxMeasurements", {}, "if specified for the given seed collection, applies absEtaMax cut in branch stopper once absEtaMaxMeasurements have been encountered"};
    Gaudi::Property<bool> m_doBranchStopper{this, "doBranchStopper", true, "use branch stopper"};
    Gaudi::Property<bool> m_doTwoWay{this, "doTwoWay", true, "run CKF twice, first with forward propagation with smoothing, then with backward propagation"};
    Gaudi::Property<double> m_branchStopperPtMinFactor{this, "branchStopperPtMinFactor", 1.0, "factor to multiply ptMin cut when used in the branch stopper"};
    Gaudi::Property<double> m_branchStopperAbsEtaMaxExtra{this, "branchStopperAbsEtaMaxExtra", 0.0, "increase absEtaMax cut when used in the branch stopper"};
    Gaudi::Property<double> m_branchStopperMeasCutReduce{this, "branchStopperMeasCutReduce", 2, "how much to reduce the minMeas requirement for the branch stopper"};
    Gaudi::Property<double> m_branchStopperAbsEtaMeasCut{this, "branchStopperAbsEtaMeasCut", 1.2, "the minimum |eta| to apply the reduction to the minMeas requirement for the branch stopper"};

    // Acts::TrackSelector cuts
    // Use max double, because mergeConfdb2.py doesn't like std::numeric_limits<double>::infinity() (produces bad Python "inf.0")
    Gaudi::Property<std::vector<double>> m_phiMin{this, "phiMin", {}, "TrackSelector: phiMin"};
    Gaudi::Property<std::vector<double>> m_phiMax{this, "phiMax", {}, "TrackSelector: phiMax"};
    Gaudi::Property<std::vector<double>> m_etaMin{this, "etaMin", {}, "TrackSelector: etaMin"};
    Gaudi::Property<std::vector<double>> m_etaMax{this, "etaMax", {}, "TrackSelector: etaMax"};
    Gaudi::Property<double> m_absEtaMin{this, "absEtaMin", 0.0, "TrackSelector: absEtaMin"};
    Gaudi::Property<double> m_absEtaMax{this, "absEtaMax", std::numeric_limits<double>::max(), "TrackSelector: absEtaMax"};
    Gaudi::Property<std::vector<double>> m_ptMin{this, "ptMin", {}, "TrackSelector: ptMin"};
    Gaudi::Property<std::vector<double>> m_ptMax{this, "ptMax", {}, "TrackSelector: ptMax"};
    Gaudi::Property<std::vector<double>> m_d0Min{this, "d0Min", {}, "TrackSelector: d0Min"};
    Gaudi::Property<std::vector<double>> m_d0Max{this, "d0Max", {}, "TrackSelector: d0Max"};
    Gaudi::Property<std::vector<double>> m_z0Min{this, "z0Min", {}, "TrackSelector: z0Min"};
    Gaudi::Property<std::vector<double>> m_z0Max{this, "z0Max", {}, "TrackSelector: z0Max"};

    Gaudi::Property<std::vector<std::size_t>> m_minMeasurements{this, "minMeasurements", {}, "TrackSelector: minMeasurements"};
    Gaudi::Property<std::vector<std::size_t>> m_maxHoles{this, "maxHoles", {}, "TrackSelector: maxHoles"};
    Gaudi::Property<std::vector<std::size_t>> m_maxOutliers{this, "maxOutliers", {}, "TrackSelector: maxOutliers"};
    Gaudi::Property<std::vector<std::size_t>> m_maxSharedHits{this, "maxSharedHits", {}, "TrackSelector: maxSharedHits"};
    Gaudi::Property<std::vector<double>> m_maxChi2{this, "maxChi2", {}, "TrackSelector: maxChi2"};

    Gaudi::Property<bool> m_addPixelStripCounts{this, "addPixelStripCounts", true, "keep separate pixel and strip counts and apply the following cuts"};
    Gaudi::Property<std::vector<std::size_t>> m_minPixelHits{this, "minPixelHits", {}, "minimum number of pixel hits"};
    Gaudi::Property<std::vector<std::size_t>> m_minStripHits{this, "minStripHits", {}, "minimum number of strip hits"};
    Gaudi::Property<std::vector<std::size_t>> m_maxPixelHoles{this, "maxPixelHoles", {}, "maximum number of pixel holes"};
    Gaudi::Property<std::vector<std::size_t>> m_maxStripHoles{this, "maxStripHoles", {}, "maximum number of strip holes"};
    Gaudi::Property<std::vector<std::size_t>> m_maxPixelOutliers{this, "maxPixelOutliers", {}, "maximum number of pixel outliers"};
    Gaudi::Property<std::vector<std::size_t>> m_maxStripOutliers{this, "maxStripOutliers", {}, "maximum number of strip outliers"};

    Gaudi::Property<std::vector<std::uint32_t>> m_endOfWorldVolumeIds {this, "EndOfTheWorldVolumeIds", {}, ""};

    // configuration of statistics tables
    Gaudi::Property<std::vector<float>> m_statEtaBins{this, "StatisticEtaBins", {-4, -2.6, -2, 0, 2., 2.6, 4}, "Gather statistics separately for these bins."};
    Gaudi::Property<std::vector<std::string>> m_seedLabels{this, "SeedLabels", {}, "One label per seed key used in outputs"};
    Gaudi::Property<bool> m_dumpAllStatEtaBins{this, "DumpEtaBinsForAll", false, "Dump eta bins of all statistics counter."};

    enum EStat : std::size_t
    {
      kNTotalSeeds,
      kNoTrackParam,
      kNUsedSeeds,
      kNoTrack,
      kNDuplicateSeeds,
      kNNoEstimatedParams,
      kNOutputTracks,
      kNRejectedRefinedSeeds,
      kNSelectedTracks,
      kNStoppedTracksMaxHoles,
      kMultipleBranches,
      kNoSecond,
      kNStoppedTracksMinPt,
      kNStoppedTracksMaxEta,
      kNTotalSharedHits,
      kNStat
    };

    using EventStats = std::vector<std::array<unsigned int, kNStat>>;

    // initialize measurement selector to be called during initialize
    StatusCode initializeMeasurementSelector();

    /**
     * @brief Setup and attach measurement selector to KF options
     *
     * @param measurements measurements container used in MeasurementSelector
     * @param options Kalman filter options
     * @return unique_ptr to MeasurementSelector
     */
    [[nodiscard]] std::unique_ptr<ActsTrk::IMeasurementSelector> setMeasurementSelector(
        const detail::TrackFindingMeasurements &measurements,
        TrackFinderOptions &options) const;

    /**
     * @brief Get CKF options for first and second pass + pointer to MeasurementSelector
     *
     * @param detContext Object holding detector-related context
     * @param measurements <easurements container used in MeasurementSelector
     * @param pSurface Raw pointer to perigee surface
     */
    TrackFindingDefaultOptions getDefaultOptions(const DetectorContextHolder &detContext,
                                                 const detail::TrackFindingMeasurements &measurements,
                                                 const Acts::PerigeeSurface* pSurface) const;

    /**
     * @brief Take the array of handle keys and for each key retrieve containers, then append them to the output vector.
     *
     * @tparam HandleArrayKeyType Type of the list of handle keys
     * @tparam ContainerType Type of the output container
     * @param ctx Event context
     * @param handleKeyArray List of handle keys
     * @param outputContainers Vector of output containers
     * @param sum Number of total elements in all retrieved containers
     * @return Status code
     */
    template <class HandleArrayKeyType, class ContainerType>
    StatusCode getContainersFromKeys(
        const EventContext &ctx,
        HandleArrayKeyType &handleKeyArray,
        std::vector<const ContainerType *> &outputContainers,
        std::size_t &sum) const;

    /**
     * @brief Retrieves track selector configuration for given eta value
     *
     * @param eta track candidate eta value
     */
    const Acts::TrackSelector::Config &getCuts(double eta) const;

    /**
     * @brief Perform Kalman Filter fit and update given initialParameters
     *
     * @tparam MeasurementSource Type of measurement source: ActsTrk::Seed or (in future) ActsTrk::ProtoTrack
     * @param measurement Measurement source for KF
     * @param initialParameters Parameters to use in KF
     * @param detContext Struct holding geometry, magnetic field and calibration contexts
     * @param paramsAtOutermostSurface Flag for searching in reverse direction
     *
     * @return Unique pointer to updated parameters
     */
    template <class MeasurementSource>
    std::unique_ptr<Acts::BoundTrackParameters> doRefit(
        const MeasurementSource &measurement,
        const Acts::BoundTrackParameters &initialParameters,
        const DetectorContextHolder &detContext,
        const bool paramsAtOutermostSurface) const;

    using TrkProxy = Acts::TrackProxy<Acts::VectorTrackContainer, Acts::VectorMultiTrajectory, Acts::detail::RefHolder, false>;

    /**
     * @brief Perform two-way track finding
     *
     * @param addTrack Function or lambda which adds newly found track to container
     * @param trackProxy Track proxy object
     * @param tracksContainerTemp Track proxy container
     * @param options Fit options
     * @param tgContext Geometry context
     * @param reverseSearch Flag for searching in reverse direction
     * @param seedType Seed type (only used for warnings/debug printouts)
     * @param iseed Seed number (only used for warnings/debug printouts)
     * @param itrack Track number (only used for warnings/debug printouts)
     *
     * @return Number of found tracks
     */
    std::vector<typename detail::RecoTrackContainer::TrackProxy>
    doTwoWayTrackFinding(const detail::RecoTrackStateContainerProxy& firstMeasurement,
                         const TrkProxy &trackProxy,
                         detail::RecoTrackContainer &tracksContainerTemp,
                         const TrackFinderOptions &options) const;

    static xAOD::UncalibMeasType measurementType (const detail::RecoTrackContainer::TrackStateProxy &trackState);

    using BranchStopperResult = Acts::CombinatorialKalmanFilterBranchStopperResult;

    /**
     * @brief Branch stopper
     *
     * @param track Track proxy object
     * @param trackState Track state proxy object
     * @param trackSelectorCfg Track selector configuration
     * @param tgContext Geometry context
     * @param measurementIndex Measurement index
     * @param typeIndex Type index
     * @param event_stat_category_i Event statistics for current category
     * @return BranchStopperResult
     */
    BranchStopperResult stopBranch(
        const detail::RecoTrackContainer::TrackProxy &track,
        const detail::RecoTrackContainer::TrackStateProxy &trackState,
        const Acts::TrackSelector::EtaBinnedConfig &trackSelectorCfg,
        const Acts::GeometryContext &tgContext,
        const detail::MeasurementIndex &measurementIndex,
        const std::size_t typeIndex,
        EventStats::value_type &event_stat_category_i) const;

    struct BranchState {
      static constexpr Acts::ProxyAccessor<unsigned int> nPixelHits{"nPixelHits"};
      static constexpr Acts::ProxyAccessor<unsigned int> nStripHits{"nStripHits"};
      static constexpr Acts::ProxyAccessor<unsigned int> nPixelHoles{"nPixelHoles"};
      static constexpr Acts::ProxyAccessor<unsigned int> nStripHoles{"nStripHoles"};
      static constexpr Acts::ProxyAccessor<unsigned int> nPixelOutliers{"nPixelOutliers"};
      static constexpr Acts::ProxyAccessor<unsigned int> nStripOutliers{"nStripOutliers"};
    };
    static constexpr BranchState s_branchState{};

    static void addPixelStripCounts(detail::RecoTrackContainer &tracksContainer);
    static void initPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track);
    static void updatePixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track,
                                       Acts::ConstTrackStateType typeFlags,
                                       xAOD::UncalibMeasType detType);
    static void copyPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track,
                                     const detail::RecoTrackContainer::TrackProxy &other);
    void checkPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track) const;
    std::array<bool, 3> selectPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track, double eta) const;

    bool selectPixelStripCountsFinal(const detail::RecoTrackContainer::TrackProxy &track) const;

    /// Private access to the logger
    const Acts::Logger &logger() const
    {
      return *m_logger;
    }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;

    // statistics
    void initStatTables();
    void copyStats(const EventStats &event_stat) const;
    void printStatTables() const;

    std::size_t nSeedCollections() const {
      return m_seedLabels.size();
    }
    std::size_t seedCollectionStride() const {
      return m_statEtaBins.size() + 1;
    }
    std::size_t getStatCategory(std::size_t seed_collection, float eta) const;
    std::size_t computeStatSum(std::size_t seed_collection, EStat counter_i, const EventStats &stat) const;

    bool m_useAbsEtaForStat = false;
    mutable std::mutex m_mutex ATLAS_THREAD_SAFE;
    mutable std::vector<std::array<std::size_t, kNStat>> m_stat ATLAS_THREAD_SAFE{};
  };
}  // namespace ActsTrk

#include "src/TrackFindingBaseAlg.icc"

#endif
