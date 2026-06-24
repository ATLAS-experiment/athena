#include "CelerOffloadTool.h"

namespace G4UA {
    
  CelerOffloadTool::CelerOffloadTool(const std::string& type, const std::string& name, const IInterface* parent)
    : UserActionToolBase<CelerOffload>(type, name, parent) {}

  StatusCode CelerOffloadTool::initialize() {
    ATH_MSG_INFO("CelerOffload::initialize " << name());

    // Celeritas initialization done in the PhysicsList

    return StatusCode::SUCCESS;
  }

  StatusCode CelerOffloadTool::finalize() {
    ATH_MSG_INFO("CelerOffload::finalize " << name());
    return StatusCode::SUCCESS;
  }

  std::unique_ptr<CelerOffload> CelerOffloadTool::makeAndFillAction(G4AtlasUserActions& actionList) {
    ATH_MSG_INFO("CelerOffload::makeAction " << name());
    auto action = std::make_unique<CelerOffload>();
    actionList.runActions.push_back( action.get() );
    actionList.runActionsMaster.push_back( action.get() );  
    return action;
  }
}

