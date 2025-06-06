/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/DataPrepUtilities.h"
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"
#include "FlavorTagInference/StringUtils.h"

namespace {
  using namespace InDetGNNHardScatterSelection;

  // define a regex literal operator
  std::regex operator "" _r(const char* c, size_t /* length */) {
    return std::regex(c);
  }

  // ____________________________________________________________________
  // High level adapter stuff
  //
  // We define a few structures to map variable names to type, default
  // value, etc. These are only used by the high level interface.
  //
  typedef std::vector<std::pair<std::regex, EDMType> > TypeRegexes;
  typedef std::vector<std::pair<std::regex, std::string> > StringRegexes;

  // Function to map the regular expressions + the list of inputs to a
  // list of variable configurations.
  std::vector<HSGNNInputConfig> get_input_config(
    const std::vector<std::string>& variable_names,
    const TypeRegexes& type_regexes,
    const StringRegexes& default_flag_regexes);

  //_______________________________________________________________________
  // Implementation of the above functions
  //
  std::vector<HSGNNInputConfig> get_input_config(
    const std::vector<std::string>& variable_names,
    const TypeRegexes& type_regexes,
    const StringRegexes& default_flag_regexes)
  {
    std::vector<HSGNNInputConfig> inputs;
    for (const auto& var: variable_names) {
      HSGNNInputConfig input;
      input.name = var;
      input.type = FlavorTagInference::str::match_first(type_regexes, var, "type matching");
      input.default_flag = FlavorTagInference::str::sub_first(default_flag_regexes, var,
                                     "default matching");
      inputs.push_back(input);
    }
    return inputs;
  }

}
// __________________________________________________________________________
// Start of functions accessible in the InDetGNNHardScatterSelection namespace

namespace InDetGNNHardScatterSelection {

  // ________________________________________________________________________
  // Internal code
  namespace internal {

    // ______________________________________________________________________
    // Internal utility functions
    //

    // The 'get' namespace is for factories that build std::function
    // objects
    namespace get {
      // factory for functions that get variables out of the vertex object
      VarFromVertex varFromVertex(const std::string& name, EDMType type,
                            const std::string& default_flag) {
        if(default_flag.size() == 0 || name==default_flag)
        {
          switch (type) {
            case EDMType::INT: return VertexVarGetterNoDefault<int>(name);
            case EDMType::FLOAT: return VertexVarGetterNoDefault<float>(name);
            case EDMType::DOUBLE: return VertexVarGetterNoDefault<double>(name);
            case EDMType::CHAR: return VertexVarGetterNoDefault<char>(name);
            case EDMType::UCHAR: return VertexVarGetterNoDefault<unsigned char>(name);
            default: {
              throw std::logic_error("Unknown EDM type");
            }
          }
        }
        else{
          switch (type) {
            case EDMType::INT: return VertexVarGetter<int>(name, default_flag);
            case EDMType::FLOAT: return VertexVarGetter<float>(name, default_flag);
            case EDMType::DOUBLE: return VertexVarGetter<double>(name, default_flag);
            case EDMType::CHAR: return VertexVarGetter<char>(name, default_flag);
            case EDMType::UCHAR: return VertexVarGetter<unsigned char>(name, default_flag);
            default: {
              throw std::logic_error("Unknown EDM type");
            }
          }
        }
      }
    } // end of get namespace
  } // end of internal namespace

  // ______________________________________________________________________
  // High level configuration functions
  //
  // Most of the NN code should be a relatively thin wrapper on these
  // functions.
  namespace dataprep {

    // Translate string config to config objects
    //
    // This parses the saved NN configuration structure and translates
    // informaton encoded as strings into structures and enums to be
    // consumed by the code that actually constructs the NN.
    //
    std::tuple<
      std::vector<HSGNNInputConfig>,
      std::vector<ConstituentsInputConfig>>
    createGetterConfig( FlavorTagInference::SaltModelGraphConfig::GraphConfig& config)
    {

      // build the standard inputs

      // type and default value-finding regexes are hardcoded for now
      TypeRegexes type_regexes = {
        {".*_isDefaults"_r, EDMType::CHAR},
        {"sumPt2|chi2Over_ndf|z_asymmetry|weighted_z_asymmetry|z_kurtosis|z_skewness|photon_deltaz|photon_deltaPhi|actualIntPerXing"_r, EDMType::FLOAT},
        {"ntrk"_r, EDMType::INT},
        {"(log_)?pt|abs_eta|eta|phi|energy|mass|sumPt"_r, EDMType::CUSTOM_GETTER},
      };

      StringRegexes default_flag_regexes{
        {"ntrk|sumPt2|chi2Over_ndf|z_asymmetry|weighted_z_asymmetry|z_kurtosis|z_skewness|photon_deltaz|photon_deltaPhi|actualIntPerXing"_r, ""},
        {"((log_)?pt|abs_eta|eta|phi|energy|mass|sumPt)"_r, ""}}; // no default for custom cases

      std::vector<HSGNNInputConfig> input_config;
      for (auto& node: config.inputs){
        std::vector<std::string> input_names;
        for (const auto& var: node.variables) {
          input_names.push_back(var.name);
        }
        input_config = get_input_config(input_names, type_regexes, default_flag_regexes);
      }

      // build the constituents inputs
      std::vector<std::pair<std::string, std::vector<std::string>>> constituent_names;
      for (auto& node: config.input_sequences) {

        std::vector<std::string> names;
        for (const auto& var: node.variables) {
          names.push_back(var.name);
        }
        constituent_names.emplace_back(node.name, names);
      }

      std::vector<ConstituentsInputConfig> constituent_configs;
      for (auto el: constituent_names){
        constituent_configs.push_back(
          createConstituentsLoaderConfig(el.first, el.second));
      }

      return std::make_tuple(input_config, constituent_configs);
    }

    // Translate configuration to getter functions
    //
    // This focuses on the scalar inputs
    // the code for the track inputs is below.
    std::vector<internal::VarFromVertex> createVertexVarGetters(
      const std::vector<HSGNNInputConfig>& inputs)
    {
      std::vector<internal::VarFromVertex> varsFromVertex;

      for (const auto& input: inputs) {
        if (input.type != EDMType::CUSTOM_GETTER) {
          auto filler = internal::get::varFromVertex(input.name, input.type,
                                         input.default_flag);
          varsFromVertex.push_back(filler);
        } else {
          varsFromVertex.push_back(getter_utils::customGetterAndName(input.name));
        }
      }

      return varsFromVertex;
    }

  } // end of dataprep namespace

} // end of InDetGNNHardScatterSelection namespace


