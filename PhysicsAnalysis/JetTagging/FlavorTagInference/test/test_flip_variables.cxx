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
  const std::string electron_node {"electrons_r22default"};
  const std::string bjr4_electron_node {"electrons_r22bjr_ptsort"};
  const std::string bjr4_muon_node {"muons_r22bjr_ptsort"};
  const std::string muon_node {"muons_r22default"};

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

  const std::vector<std::string> gn3epclv01_electrons {
    "pt", "ptfrac", "ptrel", "dr", "abs_eta", "eta", "phi", "ftag_et",
    "qOverP", "d0RelativeToBeamspot", "d0RelativeToBeamspotSignificance",
    "ftag_ptVarCone30OverPt", "numberOfPixelHits", "numberOfSCTHitsInclDead",
    "ftag_deltaPOverP", "eProbabilityHT", "deltaEta1", "deltaPhiRescaled2",
    "ftag_energyOverP", "Rhad", "Rhad1", "Eratio", "weta2", "Rphi", "Reta",
    "wtots1", "f1", "f3"
  };

  const std::vector<std::string> bjr4_electrons {
    "pt", "eta", "phi", "deltaEta1", "deltaPhiRescaled2", "Rhad", "Eratio",
    "weta2", "Rphi", "Reta", "wtots1", "f1", "f3", "ptfrac", "dr",
    "numberOfPixelHits", "eProbabilityHT", "qOverP",
    "numberOfSCTHitsInclDead", "d0RelativeToBeamspot"
  };

  // bJR4v01 is the only deployed network with a muon node
  const std::vector<std::string> bjr4_muons {
    "quality", "pt", "eta", "phi", "scatteringCurvatureSignificance",
    "scatteringNeighbourSignificance", "EnergyLoss", "MeasEnergyLoss",
    "segmentDeltaEta", "d0RelativeToBeamspot_MuonPrimaryTrack",
    "z0SinThetaRelativeToBeamspot_MuonPrimaryTrack",
    "d0RelativeToBeamspotVariance_MuonPrimaryTrack", "qOverP_MuonPrimaryTrack",
    "thetaVariance_MuonPrimaryTrack"
  };

  // the lifetime signed impact parameters foreseen for the muon inputs
  const std::vector<std::string> lifetime_signed_muons {
    "pt", "eta", "phi", "quality", "qOverPratio", "ptfrac", "dr",
    "lifetimeSignedD0", "lifetimeSignedZ0SinTheta",
    "lifetimeSignedD0Significance", "lifetimeSignedZ0SinThetaSignificance",
    "numberOfPixelHits", "eProbabilityHT"
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
    BOOST_CHECK_EQUAL(
      flipped(electron_node, gn3epclv01_electrons, FlipTagConfig::STANDARD), "");
    BOOST_CHECK_EQUAL(
      flipped(bjr4_muon_node, bjr4_muons, FlipTagConfig::STANDARD), "");
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

  BOOST_AUTO_TEST_CASE(electronImpactParameters) {
    BOOST_CHECK_EQUAL(
      flipped(electron_node, gn3epclv01_electrons, FlipTagConfig::SIMPLE_FLIP),
      "d0RelativeToBeamspot, d0RelativeToBeamspotSignificance");
    BOOST_CHECK_EQUAL(
      flipped(bjr4_electron_node, bjr4_electrons, FlipTagConfig::SIMPLE_FLIP),
      "d0RelativeToBeamspot");
    // the electron impact parameters are signed by the perigee, not the jet
    BOOST_CHECK_EQUAL(
      flipped(electron_node, gn3epclv01_electrons, FlipTagConfig::FLIP_SIGN), "");
  }

  BOOST_AUTO_TEST_CASE(muonImpactParameters) {
    // the variance alongside them must keep its sign
    BOOST_CHECK_EQUAL(
      flipped(bjr4_muon_node, bjr4_muons, FlipTagConfig::SIMPLE_FLIP),
      "d0RelativeToBeamspot_MuonPrimaryTrack, "
      "z0SinThetaRelativeToBeamspot_MuonPrimaryTrack");
    BOOST_CHECK_EQUAL(
      flipped(bjr4_muon_node, bjr4_muons, FlipTagConfig::FLIP_SIGN), "");
  }

  BOOST_AUTO_TEST_CASE(muonLifetimeSignedNaming) {
    const std::string signed_ips {
      "lifetimeSignedD0, lifetimeSignedZ0SinTheta, "
      "lifetimeSignedD0Significance, lifetimeSignedZ0SinThetaSignificance"};
    BOOST_CHECK_EQUAL(
      flipped(muon_node, lifetime_signed_muons, FlipTagConfig::FLIP_SIGN),
      signed_ips);
    BOOST_CHECK_EQUAL(
      flipped(muon_node, lifetime_signed_muons, FlipTagConfig::SIMPLE_FLIP),
      signed_ips);
    BOOST_CHECK_EQUAL(
      flipped(muon_node, lifetime_signed_muons, FlipTagConfig::STANDARD), "");
  }

BOOST_AUTO_TEST_SUITE_END()
