/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This class is used in conjunction with SaltModel to run inference on a transformer model.
  Whereas SaltModel handles the interfacing with the ONNX runtime, this class handles
  the interfacing with the ATLAS EDM. It is responsible for collecting all the inputs
  needed for inference, running inference (via SaltModel), and decorating the results
  back to ATLAS EDM.
*/

#ifndef PVNN_H
#define PVNN_H

// Tool includes
#include "InDetPVNNSelection/DataPrepUtilities.h"
#include "InDetPVNNSelection/IParticlesLoader.h"
#include "FlavorTagInference/SaltModel.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"

#include <cstddef>
#include <memory>
#include <string>
#include <map>


/**
 * @class InDetPVNNSelection::PVNN
 *
 * Implementation of the PVNN used by the
 * InDetPVNNSelection::PVNNTool
 *
 * NOTE: The PVNN relies on decorations added in
 * InDetPVNNSelection::VertexDecoratorAlg and as
 * such should only be called from within that algorithm
 *
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */
namespace InDetPVNNSelection {

  // Tool to classify hard scatter vertex using PVNN based taggers
  class PVNN
  {
  public:
    PVNN(const std::string& nnFile);
    PVNN(PVNN&&) = delete;
    PVNN(const PVNN&) = delete;
    virtual ~PVNN();

    virtual float decorate(const xAOD::Vertex& vertex) const;

  private:
    // type definitions for ONNX output decorators
    using TPC = xAOD::TrackParticleContainer;
    using TrackLinks = std::vector<ElementLink<TPC>>;

    template<typename T>
    using Dec = SG::Decorator<T>;

    template<typename T>
    using Decs = std::vector<std::pair<std::string, Dec<T>>>;

    struct Decorators {
      Decs<float> vertexFloat;
    };

    /* create all decorators */
    std::set<std::string> createDecorators(const FlavorTagInference::OutputConfig& outConfig);

    std::shared_ptr<const FlavorTagInference::SaltModel> m_saltModel;
    std::string m_modelPath;
    std::string m_input_node_name;
    std::vector<internal::VarFromVertex> m_varsFromVertex;
    std::vector<std::shared_ptr<IConstituentsLoader>> m_constituentsLoaders;
    Decorators m_decorators;
    float m_defaultValue{};
  };
} // end namespace InDetPVNNSelection
#endif // PVNN_H
