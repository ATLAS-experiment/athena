/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include "TestTools/initGaudi.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ActsCalibration/TrkMeasurementCalibrator.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"

#include <string>

namespace ActsTrk::testing {

  class MyDummyAlg : public AthReentrantAlgorithm {
  public:
      using AthReentrantAlgorithm::AthReentrantAlgorithm;
      virtual ~MyDummyAlg() override = default;
      virtual StatusCode execute(const EventContext& ) const override {
          return StatusCode::SUCCESS;
      }

  };

  class MyDummyMeasurement : public Trk::MeasurementBase {
  public:
    MyDummyMeasurement(Trk::LocalParameters locPar,
		       Amg::MatrixX locCov)
      : Trk::MeasurementBase(std::move(locPar),
			     std::move(locCov))
    {}
    const Trk::Surface& associatedSurface() const override { return *m_trkSurface; }

  private:
    // virtual methods
    virtual MeasurementBase* clone() const override
    { return nullptr; }

    virtual const Amg::Vector3D& globalPosition() const override
    { return m_globPos; }
    
    virtual bool type(Trk::MeasurementBaseType::Type) const override
    { return true; }

    virtual MsgStream& dump(MsgStream& out) const override
    { return out; }
    
    virtual std::ostream& dump(std::ostream& out) const override
    { return out; }
    
  private:
    std::shared_ptr<Trk::PerigeeSurface> m_trkSurface = std::make_shared<Trk::PerigeeSurface>();
    Amg::Vector3D m_globPos{Amg::Vector3D::Zero()};
  };
  
} // namespace ActsTrk::testing

int main() {

  ISvcLocator* pSvcLoc{nullptr};
  if (not Athena_test::initGaudi(pSvcLoc)) {
    std::cerr << " This test cannot be run without init Gaudi" << std::endl;
    return 1;
  }
  assert(pSvcLoc);  

  std::cout << "Creating dummy alg and tool for the test" << std::endl;
  ActsTrk::testing::MyDummyAlg alg("myDummyAlg", pSvcLoc);
  alg.addRef();

  std::cout << "Starting now with the testing phase" << std::endl;
  std::cout << "Creating dummy Measurement on track object" << std::endl;
  Amg::MatrixX zero;
  zero.resize(2, 2);
  zero.setZero();

  ActsTrk::testing::MyDummyMeasurement dummyMeasurementOnTrack(Trk::LocalParameters{Amg::Vector2D{}},
							       std::move(zero));
  ActsTrk::ATLASSourceLink dummySl {&dummyMeasurementOnTrack};

  std::cout << "- checking dimension of local parameter is 2" << std::endl;
  // dim has to be two so that this is recognized as pixel measurement
  auto dim = dummyMeasurementOnTrack.localParameters().dimension();
  assert( dim == 2 );
  
  std::cout << "Creating State Container ... " << std::endl;
  Acts::VectorMultiTrajectory trackStateBackend;
  assert( trackStateBackend.size() == 0ul );
  
  std::size_t nStates = 2ul;
  for (std::size_t i(0); i<nStates; ++i) {
    trackStateBackend.addTrackState(Acts::TrackStatePropMask::All);
    assert(not trackStateBackend.getTrackState(i).hasCalibrated());
  }

  std::cout << "- number of states: " << trackStateBackend.size() << std::endl;
  assert( trackStateBackend.size() == nStates );

  std::cout << "Creating dummy geometry context and calibration context" << std::endl;
  Acts::GeometryContext gctx;
  Acts::CalibrationContext cctx;

  std::cout << "Creating TrkMeasurementCalibrator" << std::endl;
  ActsTrk::detail::TrkMeasurementCalibrator measCalibrator{};

  std::cout << "Start calling calibrate" << std::endl;
  measCalibrator.template calibrate<Acts::VectorMultiTrajectory>( gctx, cctx,
								  Acts::SourceLink{dummySl},
								  trackStateBackend.getTrackState(0) );

  std::cout << "Performing some checks on the state after calibration" << std::endl;
  std::cout << "Checking calibration is available" << std::endl;
  assert( trackStateBackend.getTrackState(0).hasCalibrated() );
  assert( not trackStateBackend.getTrackState(1).hasCalibrated() );

  std::cout << "- checking calibrated size: " << trackStateBackend.getTrackState(0).calibratedSize() << std::endl;
  assert( trackStateBackend.getTrackState(0).calibratedSize() == 2 );
  std::cout << "- checking loc pos" << std::endl;
  assert( trackStateBackend.getTrackState(0).effectiveCalibrated().size() == 2 );
  std::cout << "- checking loc cov" << std::endl;
  assert( trackStateBackend.getTrackState(0).effectiveCalibratedCovariance().size() == 4 );
  std::cout << "- checking BoundSubspaceIndices" << std::endl;
  Acts::BoundSubspaceIndices expectedBoundSpaceIndices{ Acts::eBoundLoc0, Acts::eBoundLoc1 };
  assert( trackStateBackend.getTrackState(0).projectorSubspaceIndices() == expectedBoundSpaceIndices );
  return EXIT_SUCCESS;
}