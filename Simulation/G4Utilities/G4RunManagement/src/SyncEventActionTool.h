/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_SyncEventActionTool_H
#define G4RUNMANAGEMENT_SyncEventActionTool_H

#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "SyncEventAction.h"

namespace G4UA {

class SyncEventActionTool : public UserActionToolBase<SyncEventAction> {

 public:
  /// Standard constructor
  using UserActionToolBase<SyncEventAction>::UserActionToolBase;

 protected:
  /// Create the action for the current thread
  virtual std::unique_ptr<SyncEventAction> makeAndFillAction(
      G4AtlasUserActions&) override final;

};  // class SyncEventActionTool

}  // namespace G4UA

#endif