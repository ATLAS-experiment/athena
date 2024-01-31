/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "FlavorTagDiscriminants/GNNOptions.h"

#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"

#include "PathResolver/PathResolver.h"

#include <fstream>

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
    m_decorate_tracks(o.decorate_tracks)
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
    auto [inputs, track_sequences, options] = dataprep::createGetterConfig(
        lwt_config, o.flip_config, o.variable_remapping, o.track_link_type);

    // Initialize jet and b-tagging input getters.
    auto [vb, vj, ds] = dataprep::createBvarGetters(inputs);
    m_varsFromBTag = vb;
    m_varsFromJet = vj;
    m_dataDependencyNames = ds;

    // Initialize track input getters.
    auto [tsb, td, rt] = dataprep::createTrackGetters(track_sequences, options);
    m_trackSequenceBuilders = tsb;
    m_dataDependencyNames += td;

    // Initialize data dependencies and output decorators.
    FlavorTagDiscriminants::FTagDataDependencyNames dd;
    std::set<std::string> rd;

    // Retrieve the configuration for the model outputs.
    OnnxUtil::OutputConfig gnn_output_config = m_onnxUtil->getOutputConfig();

    std::tie(
        m_decorators_float,
        m_decorators_vecchar,
        m_decorators_vecfloat,
        m_decorators_tracklinks,
        m_decorators_track_char,
        m_decorators_track_float,
        dd,
        rd
    ) = dataprep::createGNDecorators(gnn_output_config, options);

    m_dataDependencyNames += dd;

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
    for (const auto& dec: m_decorators_float) {
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
      gnn_input.insert({"track_features", track_info});
    }

    // run inference
    // -------------
    auto [out_f, out_vc, out_vf] = m_onnxUtil->runInference(gnn_input);

    // decorate outputs
    // ----------------

    // with old metadata, doesn't support writing aux tasks
    if (m_onnxUtil->getOnnxModelVersion() == OnnxModelVersion::V0){
      for (const auto& dec: m_decorators_float) {
        if (out_vf.at(dec.first).size() != 1){
          throw std::logic_error("expected vectors of length 1 for float decorators");
        }
        dec.second(btag) = out_vf.at(dec.first).at(0);
      }
    }
    // the new metadata format supports writing aux tasks
    else if (m_onnxUtil->getOnnxModelVersion() == OnnxModelVersion::V1) {
      // float outputs, e.g. jet probabilities
      for (const auto& dec: m_decorators_float) {
        dec.second(btag) = out_f.at(dec.first);
      }
      // vector outputs, e.g. track predictions
      for (const auto& dec: m_decorators_vecchar) {
        dec.second(btag) = out_vc.at(dec.first);
      }
      for (const auto& dec: m_decorators_vecfloat) {
        dec.second(btag) = out_vf.at(dec.first);
      }

      // decorate links to the input tracks to the b-tagging object
      for (const auto& dec: m_decorators_tracklinks) {
        internal::TrackLinks links;
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
        for (const auto& dec: m_decorators_track_char) {
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

        for (const auto& dec: m_decorators_track_float) {
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

} // end of namespace FlavorTagDiscriminants
