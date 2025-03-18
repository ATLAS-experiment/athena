/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#define BOOST_TEST_MODULE MultiTrajectorySE_test

#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

#include "CommonHelpers/GenerateParameters.hpp"
#include "CommonHelpers/TestSourceLink.hpp"
#include "ActsEvent/MultiTrajectory.h"

#include "Acts/EventData/detail/MultiTrajectoryTestsCommon.hpp"

// Thread safety check disabled, otherwise throwing compilation warnings.
#include "CxxUtils/checker_macros.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


BOOST_AUTO_TEST_SUITE(EventDataMultiTrajectoryACTS)


namespace {

using namespace Acts;
using namespace Acts::UnitLiterals;
using namespace Acts::Test;
using namespace Acts::detail::Test;
namespace bd = boost::unit_test::data;

using ParametersVector = BoundTrackParameters::ParametersVector;
using CovarianceMatrix = BoundTrackParameters::CovarianceMatrix;
using Jacobian = BoundMatrix;

const GeometryContext gctx;
// fixed seed for reproducible tests
std::default_random_engine rng(31415);

struct Factory {
  using trajectory_t = ActsTrk::MutableMultiTrajectory;
  using const_trajectory_t = ActsTrk::MultiTrajectory;

  ActsTrk::MutableMultiTrajectory create() { return {}; }
  ActsTrk::MultiTrajectory createConst() { 
    m_trackStatesAux = std::make_unique<xAOD::TrackStateAuxContainer>();
    m_trackParametersAux = std::make_unique<xAOD::TrackParametersAuxContainer>();
    m_trackJacobiansAux = std::make_unique<xAOD::TrackJacobianAuxContainer>();
    m_trackMeasurementsAux = std::make_unique<xAOD::TrackMeasurementAuxContainer>();
    m_surfacesBackendAux = std::make_unique<xAOD::TrackSurfaceAuxContainer>();

    return {m_trackStatesAux.get(), m_trackParametersAux.get(),
          m_trackJacobiansAux.get(), m_trackMeasurementsAux.get(),
          m_surfacesBackendAux.get()};
  }

 private:
  std::unique_ptr<xAOD::TrackStateAuxContainer> m_trackStatesAux;
  std::unique_ptr<xAOD::TrackParametersAuxContainer> m_trackParametersAux;
  std::unique_ptr<xAOD::TrackJacobianAuxContainer> m_trackJacobiansAux;
  std::unique_ptr<xAOD::TrackMeasurementAuxContainer> m_trackMeasurementsAux;
  std::unique_ptr<xAOD::TrackSurfaceAuxContainer> m_surfacesBackendAux;
};

using CommonTests = MultiTrajectoryTestsCommon<Factory>;

}  // namespace 


BOOST_AUTO_TEST_CASE(Build) {
  CommonTests ct;
  ct.testBuild();
}


BOOST_AUTO_TEST_CASE(ConstCorrectness) {
  // make mutable
  VectorMultiTrajectory t;
  auto i0 = t.addTrackState();
  BOOST_CHECK(!IsReadOnlyMultiTrajectory<decltype(t)>::value);

  {
    VectorMultiTrajectory::TrackStateProxy tsp = t.getTrackState(i0);
    static_cast<void>(tsp);
    VectorMultiTrajectory::ConstTrackStateProxy ctsp = t.getTrackState(i0);
    static_cast<void>(ctsp);

    tsp.predicted().setRandom();
    // const auto& tsp_const = tsp;
    // tsp_const.predicted().setRandom();
    // ctsp.predicted().setRandom();
  }

  // is this something we actually want?
  ConstVectorMultiTrajectory ct = t;
  BOOST_CHECK_EQUAL(ct.size(), t.size());

  ConstVectorMultiTrajectory ctm{std::move(t)};
  BOOST_CHECK_EQUAL(ctm.size(), ct.size());

  {
    static_assert(
        std::is_same_v<ConstVectorMultiTrajectory::ConstTrackStateProxy,
                       decltype(ctm.getTrackState(i0))>,
        "Got mutable track state proxy");
    ConstVectorMultiTrajectory::ConstTrackStateProxy ctsp =
        ctm.getTrackState(i0);
    static_cast<void>(ctsp);

    // doesn't compile:
    // ctsp.predictedCovariance().setIdentity();
  }
  // doesn't compile:
  // ct.clear();
  // ct.addTrackState();
}

BOOST_AUTO_TEST_CASE(Clear) {
  CommonTests ct;
  ct.testClear();
}

BOOST_AUTO_TEST_CASE(ApplyWithAbort) {
  CommonTests ct;
  ct.testApplyWithAbort();
}

BOOST_AUTO_TEST_CASE(AddTrackStateWithBitMask) {
  CommonTests ct;
  ct.testAddTrackStateWithBitMask();
}

// assert expected "cross-talk" between trackstate proxies
BOOST_AUTO_TEST_CASE(TrackStateProxyCrossTalk) {
  CommonTests ct;
  ct.testTrackStateProxyCrossTalk(rng);
}

BOOST_AUTO_TEST_CASE(TrackStateReassignment) {
  CommonTests ct;
  ct.testTrackStateReassignment(rng);
}

BOOST_DATA_TEST_CASE(TrackStateProxyStorage, bd::make({1u, 2u}),
                     nMeasurements) {
  CommonTests ct;
  ct.testTrackStateProxyStorage(rng, nMeasurements);
}

BOOST_AUTO_TEST_CASE(TrackStateProxyAllocations) {
  CommonTests ct;
  ct.testTrackStateProxyAllocations(rng);
}

BOOST_AUTO_TEST_CASE(TrackStateProxyGetMask) {
  CommonTests ct;
  ct.testTrackStateProxyGetMask();
}

BOOST_AUTO_TEST_CASE(TrackStateProxyCopy) {
  CommonTests ct;
  ct.testTrackStateProxyCopy(rng);
}

BOOST_AUTO_TEST_CASE(TrackStateCopyDynamicColumns) {
  CommonTests ct;
  ct.testTrackStateCopyDynamicColumns();
}

BOOST_AUTO_TEST_CASE(TrackStateProxyCopyDiffMTJ) {
  CommonTests ct;
  ct.testTrackStateProxyCopyDiffMTJ();
}

BOOST_AUTO_TEST_CASE(ProxyAssignment) {
  CommonTests ct;
  ct.testProxyAssignment();
}

BOOST_AUTO_TEST_CASE(CopyFromConst) {
  CommonTests ct;
  ct.testCopyFromConst();
}

BOOST_AUTO_TEST_CASE(TrackStateProxyShare) {
  CommonTests ct;
  ct.testTrackStateProxyShare(rng);
}

BOOST_AUTO_TEST_CASE(MultiTrajectoryExtraColumns) {
  CommonTests ct;
  ct.testMultiTrajectoryExtraColumns();
}

BOOST_AUTO_TEST_CASE(MultiTrajectoryExtraColumnsRuntime) {
  CommonTests ct;
  ct.testMultiTrajectoryExtraColumnsRuntime();
}

BOOST_AUTO_TEST_CASE(MultiTrajectoryAllocateCalibratedInit) {
  CommonTests ct;
  ct.testMultiTrajectoryAllocateCalibratedInit(rng);
}


BOOST_AUTO_TEST_SUITE_END()
