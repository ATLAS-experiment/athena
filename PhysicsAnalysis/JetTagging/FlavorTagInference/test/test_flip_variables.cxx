/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FlavorTagInference/test/test_flip_variables.cxx
 * @brief Check which constituent inputs the flip taggers invert.
 *
 * The variable lists are the ones embedded in the deployed ONNX files.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_FLIPVARIABLES

#include <boost/test/unit_test.hpp>

#include "FlavorTagInference/ConstituentsLoader.h"

#include <string>
#include <vector>

using namespace FlavorTagInference;

namespace {

  const std::string track_node {"tracks_r22loose_sd0sort"};

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

  // the flipped inputs of one node, in the order the model declares them
  std::string flipped(
    const std::string& node,
    const std::vector<std::string>& variables,
    FlipTagConfig flip_config)
  {
    const ConstituentsInputConfig config = createConstituentsLoaderConfig(
      node, variables, flip_config);
    std::string names;
    for (const InputVariableConfig& input: config.inputs) {
      if (input.flip_sign) names += (names.empty() ? "" : ", ") + input.name;
    }
    return names;
  }

}

BOOST_AUTO_TEST_SUITE(FlipVariables)

  BOOST_AUTO_TEST_CASE(standardConfigFlipsNothing) {
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3v00_tracks, FlipTagConfig::STANDARD), "");
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3epclv01_tracks, FlipTagConfig::STANDARD), "");
  }

  BOOST_AUTO_TEST_CASE(ip3dSignedNaming) {
    const std::string signed_ips {
      "IP3D_signed_d0_significance, IP3D_signed_z0_significance"};
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3v00_tracks, FlipTagConfig::FLIP_SIGN), signed_ips);
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3v00_tracks, FlipTagConfig::NEGATIVE_IP_ONLY),
      signed_ips);
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3v00_tracks, FlipTagConfig::SIMPLE_FLIP),
      "d0, z0SinTheta, " + signed_ips);
  }

  BOOST_AUTO_TEST_CASE(lifetimeSignedNaming) {
    const std::string signed_ips {
      "lifetimeSignedD0Significance, lifetimeSignedZ0SinThetaSignificance"};
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3epclv01_tracks, FlipTagConfig::FLIP_SIGN),
      signed_ips);
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3epclv01_tracks, FlipTagConfig::NEGATIVE_IP_ONLY),
      signed_ips);
    BOOST_CHECK_EQUAL(
      flipped(track_node, gn3epclv01_tracks, FlipTagConfig::SIMPLE_FLIP),
      "d0, z0SinTheta, " + signed_ips);
  }

BOOST_AUTO_TEST_SUITE_END()
