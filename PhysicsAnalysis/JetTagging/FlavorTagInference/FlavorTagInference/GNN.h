/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This class is used in conjunction with SaltModel to run inference on a GNN model.
  Whereas SaltModel handles the interfacing with the ONNX runtime, this class handles
  the interfacing with the ATLAS EDM. It is responsible for collecting all the inputs
  needed for inference, running inference (via SaltModel), and decorating the results
  back to ATLAS EDM.
*/

#ifndef FLAVORTAGINFERENCE_GNN_H
#define FLAVORTAGINFERENCE_GNN_H

// Tool includes
#include "FlavorTagInference/DataPrepUtilities.h"
#include "FlavorTagInference/FTagDataDependencyNames.h"
#include "FlavorTagInference/FlipTagEnums.h"
#include "FlavorTagInference/GNNDataLoader.h"
#include "FlavorTagInference/GNNOptions.h"
#include "FlavorTagInference/ISaltModel.h"

// EDM includes
#include "xAODBase/IParticle.h"

#include <string>
#include <map>

namespace FlavorTagInference {

  struct GNNOptions;

  //
  // Tool to to flavor tag jet/btagging object
  // using GNN based taggers
  class GNN
  {
  public:
    // recommended constructor, file path + options
    GNN(const std::string& nnFile, const GNNOptions& opts);
    // Formerly private constructor, promoted to public to contain
    // preprocessor macros within a single class in the package: NNSharingSvc
    GNN(ISaltModelPtr, const GNNOptions& opts);
    // redefined options constructor, will share underlying network
    GNN(const GNN&, const GNNOptions& opts);
    // legacy constructor
    GNN(const std::string& nnFile,
        const FlipTagConfig& flip_config = FlipTagConfig::STANDARD,
        const std::map<std::string, std::string>& variableRemapping = {},
        float defaultOutputValue = NAN);
    GNN(GNN&&);
    GNN(const GNN&);
    virtual ~GNN();

    virtual void decorate(const xAOD::IParticle& i_jet) const;
    virtual void decorateWithDefaults(const xAOD::IParticle& jet) const;

    // cppcheck-suppress returnByReference
    FTagDataDependencyNames getDependencies() const;

  private:
    // type definitions for ONNX output decorators
    using TPC = xAOD::TrackParticleContainer;
    using TrackLinks = std::vector<ElementLink<TPC>>;

    template<typename T>
    using Dec = SG::AuxElement::Decorator<T>;

    template<typename T>
    using Decs = std::vector<std::pair<std::string, Dec<T>>>;

    struct Decorators {
      Decs<float> jetFloat;
      Decs<std::vector<char>> jetVecChar;
      Decs<std::vector<float>> jetVecFloat;
      Decs<TrackLinks> jetTrackLinks;
      Decs<char> trackChar;
      Decs<float> trackFloat;
    };

    /* create all decorators */
    std::tuple<FTagDataDependencyNames, std::set<std::string>>
    createDecorators(const OutputConfig& outConfig, const FTagOptions& options);

    ISaltModelPtr m_saltModel;
    std::string m_input_node_name;
    GNNDataLoader m_dataLoader;

    Decorators m_decorators;
    std::vector<std::pair<Dec<float>, float>> m_defaultValues;
    bool m_defaultZeroTracks;
  };
} // end namespace FlavorTagInference
#endif //GNN_H
