/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/ConstituentsLoader.h"
#include <regex>

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
      input.type = str::match_first(type_regexes, input.name,
                                "iparticle type matching");
      config.inputs.push_back(std::move(input));
    }
    return config;
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
    config.order = str::match_first(sort_regexes, name,
                              "track order matching");
    config.selection = str::match_first(select_regexes, name,
                                  "track selection matching");
    for (const auto& varname: input_variables) {
      InputVariableConfig input;
      input.name = varname;
      input.type = str::match_first(type_regexes, varname,
                                "track type matching");

      input.flip_sign=false;
      if ((flip_config != FlipTagConfig::STANDARD) && std::regex_match(varname, re)){
        input.flip_sign=true;
      }
      config.inputs.push_back(std::move(input));
    }
    return config;
  }
}

  ConstituentsInputConfig get_hits_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes) {
    ConstituentsInputConfig config;
    config.name = name;
    for (const auto& varname: input_variables) {
      InputVariableConfig input;      
      input.name = varname;
      input.type = str::match_first(type_regexes, input.name,
                                "hits type matching");
      input.flip_sign = false;
      config.inputs.push_back(std::move(input));
    }
    return config;
  }

  ConstituentsInputConfig get_electron_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes,
    const SelRegexes& select_regexes
    ) {
    ConstituentsInputConfig config;
    config.name = name;
    config.order = ConstituentsSortOrder::PT_DESCENDING;
    config.selection = str::match_first(select_regexes, name,
                                  "electron selection matching");
    for (const auto& varname: input_variables) {
      InputVariableConfig input;
      input.name = varname;
      input.type = str::match_first(type_regexes, input.name,
                                "electron type matching");
      input.flip_sign = false;
      config.inputs.push_back(std::move(input));
    }
    return config;
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
          // custom variables
          {"(ftag_.*|ptfrac|ptrel|dr|"
               "et|deltaPOverP|ptVarCone30OverPt|energyOverP)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          // variables extracted from the corresponding track
          {"(numberOf.*|d0.*|abs_eta|qOverP|eProbabilityHT)"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      TypeRegexes hits_type_regexes {
          // hits variables
          // ConstituentsEDMType picked correspond to the first matching regex
          {"(j|a|b)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(deta|dphi|radius_local)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(splitProbability1|splitProbability2)"_r, ConstituentsEDMType::CUSTOM_GETTER},
      };
      TypeRegexes flow_type_regexes {
          // FlowElement variables
          // ConstituentsEDMType picked correspond to the first matching regex
          {"(pt|deta|dphi|dr|energy|isCharged)"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      TypeRegexes trk_type_regexes {
          // Some innermost / next-to-innermost hit variables had a different
          // definition in 21p9, recomputed here with customGetter to reuse
          // existing training
          // ConstituentsEDMType picked correspond to the first matching regex
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
        {".*_r22default.*"_r, ConstituentsSelection::R22_DEFAULT}
      };
      
      if (name.find("tracks") != std::string::npos){
        std::regex flip_sequences;
        if (flip_config == FlipTagConfig::FLIP_SIGN || flip_config == FlipTagConfig::NEGATIVE_IP_ONLY){
          flip_sequences=std::regex(".*signed_[dz]0.*");
        }
        if (flip_config == FlipTagConfig::SIMPLE_FLIP){
          flip_sequences=std::regex("(.*signed_[dz]0.*)|d0|z0SinTheta");
        }
        config = get_track_input_config(
          name, input_variables,
          trk_type_regexes, trk_sort_regexes, trk_select_regexes,
          flip_sequences, flip_config);
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
        config = get_electron_input_config(
          name, input_variables,
          electron_type_regexes,
          electron_select_regexes);
        config.type = ConstituentsType::ELECTRON;
        config.output_name = "electrons";
      }
      else{
        throw std::runtime_error(
          "Unknown constituent type: " + name + ". Only tracks, flows, hits and electrons are supported."
          );
      }
      return config;
    }
}
