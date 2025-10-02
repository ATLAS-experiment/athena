/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "ActsCalibBase/MeasurementCalibratorBase.h"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
/** @brief Dummy measurement helper class */
template <size_t dim>

  struct DummyMeasurement{

    DummyMeasurement(unsigned int id, Acts::ActsVector<dim> pos,
                     Acts::ActsSquareMatrix<dim> cov):
        m_id{id}, m_pos{pos}, m_cov{cov}{}
    
    constexpr std::size_t size() const { return dim; }
    unsigned int m_id{0};
    Acts::ActsVector<dim> m_pos{Acts::ActsVector<dim>::Zero()};
    Acts::ActsSquareMatrix<dim> m_cov{Acts::ActsSquareMatrix<dim>::Identity()};

};



template <std::size_t dim,  typename trajectory_t>
void checkList(const DummyMeasurement<dim>& refMeas,
               const typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy &trackState,
               const Acts::BoundSubspaceIndices& expectedBoundSpaceIndices) {
 
  auto sourceLink = trackState.getUncalibratedSourceLink().template get<DummyMeasurement<dim>*>();
  std::cout<< "- checking the source link. "<<std::endl;
  assert( sourceLink );
  assert( &refMeas == sourceLink);

  assert( trackState.hasCalibrated() );
  std::cout << "- checking calibrated size: " << trackState.calibratedSize() << std::endl;
  assert( trackState.calibratedSize() == refMeas.size() );
  std::cout << "- checking loc pos " << Amg::toString(refMeas.m_pos) << std::endl;
  assert( trackState.effectiveCalibrated().size() == refMeas.m_pos.size() );
  assert( trackState.effectiveCalibrated() == refMeas.m_pos.template cast<double>() );
  std::cout << "- checking loc cov "  << Amg::toString(refMeas.m_cov)<< std::endl;
  assert( trackState.effectiveCalibratedCovariance().size() == refMeas.m_cov.size() );
  assert( trackState.effectiveCalibratedCovariance() == refMeas.m_cov.template cast<double>() );
  std::cout << "- checking BoundSubspaceIndices" << std::endl;
  assert( trackState.projectorSubspaceIndices() == expectedBoundSpaceIndices );
}


int main() {
  std::cout << "Creating State Container ... " << std::endl;
  Acts::VectorMultiTrajectory trackStateBackend;
  assert( trackStateBackend.size() == 0ul );

  constexpr std::size_t nStates{6ul};
  for (std::size_t i{0}; i<nStates; ++i) {
    trackStateBackend.addTrackState(Acts::TrackStatePropMask::All);
  }

  std::cout << "- number of states: " << trackStateBackend.size() << std::endl;
  assert( trackStateBackend.size() == nStates );

  ActsTrk::detail::MeasurementCalibratorBase calibrator;
  using Backend_t = Acts::VectorMultiTrajectory;
  using ProjectorType = ActsTrk::detail::MeasurementCalibratorBase::ProjectorType;
  using TrackState_t = Acts::MultiTrajectory<Acts::VectorMultiTrajectory>::TrackStateProxy;
  /// Check 1 dimensional track state  
  {
    TrackState_t simpleStripState = trackStateBackend.getTrackState(0);
    assert(not simpleStripState.hasCalibrated());

    DummyMeasurement<1> meas{1, Acts::ActsVector<1>{10.}, 20.*Acts::ActsSquareMatrix<1>::Identity()};
    calibrator.setState<1, Backend_t>(ProjectorType::e1DimNoTime, meas.m_pos, meas.m_cov, 
                                      Acts::SourceLink{&meas}, simpleStripState);
    checkList<1, Backend_t>(meas, simpleStripState, Acts::BoundSubspaceIndices{Acts::eBoundLoc0});
  }
  /// Check 1 dimensional track state - rotated version
  {
    TrackState_t rotated = trackStateBackend.getTrackState(1);
    assert(not rotated.hasCalibrated());

    DummyMeasurement<1> meas{1, Acts::ActsVector<1>{35.}, 66.*Acts::ActsSquareMatrix<1>::Identity()};
    calibrator.setState<1, Backend_t>(ProjectorType::e1DimRotNoTime, meas.m_pos, meas.m_cov, 
                                    Acts::SourceLink{&meas}, rotated);
    checkList<1, Backend_t>(meas, rotated, Acts::BoundSubspaceIndices{Acts::eBoundLoc1});
  }
  /// Check 1 dimensional track state - time version
  {
    TrackState_t oneDimTime = trackStateBackend.getTrackState(2);
    assert(not oneDimTime.hasCalibrated());

    AmgSymMatrix(2) cov{AmgSymMatrix(2)::Identity()};
    cov(0,0) = 63;
    cov(1,1) = 683;
    DummyMeasurement<2> meas{3, Acts::ActsVector<2>{35., 74.}, cov};
    calibrator.setState<2, Backend_t>(ProjectorType::e1DimWithTime, meas.m_pos, meas.m_cov, 
                                      Acts::SourceLink{&meas}, oneDimTime);
    checkList<2, Backend_t>(meas, oneDimTime, Acts::BoundSubspaceIndices{Acts::eBoundLoc0, Acts::eBoundTime});

  }
  /// Check 2 dimensional track state  - no time version
  {
    TrackState_t twoDim = trackStateBackend.getTrackState(3);
    assert( not twoDim.hasCalibrated());
    DummyMeasurement<2> meas{4, Amg::Vector2D{52,-72}, AmgSymMatrix(2){Eigen::Rotation2D{0.2*M_PI}}};
    calibrator.setState<2, Backend_t>(ProjectorType::e2DimNoTime, meas.m_pos, meas.m_cov, 
                                      Acts::SourceLink{&meas}, twoDim);
    checkList<2, Backend_t>(meas, twoDim, Acts::BoundSubspaceIndices{Acts::eBoundLoc0, Acts::eBoundLoc1});
  }
  /// Check 2 dimensional track state  - with time version
  {
    TrackState_t twoDimWithT = trackStateBackend.getTrackState(4);
    assert( not twoDimWithT.hasCalibrated());
  
    DummyMeasurement<3> meas{4, Amg::Vector3D{52,-72, 82}, Amg::getRotateZ3D(0.2*M_PI).linear()};
    calibrator.setState<3, Backend_t>(ProjectorType::e2DimWithTime, meas.m_pos, meas.m_cov, 
                                      Acts::SourceLink{&meas}, twoDimWithT);
    checkList<3, Backend_t>(meas, twoDimWithT, Acts::BoundSubspaceIndices{Acts::eBoundLoc0, Acts::eBoundLoc1, Acts::eBoundTime});
  }
  /// Check 1 dimensional complementary track state  - with time version
  {
    TrackState_t rotOneDimWithT = trackStateBackend.getTrackState(5);
    assert( not rotOneDimWithT.hasCalibrated());
    AmgSymMatrix(2) cov{AmgSymMatrix(2)::Identity()};
    cov(0,0) = 26;
    cov(1,1) = -27;
    cov(1,0) = 270;
    DummyMeasurement<2> meas{4, Amg::Vector2D{25, 0.92}, cov};
    calibrator.setState<2, Backend_t>(ProjectorType::e1DimRotWithTime, meas.m_pos, meas.m_cov, 
                                      Acts::SourceLink{&meas}, rotOneDimWithT);
    checkList<2, Backend_t>(meas, rotOneDimWithT, Acts::BoundSubspaceIndices{Acts::eBoundLoc1, Acts::eBoundTime});
  }

  return EXIT_SUCCESS;
}
