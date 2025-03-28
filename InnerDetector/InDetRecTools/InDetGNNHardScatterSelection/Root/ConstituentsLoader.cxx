/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "FlavorTagInference/StringUtils.h"

namespace {
  using namespace InDetGNNHardScatterSelection;

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
  
  ConstituentsInputConfig get_iparticle_input_config(
    const std::string& name,
    const std::vector<std::string>& input_variables,
    const TypeRegexes& type_regexes) {
    ConstituentsInputConfig config;
    config.name = name;
    config.order = ConstituentsSortOrder::PT_DESCENDING;
    for (const auto& varname: input_variables) {
      InputVariableConfig input;
      input.name = varname;
      input.type = FlavorTagInference::str::match_first(type_regexes, input.name,
                                "iparticle type matching");

      config.inputs.push_back(input);
    }
    return config;
  }
}

namespace InDetGNNHardScatterSelection {
    //
    // Create a configuration for the constituents loaders
    //
    ConstituentsInputConfig createConstituentsLoaderConfig(
      std::string name,
      std::vector<std::string> input_variables
    ){
      ConstituentsInputConfig config;

      TypeRegexes iparticle_type_regexes {
          // iparticle variables
          // ConstituentsEDMType picked correspond to the first matching regex
  //        {"(photon_deltaZ|photon_deltaZ_wBeamSpot)"_r, ConstituentsEDMType::FLOAT},
          {"(pt|eta|phi|energy|deltaZ0|vertexWeight|ntracks_ga|photon_deltaZ|photon_deltaZ_wBeamSpot)"_r, ConstituentsEDMType::CUSTOM_GETTER}
      };
      
      if (name.find("tracks_all_sd0sort") != std::string::npos){
        config = get_iparticle_input_config(
          name, input_variables,
          iparticle_type_regexes);
        config.type = ConstituentsType::TRACK;
        config.link_name = "trackParticleLinks";
        config.output_name = "track_features";
      }
      else if (name.find("electrons") != std::string::npos){
        config = get_iparticle_input_config(
          name, input_variables,
          iparticle_type_regexes);
        config.type = ConstituentsType::ELECTRON;
        config.link_name = "electronLinks";
        config.output_name = "electron_features";
      }
      else if (name.find("muons") != std::string::npos){
        config = get_iparticle_input_config(
          name, input_variables,
          iparticle_type_regexes);
        config.type = ConstituentsType::MUON;
        config.link_name = "muonLinks";
        config.output_name = "muon_features";
      }
      else if (name.find("jets") != std::string::npos){
        config = get_iparticle_input_config(
          name, input_variables,
          iparticle_type_regexes);
        config.type = ConstituentsType::JET;
        config.link_name = "jetLinks";
        config.output_name = "jet_features";
      }
      else if (name.find("photons") != std::string::npos){
        config = get_iparticle_input_config(
          name, input_variables,
          iparticle_type_regexes);
        config.type = ConstituentsType::PHOTON;
        config.link_name = "photonLinks";
        config.output_name = "photon_features";
      }
      else{
        throw std::runtime_error(
          "Unknown constituent type: " + name
          );
      }
      return config;
    }
}
