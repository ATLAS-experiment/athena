/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGINFERENCE_PASSTHROUGHMODELSVC_H
#define FLAVORTAGINFERENCE_PASSTHROUGHMODELSVC_H

#include "FlavorTagInference/INNSharingSvc.h"
#include "FlavorTagInference/GNN.h"
#include "FlavorTagInference/GNNOptions.h"
#include "AsgServices/AsgService.h"
#include "AsgTools/PropertyWrapper.h"

#include <memory>

namespace FlavorTagInference {

  /// Service that creates GNN objects wrapping a PassThroughSaltModel.
  /// Configured via a JSON file listing variables to pass through.
  class PassThroughModelSvc
    : public extends<asg::AsgService, INNSharingSvc>
  {
  public:
    using extends::extends;

    virtual StatusCode initialize() override;

    virtual std::shared_ptr<const GNN> get(
      const std::string& nn_name,
      const GNNOptions& opts) override;

  private:
    Gaudi::Property<std::string> m_jsonFile {
      this, "JsonFile", "",
      "Path to JSON config listing pass-through variables"};

    Gaudi::Property<std::map<std::string, std::string>> m_variableRemapping {
      this, "VariableRemapping", {},
      "Map from default link names to actual names, e.g. "
      "BTagTrackToJetAssociator -> GhostTrack"};

    std::shared_ptr<const GNN> m_gnn;
  };

} // namespace FlavorTagInference

#endif
