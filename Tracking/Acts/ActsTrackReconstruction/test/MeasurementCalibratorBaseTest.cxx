/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "src/detail/MeasurementCalibratorBase.h"
#include "src/detail/Definitions.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"

template < typename trajectory_t,
	   typename pos_t,
	   typename cov_t >
void checkList(const typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy &trackState,
	       const std::size_t expectedCalibratedSize,
	       const pos_t& expectedLocPos,
	       const cov_t& expectedLocCov,
	       const Acts::BoundSubspaceIndices& expectedBoundSpaceIndices)
{
  assert( trackState.hasCalibrated() );
  std::cout << "- checking calibrated size: " << trackState.calibratedSize() << std::endl;
  assert( trackState.calibratedSize() == expectedCalibratedSize );
  std::cout << "- checking loc pos" << std::endl;
  assert( trackState.effectiveCalibrated().size() == expectedLocPos.size() );
  assert( trackState.effectiveCalibrated() == expectedLocPos.template cast<double>() );
  std::cout << "- checking loc cov" << std::endl;
  assert( trackState.effectiveCalibratedCovariance().size() == expectedLocCov.size() );
  assert( trackState.effectiveCalibratedCovariance() == expectedLocCov.template cast<double>() );
  std::cout << "- checking BoundSubspaceIndices" << std::endl;
  assert( trackState.boundSubspaceIndices() == expectedBoundSpaceIndices );
}


int main() {
  xAOD::PixelClusterContainer pixelClusters;
  xAOD::PixelClusterAuxContainer auxPixelClusters;
  pixelClusters.setStore( &auxPixelClusters );

  xAOD::StripClusterContainer stripClusters;
  xAOD::StripClusterAuxContainer auxStripClusters;
  stripClusters.setStore( &auxStripClusters );

  xAOD::HGTDClusterContainer hgtdClusters;
  xAOD::HGTDClusterAuxContainer auxHgtdClusters;
  hgtdClusters.setStore( &auxHgtdClusters );


  pixelClusters.push_back( new xAOD::PixelCluster() );
  stripClusters.push_back( new xAOD::StripCluster() );
  stripClusters.push_back( new xAOD::StripCluster() );
  hgtdClusters.push_back( new xAOD::HGTDCluster() );

  std::cout << "Creating State Container ... " << std::endl;
  Acts::VectorMultiTrajectory trackStateBackend;
  assert( trackStateBackend.size() == 0ul );

  std::size_t nStates = 4ul;
  for (std::size_t i(0); i<nStates; ++i) {
    trackStateBackend.addTrackState(Acts::TrackStatePropMask::All);
  }

  std::cout << "- number of states: " << trackStateBackend.size() << std::endl;
  assert( trackStateBackend.size() == nStates );

  ActsTrk::detail::MeasurementCalibratorBase calibrator;
  std::cout << "Calling setStateFromMeasurement for pixel cluster" << std::endl;
  // For pixels, the Surface Bound is pointless
  typename Acts::MultiTrajectory<Acts::VectorMultiTrajectory>::TrackStateProxy pixelState = trackStateBackend.getTrackState(0);
  assert(not pixelState.hasCalibrated());
  calibrator.template setStateFromMeasurement<xAOD::PixelCluster, Acts::VectorMultiTrajectory>( *pixelClusters.front(),
												Acts::SurfaceBounds::eCone,
												pixelState );

  checkList<Acts::VectorMultiTrajectory>(pixelState,
					 2,
					 pixelClusters.front()->template localPosition<2>(),
					 pixelClusters.front()->template localCovariance<2>().template topLeftCorner<2, 2>(),
					 Acts::BoundSubspaceIndices{ Acts::eBoundLoc0, Acts::eBoundLoc1 } );

  std::cout << "Calling setStateFromMeasurement for strip cluster" << std::endl;
  typename Acts::MultiTrajectory<Acts::VectorMultiTrajectory>::TrackStateProxy stripState = trackStateBackend.getTrackState(1);
  assert(not stripState.hasCalibrated());
  calibrator.template setStateFromMeasurement<xAOD::StripCluster, Acts::VectorMultiTrajectory>( *stripClusters.front(),
                                                                                                Acts::SurfaceBounds::eCone,
                                                                                                stripState );

  checkList<Acts::VectorMultiTrajectory>(stripState,
                                         1,
                                         stripClusters.front()->template localPosition<1>(),
                                         stripClusters.front()->template localCovariance<1>().template topLeftCorner<1, 1>(),
                                         Acts::BoundSubspaceIndices{ Acts::eBoundLoc0 } );

  std::cout << "Calling setStateFromMeasurement for strip cluster (Acts::SurfaceBounds::eAnnulus)" << std::endl;
  typename Acts::MultiTrajectory<Acts::VectorMultiTrajectory>::TrackStateProxy stripAnnulusState = trackStateBackend.getTrackState(2);
  assert(not stripAnnulusState.hasCalibrated());
  calibrator.template setStateFromMeasurement<xAOD::StripCluster, Acts::VectorMultiTrajectory>( *stripClusters.at(1),
                                                                                                Acts::SurfaceBounds::eAnnulus,
                                                                                                stripAnnulusState );

  checkList<Acts::VectorMultiTrajectory>(stripAnnulusState,
                                         1,
                                         stripClusters.at(1)->template localPosition<1>(),
                                         stripClusters.at(1)->template localCovariance<1>().template topLeftCorner<1, 1>(),
                                         Acts::BoundSubspaceIndices{ Acts::eBoundLoc1 } );

  std::cout << "Calling setStateFromMeasurement for HGTD cluster" << std::endl;

  typename Acts::MultiTrajectory<Acts::VectorMultiTrajectory>::TrackStateProxy hgtdState = trackStateBackend.getTrackState(3);
  assert(not hgtdState.hasCalibrated());
  calibrator.template setStateFromMeasurement<xAOD::HGTDCluster, Acts::VectorMultiTrajectory>( *hgtdClusters.front(),
                                                                                                Acts::SurfaceBounds::eCone,
                                                                                                hgtdState );

  checkList<Acts::VectorMultiTrajectory>(hgtdState,
                                         3,
                                         hgtdClusters.front()->template localPosition<3>(),
                                         hgtdClusters.front()->template localCovariance<3>().template topLeftCorner<3, 3>(),
                                         Acts::BoundSubspaceIndices{ Acts::eBoundLoc0, Acts::eBoundLoc1, Acts::eBoundTime } );
}
