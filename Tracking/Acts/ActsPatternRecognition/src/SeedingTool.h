/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKSEEDINGTOOL_SEEDINGTOOL_H
#define ACTSTRKSEEDINGTOOL_SEEDINGTOOL_H


// gcc12 gives false positive warnings from copying boost::small_vector.
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wstringop-overread"
#endif


// ATHENA
#include "ActsToolInterfaces/ISeedingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ACTS CORE
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Seeding/SpacePointGrid.hpp"
#include "Acts/Utilities/GridBinFinder.hpp"
#include "Acts/Seeding/BinnedGroup.hpp"
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinder.hpp"
#include "Acts/Seeding/Seed.hpp"

#include <cmath> //for M_PI

namespace ActsTrk {

  class SeedingTool :
    public extends<AthAlgTool, ActsTrk::ISeedingTool> {
  public:
    using value_type = xAOD::SpacePoint;
    using seed_type = Acts::Seed< xAOD::SpacePoint >;
    
    SeedingTool(const std::string& type, 
		const std::string& name,
		const IInterface* parent);
    virtual ~SeedingTool() = default;
    
    virtual StatusCode initialize() override;
    
    // Interface
    virtual StatusCode
      createSeeds(const EventContext& ctx,
		  const std::vector<const xAOD::SpacePoint*>& spContainer,
		  const Acts::Vector3& beamSpotPos,
		  const Acts::Vector3& bField,
		  ActsTrk::SeedContainer& seedContainer) const override;
    
  protected:
    // metafunction to obtain correct type in iterated container given the iterator type
    template<typename spacepoint_iterator_t>
    struct external_spacepoint {
      using type = typename std::conditional< 
                       std::is_pointer< typename spacepoint_iterator_t::value_type >::value,  
                       typename std::remove_const< typename std::remove_pointer< typename spacepoint_iterator_t::value_type >::type >::type,
                       typename std::remove_const< typename spacepoint_iterator_t::value_type >::type
                       >::type;
    };

    template< typename external_iterator_t >
      StatusCode
      createSeeds( external_iterator_t spBegin,
		   external_iterator_t spEnd,
		   const Acts::Vector3& beamSpotPos,
		   const Acts::Vector3& bField,
		   std::vector< seed_type >& seeds) const;
    
    StatusCode prepareConfiguration();

    // *********************************************************************
    // *********************************************************************

  protected:
    Acts::SeedFinder< value_type > m_finder;
    Acts::SeedFinderConfig< value_type > m_finderCfg;
    Acts::CylindricalSpacePointGridConfig m_gridCfg;

    // See quality selection
    Gaudi::Property< bool > m_seedQualitySelection {this, "doSeedQualitySelection", true,
	"Select seed according to quality criteria"};

    // Properties to set SpacePointGridConfig
    Gaudi::Property< float > m_minPt {this, "minPt", 900. * Acts::UnitConstants::MeV,
      "lower pT cutoff for seeds"}; // Used in SeedfinderConfig as well
    Gaudi::Property< float > m_cotThetaMax {this, "cotThetaMax", 27.2899,
      "cot of maximum theta angle"}; // Used in SeedfinderConfig as well
    Gaudi::Property< float > m_zMin {this, "zMin", -3000. * Acts::UnitConstants::mm,
      "limiting location of measurements"}; // Used in SeedfinderConfig as well
    Gaudi::Property< float > m_zMax {this, "zMax", 3000. * Acts::UnitConstants::mm,
      "limiting location of measurements"}; // Used in SeedfinderConfig as well
    Gaudi::Property< float > m_deltaRMax {this, "deltaRMax",  280. * Acts::UnitConstants::mm,
      "maximum distance in r between two measurements within one seed"}; // Used in SeedfinderConfig as well
    Gaudi::Property< float > m_impactMax {this, "impactMax", 2. * Acts::UnitConstants::mm,
      "maximum impact parameter"}; // Used in SeedfinderConfig as well
    Gaudi::Property< std::vector< float > > m_zBinEdges {this, "zBinEdges",
      {-3000., -2500., -1400., -925., -500., -250.,  250., 500., 925.,   1400.,  2500.,  3000.},
      "enable non equidistant binning in z"}; // Used in SeedfinderConfig as well
    Gaudi::Property< float > m_gridRMax {this, "gridRMax", 320. * Acts::UnitConstants::mm,
      "radial extension of subdetector to be used in grid building"};
    Gaudi::Property< float > m_gridPhiMin {this, "gridPhiMin", -M_PI,
      "phi min for space point grid formation"};
    Gaudi::Property< float > m_gridPhiMax {this, "gridPhiMax", M_PI,
      "phi max for space point grid formation"};
    Gaudi::Property< int > m_phiBinDeflectionCoverage {this, "phiBinDeflectionCoverage", 3,
      "sets of consecutive phi bins to cover full deflection of minimum pT particle"};

    // Properties to set SeedfinderConfig
    Gaudi::Property< float > m_rMax {this, "rMax", 320. * Acts::UnitConstants::mm,
      "limiting location of measurements"};
    Gaudi::Property< float > m_binSizeR {this, "binSizeR", 1. * Acts::UnitConstants::mm,
      "defining radial bin for space point sorting"};
    Gaudi::Property< float > m_deltaRMin {this, "deltaRMin", 20. * Acts::UnitConstants::mm,
      "minimum distance in r between two measurements within one seed"}; // Used in SeedFilterConfig as well
    Gaudi::Property< float > m_deltaRMinTopSP {this, "deltaRMinTopSP", 6. * Acts::UnitConstants::mm,
      "minimum distance in r between middle and top SP"};
    Gaudi::Property< float > m_deltaRMaxTopSP {this, "deltaRMaxTopSP", 280. * Acts::UnitConstants::mm,
      "maximum distance in r between middle and top SP"};
    Gaudi::Property< float > m_deltaRMinBottomSP {this, "deltaRMinBottomSP", 6. * Acts::UnitConstants::mm,
      "minimum distance in r between middle and top SP"};
    Gaudi::Property< float > m_deltaRMaxBottomSP {this, "deltaRMaxBottomSP", 150. * Acts::UnitConstants::mm,
      "maximum distance in r between middle and top SP"};
    Gaudi::Property< float > m_deltaZMax {this, "deltaZMax",  600,
      "maximum distance in z between two measurements within one seed"};
    Gaudi::Property< float > m_collisionRegionMin {this, "collisionRegionMin", -200. * Acts::UnitConstants::mm,
      "limiting location of collision region in z"};
    Gaudi::Property< float > m_collisionRegionMax {this, "collisionRegionMax", 200. * Acts::UnitConstants::mm,
      "limiting location of collision region in z"};
    Gaudi::Property< float > m_sigmaScattering {this, "sigmaScattering", 2.,
      "how many sigmas of scattering angle should be considered"};
    Gaudi::Property< float > m_maxPtScattering {this, "maxPtScattering", 10e6,
      "Upper pt limit for scattering calculation"};
    Gaudi::Property< float > m_radLengthPerSeed {this, "radLengthPerSeed", 0.098045,
      "average radiation lengths of material on the length of a seed. used for scattering"};
    Gaudi::Property< int > m_maxSeedsPerSpM {this, "maxSeedsPerSpM", 4,
      "In dense environments many seeds may be found per middle space point. Only seeds with the highest weight will be kept if this limit is reached."}; // Used in SeedFilterConfig as well
    Gaudi::Property< bool > m_interactionPointCut {this, "interactionPointCut", true,
      "Enable cut on the compatibility between interaction point and SPs"};
    Gaudi::Property< std::vector< size_t > > m_zBinsCustomLooping {this, "zBinsCustomLooping",
      {1, 2, 3, 4, 11, 10, 9, 8, 6, 5, 7} , "defines order of z bins for looping"};
    Gaudi::Property< bool > m_useVariableMiddleSPRange {this, "useVariableMiddleSPRange", true,
      "Enable variable range to search for middle SPs"};
    Gaudi::Property< std::vector<std::vector<double>> > m_rRangeMiddleSP {this, "rRangeMiddleSP", 
      {{40.0, 90.0}, {40.0, 200.0}, {46.0, 200.0}, {46.0, 200.0}, {46.0, 250.0}, {46.0, 250.0}, {46.0, 250.0}, {46.0, 200.0}, {46.0, 200.0}, {40.0, 200.0}, {40.0, 90.0}}, 
      "radial range for middle SP"};
    Gaudi::Property< float > m_deltaRMiddleMinSPRange {this, "deltaRMiddleMinSPRange", 10.,
      "delta R for middle SP range (min)"};
    Gaudi::Property< float > m_deltaRMiddleMaxSPRange {this, "deltaRMiddleMaxSPRange", 10.,
      "delta R for middle SP range (max)"};
    Gaudi::Property< bool > m_seedConfirmation {this, "seedConfirmation", true,
      "run seed confirmation"};
    Gaudi::Property< float > m_seedConfCentralZMin {this, "seedConfCentralZMin", -250. * Acts::UnitConstants::mm,
      "minimum z for central seed confirmation "};
      // Used in SeedFilterConfig as well
    Gaudi::Property< float > m_seedConfCentralZMax {this, "seedConfCentralZMax", 250. * Acts::UnitConstants::mm,
      "maximum z for central seed confirmation "};
      // Used in SeedFilterConfig as well
    Gaudi::Property< float > m_seedConfCentralRMax {this, "seedConfCentralRMax", 140. * Acts::UnitConstants::mm,
      "maximum r for central seed confirmation "};
      // Used in SeedFilterConfig as well
    Gaudi::Property< size_t > m_seedConfCentralNTopLargeR {this, "seedConfCentralNTopLargeR", 1,
      "nTop for large R central seed confirmation"};
      // Used in SeedFilterConfig as well
    Gaudi::Property< size_t > m_seedConfCentralNTopSmallR {this, "seedConfCentralNTopSmallR", 2,
      "nTop for small R central seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< float > m_seedConfCentralMinBottomRadius {this, "seedConfCentralMinBottomRadius", 60 * Acts::UnitConstants::mm,
	"Minimum radius for bottom SP in seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< float > m_seedConfCentralMaxZOrigin {this, "seedConfCentralMaxZOrigin", 150 * Acts::UnitConstants::mm,
	"Maximum zOrigin in seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< float > m_seedConfCentralMinImpact {this, "seedConfCentralMinImpact", 1. * Acts::UnitConstants::mm,
	"Minimum impact parameter for seed confirmation"};
      // Used in SeedFilterConfig as well
    Gaudi::Property< float > m_seedConfForwardZMin {this, "seedConfForwardZMin", -3000. * Acts::UnitConstants::mm,
      "minimum z for forward seed confirmation "};
      // Used in SeedFilterConfig as well
    Gaudi::Property< float > m_seedConfForwardZMax {this, "seedConfForwardZMax", 3000. * Acts::UnitConstants::mm,
      "maximum z for forward seed confirmation "};
      // Used in SeedFilterConfig as well
    Gaudi::Property< float > m_seedConfForwardRMax {this, "seedConfForwardRMax", 140. * Acts::UnitConstants::mm,
      "maximum r for forward seed confirmation "};
      // Used in SeedFilterConfig as well
    Gaudi::Property< size_t > m_seedConfForwardNTopLargeR {this, "seedConfForwardNTopLargeR", 1,
      "nTop for large R forward seed confirmation"};
      // Used in SeedFilterConfig as well
    Gaudi::Property< size_t > m_seedConfForwardNTopSmallR {this, "seedConfForwardNTopSmallR", 2,
      "nTop for small R forward seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< float > m_seedConfForwardMinBottomRadius {this, "seedConfForwardMinBottomRadius", 60 * Acts::UnitConstants::mm,
	"Minimum radius for bottom SP in seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< float > m_seedConfForwardMaxZOrigin {this, "seedConfForwardMaxZOrigin", 150 * Acts::UnitConstants::mm,
	"Maximum zOrigin in seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< float > m_seedConfForwardMinImpact {this, "seedConfForwardMinImpact", 1. * Acts::UnitConstants::mm,
	"Minimum impact parameter for seed confirmation"};
    // Used in SeedFilterConfig as well 
    Gaudi::Property< bool > m_useDetailedDoubleMeasurementInfo {this, "useDetailedDoubleMeasurementInfo", false,
      "enable use of double measurement details"};

    Gaudi::Property<float> m_toleranceParam {this, "toleranceParam", 1.1 * Acts::UnitConstants::mm, 
      "tolerance parameter used to check the compatibility of SPs coordinates in xyz"};
    Gaudi::Property<float> m_phiMin {this, "phiMin", -M_PI, ""};
    Gaudi::Property<float> m_phiMax {this, "phiMax", M_PI, ""};
    Gaudi::Property<float> m_rMin {this, "rMin", 0 * Acts::UnitConstants::mm, ""};    
    Gaudi::Property<float> m_zAlign {this, "zAlign", 0 * Acts::UnitConstants::mm, ""};
    Gaudi::Property<float> m_rAlign {this, "rAlign", 0 * Acts::UnitConstants::mm, ""};
    Gaudi::Property<float> m_sigmaError {this, "sigmaError", 5, ""};

    // Properties to set SeedFilterConfig
    Gaudi::Property< float > m_impactWeightFactor {this, "impactWeightFactor", 100.,
      "the impact parameters (d0) is multiplied by this factor and subtracted from weight"};
    Gaudi::Property< float > m_zOriginWeightFactor {this, "zOriginWeightFactor", 1.};
    Gaudi::Property< float > m_compatSeedWeight {this, "compatSeedWeight", 100.,
      "seed weight increased by this value if a compatible seed has been found"};
    Gaudi::Property< std::size_t > m_compatSeedLimit {this, "compatSeedLimit", 3,
      "how often do you want to increase the weight of a seed for finding a compatible seed"};
    Gaudi::Property< float > m_seedWeightIncrement {this, "seedWeightIncrement", 0.,
      "increment in seed weight if needed"};
    Gaudi::Property< float > m_numSeedIncrement {this, "numSeedIncrement", 10e6,
      "increment in seed weight is applied if the number of compatible seeds is larger than numSeedIncrement"};
    Gaudi::Property< bool > m_seedConfirmationInFilter {this, "seedConfirmationInFilter", true,
      "run seed confirmation"};
    Gaudi::Property< std::size_t > m_maxSeedsPerSpMConf {this, "maxSeedsPerSpMConf", 5,
      "Maximum number of lower quality seeds in seed confirmation."};
    Gaudi::Property< std::size_t > m_maxQualitySeedsPerSpMConf {this, "maxQualitySeedsPerSpMConf", 5,
      "Maximum number of quality seeds for each middle-bottom SP-duplet in seed confirmation."};
    Gaudi::Property< bool > m_useDeltaRorTopRadius {this, "useDeltaRorTopRadius", true,
      "use deltaR (top radius - middle radius) instead of top radius"};
    Gaudi::Property<float> m_deltaInvHelixDiameter {this, "deltaInvHelixDiameter", 0.00003 * 1. / Acts::UnitConstants::mm, 
      "the allowed delta between two inverted seed radii for them to be considered compatible"};

    // Properties to set other objects used in
    // seeding algorithm
    Gaudi::Property< std::vector<std::pair<int, int>> > m_zBinNeighborsTop{this,
      "zBinNeighborsTop",
      {{0, 0},  {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 1}, {0, 1},  {0, 1}, {0, 1},  {0, 1},  {0, 0}},
      "vector containing the map of z bins in the top layers"};
    Gaudi::Property< std::vector<std::pair<int, int>> > m_zBinNeighborsBottom{this, "zBinNeighborsBottom",
      {{0, 1},  {0, 1},  {0, 1},  {0, 1}, {0, 1},  {0, 0},  {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},
      "vector containing the map of z bins in the top layers"};
    Gaudi::Property< int > m_numPhiNeighbors {this, "numPhiNeighbors", 1,
      "number of phi bin neighbors at each side of the current bin that will be used to search for SPs"};

  private:
    std::unique_ptr< Acts::GridBinFinder< 2ul > > m_bottomBinFinder{nullptr};
    std::unique_ptr< Acts::GridBinFinder< 2ul > > m_topBinFinder{nullptr};

    std::array<std::vector<std::size_t>, 2ul> m_navigation{};
  };

} // namespace

#endif

