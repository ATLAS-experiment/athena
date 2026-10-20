/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "L1MuonValidation/TruthMuonSelection.h"

#include <iostream>
#include <limits>

namespace {

bool check(const bool condition, const char* description) {
  if (!condition) {
    std::cerr << "FAILED: " << description << '\n';
    return false;
  }
  return true;
}

}  // namespace

int main() {
  bool ok = true;
  const L1Muon::TruthMuonSelectionConfig common{};
  ok &= check(L1Muon::acceptTruthMuon(13, 1, 10, 500.0, 0.0, 0.2,
                                      common),
              "common selection retains low-pT barrel references");
  ok &= check(L1Muon::acceptTruthMuon(-13, 1, 10, 1000.0, -4.0, -0.2,
                                      common),
              "common selection has no detector eta acceptance cut");
  ok &= check(!L1Muon::acceptTruthMuon(13, 1, 10, -1.0, 0.5, 0.2,
                                       common),
              "common selection rejects negative pT");
  ok &= check(!L1Muon::acceptTruthMuon(
                  13, 1, 10, 5000.0,
                  std::numeric_limits<double>::infinity(), 0.2, common),
              "common selection rejects non-finite eta");

  // The TGC composition supplies this established profile explicitly. Keep
  // its acceptance boundaries fixed when broadening the common defaults.
  const L1Muon::TruthMuonSelectionConfig config{
      2000.0, 0.95, 2.50, 1, 9999};

  ok &= check(L1Muon::acceptTruthMuon(13, 1, 10, 2000.0, 1.10, 0.2,
                                      config),
              "accept positive muon at lower pT boundary");
  ok &= check(L1Muon::acceptTruthMuon(-13, 1, -10, 50000.0, -2.40, -3.0,
                                      config),
              "accept negative muon in configured eta range");
  ok &= check(L1Muon::acceptTruthMuon(13, 1, 9999, 2000.0, 0.95, 0.2,
                                      config),
              "accept exact TGC lower boundaries and barcode limit");
  ok &= check(L1Muon::acceptTruthMuon(-13, 1, -9999, 2000.0, -2.50, -0.2,
                                      config),
              "accept exact TGC upper eta boundary");

  ok &= check(!L1Muon::acceptTruthMuon(11, 1, 10, 5000.0, 1.2, 0.2,
                                       config),
              "reject non-muon PDG ID");
  ok &= check(!L1Muon::acceptTruthMuon(13, 2, 10, 5000.0, 1.2, 0.2,
                                       config),
              "reject non-required status");
  ok &= check(!L1Muon::acceptTruthMuon(13, 1, 10000, 5000.0, 1.2, 0.2,
                                       config),
              "reject barcode outside configured range");
  ok &= check(!L1Muon::acceptTruthMuon(13, 1, 10, 1999.0, 1.2, 0.2,
                                       config),
              "reject pT below configured threshold");
  ok &= check(!L1Muon::acceptTruthMuon(13, 1, 10, 5000.0, 0.94, 0.2,
                                       config),
              "reject eta below configured range");
  ok &= check(!L1Muon::acceptTruthMuon(13, 1, 10, 5000.0, 2.51, 0.2,
                                       config),
              "reject eta above configured range");
  ok &= check(!L1Muon::acceptTruthMuon(
                  13, 1, 10, std::numeric_limits<double>::quiet_NaN(),
                  1.2, 0.2, config),
              "reject non-finite pT");
  ok &= check(!L1Muon::acceptTruthMuon(
                  13, 1, 10, 5000.0, 1.2,
                  std::numeric_limits<double>::infinity(), config),
              "reject non-finite phi");

  return ok ? 0 : 1;
}
