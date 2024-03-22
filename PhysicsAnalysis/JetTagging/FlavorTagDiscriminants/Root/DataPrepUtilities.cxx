/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/DataPrepUtilities.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/CustomGetterUtils.h"
#include "FlavorTagDiscriminants/StringUtils.h"

#include "xAODBTagging/BTaggingUtilities.h"

namespace {
  using namespace FlavorTagDiscriminants;

  // define a regex literal operator
  std::regex operator "" _r(const char* c, size_t /* length */) {
    return std::regex(c);
  }

  using FlavorTagDiscriminants::EDMType;
  using FlavorTagDiscriminants::FTagInputConfig;
  using FlavorTagDiscriminants::FlipTagConfig;
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
  std::vector<FTagInputConfig> get_input_config(
    const std::vector<std::string>& variable_names,
    const TypeRegexes& type_regexes,
    const StringRegexes& default_flag_regexes);

  // Since the names of the inputs are stored in the NN config, we
  // also allow some user-configured remapping. Items in replaced_vars
  // are removed as they are used.
  void remap_inputs(std::vector<lwt::Input>& nn,
                    std::map<std::string, std::string>& replaced_vars,
                    std::map<std::string, double>& defaults);

  // replace strings for flip taggers
  void rewriteFlipConfig(lwt::GraphConfig&, const StringRegexes&);


  //_______________________________________________________________________
  // Implementation of the above functions
  //


  std::vector<FTagInputConfig> get_input_config(
    const std::vector<std::string>& variable_names,
    const TypeRegexes& type_regexes,
    const StringRegexes& default_flag_regexes)
  {
    std::vector<FTagInputConfig> inputs;
    for (const auto& var: variable_names) {
      FTagInputConfig input;
      input.name = var;
      input.type = str::match_first(type_regexes, var, "type matching");
      input.default_flag = str::sub_first(default_flag_regexes, var,
                                     "default matching");
      inputs.push_back(input);
    }
    return inputs;
  }


  // do some input variable magic in case someone asked
  void remap_inputs(std::vector<lwt::Input>& nn,
                    std::map<std::string, std::string>& replaced_vars,
                    std::map<std::string, double>& defaults) {
    // keep track of the new default values, and which values they
    // were moved from
    std::map<std::string, double> new_defaults;
    std::set<std::string> moved_defaults;
    for (lwt::Input& input: nn) {
      std::string nn_name = input.name;
      auto replacement_itr = replaced_vars.find(nn_name);
      if (replacement_itr != replaced_vars.end()) {
        std::string new_name = replacement_itr->second;
        input.name = new_name;
        if (defaults.count(nn_name)) {
          new_defaults[new_name] = defaults.at(nn_name);
          moved_defaults.insert(nn_name);
        }
        replaced_vars.erase(replacement_itr);
      }
    }
    for (const auto& new_default: new_defaults) {
      defaults[new_default.first] = new_default.second;
      // if something was a new default we don't want to delete it
      // below.
      moved_defaults.erase(new_default.first);
    }
    // delete anything that was moved but wasn't assigned to
    for (const auto& moved: moved_defaults) {
      defaults.erase(moved);
    }
  }

  void rewriteFlipConfig(lwt::GraphConfig& config,
                         const StringRegexes& res){
    std::string context = "building negative tag b-btagger";
    for (auto& node: config.inputs) {
      for (auto& var: node.variables) {
        var.name = str::sub_first(res, var.name, context);
      }
      std::map<std::string, double> new_defaults;
      for (auto& pair: node.defaults) {
        new_defaults[str::sub_first(res, pair.first, context)] = pair.second;
      }
      node.defaults = new_defaults;
    }
    std::map<std::string, lwt::OutputNodeConfig> new_outputs;
    for (auto& pair: config.outputs) {
      new_outputs[str::sub_first(res, pair.first, context)] = pair.second;
    }
    config.outputs = new_outputs;
  }

}
// __________________________________________________________________________
// Start of functions accessible in the FlavorTagDiscriminants namespace

namespace FlavorTagDiscriminants {

  FTagOptions::FTagOptions()
    : track_prefix ("btagIp_"),
      flip (FlipTagConfig::STANDARD),
      track_link_name ("BTagTrackToJetAssociator"),
      track_link_type (TrackLinkType::TRACK_PARTICLE),
      default_output_value (NAN),
      invalid_ip_key ("invalidIp")
  {
  }

  // ________________________________________________________________________
  // Internal code
  namespace internal {

    // ______________________________________________________________________
    // Internal utility functions
    //

    // The 'get' namespace is for factories that build std::function
    // objects
    namespace get {
      // factory for functions that get variables out of the b-tagging
      // object
      VarFromBTag varFromBTag(const std::string& name, EDMType type,
                            const std::string& default_flag) {
        if(default_flag.size() == 0 || name==default_flag)
        {
          switch (type) {
            case EDMType::INT: return BVarGetterNoDefault<int>(name);
            case EDMType::FLOAT: return BVarGetterNoDefault<float>(name);
            case EDMType::DOUBLE: return BVarGetterNoDefault<double>(name);
            case EDMType::CHAR: return BVarGetterNoDefault<char>(name);
            case EDMType::UCHAR: return BVarGetterNoDefault<unsigned char>(name);
            default: {
              throw std::logic_error("Unknown EDM type");
            }
          }
        }
        else{
          switch (type) {
            case EDMType::INT: return BVarGetter<int>(name, default_flag);
            case EDMType::FLOAT: return BVarGetter<float>(name, default_flag);
            case EDMType::DOUBLE: return BVarGetter<double>(name, default_flag);
            case EDMType::CHAR: return BVarGetter<char>(name, default_flag);
            case EDMType::UCHAR: return BVarGetter<unsigned char>(name, default_flag);
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

    // Get the regex which remap the names if we're using flip taggers
    StringRegexes getNameFlippers(const FlipTagConfig& flip_config) {

      std::string flip_name = "";
      if (flip_config == FlipTagConfig::FLIP_SIGN) {
        flip_name = "Flip";
      }
      else if (flip_config == FlipTagConfig::NEGATIVE_IP_ONLY) {
        flip_name = "Neg";
      }
      else if (flip_config == FlipTagConfig::SIMPLE_FLIP) {
        flip_name = "SimpleFlip";
      }

      StringRegexes flip_converters {
        {"(GN1[^_]*|GN2[^_]*)"_r, "$1" + flip_name},
        {"(GN1[^_]*|GN2[^_]*)_(.*)"_r, "$1" + flip_name + "_$2"},
        {"(IP[23]D)_(.*)"_r, "$1Neg_$2"},
        {"(rnnip|dips[^_]*)_(.*)"_r, "$1flip_$2"},
        {"(JetFitter|SV1|JetFitterSecondaryVertex)_(.*)"_r, "$1Flip_$2"},
        {"(rnnip|dips[^_]*)"_r, "$1flip"},
        {"^(DL1|DL1r[^_]*|DL1rmu|DL1d[^_]*)$"_r, "$1" + flip_name},
        {"pt|abs_eta|eta"_r, "$&"},
        {"softMuon.*|smt.*"_r, "$&"}
      };

      return flip_converters;
    }
    
    // Translate string config to config objects
    //
    // This parses the saved NN configuration structure and translates
    // informaton encoded as strings into structures and enums to be
    // consumed by the code that actually constructs the NN.
    //
    std::tuple<
      std::vector<FTagInputConfig>,
      std::vector<ConstituentsInputConfig>,
      FTagOptions>
    createGetterConfig( lwt::GraphConfig& config,
      FlipTagConfig flip_config,
      std::map<std::string, std::string> remap_scalar,
      TrackLinkType track_link_type
    ){

      // we rewrite the inputs if we're using flip taggers
      StringRegexes flip_converters = getNameFlippers(flip_config);

      if (flip_config != FlipTagConfig::STANDARD) {
        rewriteFlipConfig(config, flip_converters);
      }

      // build the standard inputs

      // type and default value-finding regexes are hardcoded for now
      TypeRegexes type_regexes = {
        {".*_isDefaults"_r, EDMType::CHAR},
        // TODO: in the future we should migrate RNN and IPxD
        // variables to floats. This is outside the scope of the
        // current flavor tagging developments and AFT-438.
        {"IP[23]D(Neg)?_[pbc](b|c|u|tau)"_r, EDMType::FLOAT},
        {"SV1(Flip)?_[pbc](b|c|u|tau)"_r, EDMType::FLOAT},
        {"(rnnip|iprnn|dips[^_]*)(flip)?_p(b|c|u|tau)"_r, EDMType::FLOAT},
        {"(JetFitter|SV1|JetFitterSecondaryVertex)(Flip)?_[Nn].*"_r, EDMType::INT},
        {"(JetFitter|SV1|JetFitterSecondaryVertex).*"_r, EDMType::FLOAT},
        {"(log_)?pt|abs_eta|eta|phi|energy|mass"_r, EDMType::CUSTOM_GETTER},
        {"softMuon_p[bcu]"_r, EDMType::FLOAT},
        {"softMuon_.*"_r, EDMType::FLOAT},
      };

      StringRegexes default_flag_regexes{
        {"IP2D_.*"_r, "IP2D_isDefaults"},
        {"IP2DNeg_.*"_r, "IP2DNeg_isDefaults"},
        {"IP3D_.*"_r, "IP3D_isDefaults"},
        {"IP3DNeg_.*"_r, "IP3DNeg_isDefaults"},
        {"SV1_.*"_r, "SV1_isDefaults"},
        {"SV1Flip_.*"_r, "SV1Flip_isDefaults"},
        {"JetFitter_.*"_r, "JetFitter_isDefaults"},
        {"JetFitterFlip_.*"_r, "JetFitterFlip_isDefaults"},
        {"JetFitterSecondaryVertex_.*"_r, "JetFitterSecondaryVertex_isDefaults"},
        {"JetFitterSecondaryVertexFlip_.*"_r, "JetFitterSecondaryVertexFlip_isDefaults"},
        {"rnnip_.*"_r, "rnnip_isDefaults"},
        {"(dips[^_]*)_.*"_r, "$1_isDefaults"},
        {"rnnipflip_.*"_r, "rnnipflip_isDefaults"},
        {"iprnn_.*"_r, ""},
        {"smt_.*"_r, "softMuon_isDefaults"},
        {"softMuon_.*"_r, "softMuon_isDefaults"},
        {"((log_)?pt|abs_eta|eta|phi|energy|mass)"_r, ""}}; // no default for custom cases

      std::vector<FTagInputConfig> input_config;
      for (auto& node: config.inputs){
        // allow the user to remape some of the inputs
        remap_inputs(node.variables, remap_scalar,
               node.defaults);

        std::vector<std::string> input_names;
        for (const auto& var: node.variables) {
          input_names.push_back(var.name);
        }

        // check to make sure the next line doesn't overwrite something
        // TODO: figure out how to support multiple scalar input nodes
        if (!input_config.empty()) {
          throw std::logic_error(
            "We don't currently support multiple scalar input nodes");
        }
        input_config = get_input_config(
        input_names, type_regexes, default_flag_regexes);
      }

      // build the constituents inputs

      std::vector<std::pair<std::string, std::vector<std::string>>> constituent_names;
      for (auto& node: config.input_sequences) {
        remap_inputs(node.variables, remap_scalar,
		     node.defaults);

        std::vector<std::string> names;
        for (const auto& var: node.variables) {
          names.push_back(var.name);
        }
        constituent_names.emplace_back(node.name, names);
      }

      std::vector<ConstituentsInputConfig> constituent_configs;
      for (auto el: constituent_names){
        constituent_configs.push_back(
          createConstituentsLoaderConfig(el.first, el.second, flip_config));
      }

      // some additional options
      FTagOptions options;
      
      if (auto h = remap_scalar.extract(options.track_prefix)) {
        options.track_prefix = h.mapped();
      }
      if (auto h = remap_scalar.extract(options.track_link_name)) {
        options.track_link_name = h.mapped();
      }
      if (auto h = remap_scalar.extract(options.invalid_ip_key)) {
        options.invalid_ip_key = h.mapped();
      }
      options.flip = flip_config;
      options.remap_scalar = remap_scalar;
      options.track_link_type = track_link_type;
      return std::make_tuple(input_config, constituent_configs, options);
    }

    // Translate configuration to getter functions
    //
    // This focuses on the scalar inputs, i.e. the inputs for DL1d,
    // the code for the track inputs is below.
    std::tuple<
      std::vector<internal::VarFromBTag>,
      std::vector<internal::VarFromJet>,
      FTagDataDependencyNames>
    createBvarGetters(
      const std::vector<FTagInputConfig>& inputs)
    {
      FTagDataDependencyNames deps;
      std::vector<internal::VarFromBTag> varsFromBTag;
      std::vector<internal::VarFromJet> varsFromJet;

      for (const auto& input: inputs) {
        if (input.type != EDMType::CUSTOM_GETTER) {
          auto filler = internal::get::varFromBTag(input.name, input.type,
                                         input.default_flag);
          deps.bTagInputs.insert(input.name);
          varsFromBTag.push_back(filler);
        } else {
          varsFromJet.push_back(getter_utils::customGetterAndName(input.name));
        }
        if (input.default_flag.size() > 0) {
          deps.bTagInputs.insert(input.default_flag);
        }
      }

      return std::make_tuple(varsFromBTag, varsFromJet, deps);
    }


    // Translate configuration to setter functions
    //
    // This returns the "decorators" that we use to save outputs to
    // the EDM.
    std::tuple<
      std::map<std::string, internal::OutNodeFloat>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createDecorators(
      const lwt::GraphConfig& config,
      const FTagOptions& options)
    {
      FTagDataDependencyNames deps;
      std::map<std::string, internal::OutNodeFloat> decorators;
      std::map<std::string, std::string> remap = options.remap_scalar;
      std::set<std::string> used_remap;

      for (const auto& out_node: config.outputs) {
        std::string node_name = out_node.first;

        internal::OutNodeFloat node;
        for (const std::string& element: out_node.second.labels) {
          std::string name = node_name + "_" + element;

          // let user rename the output
          if (auto h = remap.extract(name)){
            name = h.mapped();
            used_remap.insert(h.key());
          }
          deps.bTagOutputs.insert(name);

          SG::AuxElement::Decorator<float> f(name);
          node.emplace_back(element, f);
        }
        decorators[node_name] = node;
      }

      return std::make_tuple(decorators, deps, used_remap);
    }

    // return a function to check IP validity
    std::tuple<
      std::function<char(const internal::Tracks&)>,
      std::vector<SG::AuxElement::Decorator<char>>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createIpChecker(
      const lwt::GraphConfig& gc, const FTagOptions& opts) {
      using namespace internal;
      FTagDataDependencyNames deps;
      std::map<std::string, std::string> remap = opts.remap_scalar;
      std::set<std::string> used_remap;
      // dummy if there's no invalid check key
      std::function checker = [](const Tracks&) -> char {return 0;};
      // if we do have a key, return 1 for invalid
      if (!opts.invalid_ip_key.empty()) {
        std::string ip_key = opts.track_prefix + opts.invalid_ip_key;
        SG::AuxElement::ConstAccessor<char> invalid_check(ip_key);
        checker = [invalid_check](const Tracks& trs){
          for (const xAOD::TrackParticle* trk: trs) {
            if (invalid_check(*trk)) return 1;
          }
          return 0;
        };
        deps.trackInputs.insert(ip_key);
      }
      std::vector<SG::AuxElement::Decorator<char>> default_decs;
      for (const auto& output: gc.outputs) {
        std::string basename = output.first;
        std::string dec_name = basename + "_isDefaults";
        if (auto h = remap.extract(dec_name)) {
          dec_name = h.mapped();
          used_remap.insert(h.key());
        }
        default_decs.emplace_back(dec_name);
        deps.bTagOutputs.insert(dec_name);
      }
      return {checker, default_decs, deps, used_remap};
    }

    void checkForUnusedRemaps(
      const std::map<std::string, std::string>& requested,
      const std::set<std::string>& used)
    {
      // we want to make sure every remapping was used
      std::set<std::string> unused;
      for (auto [k, v]: requested) {
        if (!used.count(k)) unused.insert(k);
      }
      if (unused.size() > 0) {
        std::string outputs;
        for (const auto& item: unused) {
          outputs.append(item);
          if (item != *unused.rbegin()) outputs.append(", ");
        }
        throw std::logic_error("found unused output remapping(s): " + outputs);
      }
    }
  } // end of datapre namespace

} // end of FlavorTagDiscriminants namespace

