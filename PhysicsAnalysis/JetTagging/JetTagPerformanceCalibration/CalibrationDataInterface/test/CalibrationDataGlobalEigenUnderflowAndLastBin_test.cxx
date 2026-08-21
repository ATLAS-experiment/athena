/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Sanity test for the combined SFGlobalEigen eigen-variations. They are read
// back and checked that the variation lands in the expected bins: the ROOT
// underflow bin (0) must carry no variation and the last real calibration bin
// (N) must be varied.
// Pure variation per bin: 0.5*|up(b)-down(b)|, so no nominal is needed.
// CDI path defaults to the latest MC20 GN2v01 file; override via argv.

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CalibrationDataInterface/CalibrationDataEigenVariations.h"

#include "PathResolver/PathResolver.h"

#include "TFile.h"
#include "TH1.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {

  const std::string cdiArg = (argc > 1) ? argv[1]
    : "xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root";
  const std::string tagger = (argc > 2) ? argv[2] : "GN2v01";
  const std::string wp     = (argc > 3) ? argv[3] : "Continuous";
  const std::string jc     = (argc > 4) ? argv[4] : "AntiKt4EMPFlowJets";
  const std::vector<std::string> flavs = {"B", "C", "T", "Light"};

  const std::string cdi = PathResolverFindCalibFile(cdiArg);
  if (cdi.empty()) { std::cerr << "cannot resolve " << cdiArg << "\n"; return EXIT_FAILURE; }

  std::unique_ptr<TFile> f{ TFile::Open(cdi.c_str(), "READ") };
  if (!f || f->IsZombie()) { std::cerr << "cannot open " << cdi << "\n"; return EXIT_FAILURE; }

  // representative container for the base constructor; the global class
  // self-loads all per-flavour containers from the CDI path internally.
  Analysis::CalibrationDataHistogramContainer* cB = nullptr;
  f->GetObject((tagger + "/" + jc + "/" + wp + "/B/default_SF").c_str(), cB);
  if (!cB) {
    std::cerr << "No B/default_SF container under " << tagger << "/" << jc << "/" << wp
              << " -- use an SF WP with per-flavour containers (e.g. Continuous).\n";
    return EXIT_FAILURE;
  }

  Analysis::CalibrationDataGlobalEigenVariations gev(cdi, tagger, wp, jc, flavs, cB, false);
  gev.initialize();

  // read the combined eigen-variations through a base-class reference
  Analysis::CalibrationDataEigenVariations& base = gev;
  const unsigned int nev = base.getNumberOfEigenVariations();

  std::cout << "\nSFGlobalEigen eigen-variation bin-coverage test\n";
  std::cout << "CDI       : " << cdi << "\n";
  std::cout << "tagger/WP : " << tagger << " / " << wp << "\n";
  std::cout << "combined eigenvariations : " << nev << "\n";

  if (nev == 0) { std::cerr << "no eigenvariations produced\n"; return EXIT_FAILURE; }

  int    nbins = 0;
  double max_underflow_var = 0.0;   // (A) should be exactly 0
  double max_lastbin_var   = 0.0;   // (B) should be > 0
  double max_interior_var  = 0.0;   // sanity reference
  int    ev_with_underflow = 0;

  for (unsigned int i = 0; i < nev; ++i) {
    TH1* up = nullptr; TH1* down = nullptr;
    if (!base.getEigenvectorVariation(i, up, down)) continue;
    nbins = up->GetNbinsX();

    const double uf   = 0.5 * std::fabs(up->GetBinContent(0)     - down->GetBinContent(0));
    const double last = 0.5 * std::fabs(up->GetBinContent(nbins) - down->GetBinContent(nbins));
    if (uf   > max_underflow_var) max_underflow_var = uf;
    if (last > max_lastbin_var)   max_lastbin_var   = last;
    if (uf > 1e-12) ++ev_with_underflow;

    for (int b = 1; b < nbins; ++b) {
      const double d = 0.5 * std::fabs(up->GetBinContent(b) - down->GetBinContent(b));
      if (d > max_interior_var) max_interior_var = d;
    }
  }

  std::cout << "combined histogram real bins (N) : " << nbins << "\n";
  std::cout << "(A) max variation in underflow bin (0) : " << max_underflow_var
            << "   [correct = 0]   (" << ev_with_underflow << "/" << nev << " affected)\n";
  std::cout << "(B) max variation in last real bin (N) : " << max_lastbin_var << "   [correct > 0]\n";
  std::cout << "    max variation in interior bins     : " << max_interior_var << "   (reference)\n";

  const bool sigA = (max_underflow_var > 1e-12);
  const bool sigB = (max_lastbin_var <= 1e-12 && max_interior_var > 1e-12);

  if (sigA || sigB) {
    std::cout << "TEST FAILED: eigen-variations do not cover the expected bins\n";
    if (sigA) std::cout << " (A) underflow bin carries a real eigen-variation\n";
    if (sigB) std::cout << " (B) last real bin is never varied\n";
    return EXIT_FAILURE;
  }

  std::cout << "OK: no underflow variation and last real bin is varied.\n";
  return EXIT_SUCCESS;
}
