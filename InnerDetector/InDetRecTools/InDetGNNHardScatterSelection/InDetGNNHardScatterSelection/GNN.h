/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This class is used in conjunction with SaltModel to run inference on a GNN model.
  Whereas SaltModel handles the interfacing with the ONNX runtime, this class handles
  the interfacing with the ATLAS EDM. It is responsible for collecting all the inputs
  needed for inference, running inference (via SaltModel), and decorating the results
  back to ATLAS EDM.
*/

#ifndef HS_GNN_H
#define HS_GNN_H

// Tool includes
#include "InDetGNNHardScatterSelection/DataPrepUtilities.h"
#include "InDetGNNHardScatterSelection/IParticlesLoader.h"
#include "FlavorTagInference/SaltModel.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"

#include <memory>
#include <string>
#include <map>


/**
 * @class InDetGNNHardScatterSelection::GNN
 *
 * Implementation of the GNN used by the
 * InDetGNNHardScatterSelection::GNNTool
 *
 * NOTE: The GNN relies on decorations added in
 * InDetGNNHardScatterSelection::VertexDecoratorAlg and as
 * such should only be called from within that algorithm
 *
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */
namespace InDetGNNHardScatterSelection {

  // Tool to classify hard scatter vertex using GNN based taggers
  class GNN
  {
  public:
    GNN(const std::string& nnFile);
    GNN(GNN&&);
    GNN(const GNN&);
    virtual ~GNN();

    virtual void decorate(const xAOD::Vertex& verrtex) const;

    std::shared_ptr<const FlavorTagInference::SaltModel> m_saltModel;
  private:
    // type definitions for ONNX output decorators
    using TPC = xAOD::TrackParticleContainer;
    using TrackLinks = std::vector<ElementLink<TPC>>;

    template<typename T>
    using Dec = SG::AuxElement::Decorator<T>;

    template<typename T>
    using Decs = std::vector<std::pair<std::string, Dec<T>>>;

    struct Decorators {
      Decs<float> vertexFloat;
    };

    /* create all decorators */
    std::set<std::string> createDecorators(const FlavorTagInference::SaltModel::OutputConfig& outConfig);
    
    std::string m_input_node_name;
    std::vector<internal::VarFromVertex> m_varsFromVertex;
    std::vector<std::shared_ptr<IConstituentsLoader>> m_constituentsLoaders;

    Decorators m_decorators;
    float m_defaultValue;
  };
} // end namespace InDetGNNHardScatterSelection
#endif // HS_GNN_H
