/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_ORTHOGONALSEEDINGTOOL_SEEDINGTOOL_H
#define ACTSTRK_ORTHOGONALSEEDINGTOOL_SEEDINGTOOL_H 1

// ATHENA
#include "ActsToolInterfaces/ISeedingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ACTS CORE
#include "Acts/Utilities/KDTree.hpp"
#include "Acts/Geometry/Extent.hpp"
#include "Acts/Seeding/Seed.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinderOrthogonal.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Seeding/SeedFinderOrthogonalConfig.hpp"
#include "Acts/Seeding/SeedFinderOrthogonal.hpp"
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFilter.hpp"

#include <cmath> //for M_PI

namespace ActsTrk {
  
  class OrthogonalSeedingTool :
    public extends<AthAlgTool, ActsTrk::ISeedingTool> {

  public:
    using value_type = xAOD::SpacePoint;
    using seed_type = Acts::Seed< xAOD::SpacePoint >;

    OrthogonalSeedingTool(const std::string& type, const std::string& name,
			  const IInterface* parent);
    virtual ~OrthogonalSeedingTool() = default;
    
    virtual StatusCode initialize() override;
    
    // Interface
    virtual StatusCode
      createSeeds(const EventContext& ctx,
		  const std::vector<const xAOD::SpacePoint*>& spContainer,
		  const Acts::Vector3& beamSpotPos,
		  const Acts::Vector3& bField,
		  ActsTrk::SeedContainer& seedContainer ) const override;
    
  private:
    StatusCode prepareConfiguration();
    
    // *********************************************************************
    // *********************************************************************

  private:
    Acts::SeedFinderOrthogonal<value_type> m_finder;
    Acts::SeedFinderOrthogonalConfig<value_type> m_finderCfg;

  private:
    // Used by Seed Finder Orthogonal Config
    Gaudi::Property<float> m_minPt {this, "minPt", 900. * Acts::UnitConstants::MeV, 
	"Lower cutoff for seeds"};
    Gaudi::Property<float> m_cotThetaMax {this, "cotThetaMax", 27.2899,
	"Cot of maximum theta angle"};
    Gaudi::Property<float> m_deltaRMinTopSP {this, "deltaRMinTopSP", 6 * Acts::UnitConstants::mm, 
	"minimum distance in r between middle and top SP in one seed"};
    Gaudi::Property<float> m_deltaRMaxTopSP {this, "deltaRMaxTopSP", 280 * Acts::UnitConstants::mm,
	"maximum distance in r between middle and top SP in one seed"};
    Gaudi::Property<float> m_deltaRMinBottomSP {this, "deltaRMinBottomSP", 6 * Acts::UnitConstants::mm,
	"minimum distance in r between middle and bottom SP in one seed"};
    Gaudi::Property<float> m_deltaRMaxBottomSP {this, "deltaRMaxBottomSP", 120 * Acts::UnitConstants::mm, 
	"maximum distance in r between middle and bottom SP in one seed"};

    Gaudi::Property<float> m_impactMax {this, "impactMax", 2. * Acts::UnitConstants::mm,
	"impact parameter"};

    Gaudi::Property<float> m_sigmaScattering {this, "sigmaScattering", 2, 
	"how many sigmas of scattering angle should be considered"};
    Gaudi::Property<float> m_maxPtScattering {this, "maxPtScattering", 10e6,
	"Upper pt limit for scattering calculation"};

    Gaudi::Property<unsigned int> m_maxSeedsPerSpM {this, "maxSeedsPerSpM", 5, // also used by SeedFilterConfig
	"For how many seeds can one SpacePoint be the middle SpacePoint"};

    // Geometry Settings
    // Detector ROI
    // limiting location of collision region in z
    Gaudi::Property<float> m_collisionRegionMin {this, "collisionRegionMin", -200 * Acts::UnitConstants::mm};
    Gaudi::Property<float> m_collisionRegionMax {this, "collisionRegionMax", 200 * Acts::UnitConstants::mm};
    Gaudi::Property<float> m_phiMin {this, "phiMin", -M_PI};
    Gaudi::Property<float> m_phiMax {this, "phiMax", M_PI};
    // limiting location of measurements
    Gaudi::Property<float> m_zMin {this, "zMin", -3000 * Acts::UnitConstants::mm};
    Gaudi::Property<float> m_zMax {this, "zMax", 3000 * Acts::UnitConstants::mm};
    Gaudi::Property<float> m_rMax {this, "rMax", 320 * Acts::UnitConstants::mm};
    Gaudi::Property<float> m_rMin {this, "rMin", 33 * Acts::UnitConstants::mm};

    Gaudi::Property<float> m_rMinMiddle {this, "rMinMiddle", 60.f * Acts::UnitConstants::mm};
    Gaudi::Property<float> m_rMaxMiddle {this, "rMaxMiddle", 120.f * Acts::UnitConstants::mm};

    Gaudi::Property<float> m_deltaPhiMax {this, "deltaPhiMax", 0.085};

    Gaudi::Property<float> m_deltaZMax {this, "deltaZMax", 600 * Acts::UnitConstants::mm,
	"Cut to the maximum value of delta z between SPs"};

    Gaudi::Property<bool> m_interactionPointCut {this, "interactionPointCut", true,
	"Enable cut on the compatibility between interaction point and SPs"};

    Gaudi::Property<bool> m_seedConfirmation {this, "seedConfirmation", true,
	"Seed Confirmation"};

    Gaudi::Property<float> m_radLengthPerSeed {this, "radLengthPerSeed", 0.1,
	"average radiation lengths of material on the length of a seed. used for scattering"};

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

    // Fast Track
    Gaudi::Property< bool > m_useExperimentCuts {this, "useExperimentCuts", false, ""};

    Gaudi::Property< std::vector<std::vector<double>> > m_rRangeMiddleSP {this, "rRangeMiddleSP",  
       {{40.0, 90.0}, {40.0, 200.0}, {46.0, 200.0}, {46.0, 200.0}, {46.0, 250.0}, {46.0, 250.0}, {46.0, 250.0}, {46.0, 200.0}, {46.0, 200.0}, {40.0, 200.0}, {40.0, 90.0}},
       "radial range for middle SP"};
  };
  
} // namespace

#endif

