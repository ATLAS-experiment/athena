/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_SyncRunActionTool_H
#define G4RUNMANAGEMENT_SyncRunActionTool_H

#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "SyncRunAction.h"

namespace G4UA {

class SyncRunActionTool : public UserActionToolBase<SyncRunAction> {

 public:
  /// Standard constructor
  using UserActionToolBase<SyncRunAction>::UserActionToolBase;

  void G4RunTool(IG4RunTool* g4RunTool) override {
    m_g4RunTool = g4RunTool;
  }

 protected:
  /// Create the action for the current thread
  virtual std::unique_ptr<SyncRunAction> makeAndFillAction(
      G4AtlasUserActions&) override final;

  IG4RunTool* m_g4RunTool{};

};  // class SyncRunActionTool

}  // namespace G4UA

#endif