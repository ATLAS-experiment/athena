/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "ActsEvent/Seed.h"
#include "src/detail/MeasurementIndex.h"
#include "src/detail/DuplicateSeedDetector.h"
#include <vector>
#include <ranges>

#include "../src/detail/DuplicateSeedDetector.cxx"

namespace ActsTrk::detail {
  // DuplicateSeedDetectorTest is a friend of DuplicateSeedDetector, so has access to its internals.
  struct DuplicateSeedDetectorTest {

    static ActsTrk::SeedContainer createSeeds(const xAOD::SpacePointContainer& spacePoints) {
      ActsTrk::SeedContainer seedContainer;
      for (std::size_t i(0ul); i+2ul<spacePoints.size(); i+=3ul) {
        seedContainer.push_back( new ActsTrk::Seed(*spacePoints.at(i),
                                                   *spacePoints.at(i+1),
                                                   *spacePoints.at(i+2)) );
      }
      return seedContainer;
    }

    template <std::ranges::range external_collection_t,
              typename external_t>
    requires std::same_as<typename external_collection_t::value_type, external_t>
    static void checkCollection(const external_collection_t& collection,
                                std::size_t expectedSize,
                                external_t expectedValue) {
      std::cout << "Checking collection with expected size of " << expectedSize << " and with all elements expected to be " << expectedValue << std::endl;
      assert( collection.size() == expectedSize );
      for (external_t value : collection) {
        assert( value == expectedValue );
      }
    }

    static void test() {
      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Creating cluster containers ..." << std::endl;
      std::size_t nPixelClusters = 900;
      std::size_t nStripClusters = 600;

      xAOD::PixelClusterContainer pixelClusters;
      xAOD::PixelClusterAuxContainer pixelAuxClusters;
      pixelClusters.setStore( &pixelAuxClusters );

      xAOD::StripClusterContainer stripClusters;
      xAOD::StripClusterAuxContainer stripAuxClusters;
      stripClusters.setStore( &stripAuxClusters );

      std::cout << "- nPixelClusters: " << nPixelClusters << std::endl;
      for (std::size_t i(0); i<nPixelClusters; ++i) {
        pixelClusters.push_back( new xAOD::PixelCluster() );
      }
      assert( pixelClusters.size() == nPixelClusters );
        
      std::cout << "- nStripClusters: " << nStripClusters << std::endl;
      for (std::size_t i(0); i<nStripClusters; ++i) {
        stripClusters.push_back( new xAOD::StripCluster() );
      }
      assert( stripClusters.size() == nStripClusters );

      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Creating space point containers ..." << std::endl;
      std::size_t nPixelSpacePoints = nPixelClusters;
      std::size_t nStripSpacePoints = nStripClusters / 2ul;

      xAOD::SpacePointContainer pixelSpacePoints;
      xAOD::SpacePointAuxContainer pixelAuxSpacePoints;
      pixelSpacePoints.setStore( &pixelAuxSpacePoints );

      xAOD::SpacePointContainer stripSpacePoints;
      xAOD::SpacePointAuxContainer stripAuxSpacePoints;
      stripSpacePoints.setStore( &stripAuxSpacePoints );
      
      std::cout << "- nPixelSpacePoints: " << nPixelSpacePoints << std::endl;
      for (std::size_t i(0); i<nPixelClusters; ++i) {
        pixelSpacePoints.push_back( new xAOD::SpacePoint() );
        pixelSpacePoints.back()->setMeasurements( {pixelClusters.at(i)} );
      }
      std::cout << "     * created " << pixelSpacePoints.size() << std::endl;  
      assert( pixelSpacePoints.size() == nPixelSpacePoints );

      std::cout << "- nStripSpacePoints: " << nStripSpacePoints << std::endl;
      for (std::size_t i(0); i<nStripClusters; i+=2ul) {
        stripSpacePoints.push_back( new xAOD::SpacePoint() );
        stripSpacePoints.back()->setMeasurements( {stripClusters.at(i), stripClusters.at(i+1)} );
      }
      std::cout << "     * created " << stripSpacePoints.size() << std::endl;  
      assert( stripSpacePoints.size() == nStripSpacePoints );

      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Creating seed containers ..." << std::endl;
      ActsTrk::SeedContainer pixelSeeds = createSeeds( pixelSpacePoints );
      std::cout << "- Created " << pixelSeeds.size() << " pixel seeds" << std::endl;
      ActsTrk::SeedContainer stripSeeds = createSeeds( stripSpacePoints );
      std::cout << "- Created " << stripSeeds.size() << " strip seeds" << std::endl;

      assert( pixelSeeds.size() == nPixelSpacePoints/3ul );
      assert( stripSeeds.size() == nStripSpacePoints/3ul );

      MeasurementIndex measurementIndex(2ul);
      measurementIndex.addMeasurements(pixelClusters);
      measurementIndex.addMeasurements(stripClusters);

      std::cout << "----------------------------------------------" << std::endl;
      std::size_t nTotalSeeds = pixelSeeds.size() + stripSeeds.size();
      std::cout << "Starting checks on DuplicateSeedDetector running on " << nTotalSeeds << " seeds" << std::endl;
      DuplicateSeedDetector duplicateSeedDetector(nTotalSeeds, 0, true);

      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Checks pre-fill ..." << std::endl;
      std::cout << "Checking the DuplicateSeedDetector is on/off : " << !duplicateSeedDetector.m_disabled << std::endl;
      assert( !duplicateSeedDetector.m_disabled );

      std::cout << "Checking values before filling" << std::endl;
      assert( duplicateSeedDetector.m_numSeeds == 0u );
      assert( duplicateSeedDetector.m_nextSeed == 0u );
      assert( duplicateSeedDetector.m_foundSeeds == 0ul );
      std::cout << "Checking seedOffset" << std::endl;
      assert( duplicateSeedDetector.m_seedOffset.empty() );
      std::cout << "Chacking UsedMeasurements" << std::endl;
      checkCollection( duplicateSeedDetector.m_nUsedMeasurements, nTotalSeeds, 0u );
      std::cout << "Checking nSeedMeasurements" << std::endl;
      checkCollection( duplicateSeedDetector.m_nSeedMeasurements, nTotalSeeds, 0u );
      std::cout << "Checking isDuplicateSeed" << std::endl;
      checkCollection( duplicateSeedDetector.m_isDuplicateSeed, nTotalSeeds, false );

      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Filling ..." << std::endl;
      std::cout << "- Adding Pixel Seed Container" << std::endl;
      duplicateSeedDetector.addSeeds( 0ul, pixelSeeds, measurementIndex );
      std::cout << "- Adding Strip Seed Container" << std::endl;
      duplicateSeedDetector.addSeeds( 1ul, stripSeeds, measurementIndex );

      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Checks post-fill ..." << std::endl;
      std::cout << "Checking values after filling" << std::endl;
      assert( duplicateSeedDetector.m_numSeeds == nTotalSeeds );
      assert( duplicateSeedDetector.m_nextSeed == 0u );
      assert( duplicateSeedDetector.m_foundSeeds == 0ul );
      std::cout << "Checking seedOffset" << std::endl;
      assert( duplicateSeedDetector.m_seedOffset.size() == 2ul );
      assert( duplicateSeedDetector.m_seedOffset[0] == 0ul );
      assert( duplicateSeedDetector.m_seedOffset[1] == pixelSeeds.size() );
      std::cout << "Chacking UsedMeasurements" << std::endl;
      checkCollection( duplicateSeedDetector.m_nUsedMeasurements, nTotalSeeds, 0u );
      std::cout << "Checking nSeedMeasurements" << std::endl;
      const std::vector<unsigned int>& nSeedMeasurementsPostFill = duplicateSeedDetector.m_nSeedMeasurements;
      for (std::vector<unsigned int>::const_iterator it(nSeedMeasurementsPostFill.begin()),
            itEnd(nSeedMeasurementsPostFill.begin() + pixelSeeds.size());
          it != itEnd; ++it) {
        assert( *it == 3ul );
      }
      for (std::vector<unsigned int>::const_iterator it(nSeedMeasurementsPostFill.begin() + pixelSeeds.size());
          it != nSeedMeasurementsPostFill.end(); ++it) {
        assert( *it == 6 );
      }
      std::cout << "Checking isDuplicateSeed" << std::endl;
      checkCollection( duplicateSeedDetector.m_isDuplicateSeed, nTotalSeeds, false );

      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Start with adding measurements" << std::endl;
      std::cout << "Adding existing measurement ... " << std::endl;
      duplicateSeedDetector.addMeasurement( pixelClusters.front(), measurementIndex );
      std::cout << "- found seeds is now " << duplicateSeedDetector.m_foundSeeds << std::endl;
      assert( duplicateSeedDetector.m_foundSeeds == 1ul );
      std::cout << "Adding non existing measurement ..." << std::endl;
      xAOD::PixelCluster nonExistentCluster;
      duplicateSeedDetector.addMeasurement( &nonExistentCluster, measurementIndex );
      std::cout << "- found seeds is now " << duplicateSeedDetector.m_foundSeeds << std::endl;
      assert( duplicateSeedDetector.m_foundSeeds == 1ul );

      duplicateSeedDetector.newTrajectory();
      
      std::cout << "----------------------------------------------" << std::endl;
      std::cout << "Checking isDuplicate()" << std::endl;
      std::cout << "- Pixel seeds" << std::endl;
      for (std::size_t i(0); i<pixelSeeds.size(); ++i) {
        assert( not duplicateSeedDetector.isDuplicate(0ul, i) );
      }
      std::cout << "Checking nextSeed" << std::endl;
      assert( duplicateSeedDetector.m_nextSeed == pixelSeeds.size() );
      std::cout << "- Strip seeds" << std::endl;
      for (std::size_t i(0); i<stripSeeds.size(); ++i) {
        assert( not duplicateSeedDetector.isDuplicate(1ul, i) );
      }
      std::cout << "Checking nextSeed" << std::endl;
      assert( duplicateSeedDetector.m_nextSeed == pixelSeeds.size() + stripSeeds.size() );
    }

  };
}

int main() {
  ActsTrk::detail::DuplicateSeedDetectorTest::test();
}
