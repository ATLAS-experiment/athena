/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/GNN.h"
#include "FlavorTagInference/BTagTrackIpAccessor.h"
#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/GNNOptions.h"
#include "FlavorTagInference/StringUtils.h"


#include "FlavorTagInference/TracksLoader.h"
#include "FlavorTagInference/FlowElementsLoader.h"
#include "FlavorTagInference/HitsLoader.h"
#include "FlavorTagInference/ElectronsLoader.h"

#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"

#include "PathResolver/PathResolver.h"

namespace {
  const std::string jetLinkName = "jetLink";

  auto getSaltModel(const std::string& nn_file) {
    using namespace FlavorTagInference;
    std::string fullPathToOnnxFile = PathResolverFindCalibFile(nn_file);
    return std::make_shared<const SaltModel>(fullPathToOnnxFile);
  }

  template <typename T>
  std::string join(const T& c) {
    std::string out;
    std::string sep = "";
    for (const auto& [k, v]: c) {
      out.append(sep);
      sep = ", ";
      out.append(k);
    }
    return out;
  }
}

namespace FlavorTagInference {

  GNN::GNN(const std::string& nn_file, const GNNOptions& o):
    GNN(getSaltModel(nn_file), o)
  {
  }

  GNN::GNN(const GNN& old, const GNNOptions& o):
    GNN(old.m_saltModel, o)
  {
  }

  GNN::GNN(std::shared_ptr<const SaltModel> util, const GNNOptions& o):
    m_saltModel(util),
    m_jetLink(jetLinkName)
  {

    // Extract metadata from the ONNX file, primarily about the model's inputs.
    auto graph_config = m_saltModel->getGraphConfig();

    // Create configuration objects for data preprocessing.
    auto [inputs, constituents_configs, options] = dataprep::createGetterConfig<SaltModelGraphConfig::GraphConfig, SaltModelGraphConfig::OutputNodeConfig>(
        graph_config, o.flip_config, o.variable_remapping, o.track_link_type);

    for (auto config : constituents_configs){
      switch (config.type){
      using enum ConstituentsType;
      case TRACK:
        m_constituentsLoaders.push_back(std::make_shared<TracksLoader>(config, options));
        break;
      case FLOW_ELEMENT:
        m_constituentsLoaders.push_back(std::make_shared<FlowElementsLoader>(config, options));
        break;
      case HIT:
        m_constituentsLoaders.push_back(std::make_shared<HitsLoader>(config, options));
        break;
      case ELECTRON:
        m_constituentsLoaders.push_back(std::make_shared<ElectronsLoader>(config, options));
        break;
      default:
        throw std::runtime_error("Unknown constituent type");
      }
    }

    // Initialize jet and b-tagging input getters.
    auto [vb, vj, ds] = dataprep::createBvarGetters(inputs);
    m_varsFromBTag = vb;
    m_varsFromJet = vj;
    m_dataDependencyNames = ds;

    // Retrieve the configuration for the model outputs.
    SaltModel::OutputConfig gnn_output_config = m_saltModel->getOutputConfig();

    // Create the output decorators.
    auto [dd, rd] = createDecorators(gnn_output_config, options);
    m_dataDependencyNames += dd;

    // Update dependencies and used remap from the constituents loaders.
    for (auto loader : m_constituentsLoaders){
      m_dataDependencyNames += loader->getDependencies();
      std::set<std::string> used_remap = loader->getUsedRemap();
      rd.merge(used_remap);
    }
    dataprep::checkForUnusedRemaps(options.remap_scalar, rd);

    // Build the default decorators. Note that this _must_ be called
    // after createDecorators.
    auto unused_defaults = o.default_output_values;
    for (const auto& [name, dec]: m_decorators.jetFloat) {
      float default_value = o.default_output_value;
      if (auto def = unused_defaults.extract(name)) {
        default_value = def.mapped();
      }
      m_defaultValues.emplace_back(dec, default_value);
    }
    if (!unused_defaults.empty()) {
      throw std::runtime_error(
        "unused default values: [" + join(unused_defaults) + "]");
    }
    m_defaultZeroTracks = o.default_zero_tracks;
  }

  GNN::GNN(const std::string& file,
           const FlipTagConfig& flip,
           const std::map<std::string, std::string>& remap,
           const TrackLinkType link_type,
           float def_out_val):
    GNN( file, GNNOptions { flip, remap, link_type, def_out_val, {}, false} )
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

  void GNN::decorateWithDefaults(const SG::AuxElement& jet) const {
    for (const auto& [dec, v]: m_defaultValues) {
      dec(jet) = v;
    }
    // for some networks we need to set a lot of empty vectors as well
    if (m_saltModel->getSaltModelVersion() == SaltModelVersion::V1) {
      // vector outputs, e.g. track predictions
      for (const auto& dec: m_decorators.jetVecChar) {
        dec.second(jet) = {};
      }
      for (const auto& dec: m_decorators.jetVecFloat) {
        dec.second(jet) = {};
      }
      for (const auto& dec: m_decorators.jetTrackLinks) {
        dec.second(jet) = {};
      }
    }
  }

  void GNN::decorate(const xAOD::Jet& jet, const SG::AuxElement& btag) const {
    /* Main function for decorating a jet or b-tagging object with GNN outputs. */
    using namespace internal;

    // prepare input
    // -------------
    std::map<std::string, Inputs> gnn_inputs;

    // jet level inputs
    std::vector<float> jet_feat;
    for (const auto& getter: m_varsFromBTag) {
      jet_feat.push_back(getter(btag).second);
    }
    for (const auto& getter: m_varsFromJet) {
      jet_feat.push_back(getter(jet).second);
    }
    std::vector<int64_t> jet_feat_dim = {1, static_cast<int64_t>(jet_feat.size())};
    Inputs jet_info(jet_feat, jet_feat_dim);
    if (m_saltModel->getSaltModelVersion() == SaltModelVersion::V2) {
      gnn_inputs.insert({"jets", jet_info});
    } else {
      gnn_inputs.insert({"jet_features", jet_info});
    }

    // constituent level inputs
    Tracks input_tracks;
    int64_t num_inputs = 0;
    for (auto loader : m_constituentsLoaders){
      auto [input_name, input_data, input_objects] = loader->getData(jet, btag);
      if (m_saltModel->getSaltModelVersion() != SaltModelVersion::V2) {
        input_name.pop_back();
        input_name.append("_features");
      }
      gnn_inputs.insert({input_name, input_data});
      num_inputs += input_data.first.size();

      // for now we only collect tracks for aux task decoration
      // they have to be converted back from IParticle to TrackParticle first
      if (loader->getType() == ConstituentsType::TRACK){
        for (auto constituent : input_objects){
          input_tracks.push_back(dynamic_cast<const xAOD::TrackParticle*>(constituent));
        }
      }
    }

    // run inference
    // -------------
    if (m_defaultZeroTracks && num_inputs == 0) {
      this->decorateWithDefaults(btag);
      return;
    }
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_inputs);

    // decorate outputs
    // ----------------

    // with old metadata, doesn't support writing aux tasks
    if (m_saltModel->getSaltModelVersion() == SaltModelVersion::V0) {
      for (const auto& dec: m_decorators.jetFloat) {
        if (out_vf.at(dec.first).size() != 1){
          throw std::logic_error("expected vectors of length 1 for float decorators");
        }
        dec.second(btag) = out_vf.at(dec.first).at(0);
      }
    }
    // the new metadata format supports writing aux tasks
    else if (m_saltModel->getSaltModelVersion() == SaltModelVersion::V1) {
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
  GNN::createDecorators(const SaltModel::OutputConfig& outConfig, const FTagOptions& options) {
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
        case SaltModelOutput::OutputType::FLOAT:
          m_decorators.jetFloat.emplace_back(outNode.name, Dec<float>(dec_name));
          break;
        case SaltModelOutput::OutputType::VECCHAR:
          m_decorators.jetVecChar.emplace_back(outNode.name, Dec<std::vector<char>>(dec_name));
          break;
        case SaltModelOutput::OutputType::VECFLOAT:
          m_decorators.jetVecFloat.emplace_back(outNode.name, Dec<std::vector<float>>(dec_name));
          break;
        default:
          throw std::logic_error("Unknown output data type");
      }
    }

    // Create decorators for links to the input tracks
    if (!m_decorators.jetVecChar.empty() || !m_decorators.jetVecFloat.empty()) {
      std::string name = m_saltModel->getModelName() + "_TrackLinks";

      // modify the deco name if we're using flip taggers
      if (options.flip != FlipTagConfig::STANDARD) {
        name = str::sub_first(flip_converters, name, context);
      }

      name = str::remapName(name, remap, usedRemap);
      deps.bTagOutputs.insert(name);
      m_decorators.jetTrackLinks.emplace_back(name, Dec<TrackLinks>(name));
    }

    return std::make_tuple(deps, usedRemap);
  }

} // end of namespace FlavorTagInference
