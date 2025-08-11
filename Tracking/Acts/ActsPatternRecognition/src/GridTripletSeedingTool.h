/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GRIDTRIPLETSEEDINGTOOL_GRIDTRIPLETSEEDINGTOOL_H
#define ACTSTRK_GRIDTRIPLETSEEDINGTOOL_GRIDTRIPLETSEEDINGTOOL_H

// ATHENA
#include "ActsInterop/Logger.h"
#include "ActsToolInterfaces/ISeedingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"

// ACTS CORE
#include "Acts/EventData/SeedContainer2.hpp"
#include "Acts/EventData/SpacePointContainer2.hpp"
#include "Acts/Seeding2/BroadTripletSeedFinder.hpp"
#include "Acts/Seeding2/CylindricalSpacePointGrid2.hpp"

// Other
#include <memory>
#include <optional>

namespace ActsTrk {

class GridTripletSeedingTool
    : public extends<AthAlgTool, ActsTrk::ISeedingTool> {
 public:
  GridTripletSeedingTool(const std::string& type, const std::string& name,
                         const IInterface* parent);

  virtual StatusCode initialize() override;

  StatusCode createSeeds2(
      const EventContext& ctx,
      const Acts::Experimental::SpacePointContainer2& spacePoints,
      const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
      Acts::Experimental::SeedContainer2& seedContainer) const override;

 protected:
  Gaudi::Property<bool> m_seedQualitySelection{
      this, "doSeedQualitySelection", true,
      "Select seed according to quality criteria"};

  // Cylindrical space point grid properties. Some are also used for the seed
  // finder.
  Gaudi::Property<float> m_minPt{this, "minPt", 900. * Acts::UnitConstants::MeV,
                                 "lower pT cutoff for seeds"};
  Gaudi::Property<float> m_cotThetaMax{this, "cotThetaMax", 27.2899,
                                       "cot of maximum theta angle"};
  Gaudi::Property<float> m_zMin{this, "zMin", -3000. * Acts::UnitConstants::mm,
                                "limiting location of measurements"};
  Gaudi::Property<float> m_zMax{this, "zMax", 3000. * Acts::UnitConstants::mm,
                                "limiting location of measurements"};
  Gaudi::Property<float> m_deltaRMax{
      this, "deltaRMax", 280. * Acts::UnitConstants::mm,
      "maximum distance in r between two measurements within one "
      "seed"};
  Gaudi::Property<float> m_impactMax{this, "impactMax",
                                     2. * Acts::UnitConstants::mm,
                                     "maximum impact parameter"};
  Gaudi::Property<std::vector<float>> m_zBinEdges{
      this,
      "zBinEdges",
      {-3000., -2700., -2500., -1400., -925., -500., -250., 250., 500., 925.,
       1400., 2500., 2700, 3000.},
      "enable non equidistant binning in z"};
  Gaudi::Property<std::vector<float>> m_rBinEdges{
      this,
      "rBinEdges",
      {0., 1100 * Acts::UnitConstants::mm},
      "enable non equidistant binning in radius"};
  Gaudi::Property<float> m_gridRMax{
      this, "gridRMax", 320. * Acts::UnitConstants::mm,
      "radial extension of subdetector to be used in grid building"};
  Gaudi::Property<float> m_gridPhiMin{this, "gridPhiMin",
                                      -std::numbers::pi_v<float>,
                                      "phi min for space point grid formation"};
  Gaudi::Property<float> m_gridPhiMax{this, "gridPhiMax",
                                      std::numbers::pi_v<float>,
                                      "phi max for space point grid formation"};
  Gaudi::Property<int> m_phiBinDeflectionCoverage{
      this, "phiBinDeflectionCoverage", 3,
      "sets of consecutive phi bins to cover full deflection of minimum pT "
      "particle"};
  Gaudi::Property<int> m_maxPhiBins{this, "maxPhiBins", 200,
                                    "max number of bins"};

  // Seed finder properties. Some are also used for the seed filter.
  Gaudi::Property<float> m_rMax{this, "rMax", 320. * Acts::UnitConstants::mm,
                                "limiting location of measurements"};
  Gaudi::Property<float> m_binSizeR{
      this, "binSizeR", 1. * Acts::UnitConstants::mm,
      "defining radial bin for space point sorting"};
  Gaudi::Property<float> m_deltaRMin{
      this, "deltaRMin", 20. * Acts::UnitConstants::mm,
      "minimum distance in r between two measurements within one "
      "seed"};
  Gaudi::Property<float> m_deltaRMinTopSP{
      this, "deltaRMinTopSP", 6. * Acts::UnitConstants::mm,
      "minimum distance in r between middle and top SP"};
  Gaudi::Property<float> m_deltaRMaxTopSP{
      this, "deltaRMaxTopSP", 280. * Acts::UnitConstants::mm,
      "maximum distance in r between middle and top SP"};
  Gaudi::Property<float> m_deltaRMinBottomSP{
      this, "deltaRMinBottomSP", 6. * Acts::UnitConstants::mm,
      "minimum distance in r between middle and top SP"};
  Gaudi::Property<float> m_deltaRMaxBottomSP{
      this, "deltaRMaxBottomSP", 150. * Acts::UnitConstants::mm,
      "maximum distance in r between middle and top SP"};
  Gaudi::Property<float> m_deltaZMax{
      this, "deltaZMax", 600,
      "maximum distance in z between two measurements within one seed"};
  Gaudi::Property<float> m_collisionRegionMin{
      this, "collisionRegionMin", -200. * Acts::UnitConstants::mm,
      "limiting location of collision region in z"};
  Gaudi::Property<float> m_collisionRegionMax{
      this, "collisionRegionMax", 200. * Acts::UnitConstants::mm,
      "limiting location of collision region in z"};
  Gaudi::Property<float> m_sigmaScattering{
      this, "sigmaScattering", 2.,
      "how many sigmas of scattering angle should be considered"};
  Gaudi::Property<float> m_maxPtScattering{
      this, "maxPtScattering", 10e6,
      "Upper pt limit for scattering calculation"};
  Gaudi::Property<float> m_radLengthPerSeed{
      this, "radLengthPerSeed", 0.098045,
      "average radiation lengths of material on the length of a seed. used for "
      "scattering"};
  Gaudi::Property<int> m_maxSeedsPerSpM{
      this, "maxSeedsPerSpM", 4,
      "In dense environments many seeds may be found per middle space point. "
      "Only seeds with the highest weight will be kept if this limit is "
      "reached."};
  Gaudi::Property<bool> m_interactionPointCut{
      this, "interactionPointCut", true,
      "Enable cut on the compatibility between interaction point and SPs"};
  Gaudi::Property<std::vector<size_t>> m_zBinsCustomLooping{
      this,
      "zBinsCustomLooping",
      {2, 3, 4, 5, 12, 11, 10, 9, 7, 6, 8},
      "defines order of z bins for looping"};
  Gaudi::Property<std::vector<std::size_t>> m_rBinsCustomLooping{
      this, "rBinsCustomLooping", {1}, "defines order of r bins for looping"};
  Gaudi::Property<bool> m_useVariableMiddleSPRange{
      this, "useVariableMiddleSPRange", true,
      "Enable variable range to search for middle SPs"};
  Gaudi::Property<std::vector<std::vector<double>>> m_rRangeMiddleSP{
      this,
      "rRangeMiddleSP",
      {{40.0, 90.0},
       {40.0, 90.0},
       {40.0, 200.0},
       {46.0, 200.0},
       {46.0, 200.0},
       {46.0, 250.0},
       {46.0, 250.0},
       {46.0, 250.0},
       {46.0, 200.0},
       {46.0, 200.0},
       {40.0, 200.0},
       {40.0, 90.0},
       {40.0, 90.0}},
      "radial range for middle SP"};
  Gaudi::Property<float> m_deltaRMiddleMinSPRange{
      this, "deltaRMiddleMinSPRange", 10., "delta R for middle SP range (min)"};
  Gaudi::Property<float> m_deltaRMiddleMaxSPRange{
      this, "deltaRMiddleMaxSPRange", 10., "delta R for middle SP range (max)"};
  Gaudi::Property<bool> m_seedConfirmation{this, "seedConfirmation", true,
                                           "run seed confirmation"};
  Gaudi::Property<float> m_seedConfCentralZMin{
      this, "seedConfCentralZMin", -250. * Acts::UnitConstants::mm,
      "minimum z for central seed confirmation "};
  Gaudi::Property<float> m_seedConfCentralZMax{
      this, "seedConfCentralZMax", 250. * Acts::UnitConstants::mm,
      "maximum z for central seed confirmation "};
  Gaudi::Property<float> m_seedConfCentralRMax{
      this, "seedConfCentralRMax", 140. * Acts::UnitConstants::mm,
      "maximum r for central seed confirmation "};
  Gaudi::Property<size_t> m_seedConfCentralNTopLargeR{
      this, "seedConfCentralNTopLargeR", 1,
      "nTop for large R central seed confirmation"};
  Gaudi::Property<size_t> m_seedConfCentralNTopSmallR{
      this, "seedConfCentralNTopSmallR", 2,
      "nTop for small R central seed confirmation"};
  Gaudi::Property<float> m_seedConfCentralMinBottomRadius{
      this, "seedConfCentralMinBottomRadius", 60 * Acts::UnitConstants::mm,
      "Minimum radius for bottom SP in seed confirmation"};
  Gaudi::Property<float> m_seedConfCentralMaxZOrigin{
      this, "seedConfCentralMaxZOrigin", 150 * Acts::UnitConstants::mm,
      "Maximum zOrigin in seed confirmation"};
  Gaudi::Property<float> m_seedConfCentralMinImpact{
      this, "seedConfCentralMinImpact", 1. * Acts::UnitConstants::mm,
      "Minimum impact parameter for seed confirmation"};
  Gaudi::Property<float> m_seedConfForwardZMin{
      this, "seedConfForwardZMin", -3000. * Acts::UnitConstants::mm,
      "minimum z for forward seed confirmation "};
  Gaudi::Property<float> m_seedConfForwardZMax{
      this, "seedConfForwardZMax", 3000. * Acts::UnitConstants::mm,
      "maximum z for forward seed confirmation "};
  Gaudi::Property<float> m_seedConfForwardRMax{
      this, "seedConfForwardRMax", 140. * Acts::UnitConstants::mm,
      "maximum r for forward seed confirmation "};
  Gaudi::Property<size_t> m_seedConfForwardNTopLargeR{
      this, "seedConfForwardNTopLargeR", 1,
      "nTop for large R forward seed confirmation"};
  Gaudi::Property<size_t> m_seedConfForwardNTopSmallR{
      this, "seedConfForwardNTopSmallR", 2,
      "nTop for small R forward seed confirmation"};
  Gaudi::Property<float> m_seedConfForwardMinBottomRadius{
      this, "seedConfForwardMinBottomRadius", 60 * Acts::UnitConstants::mm,
      "Minimum radius for bottom SP in seed confirmation"};
  Gaudi::Property<float> m_seedConfForwardMaxZOrigin{
      this, "seedConfForwardMaxZOrigin", 150 * Acts::UnitConstants::mm,
      "Maximum zOrigin in seed confirmation"};
  Gaudi::Property<float> m_seedConfForwardMinImpact{
      this, "seedConfForwardMinImpact", 1. * Acts::UnitConstants::mm,
      "Minimum impact parameter for seed confirmation"};
  Gaudi::Property<bool> m_useDetailedDoubleMeasurementInfo{
      this, "useDetailedDoubleMeasurementInfo", false,
      "enable use of double measurement details"};

  Gaudi::Property<float> m_toleranceParam{
      this, "toleranceParam", 1.1 * Acts::UnitConstants::mm,
      "tolerance parameter used to check the compatibility of SPs coordinates "
      "in xyz"};
  Gaudi::Property<float> m_phiMin{this, "phiMin", -std::numbers::pi_v<float>,
                                  ""};
  Gaudi::Property<float> m_phiMax{this, "phiMax", std::numbers::pi_v<float>,
                                  ""};
  Gaudi::Property<float> m_rMin{this, "rMin", 0 * Acts::UnitConstants::mm, ""};
  Gaudi::Property<float> m_zAlign{this, "zAlign", 0 * Acts::UnitConstants::mm,
                                  ""};
  Gaudi::Property<float> m_rAlign{this, "rAlign", 0 * Acts::UnitConstants::mm,
                                  ""};
  Gaudi::Property<float> m_sigmaError{this, "sigmaError", 5, ""};

  // Seed filter properties
  Gaudi::Property<float> m_impactWeightFactor{
      this, "impactWeightFactor", 100.,
      "the impact parameters (d0) is multiplied by this factor and subtracted "
      "from weight"};
  Gaudi::Property<float> m_zOriginWeightFactor{this, "zOriginWeightFactor", 1.};
  Gaudi::Property<float> m_compatSeedWeight{
      this, "compatSeedWeight", 100.,
      "seed weight increased by this value if a compatible seed has been "
      "found"};
  Gaudi::Property<std::size_t> m_compatSeedLimit{
      this, "compatSeedLimit", 3,
      "how often do you want to increase the weight of a seed for finding a "
      "compatible seed"};
  Gaudi::Property<float> m_seedWeightIncrement{
      this, "seedWeightIncrement", 0., "increment in seed weight if needed"};
  Gaudi::Property<float> m_numSeedIncrement{
      this, "numSeedIncrement", 10e6,
      "increment in seed weight is applied if the number of compatible seeds "
      "is larger than numSeedIncrement"};
  Gaudi::Property<bool> m_seedConfirmationInFilter{
      this, "seedConfirmationInFilter", true, "run seed confirmation"};
  Gaudi::Property<std::size_t> m_maxSeedsPerSpMConf{
      this, "maxSeedsPerSpMConf", 5,
      "Maximum number of lower quality seeds in seed confirmation."};
  Gaudi::Property<std::size_t> m_maxQualitySeedsPerSpMConf{
      this, "maxQualitySeedsPerSpMConf", 5,
      "Maximum number of quality seeds for each middle-bottom SP-duplet in "
      "seed confirmation."};
  Gaudi::Property<bool> m_useDeltaRorTopRadius{
      this, "useDeltaRorTopRadius", true,
      "use deltaR (top radius - middle radius) instead of top radius"};
  Gaudi::Property<float> m_deltaInvHelixDiameter{
      this, "deltaInvHelixDiameter", 0.00003 * 1. / Acts::UnitConstants::mm,
      "the allowed delta between two inverted seed radii for them to be "
      "considered compatible"};

  // Properties to set other objects used in seeding algorithm
  Gaudi::Property<std::vector<std::pair<int, int>>> m_zBinNeighborsTop{
      this,
      "zBinNeighborsTop",
      {{0, 0},
       {-1, 0},
       {-2, 0},
       {-1, 0},
       {-1, 0},
       {-1, 0},
       {-1, 1},
       {0, 1},
       {0, 1},
       {0, 1},
       {0, 2},
       {0, 1},
       {0, 0}},
      "vector containing the map of z bins in the top layers"};
  Gaudi::Property<std::vector<std::pair<int, int>>> m_zBinNeighborsBottom{
      this,
      "zBinNeighborsBottom",
      {{0, 0},
       {0, 1},
       {0, 1},
       {0, 1},
       {0, 1},
       {0, 1},
       {0, 0},
       {-1, 0},
       {-1, 0},
       {-1, 0},
       {-1, 0},
       {-1, 0},
       {0, 0}},
      "vector containing the map of z bins in the top layers"};
  Gaudi::Property<std::vector<std::pair<int, int>>> m_rBinNeighborsTop{
      this,
      "rBinNeighborsTop",
      {{0, 0}},
      "vector containing the map of radius bins in the top layers"};
  Gaudi::Property<std::vector<std::pair<int, int>>> m_rBinNeighborsBottom{
      this,
      "rBinNeighborsBottom",
      {{0, 0}},
      "vector containing the map of radius bins in the bottom layers"};
  Gaudi::Property<int> m_numPhiNeighbors{
      this, "numPhiNeighbors", 1,
      "number of phi bin neighbors at each side of the current bin that will "
      "be used to search for SPs"};

  Gaudi::Property<bool> m_useExperimentCuts{this, "useExperimentCuts", false,
                                            ""};

  Gaudi::Property<int> m_stateVectorReserveSize{
      this, "stateVectorReserveSize", 500,
      "Size of the initial Seeding State internal vectors"};

  Gaudi::Property<float> m_expCutrMin{this, "SpSelectionExpCutrMin",
                                      45. * Acts::UnitConstants::mm};

 private:
  Acts::Experimental::CylindricalSpacePointGrid2::Config m_gridCfg;
  Acts::Experimental::DoubletSeedFinder::Config m_bottomDoubletFinderCfg;
  Acts::Experimental::DoubletSeedFinder::Config m_topDoubletFinderCfg;
  Acts::Experimental::BroadTripletSeedFinder::TripletCuts m_tripletCuts;
  Acts::Experimental::BroadTripletSeedFilter::Config m_filterCfg;
  Acts::Experimental::BroadTripletSeedFinder::Options m_finderOpts;
  Acts::SeedConfirmationRangeConfig m_centralSeedConfirmationRange;
  Acts::SeedConfirmationRangeConfig m_forwardSeedConfirmationRange;

  std::optional<Acts::Experimental::BroadTripletSeedFinder> m_finder;
  std::optional<Acts::Experimental::BroadTripletSeedFilter> m_filter;

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  const PixelID* m_pixelId{nullptr};

  /// Private access to the logger
  const Acts::Logger& logger() const { return *m_logger; }

  bool spacePointSelectionFunction(
      const Acts::Experimental::ConstSpacePointProxy2& sp) const;

  bool doubletSelectionFunction(
      const Acts::Experimental::ConstSpacePointProxy2& middle,
      const Acts::Experimental::ConstSpacePointProxy2& other, float cotTheta,
      bool isBottomCandidate) const;

  std::pair<float, float> retrieveRadiusRangeForMiddle(
      const Acts::Experimental::ConstSpacePointProxy2& spM,
      const Acts::Range1D<float>& rMiddleSpRange) const;
};

}  // namespace ActsTrk

#endif
