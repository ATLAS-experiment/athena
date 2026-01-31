/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
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
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinderGbts.hpp" 
#include "Acts/Definitions/Units.hpp"
#include "Acts/Seeding/SeedFinderGbtsConfig.hpp" 
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/EventData/SeedContainer2.hpp"
#include "Acts/EventData/SpacePointContainer2.hpp"

//for det elements, not sure which need: 
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"
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

#include <numbers> // for std::numbers::pi


namespace ActsTrk {
  
  class GbtsSeedingTool final:
    public extends<AthAlgTool, ActsTrk::ISeedingTool> {

  public:

    GbtsSeedingTool(const std::string& type, const std::string& name,
			  const IInterface* parent);
    virtual ~GbtsSeedingTool() = default;
    
    virtual StatusCode initialize() override;
    
    // Interface
    virtual StatusCode
      createSeeds(const EventContext& ctx,
                  const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer,
                  const Acts::Vector3& beamSpotPos,
                  const Acts::Vector3& bField,
                  ActsTrk::SeedContainer& seedContainer ) const override;




    // own class functions
    
  private:
    
    //prints all current config settings used either with default settings or properties changed by the gaudi options
    //tells what the defaults in the config are
    void printSeedFinderGbtsConfig(const Acts::Experimental::SeedFinderGbtsConfig& cfg);
    
    //sets configs based on gaudi properties defined below
    StatusCode prepareConfiguration();

    std::unique_ptr<Acts::Experimental::GbtsConnector> m_connector = nullptr; //holds the connection information on what detector layers are linked
    
    Acts::Experimental::SeedFinderGbtsConfig m_finderCfg; //steering for seeding algorithm
    
    std::unique_ptr<Acts::Experimental::GbtsGeometry> m_gbtsGeo = nullptr; //holds all geometry information (m_layergeomtry and connection table)
    
    std::vector<Acts::Experimental::TrigInDetSiLayer> m_layerGeometry{}; //layer objects used by GBTS

    std::unique_ptr<Acts::Experimental::SeedFinderGbts> m_finder = nullptr; //the actual seed fining algorithm
    
    
    // used to create the detector layers and which modules correspond to pixels and strips 
    ToolHandle<ITrigL2LayerNumberTool> m_layerNumberTool {this, "layerNumberTool", "TrigL2LayerNumberToolITk"}; 
    
    //lists of layers that pixel and strip modules are apart of (index defines hash ID of module), 
    // used to sort Spacepoints based on the type of module its assigned to 
    const std::vector<short>* m_sct_h2l{nullptr};
    const std::vector<short>* m_pix_h2l{nullptr};
    std::vector<bool> m_are_pixels; 
    //TO DO: ADD NEW VERIABLES, DELETE OLD ONES AND CHANGE CURRENT ONES TO NEW VALUES
    //Config settings                                                              
    Gaudi::Property<std::string> m_connectorInputFile {this, "connectorInputFile","binTables_ITK_RUN4.txt", "input file for making connector object"};
    Gaudi::Property<std::string> m_lutFile{this, "lutInputFile", "gbts_ml_pixel_barrel_loose.lut", "file to LUT"}; 
      //SeedFinderGbts option
    Gaudi::Property<bool> m_LRTmode {this, "LRTmode", false, "whether strip or pixel hits are used"};
    Gaudi::Property<bool> m_useML {this, "useML", true, "use the cluster width of the spacepoint"};
    Gaudi::Property<bool> m_matchBeforeCreate {this, "matchBeforeCreate", true, "need to check what this does"};
    Gaudi::Property<bool> m_useOldTunings {this, "useOldTunings", false, "use the tunings for 900MeV cut"};
    Gaudi::Property<float> m_tau_ratio_cut {this, "cut_tau_ratio_max",0.007, "tau cut for doublets and triplets"}; 
    Gaudi::Property<float> m_tau_ratio_precut {this, "precut_tau_ratio_max",0.009f, "not sure"}; 
    Gaudi::Property<float> m_etaBinOverride {this, "etaBinOverride", 0.0f, "apply custom binning for connections"};
    Gaudi::Property<float> m_nMaxPhiSlice {this, "nMaxPhiSlice",53, "used to calculate phi slices"}; 
    Gaudi::Property<bool> m_BeamSpotCorrection {this, "beamSpotCorrection", true, "apply primary vertex corrections to spacepoints"};
    Gaudi::Property<float> m_minPt {this, "minPt", 1000.0, "Lower cutoff for seeds"};
    Gaudi::Property<float> m_phiSliceWidth {this, "phiSliceWidth",0, "initialised in loadSpacePoints function"};

      //BuildTheGraph() options
    Gaudi::Property<bool> m_useEtaBinning {this, "useEtaBinning",true, "bool to use eta binning from geometry structure"}; 
    Gaudi::Property<bool> m_doubletFilterRZ {this, "doubletFilterRZ",true, "bool applies new Z cuts on doublets"}; 
    Gaudi::Property<float> m_minDeltaRadius {this, "minDeltaRadius",2.0, " min dr for doublet"}; 
    Gaudi::Property<int> m_nMaxEdges {this, "MaxEdges",3000000, " max number of Gbts edges/doublets"};
    Gaudi::Property<double> m_ptCoeff {this, "ptCoeff", 0.29997 * 1.9972 / 2.0, "~0.3*B/2 - assumes nominal field of 2*T"}; 
      //GbtsTrackingFilter
   
    Gaudi::Property<float> m_sigmaMS {this, "sigmaMS", 0.016, "process noise from multiple scattering"};
    Gaudi::Property<float> m_radLen {this, "radLen", 0.025, "not sure"};
    Gaudi::Property<float> m_sigma_x {this, "sigma_x", 0.08, "measurement resolution for residual on y direction"}; //was 0.22
    Gaudi::Property<float> m_sigma_y {this, "sigma_y", 0.25, "measurement resolution on r-z plane"};// was 1.7
    Gaudi::Property<float> m_weight_x {this, "weight_x", 0.5, "penalty weight for track"};
    Gaudi::Property<float> m_weight_y {this, "weight_y", 0.5, "penalty weight for track"};
    Gaudi::Property<float> m_maxDChi2_x {this, "maxDChi2_x", 5.0, "gate threshold for chi2 test"}; //was 35.0
    Gaudi::Property<float> m_maxDChi2_y {this, "maxDChi2_y", 6.0, "gate threshold for chi2 test"};//was 31.0
    Gaudi::Property<float> m_add_hit {this, "add_hit", 14.0, "reward added to tracks for each accepted hit before chi2"};
    Gaudi::Property<float> m_max_curvature {this, "max_curvature", 1e-3f, "not sure"};
    Gaudi::Property<float> m_max_z0 {this, "max_z0", 170.0, "not sure"};
    Gaudi::Property<float> m_edge_mask_min_eta {this, "edge_mask_min_eta", 1.5, "not sure"};
    Gaudi::Property<float> m_hit_share_threshold {this, "hit_share_threshold", 0.49, "not sure"};

    //GbtsDataStorage
    Gaudi::Property<float> m_max_endcap_clusterwidth {this, "max_endcap_clusterwidth", 0.35, "discards any spacepoints which dr/dz cant be accurately modelled"};
    /// Private access to the logger
    const Acts::Logger &logger() const { return *m_logger; }
    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger {nullptr};

  };
  
} // namespace

#endif

