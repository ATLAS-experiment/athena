/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_SyncPrimaryGeneratorActionTool_H
#define G4RUNMANAGEMENT_SyncPrimaryGeneratorActionTool_H

#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "SyncPrimaryGeneratorAction.h"

namespace G4UA {

class SyncPrimaryGeneratorActionTool : public UserActionToolBase<SyncPrimaryGeneratorAction> {

 public:
  /// Standard constructor
  using UserActionToolBase<SyncPrimaryGeneratorAction>::UserActionToolBase;

  void G4RunTool(IG4RunTool* g4RunTool) override {
    m_g4RunTool = g4RunTool;
  }

 protected:
  /// Create the action for the current thread
  virtual std::unique_ptr<SyncPrimaryGeneratorAction> makeAndFillAction(
      G4AtlasUserActions&) override final;

  IG4RunTool* m_g4RunTool{};

};  // class SyncPrimaryGeneratorActionTool

}  // namespace G4UA

#endif