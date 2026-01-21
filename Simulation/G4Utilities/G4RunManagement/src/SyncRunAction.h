/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_SyncRunAction_H
#define G4RUNMANAGEMENT_SyncRunAction_H

#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4UserRunAction.hh"

class G4Run;

class AtlasGeant4DataInterface;

namespace G4UA
{

class SyncRunAction : public G4UserRunAction
{
  public:
    SyncRunAction(IG4RunTool*);

    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;

private:
  IG4RunTool* m_g4RunTool{};
};

} // namespace G4UA

#endif

