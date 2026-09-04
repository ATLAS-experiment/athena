/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GBTSSEEDINGTOOL_SEEDINGTOOL_H
#define ACTSTRK_GBTSSEEDINGTOOL_SEEDINGTOOL_H 1

// ATHENA
#include "ActsToolInterfaces/ISeedingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "ActsInterop/Logger.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

// ACTS CORE
#include "Acts/Seeding/GraphBasedTrackSeeder.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/SeedContainer.hpp"
#include "Acts/EventData/SpacePointContainer.hpp"

//for det elements, not sure which need: 
#include "ActsToolInterfaces/IGbtsLayerTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsToolInterfaces/IPixelSpacePointFormationTool.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include <memory>

namespace ActsTrk {

  class GbtsSeedingTool final: public extends<AthAlgTool, ActsTrk::ISeedingTool> {

  public:

    GbtsSeedingTool(const std::string& type, const std::string& name,
      const IInterface* parent);

    virtual ~GbtsSeedingTool() = default;

    virtual StatusCode initialize() override;

    // Interface
    StatusCode createSeeds(
      const EventContext& ctx,
      const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
      const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
      ActsTrk::SeedContainer& seedContainer) const override;

  private:

    /// prints all current config settings used either with default settings or properties changed by the gaudi options
    /// tells what the defaults in the config are
    void printGbtsConfig() const;

    /// sets configs based on gaudi properties defined below
    StatusCode prepareConfiguration();

    /// Private access to the logger
    const Acts::Logger &logger() const { return *m_logger; }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;

    /// steering for seeding algorithm
    Acts::Experimental::GraphBasedTrackSeeder::Config m_finderCfg;

    /// steering for track filter
    Acts::Experimental::GbtsTrackingFilter::Config m_filterCfg;

    /// the actual seed fining algorithm
    std::optional<Acts::Experimental::GraphBasedTrackSeeder> m_finder;

    /// the seed filter
    std::optional<Acts::Experimental::GbtsTrackingFilter> m_filter;

    /// region of interest for pixel seeding
    std::optional<Acts::Experimental::GbtsRoiDescriptor> m_internalRoi;

    /// Wafer hash to dense GBTS layer index, one map per technology. Used to
    /// put a space point on the layer its module belongs to.
    const std::vector<short>* m_stripHashToLayer = nullptr;
    const std::vector<short>* m_pixelHashToLayer = nullptr;
    /// Whether each dense GBTS layer index is a pixel layer.
    std::vector<bool> m_are_pixels;

    /// Builds the GBTS layers out of the ITk readout geometry.
    ToolHandle<IGbtsLayerTool> m_layerTool {this, "layerTool", "ActsTrk::GbtsLayerTool/ActsGbtsLayerTool"}; 

    // Config settings                                                              
    Gaudi::Property<std::string> m_connectorInputFile {this, "connectorInputFile","binTables_ITK_RUN4.txt", "input file for making connector object"};
    Gaudi::Property<std::string> m_lutFile {this, "lutInputFile", "gbts_ml_pixel_barrel_loose.lut", "file to LUT"}; 
    
    // GraphBasedTrackSeeder: feature option
    Gaudi::Property<bool> m_LRTmode {this, "LRTmode", false, "whether strip or pixel hits are used"};
    Gaudi::Property<bool> m_useML {this, "useML", true, "use the cluster width of the spacepoint"};
    Gaudi::Property<bool> m_matchBeforeCreate {this, "matchBeforeCreate", true, "need to check what this does"};
    Gaudi::Property<bool> m_useOldTunings {this, "useOldTunings", false, "use the tunings for 900MeV cut"};
    Gaudi::Property<bool> m_beamSpotCorrection {this, "beamSpotCorrection", true, "apply primary vertex corrections to spacepoints"};
    Gaudi::Property<bool> m_validateTriplets{this, "ValidateTriplets", true, "extra validation on pT and d0 performed to connected barrel edges"};
    Gaudi::Property<bool> m_useAdaptiveCuts{this, "UseAdaptiveCuts", true, "allows for larger accpetance of candidate edges that skip layers"};
    Gaudi::Property<bool> m_addTriplets{this, "addTriplets", false, "add seeds with three spacepoints in a defined eta region"};
    Gaudi::Property<bool> m_useEtaBinning {this, "useEtaBinning",true, "bool to use eta binning from geometry structure"}; 
    Gaudi::Property<bool> m_doubletFilterRZ {this, "doubletFilterRZ",true, "bool applies new Z cuts on doublets"};
    // GraphBasedTrackSeeder: Geometry options
    Gaudi::Property<float> m_etaBinWidthOverride {this, "etaBinOverride", 0.0f, "apply custom binning for connections"};
    Gaudi::Property<float> m_nMaxPhiSlice {this, "nMaxPhiSlice",53, "used to calculate phi slices"};
    
    // BuildTheGraph() general options
    Gaudi::Property<float> m_tauRatioCut {this, "tauRatioCut",0.007, "tau cut for doublets and triplets"}; 
    Gaudi::Property<float> m_tauRatioPrecut {this, "precutTauRatioMax",0.009f, "used to reject edges early if matchBeforCreate is on"};
    Gaudi::Property<float> m_tauRatioCorr{this, "tauRatioCorrection", 0.006, "correction added to tau accpetance if candidate edge skips a layer"}; 
    Gaudi::Property<float> m_minPt {this, "minPt", 1000.0, "Lower cutoff for seeds"};
    Gaudi::Property<float> m_maxEtaAddTriplets{this, "maxEtaAddTriplets", 1.5, "eta region in which three sapcepoint seeds are allowed"}; 
    Gaudi::Property<float> m_minDeltaRadius {this, "minDeltaRadius",2.0, " min dr for doublet"}; 
    Gaudi::Property<int> m_nMaxEdges {this, "MaxEdges",3000000, " max number of Gbts edges/doublets"};
    Gaudi::Property<float> m_cutDPhiMax {this, "cutDPhiMax", 0.012f, "not sure"};
    Gaudi::Property<float> m_cutDCurvMax {this, "cutDCurvMax", 0.001f, "not sure"};
    Gaudi::Property<float> m_minDeltaPhi {this, "minDeltaPhi", 0.001f, "not sure"};
    Gaudi::Property<float> m_maxOuterRadius {this, "maxOuterRadius", 550.0f, "not sure"};

    // these are only used for LRT mode, pixel seeding uses values from the roi
    // set to small value so that issues can be debugged if not set properly
    Gaudi::Property<float> m_minZ0 {this, "minZ0", -99999, "not sure"};
    Gaudi::Property<float> m_maxZ0 {this, "maxZ0", -99999, "not sure"};
    
    // triplet validation options
    Gaudi::Property<float> m_d0Max{this, "d0Max", 3.0, "maximum d0 value allowed when using validateTiplets"};

    // GbtsTrackingFilter
    Gaudi::Property<float> m_sigmaMS {this, "sigmaMS", 0.016, "process noise from multiple scattering"};
    Gaudi::Property<float> m_radLen {this, "radLen", 0.025, "defines how much material scattering the kalman filter should take into account"};
    Gaudi::Property<float> m_sigmaX {this, "sigmaX", 0.08, "measurement resolution for residual on y direction"}; 
    Gaudi::Property<float> m_sigmaY {this, "sigmaY", 0.25, "measurement resolution on r-z plane"};
    Gaudi::Property<float> m_weightX {this, "weightX", 0.5, "penalty weight for track"};
    Gaudi::Property<float> m_weightY {this, "weightY", 0.5, "penalty weight for track"};
    Gaudi::Property<float> m_maxDChi2X {this, "maxDChi2X", 5.0, "gate threshold for chi2 test"}; 
    Gaudi::Property<float> m_maxDChi2Y {this, "maxDChi2Y", 6.0, "gate threshold for chi2 test"};
    Gaudi::Property<float> m_addHit {this, "addHit", 14.0, "reward added to tracks for each accepted hit before chi2"};
    Gaudi::Property<float> m_maxCurvature {this, "maxCurvature", 1e-3f, "maximum curvature allowed for candiadte tracklet"};
    Gaudi::Property<float> m_filterMaxZ0 {this, "filterMaxZ0", 170.0, "maximum z0 allowed for candidate tracklet"};
    Gaudi::Property<float> m_edgeMaskMinEta {this, "edgeMaskMinEta", 1.5, "minimum eta allowed for masking edges in graph so they are not used again"};
    Gaudi::Property<float> m_hitShareThreshold {this, "hitShareThreshold", 0.49, "threshold of hits that are shared between seeds before one seed is labelled a clone"};

    // GbtsNodeStorage
    Gaudi::Property<float> m_maxEndcapClusterwidth {this, "maxEndcapClusterwidth", 0.35, "discards any spacepoints which dr/dz cant be accurately modelled"};
  };
  
} // namespace

#endif
