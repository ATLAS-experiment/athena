/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Regression test for the in-place corruption of cached eigen-variations.
// EigenVectorRecomposition() is a query and must be side-effect free, so calling
// it twice on the same instance must give the same coefficients. The buggy code
// did up->Add(nom,-1) on a histogram owned by m_eigen, so the second call
// double-subtracted the nominal and the coefficients changed. Fails on the buggy
// code, passes with the unique_ptr fix.
// CDI path defaults to the latest MC20 GN2v01 file; override via argv.

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CalibrationDataInterface/CalibrationDataEigenVariations.h"

#include "PathResolver/PathResolver.h"

#include "TFile.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>

int main(int argc, char* argv[]) {

  const std::string cdiArg = (argc > 1) ? argv[1]
    : "xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root";
  const std::string tagger = (argc > 2) ? argv[2] : "GN2v01";
  const std::string wp     = (argc > 3) ? argv[3] : "Continuous";
  const std::string jc     = (argc > 4) ? argv[4] : "AntiKt4EMPFlowJets";
  const std::string label  = (argc > 5) ? argv[5] : "B";

  const std::string cdi = PathResolverFindCalibFile(cdiArg);
  if (cdi.empty()) { std::cerr << "cannot resolve " << cdiArg << "\n"; return EXIT_FAILURE; }

  std::unique_ptr<TFile> f{ TFile::Open(cdi.c_str(), "READ") };
  if (!f || f->IsZombie()) { std::cerr << "cannot open " << cdi << "\n"; return EXIT_FAILURE; }

  Analysis::CalibrationDataHistogramContainer* cnt = nullptr;
  f->GetObject((tagger + "/" + jc + "/" + wp + "/" + label + "/default_SF").c_str(), cnt);
  if (!cnt) {
    std::cerr << "No " << label << "/default_SF container under " << tagger << "/" << jc << "/" << wp
              << " -- use an SF WP with per-flavour containers (e.g. Continuous).\n";
    return EXIT_FAILURE;
  }

  Analysis::CalibrationDataEigenVariations eigen(cdi, tagger, wp, jc, cnt, false);
  eigen.initialize();

  // two identical calls of a query that must be side-effect free
  std::map<std::string, std::map<std::string, float>> m1, m2;
  if (!eigen.EigenVectorRecomposition(label, m1)) {
    std::cerr << "first EigenVectorRecomposition() failed\n"; return EXIT_FAILURE;
  }
  if (!eigen.EigenVectorRecomposition(label, m2)) {
    std::cerr << "second EigenVectorRecomposition() failed\n"; return EXIT_FAILURE;
  }

  double maxabs = 0.0, sumabs = 0.0;
  int n = 0;
  std::string worst;
  for (const auto& [ev, inner1] : m1) {
    auto it = m2.find(ev);
    if (it == m2.end()) continue;
    for (const auto& [np, c1] : inner1) {
      const double c2 = it->second.count(np) ? it->second.at(np) : 0.0;
      const double d  = std::fabs(c1 - c2);
      sumabs += d; ++n;
      if (d > maxabs) { maxabs = d; worst = ev + " / " + np; }
    }
  }

  std::cout << "\nEigenVectorRecomposition in-place corruption regression test\n";
  std::cout << "CDI           : " << cdi << "\n";
  std::cout << "tagger/WP     : " << tagger << " / " << wp << "\n";
  std::cout << "flavour label : " << label << "\n";
  std::cout << "eigenvectors    : " << m1.size() << "\n";
  std::cout << "coeffs compared : " << n << "\n";
  std::cout << "max |coef(call1) - coef(call2)| : " << maxabs << "  (at " << worst << ")\n";
  std::cout << "mean |difference|               : " << (n ? sumabs / n : 0.0) << "\n";

  if (m1.empty() || n == 0) {
    std::cerr << "no coefficients produced -- cannot evaluate the test\n";
    return EXIT_FAILURE;
  }

  if (maxabs > 1e-6) {
    std::cout << "TEST FAILED: query is not side-effect free; the recomposition\n"
                 "mutated the cached eigen-variations (up->Add(nom,-1) on an owned pointer).\n";
    return EXIT_FAILURE;
  }

  std::cout << "OK: repeated calls give identical coefficients -- cache is not corrupted.\n";
  return EXIT_SUCCESS;
}
