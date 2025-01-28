/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include <array>
#include <string>

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/SharedHitCounter.h"

#include "Acts/Geometry/GeometryIdentifier.hpp"

#include "../src/detail/TrackFindingMeasurements.cxx"

template <typename std::size_t N>
void checkList(const ActsTrk::detail::TrackFindingMeasurements& measurements,
               const ActsTrk::detail::SharedHitCounter& sharedHits,
	       const std::array<std::size_t, N>& entries,
	       bool isFilled) {
  const std::vector<std::size_t>& offsets = sharedHits.measurementOffsets();
  std::size_t cumulativeOffset = 0ul;
  for (std::size_t i(0); i<N; ++i) {
    std::cout << "Checking measurementOffset " << i << " : " << offsets.at(i) << " with expected " << cumulativeOffset << std::endl;
    assert( offsets.at(i) == cumulativeOffset );
    cumulativeOffset += entries.at(i);
  }
  
  std::cout << "Checking offsets size: " << offsets.size() << " with expected " <<	N << std::endl;
  assert( offsets.size() == N );

  cumulativeOffset = 0ul;
  for (std::size_t i(0); i<N; ++i) {
    std::cout << "Checking offsets " << i << " : " << offsets.at(i) << " with expected " << cumulativeOffset << std::endl;
    assert( offsets.at(i) == cumulativeOffset );
    cumulativeOffset += entries.at(i);
  }
  
  const ActsTrk::detail::MeasurementRangeList& rangeList = measurements.measurementRanges();
  std::cout << "Checking rangeListPreFill size: " << rangeList.size() << " with expected " << (isFilled ? N : 0ul) << std::endl;
  std::cout << "Checking numContainers: " << rangeList.numContainers() << " with expected " << (isFilled ? N : 0ul) << std::endl;
  assert( rangeList.size() == (isFilled ? N : 0ul) );
  assert( rangeList.numContainers() == (isFilled ? N : 0ul) );

  for (auto [typeIndex, measurementRange] : rangeList) {
    auto [minVal, maxVal] = measurementRange;
    std::cout << "Checking typeIndex: " << typeIndex << " : [" << minVal << ", " << maxVal << "]" << std::endl;
    std::cout << "Checking n. elements for typeIndex " << typeIndex << " : " << (maxVal - minVal) << " with expected " << entries.at(typeIndex - 1) << std::endl;
    assert( (maxVal - minVal) == entries.at(typeIndex - 1) );
  }
}

int main() {
  constexpr std::size_t N = 3ul;

  // create containers
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "Creating cluster containers ..." << std::endl;
  xAOD::PixelClusterContainer pixelContainer;
  xAOD::PixelClusterAuxContainer auxPixelContainer;
  pixelContainer.setStore( &auxPixelContainer );

  xAOD::StripClusterContainer stripContainer;
  xAOD::StripClusterAuxContainer auxStripContainer;
  stripContainer.setStore( &auxStripContainer );

  xAOD::HGTDClusterContainer hgtdContainer;
  xAOD::HGTDClusterAuxContainer hgtdAuxContainer;
  hgtdContainer.setStore( &hgtdAuxContainer );

  // fill containers
  std::cout << "Filling cluster containers ..." << std::endl;
  std::size_t nPixelClusters = 2000ul;
  std::size_t nStripClusters = 700ul;
  std::size_t nHgtdClusters = 900ul;
  
  for (std::size_t i(0); i<nPixelClusters; ++i) {
    pixelContainer.push_back( new xAOD::PixelCluster() );
  }

  for (std::size_t i(0); i<nStripClusters; ++i) {
    stripContainer.push_back( new xAOD::StripCluster() );
  }

  for (std::size_t i(0); i<nHgtdClusters; ++i) {
    hgtdContainer.push_back( new xAOD::HGTDCluster() );
  }

  std::cout << "Checking pixel container size: " << pixelContainer.size() << " with the expected " << nPixelClusters << std::endl;
  std::cout << "Checking strip container size: " << stripContainer.size() << " with the expected " << nStripClusters << std::endl;
  std::cout << "Checking hgtd container size: " << hgtdContainer.size() << " with the expected " << nHgtdClusters << std::endl;
  assert(pixelContainer.size() == nPixelClusters);
  assert(stripContainer.size() == nStripClusters);
  assert(hgtdContainer.size() == nHgtdClusters);

  // checks pre-fill
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "Checking tests pre-fill ..." << std::endl;
  ActsTrk::detail::TrackFindingMeasurements measurements;
  ActsTrk::detail::SharedHitCounter sharedHits(N);
  std::cout << "Checking nMeasurements: " << measurements.nMeasurements() << " with the expected " << 0ul << std::endl;
  assert( measurements.nMeasurements() == 0ul );

  std::array<std::size_t, N> entries {};
  checkList<N>(measurements, sharedHits, entries, false);

  // fill
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "Filling the measurements ..." << std::endl;
  ActsTrk::DetectorElementToActsGeometryIdMap geometryIdMap;
  Acts::GeometryIdentifier pixelGeoId {1};
  Acts::GeometryIdentifier stripGeoId {2};
  Acts::GeometryIdentifier hgtdGeoId {3};
  
  geometryIdMap.insert( std::make_pair( ActsTrk::makeDetectorElementKey( xAOD::UncalibMeasType::PixelClusterType, pixelContainer.front()->identifierHash() ),
					ActsTrk::DetectorElementToActsGeometryIdMap::makeValue( pixelGeoId ) ) );
  geometryIdMap.insert( std::make_pair( ActsTrk::makeDetectorElementKey( xAOD::UncalibMeasType::StripClusterType, stripContainer.front()->identifierHash() ),
					ActsTrk::DetectorElementToActsGeometryIdMap::makeValue( stripGeoId ) ) );
  geometryIdMap.insert( std::make_pair( ActsTrk::makeDetectorElementKey( xAOD::UncalibMeasType::HGTDClusterType, hgtdContainer.front()->identifierHash() ),
					ActsTrk::DetectorElementToActsGeometryIdMap::makeValue( hgtdGeoId ) ) );
  
  measurements.addMeasurements(0, pixelContainer, geometryIdMap);
  measurements.addMeasurements(1, stripContainer, geometryIdMap);
  measurements.addMeasurements(2, hgtdContainer, geometryIdMap);

  sharedHits.addMeasurements(0, pixelContainer);
  sharedHits.addMeasurements(1, stripContainer);
  sharedHits.addMeasurements(2, hgtdContainer);

  // check post-fill
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "Checking tests post-fill ..." << std::endl;
  std::cout << "Checking nMeasurements: " << measurements.nMeasurements() << " with the expected " << nPixelClusters + nStripClusters + nHgtdClusters << std::endl;
  entries = { nPixelClusters, nStripClusters, nHgtdClusters };
  checkList<N>(measurements, sharedHits, entries, true);

  //  collections
  std::size_t index = sharedHits.measurementIndex(*pixelContainer[0]);
  std::cout << "Checking index value : " << index << " with expected " << 0ul << std::endl;
  assert( index == 0ul );

  std::size_t index2 = sharedHits.measurementIndex(*stripContainer[0]);
  std::cout << "Checking index value : " << index2 << " with expected " << pixelContainer.size() << std::endl;
  assert( index2 == pixelContainer.size() );

  std::size_t index3 = sharedHits.measurementIndex(*hgtdContainer[0]);
  std::cout << "Checking index value : " << index3 << " with expected " << pixelContainer.size() + stripContainer.size() << std::endl;
  assert( index3 == pixelContainer.size() + stripContainer.size() );
}
