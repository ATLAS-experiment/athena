/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKSEEDINGTOOL_SEEDINGTOOL_H
#define ACTSTRKSEEDINGTOOL_SEEDINGTOOL_H



// gcc12 gives false positive warnings from copying boost::small_vector.
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wstringop-overread"
#endif

// Super-nasty hack to work round explicit uses of Acts::Seed in Acts Core Seeding.
// A better fix would be to change Acts::Seed to the templates that are used elsewhere in Acts Core.
// The even better fix would be to change Acts::Seed to support more than 3 SPs/seed.
#include "Acts/EventData/Seed.hpp"
#include "ActsEvent/Seed.h"
namespace Acts {
  template <typename external_spacepoint_t, std::size_t N = 3ul>
  using AthenaSeed = typename ActsTrk::ActsSeed<external_spacepoint_t, N>;
}

#define Seed AthenaSeed
#include "Acts/Seeding/SeedFinder.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#undef Seed

// ATHENA
#include "ActsToolInterfaces/ISeedingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsInterop/Logger.h"
#include "ActsEvent/SeedContainer.h"

// ACTS CORE
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Seeding/SpacePointGrid.hpp"
#include "Acts/Utilities/GridBinFinder.hpp"
#include "Acts/Seeding/BinnedGroup.hpp"
#include "Acts/Seeding/SeedFinderConfig.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/EventData/Seed.hpp"

#include <numbers>

namespace ActsTrk {

  class SeedingTool :
    public extends<AthAlgTool, ActsTrk::ISeedingTool> {
  public:
    using value_type = typename Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>::SpacePointProxyType;
    using seed_type = ActsTrk::ActsSeed< value_type, 3ul >;
    using external_type = typename std::conditional< 
      std::is_const< typename value_type::ValueType >::value,
      typename std::remove_const< typename value_type::ValueType >::type,
      typename value_type::ValueType
      >::type;
    

    SeedingTool(const std::string& type, 
		const std::string& name,
		const IInterface* parent);
    virtual ~SeedingTool() = default;
    
    virtual StatusCode initialize() override;
    
    // Interface
    virtual StatusCode
      createSeeds(const EventContext& ctx,
		  const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer,
		  const Acts::Vector3& beamSpotPos,
		  const Acts::Vector3& bField,
		  ActsTrk::SeedContainer& seedContainer ) const override;
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
		   DataVector< ActsTrk::ActsSeed< external_type, 3ul > >& seeds ) const;
    
    StatusCode prepareConfiguration();

    // *********************************************************************
    // *********************************************************************

  protected:
    Acts::SeedFinder< value_type, Acts::CylindricalSpacePointGrid<value_type> > m_finder;
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
      {-3000., -2700., -2500., -1400., -925., -500., -250.,  250., 500., 925.,   1400.,  2500., 2700, 3000.},
      "enable non equidistant binning in z"}; // Used in SeedfinderConfig as well
    Gaudi::Property< std::vector< float > > m_rBinEdges {this, "rBinEdges", {0., 1100 * Acts::UnitConstants::mm},
      "enable non equidistant binning in radius"};
    Gaudi::Property< float > m_gridRMax {this, "gridRMax", 320. * Acts::UnitConstants::mm,
      "radial extension of subdetector to be used in grid building"};
    Gaudi::Property< float > m_gridPhiMin {this, "gridPhiMin", -std::numbers::pi_v<float>,
      "phi min for space point grid formation"};
    Gaudi::Property< float > m_gridPhiMax {this, "gridPhiMax", std::numbers::pi_v<float>,
      "phi max for space point grid formation"};
    Gaudi::Property< int > m_phiBinDeflectionCoverage {this, "phiBinDeflectionCoverage", 3,
      "sets of consecutive phi bins to cover full deflection of minimum pT particle"};
    Gaudi::Property< int > m_maxPhiBins {this, "maxPhiBins", 200, "max number of bins"};

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
      {2, 3, 4, 5, 12, 11, 10, 9, 7, 6, 8} , "defines order of z bins for looping"};
    Gaudi::Property< std::vector<std::size_t> > m_rBinsCustomLooping {this, "rBinsCustomLooping", {1},
      "defines order of r bins for looping"};
    Gaudi::Property< bool > m_useVariableMiddleSPRange {this, "useVariableMiddleSPRange", true,
      "Enable variable range to search for middle SPs"};
    Gaudi::Property< std::vector<std::vector<double>> > m_rRangeMiddleSP {this, "rRangeMiddleSP", 
      {{40.0, 90.0}, {40.0, 90.0}, {40.0, 200.0}, {46.0, 200.0}, {46.0, 200.0}, {46.0, 250.0}, {46.0, 250.0}, {46.0, 250.0}, {46.0, 200.0}, {46.0, 200.0}, {40.0, 200.0}, {40.0, 90.0}, {40.0, 90.0}}, 
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
    Gaudi::Property<float> m_phiMin {this, "phiMin", -std::numbers::pi_v<float>, ""};
    Gaudi::Property<float> m_phiMax {this, "phiMax", std::numbers::pi_v<float>, ""};
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
      {{0, 0},  {-1, 0}, {-2, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 1}, {0, 1},  {0, 1}, {0, 1}, {0, 2} , {0, 1},  {0, 0}},
      "vector containing the map of z bins in the top layers"};
    Gaudi::Property< std::vector<std::pair<int, int>> > m_zBinNeighborsBottom{this, "zBinNeighborsBottom",
      {{0, 0}, {0, 1},  {0, 1},  {0, 1},  {0, 1}, {0, 1},  {0, 0},  {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {0, 0}},
      "vector containing the map of z bins in the top layers"};
    Gaudi::Property< std::vector<std::pair<int, int>> > m_rBinNeighborsTop{this, "rBinNeighborsTop", {{0, 0}},
      "vector containing the map of radius bins in the top layers"};
    Gaudi::Property< std::vector<std::pair<int, int>> > m_rBinNeighborsBottom{this, "rBinNeighborsBottom", {{0, 0}},
      "vector containing the map of radius bins in the bottom layers"};
    Gaudi::Property< int > m_numPhiNeighbors {this, "numPhiNeighbors", 1,
      "number of phi bin neighbors at each side of the current bin that will be used to search for SPs"};

    Gaudi::Property< bool > m_useExperimentCuts {this, "useExperimentCuts", false, ""};

    Gaudi::Property< int > m_stateVectorReserveSize {this, "stateVectorReserveSize", 500, "Size of the initial Seeding State internal vectors"};
    
  private:
    std::unique_ptr< Acts::GridBinFinder< 3ul > > m_bottomBinFinder{nullptr};
    std::unique_ptr< Acts::GridBinFinder< 3ul > > m_topBinFinder{nullptr};

    std::array<std::vector<std::size_t>, 3ul> m_navigation{};

    /// Private access to the logger
    const Acts::Logger &logger() const { return *m_logger; }
    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger {nullptr};

    // A conservative guess of the size of the vectors needed for seeding
    
    
    static constexpr float m_ExpCutrMin = 45.;
    
    static inline bool itkFastTrackingSPselect(const value_type& sp) {
      // At small r we remove points beyond |z| > 200.
      float r = sp.radius();
      float zabs = std::abs(sp.z());

      // We perform a triangular cut and remove the space points
      // that have |z| < 200 and radius < m_ExpCutrMin
      // But we do that only if the eta of the space point wrt origin is < 3.6
      // eta 3.6 corresponds to 18.2855
      if (zabs > 200. and
	  zabs < 18.2855 * r and
	  r < m_ExpCutrMin) {
	return false;
      }
            
      /// Remove space points beyond eta=4 if their z is
      /// larger than the max seed z0 (150.)
      float cotTheta = 27.2899;  // corresponds to eta=4
      if ((zabs - 150.) > cotTheta * r) {
	return false;
      }

      return true;
    }

    static inline bool itkFastDoubletCut(float bottomRadius, float cotTheta) {
      // We remove here some seeds, in case the bottom space point radius is
      // too small (i.e. < fastTrackingRMin)

      // This operation is done only within a specific eta window
      // Instead of eta we use the doublet cottheta
      // We require:
      //     fastTrackingCotThetaWindowMin < cottheta doublet < fastTrackingCotThetaWindowMax
      // with stranslates to an eta window.
      // cottheta of 1.5 is about eta 1.2
      // cottheta of 18.2855 is about eta of 3.6
      float fastTrackingCotThetaWindowMin = 1.5;
      float fastTrackingCotThetaWindowMax = 18.2855;

      float absCotTheta = std::abs(cotTheta);
      if (bottomRadius < m_ExpCutrMin and
	  absCotTheta > fastTrackingCotThetaWindowMin and
	  absCotTheta < fastTrackingCotThetaWindowMax) {
	return false;
      }

      return true;
    }
  };

  
  
} // namespace

#endif

