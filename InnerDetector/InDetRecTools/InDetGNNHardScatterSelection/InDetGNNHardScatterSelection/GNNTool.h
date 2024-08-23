/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HS_GNN_TOOL_H
#define HS_GNN_TOOL_H

// Tool includes
#include "AsgTools/AsgTool.h"
#include "IVertexDecorator.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"
#include "InDetGNNHardScatterSelection/GNN.h"

#include <memory>
#include <string>
#include <map>

/**
 * @class GNNTool 
 *
 * Tool to score vertex hard-scatter probability
 * using GNN based taggers
 *
 * NOTE: The GNN relies on decorations added in
 * InDetGNNHardScatterSelection::VertexDecoratorAlg and as
 * such should only be called from within that algorithm
 *
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */

namespace InDetGNNHardScatterSelection {

  class GNNTool : public asg::AsgTool,
                  virtual public IVertexDecorator
  {

    ASG_TOOL_CLASS(GNNTool, IVertexDecorator)

    public:
      GNNTool(const std::string& name);
      ~GNNTool();

      StatusCode initialize() override;

      virtual void decorate(const xAOD::Vertex& vertex) const override;

    private:
      Gaudi::Property<std::string> m_nn_file {
        this,
        "nnFile",
        "InDetGNNHardScatterSelection/v0/HSGN2_export_090824.onnx",
        "the path to the netowrk file used to run inference"
      };

      std::unique_ptr<const GNN> m_gnn;
  };
}
#endif
