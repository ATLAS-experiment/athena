/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNN_TOOL_H
#define GNN_TOOL_H

// Tool includes
#include "AsgTools/AsgTool.h"
#include "AsgServices/ServiceHandle.h"
#include "FlavorTagInference/INNSharingSvc.h"
#include "FlavorTagInference/IJetTagConditionalDecorator.h"

#include "FlavorTagInference/GNNToolifiers.h"

// EDM includes
#include "xAODJet/JetFwd.h"

#include <memory>
#include <string>
#include <map>

namespace FlavorTagInference {

  class GNN;

  //
  // Tool to to flavor tag jet/btagging object
  // using GNN based taggers
  class GNNTool : public asg::AsgTool,
                  virtual public IJetTagConditionalDecorator
  {

    ASG_TOOL_CLASS(
      GNNTool,
      IJetTagConditionalDecorator)
    public:
      GNNTool(const std::string& name);
      ~GNNTool();

      StatusCode initialize() override;

      virtual void decorate(const xAOD::IParticle& i_jet) const override;
      virtual void decorateWithDefaults(const xAOD::IParticle& i_jet) const override;

      virtual std::set<std::string> getDecoratorKeys() const override;
      virtual std::set<std::string> getAuxInputKeys() const override;
      virtual std::set<std::string> getConstituentAuxInputKeys() const override;

    private:

    ServiceHandle<INNSharingSvc> m_nnsvc {
      this, "nnSharingService", "", "NN sharing service"};
    std::string m_nn_file;
    GNNToolProperties m_props;
    std::shared_ptr<const GNN> m_gnn;
  };
}
#endif
