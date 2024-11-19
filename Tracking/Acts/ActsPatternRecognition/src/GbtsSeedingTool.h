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

// ACTS CORE
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/EventData/Seed.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinderGbts.hpp" 
#include "Acts/Definitions/Units.hpp"
#include "Acts/Seeding/SeedFinderGbtsConfig.hpp" 
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFilter.hpp"

//for det elements, not sure which need: 
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsToolInterfaces/IPixelSpacePointFormationTool.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include <numbers> // for std::numbers::pi


namespace ActsTrk {
  
  class GbtsSeedingTool final:
    public extends<AthAlgTool, ActsTrk::ISeedingTool> {

  public:
    using seed_type = ActsTrk::Seed;

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
    std::vector<Acts::TrigInDetSiLayer> LayerNumbering() const;

    std::pair<int,int> getCombinedID(const int eta_mod, const short barrel_ec, const int lay_id) const;  


    
  private:

      //trying new SP class as temporary solution
    struct GbtsSpacePoint { 
      float m_x {0.f};
      float m_y {0.f};
      float m_z {0.f};
      float m_r {0.f}; 
      std::vector<int> SourceLinks ; //temp solution, used to check if pixel sp in acts core
      const xAOD::SpacePoint* input_SP ;  //temporary 
      const xAOD::SpacePoint* return_SP() const {return input_SP;} ;  
      float x() const {return m_x;}
      float y() const {return m_y;}
      float z() const {return m_z;}
      float r() const {return m_r;}
      const std::vector<int>& sourceLinks() const {return SourceLinks;}
      GbtsSpacePoint(float x, float y, float z, float r, const xAOD::SpacePoint* SP ) : m_x(x), m_y(y), m_z(z), m_r(r), SourceLinks{1}, input_SP(SP) {
     
      }; 
    };

    StatusCode prepareConfiguration();
    
    Acts::SeedFinderGbtsConfig<ActsTrk::GbtsSeedingTool::GbtsSpacePoint> m_finderCfg; //temp for new seed type 
    
    std::unique_ptr<Acts::GbtsGeometry<ActsTrk::GbtsSeedingTool::GbtsSpacePoint>> m_gbtsGeo = nullptr;

    const PixelID* m_pixelId = nullptr ; 
    const InDetDD::PixelDetectorManager* m_pixelManager = nullptr ; 

    // Used by Seed Finder Gbts Config
    Gaudi::Property<float> m_minPt {this, "minPt", 900. * Acts::UnitConstants::MeV, 
	"Lower cutoff for seeds"};

    Gaudi::Property<float> m_sigmaScattering {this, "sigmaScattering", 2, 
	"how many sigmas of scattering angle should be considered"};

    Gaudi::Property<unsigned int> m_maxSeedsPerSpM {this, "maxSeedsPerSpM", 5, // also used by SeedFilterConfig
	"For how many seeds can one SpacePoint be the middle SpacePoint"};

    // Geometry Settings
    // Detector ROI
    Gaudi::Property<float> m_highland {this, "highland",0, "need to check "};
    Gaudi::Property<float> m_maxScatteringAngle2 {this, "maxScatteringAngle2",0, "need to check "};

    Gaudi::Property<float> m_helixCutTolerance {this, "helixCutTolerance",1, "Parameter which can loosen the tolerance of the track seed to form a helix. This is useful for e.g. misaligned seeding."};

    // for load space points
    Gaudi::Property<float> m_phiSliceWidth {this, "phiSliceWidth",0, "initialised in loadSpacePoints function"};
    Gaudi::Property<float> m_nMaxPhiSlice {this, "nMaxPhiSlice",53, "used to calculate phi slices"};
    Gaudi::Property<bool> m_useClusterWidth {this, "useClusterWidth",false, "bool for use of cluster width in loadSpacePoints function"};
    Gaudi::Property<std::string> m_connectorInputFile {this, "connector_input_file","binTables_ITK_RUN4.txt", "input file for making connector object"}; //add filepath name from athena 
   
    // for runGbts_TrackFinder
    Gaudi::Property<bool> m_LRTmode {this, "LRTmode",true, "bool for use of in run function"};
    Gaudi::Property<bool> m_useEtaBinning {this, "useEtaBinning",true, "bool to use eta binning from geometry structure"};
    Gaudi::Property<bool> m_doubletFilterRZ {this, "doubletFilterRZ",true, "bool applies new Z cuts on doublets"};
    Gaudi::Property<float> m_minDeltaRadius {this, "minDeltaRadius",2.0, " min dr for doublet"};
    Gaudi::Property<float> m_tripletD0Max {this, "tripletD0Max",4.0, " D0 cut for triplets"};
    Gaudi::Property<unsigned int> m_maxTripletBufferLength {this, "maxTripletBufferLength",3, " maximum number of space points per triplet"};
    Gaudi::Property<int> m_MaxEdges {this, "MaxEdges",2000000, " max number of Gbts edges/doublets"};
    Gaudi::Property<float> m_cut_dphi_max {this, "cut_dphi_max",0.012, " phi cut for triplets"};
    Gaudi::Property<float> m_cut_dcurv_max {this, "cut_dcurv_max",0.001, " curv cut for triplets"};
    Gaudi::Property<float> m_cut_tau_ratio_max {this, "cut_tau_ratio_max",0.007, "tau cut for doublets and triplets"};
    Gaudi::Property<float> m_maxOuterRadius {this, "maxOuterRadius",550.0, "used to calculate Z cut on doublets"};
    Gaudi::Property<float> m_PtMin {this, "PtMin",1000.0, "pt limit used to caluclate triplet pT"};
    Gaudi::Property<float> m_tripletPtMinFrac {this, "tripletPtMinFrac",0.3, "used to caluclate triplet pt"};
    Gaudi::Property<float> m_tripletPtMin {this, "tripletPtMin",m_PtMin * m_tripletPtMinFrac, "Limit on triplet pt"};
    Gaudi::Property<double> m_ptCoeff {this, "ptCoeff", 0.29997 * 1.9972 / 2.0, "~0.3*B/2 - assumes nominal field of 2*T"};


    // Used by Seed Filter Config
    Gaudi::Property<float> m_deltaInvHelixDiameter {this, "deltaInvHelixDiameter", 0.00003 * 1. / Acts::UnitConstants::mm,
	"The allowed delta between two inverted seed radii for them to be considered compatible"};
    Gaudi::Property<float> m_impactWeightFactor {this, "impactWeightFactor", 100., 
	"The transverse impact parameters (d0) is multiplied by this factor and subtracted from weight"};
    Gaudi::Property<float> m_zOriginWeightFactor {this, "zOriginWeightFactor", 1.,
"The logitudinal impact parameters (z0) is multiplied by this factor and subtracted from weight"};
    Gaudi::Property<float> m_compatSeedWeight {this, "compatSeedWeight", 100.,
	"Seed weight increased by this value if a compatible seed has been found."};
    Gaudi::Property<float> m_deltaRMin {this, "deltaRMin", 20. * Acts::UnitConstants::mm,
	"Minimum distance between compatible seeds to be considered for weight boost"};
    Gaudi::Property<std::size_t> m_compatSeedLimit {this, "compatSeedLimit", 3,
	"How often do you want to increase the weight of a seed for finding a compatible seed"};

    Gaudi::Property<float> m_seedWeightIncrement {this, "seedWeightIncrement", 0};
    Gaudi::Property<float> m_numSeedIncrement {this, "numSeedIncrement", 3.40282e+38}; // cannot use std::numeric_limits<float>::infinity() 

    Gaudi::Property<bool> m_seedConfirmationInFilter {this, "seedConfirmationInFilter", true,
	"Seed Confirmation"};

    Gaudi::Property<int> m_maxSeedsPerSpMConf {this, "maxSeedsPerSpMConf", 5,
	"maximum number of lower quality seeds in seed confirmation"};
    Gaudi::Property<int> m_maxQualitySeedsPerSpMConf {this, "maxQualitySeedsPerSpMConf", 5,
	"maximum number of quality seeds for each middle-bottom SP-duplet in seed confirmation if the limit is reached we check if there is a lower quality seed to be replaced"};

    Gaudi::Property<bool> m_useDeltaRorTopRadius {this, "useDeltaRorTopRadius", true,
	"use deltaR between top and middle SP instead of top radius to search for compatible SPs"};



    // Used by SeedConfirmationRangeConfig 
    Gaudi::Property< float > m_seedConfCentralZMin {this, "seedConfCentralZMin", -250. * Acts::UnitConstants::mm,
	"minimum z for central seed confirmation "};
    Gaudi::Property< float > m_seedConfCentralZMax {this, "seedConfCentralZMax", 250. * Acts::UnitConstants::mm,
	"maximum z for central seed confirmation "};
    Gaudi::Property< float > m_seedConfCentralRMax {this, "seedConfCentralRMax", 140. * Acts::UnitConstants::mm,
	"maximum r for central seed confirmation "};
    Gaudi::Property< size_t > m_seedConfCentralNTopLargeR {this, "seedConfCentralNTopLargeR", 1,
	"nTop for large R central seed confirmation"};
    Gaudi::Property< size_t > m_seedConfCentralNTopSmallR {this, "seedConfCentralNTopSmallR", 2,
	"nTop for small R central seed confirmation"};
    Gaudi::Property< float > m_seedConfCentralMinBottomRadius {this, "seedConfCentralMinBottomRadius", 60 * Acts::UnitConstants::mm,
        "Minimum radius for bottom SP in seed confirmation"};
    Gaudi::Property< float > m_seedConfCentralMaxZOrigin {this, "seedConfCentralMaxZOrigin", 150 * Acts::UnitConstants::mm,
        "Maximum zOrigin in seed confirmation"};
    Gaudi::Property< float > m_seedConfCentralMinImpact {this, "seedConfCentralMinImpact", 1. * Acts::UnitConstants::mm,
        "Minimum impact parameter for seed confirmation"};

    Gaudi::Property< float > m_seedConfForwardZMin {this, "seedConfForwardZMin", -3000. * Acts::UnitConstants::mm,
	"minimum z for forward seed confirmation "};
    Gaudi::Property< float > m_seedConfForwardZMax {this, "seedConfForwardZMax", 3000. * Acts::UnitConstants::mm,
	"maximum z for forward seed confirmation "};
    Gaudi::Property< float > m_seedConfForwardRMax {this, "seedConfForwardRMax", 140. * Acts::UnitConstants::mm,
	"maximum r for forward seed confirmation "};
    Gaudi::Property< size_t > m_seedConfForwardNTopLargeR {this, "seedConfForwardNTopLargeR", 1,
	"nTop for large R forward seed confirmation"};
    Gaudi::Property< size_t > m_seedConfForwardNTopSmallR {this, "seedConfForwardNTopSmallR", 2,
	"nTop for small R forward seed confirmation"};
    Gaudi::Property< float > m_seedConfForwardMinBottomRadius {this, "seedConfForwardMinBottomRadius", 60 * Acts::UnitConstants::mm,
	"Minimum radius for bottom SP in seed confirmation"};
    Gaudi::Property< float > m_seedConfForwardMaxZOrigin {this, "seedConfForwardMaxZOrigin", 150 * Acts::UnitConstants::mm,
        "Maximum zOrigin in seed confirmation"};
    Gaudi::Property< float > m_seedConfForwardMinImpact {this, "seedConfForwardMinImpact", 1. * Acts::UnitConstants::mm,
        "Minimum impact parameter for seed confirmation"};


    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetectorElements", "ITkPixelDetectorElementCollection", "Key of input SiDetectorElementCollection for Pixel"}; 

    /// Private access to the logger
    const Acts::Logger &logger() const { return *m_logger; }
    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger {nullptr};

  };
  
} // namespace

#endif

