/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Unit test for getPhiRanges, the phi window search JetHitAssociationAlg uses
// to pull the hits near a jet out of a phi-sorted vector.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_JetHitPhiRanges

#include <boost/test/unit_test.hpp>

#include "../src/JetHitPhiRanges.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace FlavorTagDiscriminants;

namespace {

  // getPhiRanges only ever looks at phi. The algorithm's own hit also carries
  // z, r and a link to the xAOD object, none of which matter here.
  struct JetHit {
    float phi;
  };

  std::vector<JetHit> makeHits(const std::vector<float>& phis) {
    std::vector<JetHit> hits;
    for(float phi : phis) hits.push_back({phi});
    return hits;
  }

  // n hits evenly spaced over the circle, offset by half a step so that none
  // of them sits exactly on -pi
  std::vector<JetHit> evenlySpacedHits(size_t n) {
    std::vector<float> phis;
    for(size_t i = 0; i < n; i++) {
      phis.push_back(-M_PI + 2*M_PI*(i + 0.5)/n);
    }
    return makeHits(phis);
  }

  // The phis the window selects, in ascending order. No two hits below share a
  // phi, so these name the hits and can be read straight off the input.
  std::vector<float> foundPhis(const std::vector<JetHit>& hits,
                               float phi, float halfWidth) {
    std::vector<float> found;
    for(const auto& [first, last] : getPhiRanges(hits, phi, halfWidth)) {
      for(auto it = first; it != last; ++it) found.push_back(it->phi);
    }
    std::sort(found.begin(), found.end());
    return found;
  }

  // Compare with the answer found the slow way. Hits within a thin band around
  // the window edge are skipped: the two ways of getting there differ by a
  // float rounding, so either verdict is fine for them.
  void checkAgainstBruteForce(const std::vector<JetHit>& hits,
                              float phi, float halfWidth) {
    bool valid = true;
    for(const auto& [first, last] : getPhiRanges(hits, phi, halfWidth)) {
      if(first > last || first < hits.begin() || last > hits.end()) valid = false;
    }
    BOOST_CHECK_MESSAGE(valid, "ranges lie inside hits");

    const std::vector<float> found = foundPhis(hits, phi, halfWidth);
    BOOST_CHECK_MESSAGE(
      std::adjacent_find(found.begin(), found.end()) == found.end(),
      "no hit is counted twice");

    bool agrees = true;
    for(const JetHit& hit : hits) {
      const bool isFound = std::binary_search(found.begin(), found.end(), hit.phi);
      const double dPhi = std::abs(std::remainder(hit.phi - double(phi), 2*M_PI));
      if(dPhi < halfWidth - 1e-4 && !isFound) agrees = false;
      if(dPhi > halfWidth + 1e-4 && isFound) agrees = false;
    }
    BOOST_CHECK_MESSAGE(agrees, "same hits as the brute force scan");
  }

}


BOOST_AUTO_TEST_SUITE( JetHitPhiRangesTest )

// Every window over a ring of hits, against the brute force answer. The widths
// bracket the ones we run with (0.1 for the wedge, 0.4 for the cone) and go
// past pi, where the window is the whole circle. The jet phi sweeps the full
// circle, so the windows wrapping at +-pi are covered here too -- including the
// ones where a wrapped half comes back empty.
BOOST_AUTO_TEST_CASE( against_the_brute_force_answer )
{
  const std::vector<JetHit> ring = evenlySpacedHits(64);
  for(double halfWidth : {0.0, 0.01, 0.05, 0.2, 0.4, 1.0, 3.0, M_PI, 4.0}) {
    for(int i = -100; i <= 100; i++) {
      const double phi = i * M_PI / 100;
      // one case covers ~1800 windows, so name the one that fails
      BOOST_TEST_CONTEXT("phi = " << phi << ", halfWidth = " << halfWidth) {
        checkAgainstBruteForce(ring, phi, halfWidth);
      }
    }
  }
}

// That the window is closed is the one thing the scan above cannot see: it
// lets either verdict stand for a hit within a rounding of the edge. The phis
// here are all exact in binary, so there is nothing to round.
BOOST_AUTO_TEST_CASE( the_window_is_closed )
{
  const std::vector<JetHit> quarters =
    makeHits({-1.f, -0.5f, -0.25f, 0.f, 0.25f, 0.5f, 0.75f, 1.f});

  const std::vector<float> onEdge = foundPhis(quarters, 0.5f, 0.25f);
  const std::vector<float> kept{0.25f, 0.5f, 0.75f};
  BOOST_CHECK_EQUAL_COLLECTIONS(onEdge.begin(), onEdge.end(),
                                kept.begin(), kept.end());

  const std::vector<float> outside = foundPhis(quarters, 0.5f, 0.2f);
  const std::vector<float> dropped{0.5f};
  BOOST_CHECK_EQUAL_COLLECTIONS(outside.begin(), outside.end(),
                                dropped.begin(), dropped.end());
}

// A hit at exactly +-pi is at zero distance from a jet pointing at pi
BOOST_AUTO_TEST_CASE( hits_sitting_on_pi )
{
  const float pi_f = static_cast<float>(M_PI);
  const std::vector<JetHit> atPi = makeHits({-pi_f, -1.f, 0.f, 1.f, pi_f});

  const std::vector<float> found = foundPhis(atPi, pi_f, 0.1f);
  const std::vector<float> expected{-pi_f, pi_f};
  BOOST_CHECK_EQUAL_COLLECTIONS(found.begin(), found.end(),
                                expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE( no_hits_at_all )
{
  const std::vector<JetHit> none;
  BOOST_CHECK(foundPhis(none, 0.f, 0.2f).empty());
}

BOOST_AUTO_TEST_CASE( no_hits_in_the_window )
{
  const std::vector<JetHit> clump = makeHits({1.f, 1.1f, 1.2f});
  BOOST_CHECK(foundPhis(clump, -1.f, 0.2f).empty());
}

BOOST_AUTO_TEST_CASE( window_covering_the_whole_circle )
{
  const std::vector<JetHit> clump = makeHits({1.f, 1.1f, 1.2f});

  const std::vector<float> found = foundPhis(clump, 1.1f, 4.f);
  const std::vector<float> expected{1.f, 1.1f, 1.2f};
  BOOST_CHECK_EQUAL_COLLECTIONS(found.begin(), found.end(),
                                expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE( window_of_zero_width )
{
  const std::vector<JetHit> clump = makeHits({1.f, 1.1f, 1.2f});

  const std::vector<float> onHit = foundPhis(clump, 1.1f, 0.f);
  const std::vector<float> expected{1.1f};
  BOOST_CHECK_EQUAL_COLLECTIONS(onHit.begin(), onHit.end(),
                                expected.begin(), expected.end());

  BOOST_CHECK(foundPhis(clump, 1.05f, 0.f).empty());
}

BOOST_AUTO_TEST_SUITE_END()
