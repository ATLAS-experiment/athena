/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/SubspaceHelpers.hpp"
#include "Acts/EventData/Types.hpp"
#undef NDEBUG
#define BOOST_TEST_MODULE MultiTrajectoryBasic_test
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

#include "Acts/EventData/MeasurementHelpers.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/CurvilinearSurface.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "CommonHelpers/GenerateParameters.hpp"
#include "CommonHelpers/TestSourceLink.hpp"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"
#include "xAODTracking/TrackState.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"

namespace {

using namespace Acts;
using namespace Acts::UnitLiterals;
using namespace Acts::Test;
namespace bd = boost::unit_test::data;

using ParametersVector = BoundTrackParameters::ParametersVector;
using CovarianceMatrix = BoundTrackParameters::CovarianceMatrix;
using Jacobian = Acts::BoundMatrix;

struct TestTrackState {
  std::shared_ptr<Surface> surface;
  TestSourceLink sourceLink;
  BoundTrackParameters predicted;
  BoundTrackParameters filtered;
  BoundTrackParameters smoothed;
  Jacobian jacobian;
  float chi2;
  double pathLength;

  // Generate a random TestTrackState.
  //
  // @param rng Random number generator
  // @param size_t nMeasurement either 1 or 2
  template <typename rng_t>
  TestTrackState(rng_t &rng, size_t nMeasurements)
      : surface(Acts::CurvilinearSurface(
            Vector3::Zero(), Vector3::UnitZ()).planeSurface()),
        // set bogus parameters first since they are not default-constructible
        predicted(surface, BoundVector::Zero(), std::nullopt, ParticleHypothesis::pion()),
        filtered(surface, BoundVector::Zero(), std::nullopt, ParticleHypothesis::pion()),
        smoothed(surface, BoundVector::Zero(), std::nullopt, ParticleHypothesis::pion()),
        jacobian(Jacobian::Identity()),
        chi2(std::chi_squared_distribution<float>(nMeasurements)(rng)),
        pathLength(
            std::uniform_real_distribution<double>(1_mm, 10_mm)(rng)) {
    // set a random geometry identifier to uniquely identify each surface
    GeometryIdentifier geoId{
        std::uniform_int_distribution<GeometryIdentifier::Value>()(rng)};
    surface->assignGeometryId(geoId);

    // create source link w/ inline 1d or 2d measurement data
    if (nMeasurements == 1u) {
      auto [par, cov] = generateParametersCovariance<double, 1u>(rng);
      sourceLink = TestSourceLink(eBoundLoc0, par[0], cov(0, 0), geoId);
    } else if (nMeasurements == 2u) {
      auto [par, cov] = generateParametersCovariance<double, 2u>(rng);
      sourceLink = TestSourceLink(eBoundLoc1, eBoundQOverP, par, cov, geoId);
    } else {
      throw std::runtime_error("invalid number of measurement dimensions");
    }

    // create track parameters
    auto [trkPar, trkCov] = generateBoundParametersCovariance(rng);
    trkPar[eBoundPhi] = 45_degree;
    trkPar[eBoundTheta] = 90_degree;
    trkPar[eBoundQOverP] = 5.;
    // predicted
    predicted = BoundTrackParameters(surface, trkPar, trkCov, ParticleHypothesis::pion());
    // filtered, modified q/p, reduced covariance
    trkPar[eBoundQOverP] = 10.;
    filtered = BoundTrackParameters(surface, trkPar, 0.75 * trkCov, ParticleHypothesis::pion());
    // smoothed, modified q/p, further reduced covariance
    trkPar[eBoundQOverP] = 15.;
    smoothed = BoundTrackParameters(surface, trkPar, 0.5 * trkCov, ParticleHypothesis::pion());

    // propagation jacobian is identity + corrections
    for (Eigen::Index c = 0; c < jacobian.cols(); ++c) {
      for (Eigen::Index r = 0; r < jacobian.rows(); ++r) {
        jacobian(c, r) +=
            std::uniform_real_distribution<double>(-0.125, 0.125)(rng);
      }
    }
  }
};

struct EmptyTrackStatesAux {
  ActsTrk::MutableMultiTrajectory m;
};

BOOST_FIXTURE_TEST_CASE(AllMtjStaticxAODVariablesAreKnown, EmptyTrackStatesAux) {
    for (auto id : m.trackStatesAux()->getStore()->getAuxIDs()) {
        const std::string name = SG::AuxTypeRegistry::instance().getName(id);
        BOOST_CHECK(ActsTrk::MutableMultiTrajectory::s_staticVariables.count(name) == 1);
    }
    BOOST_CHECK_MESSAGE( m.trackStatesAux()->getAuxIDs().size() + 2 /* two dynamic variables: measurement pointer and element link*/
                         == ActsTrk::MutableMultiTrajectory::s_staticVariables.size(),
        "getAuxIDs.size() = " << m.trackStatesAux()->getAuxIDs().size() << "  staticVariables.size()+2 = " << ActsTrk::MutableMultiTrajectory::s_staticVariables.size());
}

// Fill a TrackStateProxy with values from a TestTrackState.
//
// @param[in] pc TestTrackState with the input values
// @param[in] mask Specifies which components are used/filled
// @param[out] ts TrackStateProxy which is filled
// @param [in] nMeasurements Dimension of the measurement
template <typename track_state_t>
void fillTrackState(const TestTrackState &pc, TrackStatePropMask mask,
                    track_state_t &ts, bool checkSurf = true) {
  // always set the reference surface
  if (checkSurf) BOOST_CHECK_EQUAL(ts.hasReferenceSurface(), false);
  ts.setReferenceSurface(pc.predicted.referenceSurface().getSharedPtr());
  BOOST_CHECK(ts.hasReferenceSurface());

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Predicted)) {
    ts.predicted() = pc.predicted.parameters();
    BOOST_CHECK_EQUAL(pc.predicted.parameters(), ts.predicted());
    BOOST_CHECK(pc.predicted.covariance().has_value());
    ts.predictedCovariance() = *(pc.predicted.covariance());
    BOOST_CHECK(*(pc.predicted.covariance()) == ts.predictedCovariance());
  }

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Filtered)) {
    ts.filtered() = pc.filtered.parameters();
    BOOST_CHECK(pc.filtered.covariance().has_value());
    ts.filteredCovariance() = *(pc.filtered.covariance());
  }

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Smoothed)) {
    ts.smoothed() = pc.smoothed.parameters();
    BOOST_CHECK(pc.smoothed.covariance().has_value());
    ts.smoothedCovariance() = *(pc.smoothed.covariance());
  }

  if (ACTS_CHECK_BIT(mask, TrackStatePropMask::Jacobian)) {
    ts.jacobian() = pc.jacobian;
  }
  ts.chi2() = pc.chi2;
  ts.pathLength() = pc.pathLength;

  // TODO Add here the part of code from (not all components implemented):
  // https://github.com/acts-project/acts/blob/d8cb0fac3a44e1d44595a481f977df9bd70195fb/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L139
}

const GeometryContext gctx;
// fixed seed for reproducible tests
std::default_random_engine rng(31415);

}  // namespace

BOOST_AUTO_TEST_SUITE(EventDataMultiTrajectory)

struct EmptyMTJ {  // setup empty MTJ
  EmptyMTJ() :
    mtj(std::make_unique<ActsTrk::MutableMultiTrajectory>())
  {}

  std::unique_ptr<ActsTrk::MutableMultiTrajectory> mtj;
  ActsTrk::MultiTrajectory* ro_mtj() {
    if ( !m_ro_mtj ) {
      m_ro_mtj = std::make_unique<ActsTrk::MultiTrajectory>(
        mtj->trackStatesAux(), 
        mtj->trackParametersAux(),
        mtj->trackJacobiansAux(),
        mtj->trackMeasurementsAux(),
        mtj->trackSurfacesAux()
      );

    }
    return m_ro_mtj.get();
  }
  std::unique_ptr<ActsTrk::MultiTrajectory> m_ro_mtj;
};

BOOST_AUTO_TEST_CASE(OwningMTJ) {
  ActsTrk::MutableMultiTrajectory mtj;
}

BOOST_FIXTURE_TEST_CASE(Fill, EmptyMTJ) {
  BOOST_CHECK(mtj->has_backends());
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;
  auto i0 = mtj->addTrackState(kMask);
  // trajectory bifurcates here into multiple hypotheses
  auto i1a = mtj->addTrackState(kMask, i0);
  auto i1b = mtj->addTrackState(kMask, i0);
  auto i2a = mtj->addTrackState(kMask, i1a);
  auto i2b = mtj->addTrackState(kMask, i1b);

  ActsTrk::MutableMultiTrajectory copy(*mtj.get());

  std::vector<size_t> act;
  auto collect = [&](auto p) {
    act.push_back(p.index());
    BOOST_CHECK(!p.hasCalibrated());
    BOOST_CHECK(!p.hasFiltered());
    BOOST_CHECK(!p.hasSmoothed());
    BOOST_CHECK(!p.hasJacobian());
    BOOST_CHECK(!p.hasProjector());
  };

  std::vector<size_t> exp = {i2a, i1a, i0};
  mtj->applyBackwards(i2a, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());

  act.clear();
  ro_mtj()->visitBackwards(i2a, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());

  act.clear();
  copy.applyBackwards(i2a, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());

  exp = {i2b, i1b, i0};

  act.clear();
  mtj->applyBackwards(i2b, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());

  act.clear();
  ro_mtj()->visitBackwards(i2b, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());

  act.clear();
  copy.visitBackwards(i2b, collect);
  BOOST_CHECK_EQUAL_COLLECTIONS(act.begin(), act.end(), exp.begin(), exp.end());
}

BOOST_FIXTURE_TEST_CASE(Dynamic_columns, EmptyMTJ) {
  using namespace Acts::HashedStringLiteral;
  BOOST_CHECK(mtj->has_backends());
  BOOST_CHECK_EQUAL(mtj->hasColumn("jacobian"_hash),
                    true);  // not dynamic column
  BOOST_CHECK_EQUAL(mtj->hasColumn("author"_hash),
                    false);         // dynamic column absent initially
  mtj->addColumn<short>("author");  // add dynamic column

  BOOST_CHECK_EQUAL(mtj->hasColumn("author"_hash),
                    true);  // dynamic column present now
  mtj->addColumn<float>("mcprob");
  BOOST_CHECK_EQUAL(mtj->hasColumn("mcprob"_hash),
                    true);  // dynamic column present now

  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;
  auto i0 = mtj->addTrackState(kMask);
  auto i1 = mtj->addTrackState(kMask, i0);
  auto i2 = mtj->addTrackState(kMask, i1);
  // dynamic column enabled late
  auto ts0 = mtj->getTrackState(i0);
  auto ts1 = mtj->getTrackState(i1);
  auto ts2 = mtj->getTrackState(i2);

  ts0.component<short, "author"_hash>() = 5;
  ts1.component<short, "author"_hash>() = 6;
  ts2.component<short, "author"_hash>() = 4;

  ts0.component<float, "mcprob"_hash>() = 0.5;
  ts1.component<float, "mcprob"_hash>() = 0.9;

  // unset for ts2

  // read them back
  BOOST_CHECK_EQUAL((ts0.component<short, "author"_hash>()), 5);
  BOOST_CHECK_EQUAL((ts1.component<short, "author"_hash>()), 6);
  BOOST_CHECK_EQUAL((ts2.component<short, "author"_hash>()), 4);

  BOOST_TEST((ts0.component<float, "mcprob"_hash>()) == 0.5,
             boost::test_tools::tolerance(0.01));
  BOOST_TEST((ts1.component<float, "mcprob"_hash>()) == 0.9,
             boost::test_tools::tolerance(0.01));
  BOOST_TEST((ts2.component<float, "mcprob"_hash>()) == 0.0,
             boost::test_tools::tolerance(0.01));

  BOOST_CHECK_THROW((ts2.component<float, "sth"_hash>()), std::runtime_error);

  // RO MTJ needs to be remade now because only at construction it can recognise
  // the dynamic columns

  BOOST_CHECK_EQUAL(ro_mtj()->hasColumn("author"_hash),
                    true);  // dynamic column present now in const version too
  BOOST_CHECK_EQUAL(ro_mtj()->hasColumn("mcprob"_hash),
                    true);  // dynamic column present now in const version too

  auto ro_ts0 = ro_mtj()->getTrackState(i0);
  auto ro_ts1 = ro_mtj()->getTrackState(i1);
  auto ro_ts2 = ro_mtj()->getTrackState(i2);

  BOOST_CHECK_EQUAL((ro_ts0.component<short, "author"_hash>()), 5);
  BOOST_CHECK_EQUAL((ro_ts1.component<short, "author"_hash>()), 6);
  BOOST_CHECK_EQUAL((ro_ts2.component<short, "author"_hash>()), 4);

  BOOST_TEST((ro_ts0.component<float, "mcprob"_hash>()) == 0.5,
             boost::test_tools::tolerance(0.01));
  BOOST_TEST((ro_ts1.component<float, "mcprob"_hash>()) == 0.9,
             boost::test_tools::tolerance(0.01));
  BOOST_TEST((ro_ts2.component<float, "mcprob"_hash>()) == 0.0,
             boost::test_tools::tolerance(0.01));

  auto dest_mtj = std::make_unique<ActsTrk::MutableMultiTrajectory>();
  dest_mtj->addColumn<short>("author");
  dest_mtj->addColumn<float>("mcprob");
  using namespace Acts::HashedStringLiteral;

  auto dest_i0 = dest_mtj->addTrackState(kMask);
  auto dest_ts0 = dest_mtj->getTrackState(dest_i0);
  dest_ts0.copyFrom(ts0);

  BOOST_CHECK_EQUAL((ro_ts0.component<short, "author"_hash>()),
                    (dest_ts0.component<short, "author"_hash>()));

  BOOST_CHECK_EQUAL((ro_ts0.component<float, "mcprob"_hash>()),
                    (dest_ts0.component<float, "mcprob"_hash>()));


  // make completely new r/o MTJ
  auto fresh_ro_mtj = std::make_unique<ActsTrk::MultiTrajectory>(*mtj.get());


  BOOST_CHECK_EQUAL(fresh_ro_mtj->size(), mtj->size());
}

// FIXME - test below should use ACTS::MTJ api once available in needed shape
BOOST_FIXTURE_TEST_CASE(UncalibratedSourceLink, EmptyMTJ) {
  auto index = mtj->addTrackState();
  using namespace Acts::HashedStringLiteral;

  auto link1 = Acts::SourceLink(reinterpret_cast<const xAOD::UncalibratedMeasurement *>(0xDEADBEEF));  // a fictional geometry ID
  auto ts = mtj->getTrackState(index);
  ts.setUncalibratedSourceLink(std::move(link1));  // set link at position 0
  // get it back
  auto link1Back = ts.getUncalibratedSourceLink();
  auto ptrBack =
       link1Back.get<const xAOD::UncalibratedMeasurement *>();
  BOOST_CHECK_EQUAL(static_cast<const void *>(ptrBack), reinterpret_cast<const void *>(0xDEADBEEF));
}

BOOST_FIXTURE_TEST_CASE(Clear, EmptyMTJ) {
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;

  BOOST_CHECK_EQUAL(mtj->size(), 0);

  auto i0 = mtj->addTrackState(kMask);
  // trajectory bifurcates here into multiple hypotheses
  auto i1a = mtj->addTrackState(kMask, i0);
  auto i1b = mtj->addTrackState(kMask, i0);
  mtj->addTrackState(kMask, i1a);
  mtj->addTrackState(kMask, i1b);

  BOOST_CHECK_EQUAL(mtj->size(), 5);
  mtj->clear();
  BOOST_CHECK_EQUAL(mtj->size(), 0);
}

BOOST_FIXTURE_TEST_CASE(ApplyWithAbort, EmptyMTJ) {
  constexpr auto kMask = Acts::TrackStatePropMask::Predicted;

  // construct trajectory with three components

  auto i0 = mtj->addTrackState(kMask);
  auto i1 = mtj->addTrackState(kMask, i0);
  auto i2 = mtj->addTrackState(kMask, i1);

  size_t n = 0;
  mtj->applyBackwards(i2, [&](const auto &) {
    n++;
    return false;
  });
  BOOST_CHECK_EQUAL(n, 1u);

  n = 0;
  mtj->applyBackwards(i2, [&](const auto &ts) {
    n++;
    if (ts.index() == i1) {
      return false;
    }
    return true;
  });
  BOOST_CHECK_EQUAL(n, 2u);

  n = 0;
  mtj->applyBackwards(i2, [&](const auto &) {
    n++;
    return true;
  });
  BOOST_CHECK_EQUAL(n, 3u);
}

BOOST_FIXTURE_TEST_CASE(BitmaskOperators, EmptyMTJ) {
  using PM = Acts::TrackStatePropMask;

  auto bs1 = PM::Predicted;

  BOOST_CHECK(ACTS_CHECK_BIT(bs1, PM::Predicted));
  BOOST_CHECK(!ACTS_CHECK_BIT(bs1, PM::Calibrated));

  auto bs2 = PM::Calibrated;

  BOOST_CHECK(!ACTS_CHECK_BIT(bs2, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(bs2, PM::Calibrated));

  auto bs3 = PM::Calibrated | PM::Predicted;

  BOOST_CHECK(ACTS_CHECK_BIT(bs3, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(bs3, PM::Calibrated));

  BOOST_CHECK(ACTS_CHECK_BIT(PM::All, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(PM::All, PM::Calibrated));

  auto bs4 = PM::Predicted | PM::Jacobian | PM::Smoothed;
  BOOST_CHECK(ACTS_CHECK_BIT(bs4, PM::Predicted));
  BOOST_CHECK(ACTS_CHECK_BIT(bs4, PM::Jacobian));
  BOOST_CHECK(ACTS_CHECK_BIT(bs4, PM::Smoothed));
  BOOST_CHECK(!ACTS_CHECK_BIT(bs4, PM::Calibrated));
  BOOST_CHECK(!ACTS_CHECK_BIT(bs4, PM::Filtered));

  auto cnv = [](auto a) -> std::bitset<8> {
    return static_cast<std::underlying_type<PM>::type>(a);
  };

  BOOST_CHECK(cnv(PM::All).all());    // all ones
  BOOST_CHECK(cnv(PM::None).none());  // all zeros

  // test orthogonality
  std::array<PM, 5> values{PM::Predicted, PM::Filtered, PM::Smoothed,
                           PM::Jacobian, PM::Calibrated};
  for (size_t i = 0; i < values.size(); i++) {
    for (size_t j = 0; j < values.size(); j++) {
      PM a = values[i];
      PM b = values[j];

      if (i == j) {
        BOOST_CHECK(cnv(a & b).count() == 1);
      } else {
        BOOST_CHECK(cnv(a & b).none());
      }
    }
  }

  BOOST_CHECK(cnv(PM::Predicted ^ PM::Filtered).count() == 2);
  BOOST_CHECK(cnv(PM::Predicted ^ PM::Predicted).none());
  BOOST_CHECK(~(PM::Predicted | PM::Calibrated) ==
              (PM::All ^ PM::Predicted ^ PM::Calibrated));

  PM base = PM::None;
  BOOST_CHECK(cnv(base) == 0);

  base &= PM::Filtered;
  BOOST_CHECK(cnv(base) == 0);

  base |= PM::Filtered;
  BOOST_CHECK(base == PM::Filtered);

  base |= PM::Calibrated;
  BOOST_CHECK(base == (PM::Filtered | PM::Calibrated));

  base ^= PM::All;
  BOOST_CHECK(base == ~(PM::Filtered | PM::Calibrated));
}

BOOST_FIXTURE_TEST_CASE(AddTrackStateWithBitMask, EmptyMTJ) {

  using PM = Acts::TrackStatePropMask;
  using namespace Acts::HashedStringLiteral;

  // TODO : add tests checking
  // "calibratedSourceLink","referenceSurface","typeFlags" see:
  // https://github.com/acts-project/acts/blob/5cbcbf01f1d6bbe79a98b84bdc6ef076cf763c01/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L321
  auto alwaysPresent = [](auto &ts) {
    BOOST_CHECK(ts.template has<"measdim"_hash>());
    BOOST_CHECK(ts.template has<"chi2"_hash>());
    BOOST_CHECK(ts.template has<"pathLength"_hash>());
    BOOST_CHECK(ts.template has<"typeFlags"_hash>());
  };

  auto ts = mtj->getTrackState(mtj->addTrackState(PM::All));
  BOOST_CHECK(ts.hasPredicted());
  BOOST_CHECK(ts.hasFiltered());
  BOOST_CHECK(ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(ts.hasProjector());
  BOOST_CHECK(ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::None));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Predicted));
  BOOST_CHECK(ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Filtered));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Smoothed));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());
  alwaysPresent(ts);

  ts = mtj->getTrackState(mtj->addTrackState(PM::Calibrated));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(ts.hasProjector());
  BOOST_CHECK(!ts.hasJacobian());

  ts = mtj->getTrackState(mtj->addTrackState(PM::Jacobian));
  BOOST_CHECK(!ts.hasPredicted());
  BOOST_CHECK(!ts.hasFiltered());
  BOOST_CHECK(!ts.hasSmoothed());
  BOOST_CHECK(!ts.hasCalibrated());
  BOOST_CHECK(!ts.hasProjector());
  BOOST_CHECK(ts.hasJacobian());
  alwaysPresent(ts);
}

BOOST_FIXTURE_TEST_CASE(UseFillTrackState, EmptyMTJ) {
  std::default_random_engine rng(31415);
  TestTrackState pc(rng, 1);
  size_t index = mtj->addTrackState();
  auto ts = mtj->getTrackState(index);
  fillTrackState(pc, TrackStatePropMask::All, ts);
}

// assert expected "cross-talk" between trackstate proxies
BOOST_FIXTURE_TEST_CASE(TrackStateProxyCrossTalk, EmptyMTJ) {
  std::default_random_engine rng(12345);
  TestTrackState pc(rng, 2u);

  // multi trajectory w/ a single, fully set, track state
  size_t index = mtj->addTrackState();
  {
    auto ts = mtj->getTrackState(index);
    fillTrackState(pc, TrackStatePropMask::All, ts);
  }
  // get two TrackStateProxies that reference the same data
  auto tsa = mtj->getTrackState(index);
  auto tsb = mtj->getTrackState(index);
  // then modify one and check that the other was modified as well
  {
    auto [par, cov] = generateBoundParametersCovariance(rng);
    tsb.predicted() = par;
    tsb.predictedCovariance() = cov;
    BOOST_CHECK_EQUAL(tsa.predicted(), par);
    BOOST_CHECK_EQUAL(tsa.predictedCovariance(), cov);
    BOOST_CHECK_EQUAL(tsb.predicted(), par);
    BOOST_CHECK_EQUAL(tsb.predictedCovariance(), cov);
  }
  {
    auto [par, cov] = generateBoundParametersCovariance(rng);
    tsb.filtered() = par;
    tsb.filteredCovariance() = cov;
    BOOST_CHECK_EQUAL(tsa.filtered(), par);
    BOOST_CHECK_EQUAL(tsa.filteredCovariance(), cov);
    BOOST_CHECK_EQUAL(tsb.filtered(), par);
    BOOST_CHECK_EQUAL(tsb.filteredCovariance(), cov);
  }
  {
    auto [par, cov] = generateBoundParametersCovariance(rng);
    tsb.smoothed() = par;
    tsb.smoothedCovariance() = cov;
    BOOST_CHECK_EQUAL(tsa.smoothed(), par);
    BOOST_CHECK_EQUAL(tsa.smoothedCovariance(), cov);
    BOOST_CHECK_EQUAL(tsb.smoothed(), par);
    BOOST_CHECK_EQUAL(tsb.smoothedCovariance(), cov);
  }

  // TODO in the next MR
  // problem with a link tsa.uncalibrated()
  // https://github.com/acts-project/acts/blob/980f9ef66ce2df426be87e611f9a8c813904ad7c/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L479

  {
    // reset measurements w/ full parameters
    auto [measPar, measCov] = generateBoundParametersCovariance(rng);
    tsb.allocateCalibrated(eBoundSize);
    tsb.calibrated<eBoundSize>() = measPar;
    tsb.calibratedCovariance<eBoundSize>() = measCov;
    BOOST_CHECK_EQUAL(tsa.calibrated<eBoundSize>(), measPar);
    BOOST_CHECK_EQUAL(tsa.calibratedCovariance<eBoundSize>(), measCov);
    BOOST_CHECK_EQUAL(tsb.calibrated<eBoundSize>(), measPar);
    BOOST_CHECK_EQUAL(tsb.calibratedCovariance<eBoundSize>(), measCov);
  }
  {
    // reset only the effective measurements
    auto [measPar, measCov] = generateBoundParametersCovariance(rng);
    tsb.allocateCalibrated(eBoundSize);
    size_t nMeasurements = tsb.effectiveCalibrated().rows();
    auto effPar = measPar.head(nMeasurements);
    auto effCov = measCov.topLeftCorner(nMeasurements, nMeasurements);
    tsb.allocateCalibrated(eBoundSize);
    tsb.effectiveCalibrated() = effPar;
    tsb.effectiveCalibratedCovariance() = effCov;
    BOOST_CHECK_EQUAL(tsa.effectiveCalibrated(), effPar);
    BOOST_CHECK_EQUAL(tsa.effectiveCalibratedCovariance(), effCov);
    BOOST_CHECK_EQUAL(tsb.effectiveCalibrated(), effPar);
    BOOST_CHECK_EQUAL(tsb.effectiveCalibratedCovariance(), effCov);
  }
  {
    Jacobian jac = Jacobian::Identity();
    BOOST_CHECK_NE(tsa.jacobian(), jac);
    BOOST_CHECK_NE(tsb.jacobian(), jac);
    tsb.jacobian() = jac;
    BOOST_CHECK_EQUAL(tsa.jacobian(), jac);
    BOOST_CHECK_EQUAL(tsb.jacobian(), jac);
  }
  {
    tsb.chi2() = 98.0;
    BOOST_CHECK_EQUAL(tsa.chi2(), 98.0);
    BOOST_CHECK_EQUAL(tsb.chi2(), 98.0);
  }
  {
    tsb.pathLength() = 66.0;
    BOOST_CHECK_EQUAL(tsa.pathLength(), 66.0);
    BOOST_CHECK_EQUAL(tsb.pathLength(), 66.0);
  }
}

// TODO Can't work because of missing part of code in fillTrackState:
// https://github.com/acts-project/acts/blob/d8cb0fac3a44e1d44595a481f977df9bd70195fb/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L139

// TODO
// Missing tests from here to be added:
// https://github.com/acts-project/acts/blob/fa7bd8248f55e030f117f450ea315f60b2ce4335/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L559

// TODO
// The test below has few lines to be added after fixing ReferenceSurface,
// allocateCalibrated etc. See:
// https://github.com/acts-project/acts/blob/fa7bd8248f55e030f117f450ea315f60b2ce4335/Tests/UnitTests/Core/EventData/MultiTrajectoryTests.cpp#L641
BOOST_FIXTURE_TEST_CASE(TrackStateProxyAllocations, EmptyMTJ) {
  using namespace Acts::HashedStringLiteral;
  std::default_random_engine rng(12345);
  TestTrackState pc(rng, 2u);

  // this should allocate for all components in the trackstate, plus filtered
  size_t i = mtj->addTrackState(TrackStatePropMask::Predicted |
                                TrackStatePropMask::Filtered |
                                TrackStatePropMask::Jacobian);
  auto tso = mtj->getTrackState(i);

  fillTrackState(pc, TrackStatePropMask::Predicted, tso);
  fillTrackState(pc, TrackStatePropMask::Filtered, tso, false);
  fillTrackState(pc, TrackStatePropMask::Jacobian, tso, false);

  BOOST_CHECK(tso.hasPredicted());
  BOOST_CHECK(tso.hasFiltered());
  BOOST_CHECK(!tso.hasSmoothed());
  BOOST_CHECK(!tso.hasCalibrated());
  BOOST_CHECK(tso.hasJacobian());

  auto tsnone =
      mtj->getTrackState(mtj->addTrackState(TrackStatePropMask::None));
  BOOST_CHECK(!tsnone.has<"predicted"_hash>());
  BOOST_CHECK(!tsnone.has<"filtered"_hash>());
  BOOST_CHECK(!tsnone.has<"smoothed"_hash>());
  BOOST_CHECK(!tsnone.has<"jacobian"_hash>());
  BOOST_CHECK(!tsnone.has<"calibrated"_hash>());
  BOOST_CHECK(!tsnone.has<"projector"_hash>());

  BOOST_CHECK(!tsnone.has<"calibratedSourceLink"_hash>());
  // TODO referenceSurface not implemented
  BOOST_CHECK(tsnone.has<"referenceSurface"_hash>());
  BOOST_CHECK(tsnone.has<"measdim"_hash>());
  BOOST_CHECK(tsnone.has<"chi2"_hash>());
  // TODO pathLength and typeFlags not implemented

  auto tsall = mtj->getTrackState(mtj->addTrackState(TrackStatePropMask::All));
  BOOST_CHECK(tsall.has<"predicted"_hash>());
  BOOST_CHECK(tsall.has<"filtered"_hash>());
  BOOST_CHECK(tsall.has<"smoothed"_hash>());
  BOOST_CHECK(tsall.has<"jacobian"_hash>());
  // TODO we create a trackState already with "calibrated" and
  // allocateCalibrated not implemented

  BOOST_CHECK(!tsall.has<"calibrated"_hash>());
  BOOST_CHECK(tsall.has<"projector"_hash>());
  // TODO referenceSurface not implemented

  BOOST_CHECK(tsall.has<"measdim"_hash>());
  BOOST_CHECK(tsall.has<"chi2"_hash>());
  // TODO pathLength and typeFlags not implemented

  tsall.unset(TrackStatePropMask::Predicted);
  BOOST_CHECK(!tsall.has<"predicted"_hash>());
  tsall.unset(TrackStatePropMask::Filtered);
  BOOST_CHECK(!tsall.has<"filtered"_hash>());
  tsall.unset(TrackStatePropMask::Smoothed);
  BOOST_CHECK(!tsall.has<"smoothed"_hash>());
  tsall.unset(TrackStatePropMask::Jacobian);
  BOOST_CHECK(!tsall.has<"jacobian"_hash>());
  tsall.unset(TrackStatePropMask::Calibrated);
  BOOST_CHECK(!tsall.has<"calibrated"_hash>());
}

BOOST_FIXTURE_TEST_CASE(TrackStateProxyShare, EmptyMTJ) {
  std::default_random_engine rng(12345);
  TestTrackState pc(rng, 2u);

  {
    size_t ia = mtj->addTrackState(TrackStatePropMask::All);
    size_t ib = mtj->addTrackState(TrackStatePropMask::None);

    auto tsa = mtj->getTrackState(ia);
    auto tsb = mtj->getTrackState(ib);

    fillTrackState(pc, TrackStatePropMask::All, tsa);

    BOOST_CHECK(tsa.hasPredicted());
    BOOST_CHECK(!tsb.hasPredicted());
    tsb.shareFrom(tsa, TrackStatePropMask::Predicted);
    BOOST_CHECK(tsa.hasPredicted());
    BOOST_CHECK(tsb.hasPredicted());
    BOOST_CHECK_EQUAL(tsa.predicted(), tsb.predicted());
    BOOST_CHECK_EQUAL(tsa.predictedCovariance(), tsb.predictedCovariance());

    BOOST_CHECK(tsa.hasFiltered());
    BOOST_CHECK(!tsb.hasFiltered());
    tsb.shareFrom(tsa, TrackStatePropMask::Filtered);
    BOOST_CHECK(tsa.hasFiltered());
    BOOST_CHECK(tsb.hasFiltered());
    BOOST_CHECK_EQUAL(tsa.filtered(), tsb.filtered());
    BOOST_CHECK_EQUAL(tsa.filteredCovariance(), tsb.filteredCovariance());

    BOOST_CHECK(tsa.hasSmoothed());
    BOOST_CHECK(!tsb.hasSmoothed());
    tsb.shareFrom(tsa, TrackStatePropMask::Smoothed);
    BOOST_CHECK(tsa.hasSmoothed());
    BOOST_CHECK(tsb.hasSmoothed());
    BOOST_CHECK_EQUAL(tsa.smoothed(), tsb.smoothed());
    BOOST_CHECK_EQUAL(tsa.smoothedCovariance(), tsb.smoothedCovariance());

    BOOST_CHECK(tsa.hasJacobian());
    BOOST_CHECK(!tsb.hasJacobian());
    tsb.shareFrom(tsa, TrackStatePropMask::Jacobian);
    BOOST_CHECK(tsa.hasJacobian());
    BOOST_CHECK(tsb.hasJacobian());
    BOOST_CHECK_EQUAL(tsa.jacobian(), tsb.jacobian());
  }

  {
    size_t i = mtj->addTrackState(TrackStatePropMask::All &
                                  ~TrackStatePropMask::Filtered &
                                  ~TrackStatePropMask::Smoothed);

    auto ts = mtj->getTrackState(i);

    BOOST_CHECK(ts.hasPredicted());
    BOOST_CHECK(!ts.hasFiltered());
    BOOST_CHECK(!ts.hasSmoothed());
    ts.predicted().setRandom();
    ts.predictedCovariance().setRandom();

    ts.shareFrom(TrackStatePropMask::Predicted, TrackStatePropMask::Filtered);
    BOOST_CHECK(ts.hasPredicted());
    BOOST_CHECK(ts.hasFiltered());
    BOOST_CHECK(!ts.hasSmoothed());
    BOOST_CHECK_EQUAL(ts.predicted(), ts.filtered());
    BOOST_CHECK_EQUAL(ts.predictedCovariance(), ts.filteredCovariance());

    ts.shareFrom(TrackStatePropMask::Predicted, TrackStatePropMask::Smoothed);
    BOOST_CHECK(ts.hasPredicted());
    BOOST_CHECK(ts.hasFiltered());
    BOOST_CHECK(ts.hasSmoothed());
    BOOST_CHECK_EQUAL(ts.predicted(), ts.filtered());
    BOOST_CHECK_EQUAL(ts.predicted(), ts.smoothed());
    BOOST_CHECK_EQUAL(ts.predictedCovariance(), ts.filteredCovariance());
    BOOST_CHECK_EQUAL(ts.predictedCovariance(), ts.smoothedCovariance());
  }
}

BOOST_FIXTURE_TEST_CASE(TrackStateProjector, EmptyMTJ) {

  std::default_random_engine rng(12345);
  TestTrackState pc(rng, 2u);

  size_t ia = mtj->addTrackState(TrackStatePropMask::All);
  auto ts = mtj->getTrackState(ia);

  fillTrackState(pc, TrackStatePropMask::All, ts);

  // reset only the effective measurements
  auto [measPar, measCov] = generateBoundParametersCovariance(rng);
  ts.allocateCalibrated(eBoundSize);
  ts.calibrated<eBoundSize>() = measPar;
  ts.calibratedCovariance<eBoundSize>() = measCov;

  // assert contents of original measurement (just to be safe)
  BOOST_CHECK_EQUAL(ts.calibratedSize(), eBoundSize);
  BOOST_CHECK_EQUAL(ts.effectiveCalibrated(), measPar);
  BOOST_CHECK_EQUAL(ts.effectiveCalibratedCovariance(), measCov);

  // Set and test subspace
  Acts::BoundSubspaceIndices subspace = {Acts::eBoundLoc0, Acts::eBoundLoc0, Acts::eBoundPhi, 
                                         Acts::eBoundTheta, Acts::eBoundQOverP, Acts::eBoundTime};
  ts.setProjectorSubspaceIndices(subspace);

  BOOST_CHECK(ts.projectorSubspaceIndices() == subspace);
}

BOOST_FIXTURE_TEST_CASE(TrackStateProxyStorage, EmptyMTJ) {

  std::default_random_engine rng(12345);
  TestTrackState pc(rng, 2u);

  size_t ia = mtj->addTrackState(TrackStatePropMask::All);
  auto ts = mtj->getTrackState(ia);
  fillTrackState(pc, TrackStatePropMask::All, ts);

  // check that the track parameters are set
  BOOST_CHECK(ts.hasPredicted());
  BOOST_CHECK_EQUAL(ts.predicted(), pc.predicted.parameters());
  BOOST_CHECK(pc.predicted.covariance().has_value());
  BOOST_CHECK_EQUAL(ts.predictedCovariance(), *pc.predicted.covariance());
  BOOST_CHECK(ts.hasFiltered());
  BOOST_CHECK_EQUAL(ts.filtered(), pc.filtered.parameters());
  BOOST_CHECK(pc.filtered.covariance().has_value());
  BOOST_CHECK_EQUAL(ts.filteredCovariance(), *pc.filtered.covariance());
  BOOST_CHECK(ts.hasSmoothed());
  BOOST_CHECK_EQUAL(ts.smoothed(), pc.smoothed.parameters());
  BOOST_CHECK(pc.smoothed.covariance().has_value());
  BOOST_CHECK_EQUAL(ts.smoothedCovariance(), *pc.smoothed.covariance());

  // check that the jacobian is set
  BOOST_CHECK(ts.hasJacobian());
  BOOST_CHECK_EQUAL(ts.jacobian(), pc.jacobian);
  BOOST_CHECK_EQUAL(ts.pathLength(), pc.pathLength);
  // check that chi2 is set
  BOOST_CHECK_EQUAL(ts.chi2(), pc.chi2);

  // set SourceLink to a pointer to a fictional measurement and get it back
  auto link = Acts::SourceLink(reinterpret_cast<const xAOD::UncalibratedMeasurement *>(0xDEADBEEF));
  ts.setUncalibratedSourceLink(Acts::SourceLink{link});
  BOOST_CHECK_EQUAL(
      static_cast<const void *>(ts.getUncalibratedSourceLink()
                                .get<const xAOD::UncalibratedMeasurement *>()),
      static_cast<const void *>(link.get<const xAOD::UncalibratedMeasurement *>()));
}

BOOST_FIXTURE_TEST_CASE(InsertRefSurface, EmptyMTJ) {
  size_t ia = mtj->addTrackState(TrackStatePropMask::All);
  auto ts = mtj->getTrackState(ia);\
  auto surf = Acts::CurvilinearSurface(
            Vector3::Zero(), Vector3::UnitZ()).planeSurface();
  ts.setReferenceSurface(std::shared_ptr<const Acts::Surface>(surf));
}

// TODO remaining tests
BOOST_AUTO_TEST_SUITE_END()
