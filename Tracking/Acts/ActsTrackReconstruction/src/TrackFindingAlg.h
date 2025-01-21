/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGALG_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Tools
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "src/TrackStatePrinterTool.h"
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

// ACTS
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/ProxyAccessor.hpp"

// ActsTrk
#include "ActsEvent/Seed.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackParametersContainer.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "IMeasurementSelector.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/EventContext.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

// STL
#include <limits>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

// Handle Keys
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "src/detail/Definitions.h"
#include "src/detail/DuplicateSeedDetector.h"
#include "src/detail/TrackFindingMeasurements.h"

namespace ActsTrk
{

  class TrackFindingAlg : public AthReentrantAlgorithm
  {
  public:

    TrackFindingAlg(const std::string &name,
                    ISvcLocator *pSvcLocator);
    virtual ~TrackFindingAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:


    // Tool Handles
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
    ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
    ToolHandle<ActsTrk::TrackStatePrinterTool> m_trackStatePrinter{this, "TrackStatePrinter", "", "optional track state printer"};
    ToolHandle< ActsTrk::IActsToTrkConverterTool > m_ATLASConverterTool{this, "ATLASConverterTool", ""};
    ToolHandle< ActsTrk::ITrackParamsEstimationTool > m_paramEstimationTool{this, "TrackParamsEstimationTool", "", "Track Param Estimation from Seeds"};
    ToolHandle<ActsTrk::IFitterTool> m_fitterTool{this, "FitterTool", "", "Fitter Tool for Seeds"};
    ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>> m_pixelCalibTool{
      this, "PixelCalibrator", "", "Opt. pixel measurement calibrator"};
    ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>> m_stripCalibTool{
      this, "StripCalibrator", "", "Opt. strip measurement calibrator"};
    ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>> m_hgtdCalibTool{
      this, "HGTDCalibrator", "", "Opt. HGTD measurement calibrator"};

    // Handle Keys
    // Seed collections. These 2 vectors must match element for element.
    SG::ReadHandleKeyArray<ActsTrk::SeedContainer> m_seedContainerKeys{this, "SeedContainerKeys", {}, "Seed containers"};
    SG::ReadCondHandleKeyArray<InDetDD::SiDetectorElementCollection> m_detEleCollKeys{this, "DetectorElementsKeys", {}, "Keys of input SiDetectorElementCollection"};
    // Measurement collections. These 2 vectors must match element for element.
    SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_uncalibratedMeasurementContainerKeys{this, "UncalibratedMeasurementContainerKeys", {}, "input cluster collections"};
    SG::ReadCondHandleKey<ActsTrk::DetectorElementToActsGeometryIdMap> m_detectorElementToGeometryIdMapKey
       {this, "DetectorElementToActsGeometryIdMapKey", "DetectorElementToActsGeometryIdMap",
        "Map which associates detector elements to Acts Geometry IDs"};

    SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "", "Output track collection (ActsTrk variant)"};
    ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper;

    // Configuration
    Gaudi::Property<unsigned int> m_maxPropagationStep{this, "maxPropagationStep", 1000, "Maximum number of steps for one propagate call"};
    Gaudi::Property<bool> m_skipDuplicateSeeds{this, "skipDuplicateSeeds", true, "skip duplicate seeds before calling CKF"};
    Gaudi::Property<std::vector<bool>> m_refitSeeds{this, "refitSeeds", {}, "Run KalmanFitter on seeds before passing to CKF, specified separately for each seed collection"};
    Gaudi::Property<std::vector<double>> m_etaBins{this, "etaBins", {}, "bins in |eta| to specify variable selections"};
    // Acts::MeasurementSelector selection cuts for associating measurements with predicted track parameters on a surface.
    Gaudi::Property<std::vector<double>> m_chi2CutOff{this, "chi2CutOff", {}, "MeasurementSelector: maximum local chi2 contribution"};
    Gaudi::Property<std::vector<double>> m_chi2OutlierCutOff{this, "chi2OutlierCutOff", {}, "MeasurementSelector: maximum local chi2 contribution for outlier"};
    Gaudi::Property<std::vector<size_t>> m_numMeasurementsCutOff{this, "numMeasurementsCutOff", {}, "MeasurementSelector: maximum number of associated measurements on a single surface"};
    Gaudi::Property<std::vector<std::size_t>> m_ptMinMeasurements{this, "ptMinMeasurements", {}, "if specified for the given seed collection, applies ptMin cut in branch stopper once ptMinMinMeasurements have been encountered"};
    Gaudi::Property<std::vector<std::size_t>> m_absEtaMaxMeasurements{this, "absEtaMaxMeasurements", {}, "if specified for the given seed collection, applies absEtaMax cut in branch stopper once absEtaMaxMeasurements have been encountered"};
    Gaudi::Property<bool> m_doBranchStopper{this, "doBranchStopper", true, "use branch stopper"};
    Gaudi::Property<bool> m_doTwoWay{this, "doTwoWay", true, "run CKF twice, first with forward propagation with smoothing, then with backward propagation"};
    Gaudi::Property<std::vector<bool>> m_reverseSearch {this, "reverseSearch", {}, "Whether to run the finding in seed parameter direction (false or not specified) or reverse direction (true), specified separately for each seed collection"};
    Gaudi::Property<double> m_branchStopperPtMinFactor{this, "branchStopperPtMinFactor", 1.0, "factor to multiply ptMin cut when used in the branch stopper"};
    Gaudi::Property<double> m_branchStopperAbsEtaMaxExtra{this, "branchStopperAbsEtaMaxExtra", 0.0, "increase absEtaMax cut when used in the branch stopper"};

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

    // configuration of statistics tables
    Gaudi::Property<std::vector<float>> m_statEtaBins{this, "StatisticEtaBins", {-4, -2.6, -2, 0, 2., 2.6, 4}, "Gather statistics separately for these bins."};
    Gaudi::Property<std::vector<std::string>> m_seedLabels{this, "SeedLabels", {}, "One label per seed key used in outputs"};
    Gaudi::Property<bool> m_dumpAllStatEtaBins{this, "DumpEtaBinsForAll", false, "Dump eta bins of all statistics counter."};

    Gaudi::Property<bool> m_useDefaultMeasurementSelector{this, "UseDefaultActsMeasurementSelector", true, ""};

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
      kNStat
    };
    using EventStats = std::vector<std::array<unsigned int, kNStat>>;

    // initialize measurement selector to be called during initialize
    StatusCode initializeMeasurementSelector();

    /**
     * @brief invoke track finding procedure
     *
     * @param ctx - event context
     * @param measurements - measurements container
     * @param seeds - spacepoint triplet seeds
     * @param tracksContainer - output tracks
     * @param tracksCollection - auxiliary output for downstream tools compatibility (to be removed in the future)
     * @param seedCollectionIndex - index of seeds in measurements
     * @param seedType name of type of seeds (strip or pixel) - only used for messages
     */
    StatusCode
    findTracks(const EventContext &ctx,
               const Acts::TrackingGeometry &trackingGeometry,
               const ActsTrk::DetectorElementToActsGeometryIdMap &detectorElementToGeoId,
               const detail::TrackFindingMeasurements &measurements,
               detail::DuplicateSeedDetector &duplicateSeedDetector,
               const ActsTrk::SeedContainer &seeds,
               const InDetDD::SiDetectorElementCollection& detElements,
               ActsTrk::MutableTrackContainer &tracksContainer,
               size_t seedCollectionIndex,
               const char *seedType,
               EventStats &event_stat) const;

    // Create tracks from one seed's CKF result, appending to tracksContainer

    void storeSeedInfo(const detail::RecoTrackContainer &tracksContainer,
                       const detail::RecoTrackContainerProxy &track,
                       detail::DuplicateSeedDetector &duplicateSeedDetector) const;

    // Access Acts::CombinatorialKalmanFilter etc using "pointer to implementation"
    // so we don't have to instantiate the heavily templated classes in the header.
    // To maintain const-correctness, only use this via the accessor functions.
    struct CKF_pimpl;
    CKF_pimpl &trackFinder();
    const CKF_pimpl &trackFinder() const;

    std::unique_ptr<ActsTrk::IMeasurementSelector> m_measurementSelector;
    std::unique_ptr<CKF_pimpl> m_trackFinder;

    static xAOD::UncalibMeasType measurementType (const detail::RecoTrackContainer::TrackStateProxy &trackState);

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

    // statistics
    void initStatTables();
    void copyStats(const EventStats &event_stat) const;
    void printStatTables() const;

    std::size_t nSeedCollections() const
    {
      return m_seedLabels.size();
    }
    std::size_t seedCollectionStride() const
    {
      return m_statEtaBins.size() + 1;
    }
    std::size_t getStatCategory(std::size_t seed_collection, float eta) const;
    std::size_t computeStatSum(std::size_t seed_collection, EStat counter_i, const EventStats &stat) const;

    bool m_useAbsEtaForStat = false;
    mutable std::mutex m_mutex ATLAS_THREAD_SAFE;
    mutable std::vector<std::array<std::size_t, kNStat>> m_stat ATLAS_THREAD_SAFE{};

    /// Private access to the logger
    const Acts::Logger &logger() const
    {
      return *m_logger;
    }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;
  };

} // namespace

#endif
