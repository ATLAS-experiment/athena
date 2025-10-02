/*
+  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MULTIFOLD_GNN_TOOL_H
#define MULTIFOLD_GNN_TOOL_H

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

// needed for map<string,<map<string,float>>
#include "Gaudi/Parsers/Factory.h"


namespace FlavorTagInference {

  class MultifoldGNN;

  //
  // Tool to to flavor tag jet/btagging object
  // using GNN based taggers
  class MultifoldGNNTool : public asg::AsgTool,
                           virtual public IJetTagConditionalDecorator
  {

    ASG_TOOL_CLASS(
      MultifoldGNNTool,
      IJetTagConditionalDecorator)
    public:
      MultifoldGNNTool(const std::string& name);
      ~MultifoldGNNTool();

      StatusCode initialize() override;

      virtual void decorate(const xAOD::IParticle& i_jet) const override;
      virtual void decorateWithDefaults(const xAOD::IParticle& i_jet) const override;

      virtual std::set<std::string> getDecoratorKeys() const override;
      virtual std::set<std::string> getAuxInputKeys() const override;
      virtual std::set<std::string> getConstituentAuxInputKeys() const override;

    private:

    using MMD = std::map<std::string, std::map<std::string, float>>;

    ServiceHandle<FlavorTagInference::INNSharingSvc> m_nnsvc {
      this, "nnSharingService", "", "NN sharing service"};
    std::vector<std::string> m_nn_files;
    std::string m_fold_hash_name;
    FlavorTagInference::GNNToolProperties m_props;
    std::shared_ptr<const MultifoldGNN> m_gnn;
    Gaudi::Property<MMD> m_defaults {
      this, "perFoldDefaultOutputValues", {}, "per-fold defaults"};
  };
}
#endif
