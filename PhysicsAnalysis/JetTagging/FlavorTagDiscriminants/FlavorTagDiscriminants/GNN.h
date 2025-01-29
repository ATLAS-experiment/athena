/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This class is used in conjunction with OnnxUtil to run inference on a GNN model.
  Whereas OnnxUtil handles the interfacing with the ONNX runtime, this class handles
  the interfacing with the ATLAS EDM. It is responsible for collecting all the inputs
  needed for inference, running inference (via OnnxUtil), and decorating the results
  back to ATLAS EDM.
*/

#ifndef GNN_H
#define GNN_H

// Tool includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "FlavorTagDiscriminants/GNNOptions.h"

#include "FlavorTagDiscriminants/DataPrepUtilities.h"

// EDM includes
#include "xAODBTagging/BTaggingFwd.h"
#include "xAODJet/JetContainer.h"

#include <memory>
#include <string>
#include <map>

namespace FlavorTagDiscriminants {

  struct GNNOptions;
  class OnnxUtil;
  //
  // Tool to to flavor tag jet/btagging object
  // using GNN based taggers
  class GNN
  {
  public:
    // recommended constructor, file path + options
    GNN(const std::string& nnFile, const GNNOptions& opts);
    // redefined options constructor, will share underlying network
    GNN(const GNN&, const GNNOptions& opts);
    // legacy constructor
    GNN(const std::string& nnFile,
        const FlipTagConfig& flip_config = FlipTagConfig::STANDARD,
        const std::map<std::string, std::string>& variableRemapping = {},
        const TrackLinkType trackLinkType = TrackLinkType::TRACK_PARTICLE,
        float defaultOutputValue = NAN);
    GNN(GNN&&);
    GNN(const GNN&);
    virtual ~GNN();

    virtual void decorate(const xAOD::BTagging& btag) const;
    virtual void decorate(const xAOD::Jet& jet) const;
    virtual void decorateWithDefaults(const SG::AuxElement& jet) const;
    void decorate(const xAOD::Jet& jet, const SG::AuxElement& decorated) const;

    virtual std::set<std::string> getDecoratorKeys() const;
    virtual std::set<std::string> getAuxInputKeys() const;
    virtual std::set<std::string> getConstituentAuxInputKeys() const;

    std::shared_ptr<const OnnxUtil> m_onnxUtil;
  private:
    // private constructor, delegate of the above public ones
    GNN(std::shared_ptr<const OnnxUtil>, const GNNOptions& opts);
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
    createDecorators(const OnnxUtil::OutputConfig& outConfig, const FTagOptions& options);

    SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>> m_jetLink;
    std::string m_input_node_name;
    std::vector<internal::VarFromBTag> m_varsFromBTag;
    std::vector<internal::VarFromJet> m_varsFromJet;
    std::vector<std::shared_ptr<IConstituentsLoader>> m_constituentsLoaders;

    Decorators m_decorators;
    std::vector<std::pair<Dec<float>, float>> m_defaultValues;
    FTagDataDependencyNames m_dataDependencyNames;
    bool m_defaultZeroTracks;
  };
} // end namespace FlavorTagDiscriminants
#endif //GNN_H
