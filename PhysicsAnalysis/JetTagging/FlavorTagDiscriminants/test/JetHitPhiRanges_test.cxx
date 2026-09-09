/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Unit test for getPhiRanges, the phi window search JetHitAssociationAlg uses
// to pull the hits near a jet out of a phi-sorted vector.

#include "../src/JetHitPhiRanges.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace FlavorTagDiscriminants;

namespace {

  // getPhiRanges only ever looks at phi. The algorithm's own hit also carries
  // z, r and a link to the xAOD object, none of which matter here.
  struct JetHit {
    float phi;
  };

  // Counting rather than asserting keeps one failure from hiding the rest.
  // An instance in main, not a file static, which the thread-safety checker
  // would rightly complain about.
  struct Checks {
    int n = 0;
    int failed = 0;
    void operator()(bool ok, const std::string& what) {
      n++;
      if(!ok) {
        failed++;
        std::cout << "FAILED: " << what << std::endl;
      }
    }
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

  // the hits the window selects, as indices into hits, in ascending order
  std::vector<size_t> foundIndices(const std::vector<JetHit>& hits,
                                   float phi, float halfWidth) {
    std::vector<size_t> found;
    for(const auto& [first, last] : getPhiRanges(hits, phi, halfWidth)) {
      for(auto it = first; it != last; ++it) {
        found.push_back(it - hits.begin());
      }
    }
    std::sort(found.begin(), found.end());
    return found;
  }

  // Compare with the answer found the slow way. Hits within a thin band around
  // the window edge are skipped: the two ways of getting there differ by a
  // float rounding, so either verdict is fine for them.
  void checkAgainstBruteForce(Checks& check, const std::vector<JetHit>& hits,
                              float phi, float halfWidth,
                              const std::string& what) {
    bool valid = true;
    for(const auto& [first, last] : getPhiRanges(hits, phi, halfWidth)) {
      if(first > last || first < hits.begin() || last > hits.end()) valid = false;
    }
    check(valid, what + ": ranges lie inside hits");

    const std::vector<size_t> found = foundIndices(hits, phi, halfWidth);
    check(std::adjacent_find(found.begin(), found.end()) == found.end(),
          what + ": no hit is counted twice");

    bool agrees = true;
    for(size_t i = 0; i < hits.size(); i++) {
      const bool isFound = std::binary_search(found.begin(), found.end(), i);
      const double dPhi = std::abs(std::remainder(hits[i].phi - double(phi), 2*M_PI));
      if(dPhi < halfWidth - 1e-4 && !isFound) agrees = false;
      if(dPhi > halfWidth + 1e-4 && isFound) agrees = false;
    }
    check(agrees, what + ": same hits as the brute force scan");
  }

}


int main() {

  Checks check;

  // Every window over a ring of hits, against the brute force answer. The
  // widths bracket the ones we run with (0.1 for the wedge, 0.4 for the cone)
  // and go past pi, where the window is the whole circle.
  const std::vector<JetHit> ring = evenlySpacedHits(64);
  for(double halfWidth : {0.0, 0.01, 0.05, 0.2, 0.4, 1.0, 3.0, M_PI, 4.0}) {
    for(int i = -100; i <= 100; i++) {
      const double phi = i * M_PI / 100;
      checkAgainstBruteForce(check, ring, phi, halfWidth,
                             "ring, phi = " + std::to_string(phi)
                             + ", halfWidth = " + std::to_string(halfWidth));
    }
  }

  // The window is closed: a hit exactly on the edge is kept. Every phi below
  // is exact in binary, so there is no rounding to allow for.
  const std::vector<JetHit> quarters =
    makeHits({-1.f, -0.5f, -0.25f, 0.f, 0.25f, 0.5f, 0.75f, 1.f});
  check(foundIndices(quarters, 0.5f, 0.25f) == std::vector<size_t>({4, 5, 6}),
        "hits on both edges are kept");
  check(foundIndices(quarters, 0.5f, 0.2f) == std::vector<size_t>({5}),
        "hits just outside the edges are dropped");

  // Hits sharing a phi, including on the edges, are kept or dropped together
  const std::vector<JetHit> ties =
    makeHits({0.f, 0.25f, 0.25f, 0.25f, 0.5f, 0.75f, 0.75f, 1.f});
  check(foundIndices(ties, 0.5f, 0.25f) == std::vector<size_t>({1, 2, 3, 4, 5, 6}),
        "repeated phi on the edges");

  // Windows that wrap: the hits at -3.125 and 3.125 are 0.033 apart across the
  // branch cut, the ones at -3.0 and 3.0 are 0.283 away from each of those.
  const std::vector<JetHit> nearPi =
    makeHits({-3.125f, -3.f, -0.5f, 0.f, 3.f, 3.125f});
  const std::vector<size_t> acrossTheCut = {0, 1, 4, 5};
  check(foundIndices(nearPi, 3.125f, 0.25f) == acrossTheCut,
        "window wrapping above +pi");
  check(foundIndices(nearPi, -3.125f, 0.25f) == acrossTheCut,
        "window wrapping below -pi");
  check(foundIndices(nearPi, 3.125f, 0.02f) == std::vector<size_t>({5}),
        "window that stops short of the cut");

  // A hit at exactly +-pi is at zero distance from a jet pointing at pi
  const float pi_f = static_cast<float>(M_PI);
  const std::vector<JetHit> atPi = makeHits({-pi_f, -1.f, 0.f, 1.f, pi_f});
  check(foundIndices(atPi, pi_f, 0.1f) == std::vector<size_t>({0, 4}),
        "hits sitting on +-pi, jet at +pi");
  check(foundIndices(atPi, -pi_f, 0.1f) == std::vector<size_t>({0, 4}),
        "hits sitting on +-pi, jet at -pi");

  // Degenerate cases
  const std::vector<JetHit> none;
  check(foundIndices(none, 0.f, 0.2f).empty(), "no hits at all");
  check(foundIndices(none, 0.f, 4.f).empty(), "no hits at all, window wider than pi");
  const std::vector<JetHit> clump = makeHits({1.f, 1.1f, 1.2f});
  check(foundIndices(clump, -1.f, 0.2f).empty(), "no hits in the window");
  check(foundIndices(clump, 1.1f, 4.f) == std::vector<size_t>({0, 1, 2}),
        "window covering the whole circle");
  check(foundIndices(clump, 1.1f, 0.f) == std::vector<size_t>({1}),
        "window of zero width, on a hit");
  check(foundIndices(clump, 1.05f, 0.f).empty(),
        "window of zero width, between hits");

  std::cout << check.n - check.failed << " of " << check.n << " checks passed"
            << std::endl;
  return check.failed > 0 ? 1 : 0;
}
