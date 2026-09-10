/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_HIT_PHI_RANGES_H
#define JET_HIT_PHI_RANGES_H

#include <algorithm> //lower_bound, upper_bound, is_sorted
#include <cassert>
#include <cmath>
#include <utility> //std::pair
#include <vector>

namespace FlavorTagDiscriminants {

  // The hits within halfWidth in phi of the given phi, as half-open
  // [first, last) ranges: one range, or two where the window wraps around
  // +-pi. The boundary belongs to the window, and the ranges never overlap.
  //
  // Hit is anything with a float phi; that is all this looks at. The hits must
  // be sorted by phi, and their phi, like the phi asked for, must lie in
  // [-pi, pi] -- as they do coming from std::atan2 or from an xAOD object.
  template <typename Hit>
  std::vector<std::pair<typename std::vector<Hit>::const_iterator,
                        typename std::vector<Hit>::const_iterator>>
  getPhiRanges(const std::vector<Hit>& hits, float phi, float halfWidth) {

    // the preconditions above, checked in dbg builds only
    assert(halfWidth >= 0);
    assert(std::abs(phi) <= M_PI + 1e-5);
    assert(std::is_sorted(hits.begin(), hits.end(),
                          [](const Hit& a, const Hit& b) {
                            return a.phi < b.phi;
                          }));
    // sorted, so the two ends bracket every phi in between
    assert(hits.empty() || (hits.front().phi >= -M_PI - 1e-5 &&
                            hits.back().phi <= M_PI + 1e-5));

    // first hit at or above p
    auto lower = [&hits](float p) {
      return std::lower_bound(hits.begin(), hits.end(), p,
                              [](const Hit& h, float q) { return h.phi < q; });
    };
    // first hit above p, i.e. the end of a window that includes p itself
    auto upper = [&hits](float p) {
      return std::upper_bound(hits.begin(), hits.end(), p,
                              [](float q, const Hit& h) { return q < h.phi; });
    };

    using HitItr = typename std::vector<Hit>::const_iterator;
    std::vector<std::pair<HitItr, HitItr>> ranges;
    const float lo = phi - halfWidth;
    const float hi = phi + halfWidth;
    if(halfWidth >= M_PI) {           // the whole circle
      ranges.emplace_back(hits.begin(), hits.end());
    }
    else if(lo < -M_PI) {             // wraps below -pi
      ranges.emplace_back(lower(lo + 2*M_PI), hits.end());
      ranges.emplace_back(hits.begin(), upper(hi));
    }
    else if(hi > M_PI) {              // wraps above +pi
      ranges.emplace_back(lower(lo), hits.end());
      ranges.emplace_back(hits.begin(), upper(hi - 2*M_PI));
    }
    else {
      ranges.emplace_back(lower(lo), upper(hi));
    }

    return ranges;
  }

}

#endif
