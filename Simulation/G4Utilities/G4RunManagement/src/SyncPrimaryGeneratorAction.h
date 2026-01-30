/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_SyncPrimaryGeneratorAction_H
#define G4RUNMANAGEMENT_SyncPrimaryGeneratorAction_H

#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4VUserPrimaryGeneratorAction.hh"

class G4Event;

class AtlasGeant4DataInterface;

namespace G4UA
{

class SyncPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  SyncPrimaryGeneratorAction(IG4RunTool*);

  virtual void GeneratePrimaries(G4Event*) override;

private:
  IG4RunTool* m_g4RunTool{};
};

} // namespace G4UA

#endif
