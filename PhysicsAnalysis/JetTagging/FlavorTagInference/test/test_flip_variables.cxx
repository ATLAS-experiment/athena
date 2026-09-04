/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Check which constituent input variables the flip taggers invert. The
// variable lists below are the ones embedded in the deployed ONNX files.

#include "FlavorTagInference/ConstituentsLoader.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace FlavorTagInference;

namespace {

  struct FlipCase {
    std::string node;
    std::vector<std::string> variables;
    FlipTagConfig flip_config;
    std::set<std::string> expected;
  };

  const std::vector<std::string> gn3v00_tracks {
    "d0", "z0SinTheta", "dphi", "deta", "qOverP",
    "IP3D_signed_d0_significance", "IP3D_signed_z0_significance",
    "phiUncertainty", "thetaUncertainty", "qOverPUncertainty",
    "numberOfPixelHits", "numberOfSCTHits",
    "numberOfInnermostPixelLayerHits", "numberOfNextToInnermostPixelLayerHits",
    "numberOfInnermostPixelLayerSharedHits",
    "numberOfInnermostPixelLayerSplitHits",
    "numberOfPixelSharedHits", "numberOfPixelSplitHits",
    "numberOfSCTSharedHits"
  };

  const std::vector<std::string> gn3epclv01_tracks {
    "d0", "z0SinTheta", "dphi", "deta", "qOverP",
    "lifetimeSignedD0Significance", "lifetimeSignedZ0SinThetaSignificance",
    "phiUncertainty", "thetaUncertainty", "qOverPUncertainty",
    "numberOfPixelHits", "numberOfSCTHits",
    "numberOfInnermostPixelLayerHits", "numberOfNextToInnermostPixelLayerHits",
    "numberOfInnermostPixelLayerSharedHits",
    "numberOfInnermostPixelLayerSplitHits",
    "numberOfPixelSharedHits", "numberOfPixelSplitHits",
    "numberOfSCTSharedHits", "leptonID", "muon_quality", "muon_qOverPratio",
    "muon_momentumBalanceSignificance", "muon_scatteringNeighbourSignificance"
  };

  std::set<std::string> flippedVariables(
    const std::string& node,
    const std::vector<std::string>& variables,
    FlipTagConfig flip_config)
  {
    const ConstituentsInputConfig config = createConstituentsLoaderConfig(
      node, variables, flip_config);
    std::set<std::string> flipped;
    for (const InputVariableConfig& input: config.inputs) {
      if (input.flip_sign) flipped.insert(input.name);
    }
    return flipped;
  }

  std::string join(const std::set<std::string>& names) {
    std::string out;
    for (const std::string& name: names) out += (out.empty() ? "" : ", ") + name;
    return "{" + out + "}";
  }

}

int main() {

  const std::set<std::string> v00_signed {
    "IP3D_signed_d0_significance", "IP3D_signed_z0_significance"
  };
  const std::set<std::string> v01_signed {
    "lifetimeSignedD0Significance", "lifetimeSignedZ0SinThetaSignificance"
  };
  const std::set<std::string> perigee {"d0", "z0SinTheta"};

  auto with_perigee = [&perigee](std::set<std::string> signed_ips) {
    signed_ips.insert(perigee.begin(), perigee.end());
    return signed_ips;
  };

  const std::vector<FlipCase> cases {
    {"tracks_r22loose_sd0sort", gn3v00_tracks, FlipTagConfig::STANDARD, {}},
    {"tracks_r22loose_sd0sort", gn3v00_tracks, FlipTagConfig::FLIP_SIGN, v00_signed},
    {"tracks_r22loose_sd0sort", gn3v00_tracks, FlipTagConfig::NEGATIVE_IP_ONLY, v00_signed},
    {"tracks_r22loose_sd0sort", gn3v00_tracks, FlipTagConfig::SIMPLE_FLIP, with_perigee(v00_signed)},
    {"tracks_r22loose_sd0sort", gn3epclv01_tracks, FlipTagConfig::STANDARD, {}},
    {"tracks_r22loose_sd0sort", gn3epclv01_tracks, FlipTagConfig::FLIP_SIGN, v01_signed},
    {"tracks_r22loose_sd0sort", gn3epclv01_tracks, FlipTagConfig::NEGATIVE_IP_ONLY, v01_signed},
    {"tracks_r22loose_sd0sort", gn3epclv01_tracks, FlipTagConfig::SIMPLE_FLIP, with_perigee(v01_signed)},
  };

  int failures = 0;
  for (const FlipCase& c: cases) {
    auto flipped = flippedVariables(c.node, c.variables, c.flip_config);
    if (flipped != c.expected) {
      failures++;
      std::cerr << "FAIL " << c.node << ": expected " << join(c.expected)
                << ", got " << join(flipped) << std::endl;
    }
  }

  if (failures) {
    std::cerr << failures << " flip configuration(s) wrong" << std::endl;
    return 1;
  }
  std::cout << "all flip configurations as expected" << std::endl;
  return 0;
}
