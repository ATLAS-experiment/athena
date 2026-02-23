/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/PhysliteTest.h>

//
// method implementations
//

namespace columnar
{
  namespace TestUtils
  {
    // I never figured out how the keys get calculated, so I looked
    // at what's in the input file, and hard-coded it here.
    const std::unordered_map<std::string,SG::sgkey_t> knownKeys =
    {
      {"AnalysisMuons", 0x3a6b126f},
      {"AnalysisElectrons", 0x3902fec0},
      {"AnalysisPhotons", 0x35d1472f},
      {"AnalysisJets", 0x1afd1919},
      {"egammaClusters", 0x15788d1f},
      {"GSFConversionVertices", 0x1f3e85c9},
      {"InDetTrackParticles", 0x1d3890db},
      {"CombinedMuonTrackParticles", 0x340d9196},
      {"ExtrapolatedMuonTrackParticles", 0x14e35e9f},
      {"GSFTrackParticles", 0x2e42db0b},
      {"InDetForwardTrackParticles", 0x143c6846},
      {"MuonSpectrometerTrackParticles", 0x3993c8f3},
    };
  }
}
