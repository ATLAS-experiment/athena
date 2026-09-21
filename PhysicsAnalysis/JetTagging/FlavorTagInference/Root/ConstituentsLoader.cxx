/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/StringUtils.h"
#include <regex>
#include <utility> //std::pair

namespace {
  using namespace FlavorTagInference;

  // define a regex literal operator
  std::regex operator "" _r(const char* c, size_t /* length */) {
    return std::regex(c);
  }

  // ____________________________________________________________________
  //
  // We define a few structures to map variable names to type, default
  // value, etc.
  //
  typedef std::vector<std::pair<std::regex, ConstituentsEDMType>> TypeRegexes;
  typedef std::vector<std::pair<std::regex, std::string>> StringRegexes;
  typedef std::vector<std::pair<std::regex, ConstituentsSortOrder>> SortRegexes;
  typedef std::vector<std::pair<std::regex, ConstituentsSelection>> SelRegexes;
  
  ConstituentsInputConfig get_flow_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes) {
    ConstituentsInputConfig config;
    config.name = name;
    config.order = ConstituentsSortOrder::PT_DESCENDING;
    const std::string typeMatchStr{"iparticle type matching"};
    for (const auto& varname: input_variables) {
      InputVariableConfig input;
      size_t pos = varname.find("flow_");
      if (pos != std::string::npos){
        input.name = varname.substr(pos+5);
      }
      else{
        input.name = varname;
      }
      input.flip_sign = false;
      input.type = str::match_first(type_regexes, input.name, typeMatchStr);
      config.inputs.push_back(std::move(input));
    }
    return config;
  }

  // Impact parameters whose sign the flip taggers invert. Each quantity is
  // listed under every name the custom getters accept for it, see
  // CustomGetterUtils; variances and uncertainties are never flipped.
  std::regex flip_variable_regex(FlipTagConfig flip_config) {
    // lifetime sign, referenced to the jet axis
    const std::string jet_signed =
      "IP2D_signed_d0|IP3D_signed_[dz]0(_significance)?"
      "|lifetimeSigned(D0|Z0SinTheta)(Significance)?";
    // perigee sign, no jet reference
    const std::string perigee_signed =
      "(btagIp_)?(d0|z0SinTheta)"
      "|(d0|z0|z0SinTheta)RelativeToBeamspot(Significance)?";
    const std::string suffix = "(_MuonPrimaryTrack)?";
    if (flip_config == FlipTagConfig::SIMPLE_FLIP) {
      return std::regex("(" + jet_signed + "|" + perigee_signed + ")" + suffix);
    }
    return std::regex("(" + jet_signed + ")" + suffix);
  }

  ConstituentsInputConfig get_track_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const SelRegexes& select_regexes,
    const std::regex& re,
    const FlipTagConfig& flip_config) {
    ConstituentsInputConfig config = {};
    config.name = name;
    config.order = str::match_first(sort_regexes, name, "track order matching");
    config.selection = str::match_first(select_regexes, name, "track selection matching");
    const std::string typeMatchStr{"track type matching"};
    for (const auto& varname: input_variables) {
      InputVariableConfig input;
      input.name = varname;
      input.type = str::match_first(type_regexes, varname,typeMatchStr);
      input.flip_sign=false;
      if ((flip_config != FlipTagConfig::STANDARD) && std::regex_match(varname, re)){
        input.flip_sign=true;
      }
      config.inputs.push_back(std::move(input));
    }
    return config;
  }
  ConstituentsInputConfig get_hits_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes) {
    ConstituentsInputConfig config;
    config.name = name;
    const std::string typeMatchStr{"hits type matching"};
    for (const auto& varname: input_variables) {
      InputVariableConfig input;      
      input.name = varname;
      input.type = str::match_first(type_regexes, input.name, typeMatchStr);
      input.flip_sign = false;
      config.inputs.push_back(std::move(input));
    }
    return config;
  }

  ConstituentsInputConfig get_lepton_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes,
    const SelRegexes& select_regexes,
    const std::regex& re,
    const FlipTagConfig& flip_config
    ) {
    ConstituentsInputConfig config;
    config.name = name;
    // leptons are ordered by pt, so unlike the tracks there is no
    // ordering to reverse, and NEGATIVE_IP_ONLY does not drop any of them
    config.order = ConstituentsSortOrder::PT_DESCENDING;
    const std::string typeMatchStr{"lepton type matching"};
    config.selection = str::match_first(select_regexes, name,
                                  "lepton selection matching");
    for (const auto& varname: input_variables) {
      InputVariableConfig input;
      input.name = varname;
      input.type = str::match_first(type_regexes, input.name, typeMatchStr);
      input.flip_sign = (flip_config != FlipTagConfig::STANDARD)
        && std::regex_match(varname, re);
      config.inputs.push_back(std::move(input));
    }
    return config;
  }
}

namespace FlavorTagInference {
    //
    // Create a configuration for the constituents loaders
    //
    ConstituentsInputConfig createConstituentsLoaderConfig(
      const std::string & name,
      const std::vector<std::string> & input_variables,
      FlipTagConfig flip_config
    ){
      ConstituentsInputConfig config;
      TypeRegexes electron_type_regexes {
          // default electron variables
          {"(deltaEta1|deltaPhiRescaled2|Rhad|Rhad1|"
               "Eratio|weta2|Rphi|Reta|wtots1|f1|f3|pt|eta|phi)"_r, ConstituentsEDMType::FLOAT},
          // truth labels
          {"ftagTruth.*"_r, ConstituentsEDMType::INT},
          // custom variables that require special computation
          {"(ftag_et|ftag_deltaPOverP|ftag_energyOverP|ftag_ptVarCone30OverPt|"
               "ptfrac|ptrel|dr|deta|dphi|et|deltaPOverP|ptVarCone30OverPt|energyOverP)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          // ftag_ float decorations (SoftElectronDecoratorAlg, ElectronGSFTrackDecoratorAlg)
          {"ftag_.*"_r, ConstituentsEDMType::FLOAT},
          // variables extracted from the corresponding track
          {"(numberOf.*|d0.*|abs_eta|qOverP|eProbabilityHT)"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      TypeRegexes muon_type_regexes {
          // default muon variables
          {"(pt|eta|phi|momentumBalanceSignificance|"
               "scatteringNeighbourSignificance|scatteringCurvatureSignificance|"
               "segmentDelta.*|EnergyLoss|ParamEnergyLoss.*|MeasEnergyLoss.*|"
               "CaloMuonScore)"_r, ConstituentsEDMType::FLOAT},
          {"(quality)"_r, ConstituentsEDMType::UCHAR},
          // custom variables
          {"(ptfrac|ptrel|dr|qOverPratio)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          // variables extracted from the corresponding track
          {"(^.*)?(D|Z)0.*"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(numberOf.*|expect.*|eProbabilityHT|qOverP)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          // Extra variables for bJR4 that use primary track associated to muon instead of ID track
          {"(d0|z0SinTheta|theta|qOverP)(RelativeToBeamspot)?(Variance)?(_MuonPrimaryTrack)?"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      TypeRegexes hits_type_regexes {
          // hits variables
          // ConstituentsEDMType picked correspond to the first matching regex
          {"(j|a|b)"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      TypeRegexes flow_type_regexes {
          // FlowElement variables
          // ConstituentsEDMType picked correspond to the first matching regex
          {"(eta|phi)"_r, ConstituentsEDMType::FLOAT},
          {"(pt|deta|dphi|dr|energy|isCharged)"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      TypeRegexes trk_type_regexes {
          // Some innermost / next-to-innermost hit variables had a different
          // definition in 21p9, recomputed here with customGetter to reuse
          // existing training
          // ConstituentsEDMType picked correspond to the first matching regex
          {"ftagTruth.*"_r, ConstituentsEDMType::INT},
          {"numberOf.*21p9"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"numberOf.*"_r, ConstituentsEDMType::UCHAR},
          {"btagIp_(d0|z0SinTheta)Uncertainty"_r, ConstituentsEDMType::FLOAT},
          {"(numberDoF|chiSquared|qOverP|theta)"_r, ConstituentsEDMType::FLOAT},
          {"(^.*[_])?(d|z)0.*"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(log_)?(ptfrac|dr|pt).*"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(deta|dphi)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"phi|theta|qOverP"_r, ConstituentsEDMType::FLOAT},
          {"(phi|theta|qOverP)Uncertainty"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(leptonID|muon_quality)"_r, ConstituentsEDMType::CHAR},
          {"(pT_wrtJet|pZ_wrtJet|EFrac_wrtJet).*"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"muon_(qOverPratio|momentumBalanceSignificance|scatteringNeighbourSignificance)"_r, ConstituentsEDMType::FLOAT},
          {"lifetimeSigned.*"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      // We have a number of special naming conventions to sort and
      // filter tracks. The track nodes should be named according to
      //
      // tracks_<selection>_<sort-order>
      //
      SortRegexes trk_sort_regexes {
          {".*absSd0sort"_r, ConstituentsSortOrder::ABS_D0_SIGNIFICANCE_DESCENDING},
          {".*sd0sort"_r, ConstituentsSortOrder::D0_SIGNIFICANCE_DESCENDING},
          {".*ptsort"_r, ConstituentsSortOrder::PT_DESCENDING},
          {".*absD0DescendingSort"_r, ConstituentsSortOrder::ABS_D0_DESCENDING},
      };
      SelRegexes trk_select_regexes {
          {".*_ip3d_.*"_r, ConstituentsSelection::IP3D_2018},
          {".*_dipsTightUpgrade_.*"_r, ConstituentsSelection::DIPS_TIGHT_UPGRADE},
          {".*_dipsLooseUpgrade_.*"_r, ConstituentsSelection::DIPS_LOOSE_UPGRADE},
          {".*_all_.*"_r, ConstituentsSelection::ALL},
          {".*_dipsLoose202102_.*"_r, ConstituentsSelection::DIPS_LOOSE_202102},
          {".*_loose202102NoIpCuts_.*"_r, ConstituentsSelection::LOOSE_202102_NOIP},
          {".*_r22default_.*"_r, ConstituentsSelection::R22_DEFAULT},
          {".*_r22loose_.*"_r, ConstituentsSelection::R22_LOOSE},
      };

      // For now we have only one selection for electrons
      SelRegexes electron_select_regexes {
        {".*_r22default.*"_r, ConstituentsSelection::R22_DEFAULT},
        {".*_r22bjr.*"_r, ConstituentsSelection::R22_BJR}
      };
      // And one for muons
      SelRegexes muon_select_regexes {
        {".*_r22default.*"_r, ConstituentsSelection::R22_DEFAULT},
        {".*_r22bjr.*"_r, ConstituentsSelection::R22_BJR}
      };
      
      const std::regex flip_variables = flip_variable_regex(flip_config);

      if (name.find("tracks") != std::string::npos){
        config = get_track_input_config(
          name, input_variables,
          trk_type_regexes, trk_sort_regexes, trk_select_regexes,
          flip_variables, flip_config);
        config.type = ConstituentsType::TRACK;
        config.output_name = "tracks";
      }
      else if (name.find("flows") != std::string::npos){
        config = get_flow_input_config(
          name, input_variables,
          flow_type_regexes);
        config.type = ConstituentsType::FLOW_ELEMENT;
        config.output_name = "flows";
      }
      else if (name.find("hits") != std::string::npos){
        config = get_hits_input_config(
          name, input_variables,
          hits_type_regexes);
        config.type = ConstituentsType::HIT;
        config.output_name = "hits";
      }
      else if (name.find("electrons") != std::string::npos){
        config = get_lepton_input_config(
          name, input_variables,
          electron_type_regexes,
          electron_select_regexes,
          flip_variables, flip_config);
        config.type = ConstituentsType::ELECTRON;
        config.output_name = "electrons";
      }
      else if (name.find("muons") != std::string::npos){
        config = get_lepton_input_config(
          name, input_variables,
          muon_type_regexes,
          muon_select_regexes,
          flip_variables, flip_config);
        config.type = ConstituentsType::MUON;
        config.output_name = "muons";
      }
      else if (name.find("clusters") != std::string::npos
               && name.find("taucluster") == std::string::npos){
        TypeRegexes cluster_type_regexes {
            // All CaloCluster variables are handled by custom getters:
            // moments, kinematics, samplings, flags, and IParticle vars
            {".*"_r, ConstituentsEDMType::CUSTOM_GETTER}
        };
        config = get_flow_input_config(
          name, input_variables,
          cluster_type_regexes);
        config.type = ConstituentsType::CALO_CLUSTER;
        config.output_name = "clusters";
      }
      else if (name.find("towers") != std::string::npos){
        // Towers are CaloCluster objects accessed via GhostTower links.
        // All variables are custom getters (same as clusters).
        TypeRegexes tower_type_regexes {
            {".*"_r, ConstituentsEDMType::CUSTOM_GETTER}
        };
        config = get_flow_input_config(
          name, input_variables,
          tower_type_regexes);
        config.type = ConstituentsType::TOWER;
        config.output_name = "towers";
      }
      else{
        throw std::runtime_error(
          "Unknown constituent type: " + name + ". Only tracks, flows, hits, electrons, muons, clusters and towers are supported."
          );
      }
      return config;
    }
}
