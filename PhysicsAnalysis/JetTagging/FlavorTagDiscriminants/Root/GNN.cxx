/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "FlavorTagDiscriminants/GNNOptions.h"
#include "FlavorTagDiscriminants/StringUtils.h"

#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"

#include "PathResolver/PathResolver.h"

#include <fstream>
#include <iostream>

namespace {
  const std::string jetLinkName = "jetLink";
  const std::string unsafeEnvVar = "ALLOW_FTAG_TO_BREAK_THE_EDM";
  const std::string envVarTrue = "Yes please.";
}

namespace FlavorTagDiscriminants {

  GNN::GNN(const std::string& nn_file, const GNNOptions& o):
    m_onnxUtil(nullptr),
    m_jetLink(jetLinkName),
    m_defaultValue(o.default_output_value),
    m_decorate_tracks(o.decorate_tracks),
    // m_trackLoader(nullptr),
    m_flowLoader(nullptr)
  {
    // track decoration is allowed only for non-production builds
    if (m_decorate_tracks) {
      const char* break_edm = std::getenv(unsafeEnvVar.c_str());
      if (break_edm == nullptr || std::string(break_edm) != envVarTrue) {
        throw std::runtime_error(
          "Flavor tagging is trying to break the EDM!\n\n"
          "You are trying to break the EDM!!! "
          "We'll let you do this, if you set an environement variable. "
          "Which one? Not telling. "
          "Now go think about what you just did.\n\n");
      }
    }

    // Load and initialize the neural network model from the given file path.
    std::string fullPathToOnnxFile = PathResolverFindCalibFile(nn_file);
    m_onnxUtil = std::make_shared<OnnxUtil>(fullPathToOnnxFile);

    // Extract metadata from the ONNX file, primarily about the model's inputs.
    auto lwt_config = m_onnxUtil->getLwtConfig();

    // Create configuration objects for data preprocessing.
    auto [inputs, constituents_configs, options] = dataprep::createGetterConfigNew(
        lwt_config, o.flip_config, o.variable_remapping, o.track_link_type);
    std::cout << "TEST 1 " << std::endl;
    // auto [tracksLoaderConfig, tmp_options] = createTracksLoaderConfig(
    //   config, flip_config, variableRemapping, trackLinkType
    // );
    std::cout << constituents_configs.size() << std::endl;
    std::vector<FTagTrackSequenceConfig> track_sequences;
    if (constituents_configs.size() > 0){
      auto tracksLoaderConfig = constituents_configs[0];
      std::cout << "TEST 2 " << std::endl; 
      // m_trackLoader = std::make_shared<TracksLoader>(tracksLoaderConfig, options);
      track_sequences = convertTracksConfigBack(constituents_configs[0]);
      std::cout << "TEST 2F " << std::endl; 
      m_flowLoader = std::make_shared<IParticlesLoader>(tracksLoaderConfig, options);
      // m_constituentsLoaders.push_back(std::make_shared<IParticlesLoader>(tracksLoaderConfig, options));
    }
    std::cout << "TEST 3 " << std::endl;
    // Initialize jet and b-tagging input getters.
    auto [vb, vj, ds] = dataprep::createBvarGetters(inputs);
    m_varsFromBTag = vb;
    m_varsFromJet = vj;
    m_dataDependencyNames = ds;

    // Initialize track input getters.
    auto [tsb, td, rt] = dataprep::createTrackGetters(track_sequences, options);
    m_trackSequenceBuilders = tsb;
    m_dataDependencyNames += td;

    // Retrieve the configuration for the model outputs.
    OnnxUtil::OutputConfig gnn_output_config = m_onnxUtil->getOutputConfig();

    // Create the output decorators.
    auto [dd, rd] = createDecorators(gnn_output_config, options);
    m_dataDependencyNames += dd;

    // Check that all remaps have been used.
    rd.merge(rt);
    dataprep::checkForUnusedRemaps(options.remap_scalar, rd);
  }

  GNN::GNN(const std::string& file,
           const FlipTagConfig& flip,
           const std::map<std::string, std::string>& remap,
           const TrackLinkType link_type,
           float def_out_val,
           bool dt):
    GNN( file, GNNOptions { flip, remap, link_type, def_out_val, dt} )
  {}

  GNN::GNN(GNN&&) = default;
  GNN::GNN(const GNN&) = default;
  GNN::~GNN() = default;

  void GNN::decorate(const xAOD::BTagging& btag) const {
    /* tag a b-tagging object */
    auto jetLink = m_jetLink(btag);
    if (!jetLink.isValid()) {
      throw std::runtime_error("invalid jetLink");
    }
    const xAOD::Jet& jet = **jetLink;
    decorate(jet, btag);
  }

  void GNN::decorate(const xAOD::Jet& jet) const {
    /* tag a jet */
    decorate(jet, jet);
  }

  void GNN::decorateWithDefaults(const xAOD::Jet& jet) const {
    for (const auto& dec: m_decorators.jetFloat) {
      dec.second(jet) = m_defaultValue;
    }
  }

  void GNN::decorate(const xAOD::Jet& jet, const SG::AuxElement& btag) const {
    /* Main function for decorating a jet or b-tagging object with GNN outputs. */
    using namespace internal;

    // prepare input
    // -------------
    std::map<std::string, input_pair> gnn_input;

    std::vector<float> jet_feat;
    for (const auto& getter: m_varsFromBTag) {
      jet_feat.push_back(getter(btag).second);
    }
    for (const auto& getter: m_varsFromJet) {
      jet_feat.push_back(getter(jet).second);
    }
    std::vector<int64_t> jet_feat_dim = {1, static_cast<int64_t>(jet_feat.size())};

    input_pair jet_info (jet_feat, jet_feat_dim);
    gnn_input.insert({"jet_features", jet_info});

    // Only one track sequence is allowed because the tracks are declared
    // outside the loop over sequences.
    // Having more than one sequence would overwrite them.
    // These are only used outside the loop to write the track links.
    if (m_trackSequenceBuilders.size() > 1) {
      throw std::runtime_error("Only one track sequence is supported");
    }
    Tracks input_tracks;
    for (const auto& builder: m_trackSequenceBuilders) {
      std::vector<float> track_feat; // (#tracks, #feats).flatten
      int num_track_vars = static_cast<int>(builder.sequencesFromTracks.size());
      int num_tracks = 0;

      Tracks sorted_tracks = builder.tracksFromJet(jet, btag);
      input_tracks = builder.flipFilter(sorted_tracks, jet);

      int track_var_idx=0;
      for (const auto& seq_builder: builder.sequencesFromTracks) {
        auto double_vec = seq_builder(jet, input_tracks).second;

        if (track_var_idx==0){
          num_tracks = static_cast<int>(double_vec.size());
          track_feat.resize(num_tracks * num_track_vars);
        }

        // need to transpose + flatten
        for (unsigned int track_idx=0; track_idx<double_vec.size(); track_idx++){
          track_feat.at(track_idx*num_track_vars + track_var_idx)
            = double_vec.at(track_idx);
        }
        track_var_idx++;
      }
      std::vector<int64_t> track_feat_dim = {num_tracks, num_track_vars};

      input_pair track_info (track_feat, track_feat_dim);
      if (m_flowLoader){
        auto loader_out = m_flowLoader->getData(jet, btag);
        auto loader_track_feat = loader_out.second.first;
        std::cout << "Vector size: TRACK " << track_feat.size() << " FLOW " << loader_track_feat.size() << std::endl;
        // for (uint64_t i = 0; i < track_feat.size(); i++){
        //   if (std::fabs(loader_track_feat.at(i) - track_feat.at(i)) > 0.001) {
        //       std::cout << "DIFFERENCE " << i << std::endl;
        //   }
        // }
        gnn_input.insert({"flow_features", loader_out.second});
      }
      // if (m_trackLoader){
      //   auto loader_out = m_trackLoader->getData(jet, btag);
      //   auto loader_track_feat = loader_out.second.first;
      //   std::cout << "Vector size: OLD " << track_feat.size() << " NEW " << loader_track_feat.size() << std::endl;
      //   for (uint64_t i = 0; i < track_feat.size(); i++){
      //     if (std::fabs(loader_track_feat.at(i) - track_feat.at(i)) > 0.001) {
      //         std::cout << "DIFFERENCE " << i << std::endl;
      //     }
      //   }
      // }
      // if (m_constituentsLoaders.size() > 0){
      //   auto flow_out = m_constituentsLoaders[0]->getData(jet, btag);
      // }
      // gnn_input.insert({"flow_features", track_info});
    }

    // run inference
    // -------------
    auto [out_f, out_vc, out_vf] = m_onnxUtil->runInference(gnn_input);

    // decorate outputs
    // ----------------

    // with old metadata, doesn't support writing aux tasks
    if (m_onnxUtil->getOnnxModelVersion() == OnnxModelVersion::V0) {
      for (const auto& dec: m_decorators.jetFloat) {
        if (out_vf.at(dec.first).size() != 1){
          throw std::logic_error("expected vectors of length 1 for float decorators");
        }
        dec.second(btag) = out_vf.at(dec.first).at(0);
      }
    }
    // the new metadata format supports writing aux tasks
    else if (m_onnxUtil->getOnnxModelVersion() == OnnxModelVersion::V1) {
      // float outputs, e.g. jet probabilities
      for (const auto& dec: m_decorators.jetFloat) {
        dec.second(btag) = out_f.at(dec.first);
      }
      // vector outputs, e.g. track predictions
      for (const auto& dec: m_decorators.jetVecChar) {
        dec.second(btag) = out_vc.at(dec.first);
      }
      for (const auto& dec: m_decorators.jetVecFloat) {
        dec.second(btag) = out_vf.at(dec.first);
      }

      // decorate links to the input tracks to the b-tagging object
      for (const auto& dec: m_decorators.jetTrackLinks) {
        TrackLinks links;
        for (const xAOD::TrackParticle* it: input_tracks) {
          TrackLinks::value_type link;
          const auto* itc = dynamic_cast<const xAOD::TrackParticleContainer*>(
            it->container());
          link.toIndexedElement(*itc, it->index());
          links.push_back(link);
        }
        dec.second(btag) = links;
      }

      // decorate tracks directly
      if (m_decorate_tracks) {
        for (const auto& dec: m_decorators.trackChar) {
          std::vector<char>& values = out_vc.at(dec.first);
          if (values.size() != input_tracks.size()) {
            throw std::logic_error("Track aux task output size doesn't match the size of track list");
          }
          Tracks::const_iterator it = input_tracks.begin();
          std::vector<char>::const_iterator ival = values.begin();
          for (; it != input_tracks.end() && ival != values.end(); ++it, ++ival) {
            dec.second(**it) = *ival;
          }
        }

        for (const auto& dec: m_decorators.trackFloat) {
          std::vector<float>& values = out_vf.at(dec.first);
          if (values.size() != input_tracks.size()) {
            throw std::logic_error("Track aux task output size doesn't match the size of track list");
          }
          Tracks::const_iterator it = input_tracks.begin();
          std::vector<float>::const_iterator ival = values.begin();
          for (; it != input_tracks.end() && ival != values.end(); ++it, ++ival) {
            dec.second(**it) = *ival;
          }
        }
      }
    }
    else {
      throw std::logic_error("unsupported ONNX metadata version");
    }
  } // end of decorate()

  // Dependencies
  std::set<std::string> GNN::getDecoratorKeys() const {
    return m_dataDependencyNames.bTagOutputs;
  }
  std::set<std::string> GNN::getAuxInputKeys() const {
    return m_dataDependencyNames.bTagInputs;
  }
  std::set<std::string> GNN::getConstituentAuxInputKeys() const {
    return m_dataDependencyNames.trackInputs;
  }

  std::tuple<FTagDataDependencyNames, std::set<std::string>>
  GNN::createDecorators(const OnnxUtil::OutputConfig& outConfig, const FTagOptions& options) {
    FTagDataDependencyNames deps;
    Decorators decs;

    std::map<std::string, std::string> remap = options.remap_scalar;
    std::set<std::string> usedRemap;

    // get the regex to rewrite the outputs if we're using flip taggers
    auto flip_converters = dataprep::getNameFlippers(options.flip);
    std::string context = "building negative tag b-btagger";

    for (const auto& outNode : outConfig) {
      // the node's output name will be used to define the decoration name
      std::string dec_name = outNode.name;

      // modify the deco name if we're using flip taggers
      if (options.flip != FlipTagConfig::STANDARD) {
        dec_name = str::sub_first(flip_converters, dec_name, context);
      }

      // remap the deco name if necessary
      dec_name = str::remapName(dec_name, remap, usedRemap);

      // keep track of dependencies for EDM bookkeeping
      deps.bTagOutputs.insert(dec_name);

      // Create decorators based on output type and target
      switch (outNode.type) {
        case OnnxOutput::OutputType::FLOAT:
          m_decorators.jetFloat.emplace_back(outNode.name, Dec<float>(dec_name));
          break;
        case OnnxOutput::OutputType::VECCHAR:
          if (!m_decorate_tracks or outNode.target == OnnxOutput::OutputTarget::JET) {
            m_decorators.jetVecChar.emplace_back(outNode.name, Dec<std::vector<char>>(dec_name));
          } else if (outNode.target == OnnxOutput::OutputTarget::TRACK) {
            m_decorators.trackChar.emplace_back(outNode.name, Dec<char>(dec_name));
          } else {
            throw std::logic_error("Unknown VECCHAR output node target");
          }
          break;
        case OnnxOutput::OutputType::VECFLOAT:
          if (!m_decorate_tracks or outNode.target == OnnxOutput::OutputTarget::JET) {
            m_decorators.jetVecFloat.emplace_back(outNode.name, Dec<std::vector<float>>(dec_name));
          } else if (outNode.target == OnnxOutput::OutputTarget::TRACK) {
            m_decorators.trackFloat.emplace_back(outNode.name, Dec<float>(dec_name));
          } else {
            throw std::logic_error("Unknown VECFLOAT output node target");
          }
          break;
        default:
          throw std::logic_error("Unknown output node type");
      }
    }

    // Create decorators for links to the input tracks
    if (!m_decorators.jetVecChar.empty() || !m_decorators.jetVecFloat.empty()) {
      std::string name = m_onnxUtil->getModelName() + "_TrackLinks";
      name = str::remapName(name, remap, usedRemap);
      deps.bTagOutputs.insert(name);
      m_decorators.jetTrackLinks.emplace_back(name, Dec<TrackLinks>(name));
    }

    return std::make_tuple(deps, usedRemap);
  }

} // end of namespace FlavorTagDiscriminants
