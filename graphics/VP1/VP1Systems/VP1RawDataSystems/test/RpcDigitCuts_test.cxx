/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../src/RpcDigitCuts.h"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void check(bool condition, const char* message)
{
  if (!condition)
    throw std::runtime_error(message);
}
}

int runTests()
{
  const std::array<bool RpcDigitCuts::*, 5> selections = {
    &RpcDigitCuts::showEta, &RpcDigitCuts::showPhi,
    &RpcDigitCuts::showPrimarySide, &RpcDigitCuts::showOppositeSide,
    &RpcDigitCuts::keepMissingToT
  };
  RpcDigitCuts cuts;
  for (bool phi : {false, true})
    for (bool side : {false, true})
      for (double time : {-100., 0., 10000.})
        for (double tot : {-1., 0., 10., 10000.})
          check(cuts.accepts(phi, time, tot, side), "Default cuts must retain digits");

  cuts.timeRange = VP1Interval(10, 20, false, false);
  cuts.totRange = VP1Interval(5, 15, false, false);
  for (unsigned mask = 0; mask < 32; ++mask) {
    for (unsigned bit = 0; bit < selections.size(); ++bit)
      cuts.*selections[bit] = mask & (1U << bit);
    for (bool phi : {false, true}) {
      for (bool side : {false, true}) {
        for (double time : {9.999, 10., 15., 20., 20.001}) {
          for (double tot : {-100., -1., 0., 4.999, 5., 10., 15., 15.001}) {
            const bool expected = (mask & (1U << (phi ? 1 : 0))) &&
                                  (mask & (1U << (side ? 3 : 2))) && time >= 10 && time <= 20 &&
                                  (tot < 0 ? bool(mask & 16U) : tot >= 5 && tot <= 15);
            check(cuts.accepts(phi, time, tot, side) == expected,
                  "Combined view, side, inclusive time/ToT and missing-ToT selection");
          }
        }
      }
    }
  }
  cuts = RpcDigitCuts();
  cuts.keepMissingToT = false;
  check(!cuts.accepts(false, 0, -1, false), "Exclude absent ToT");
  check(cuts.accepts(false, 0, 0, false), "Zero ToT is a measurement");
  cuts.keepMissingToT = true;
  cuts.timeRange = VP1Interval(-VP1Interval::inf(), 0, false, false);
  cuts.totRange = VP1Interval(1000, VP1Interval::inf(), false, false);
  check(cuts.accepts(true, -10, 1000, true), "One-sided bounds");
  check(cuts.accepts(true, 0, -1, true), "Absent ToT bypasses finite ToT bounds when kept");
  check(!cuts.accepts(true, .001, -1, true), "Missing ToT still obeys time cut");
  check(!cuts.accepts(true, -10, 999.999, true), "Measured ToT obeys lower bound");
  cuts.timeRange = VP1Interval();
  check(!cuts.accepts(false, 0, -1, false), "Empty time range rejects missing ToT too");
  cuts = RpcDigitCuts();
  cuts.totRange = VP1Interval();
  check(!cuts.accepts(false, 0, -1, false) && !cuts.accepts(false, 0, 10, false),
        "Inverted/empty ToT range rejects all digits");
  cuts = RpcDigitCuts();
  for (double invalid : {std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()}) {
    check(!cuts.accepts(false, invalid, 10, false), "Non-finite time rejected");
    check(!cuts.accepts(false, 0, invalid, false), "Non-finite ToT rejected");
  }
  std::cout << "RPC cut selections, ranges, boundaries and missing ToT passed\n";
  return 0;
}

int main()
{
  try {
    return runTests();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
