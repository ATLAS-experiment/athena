/*
  Copyright (C) 2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASINTERFACES_IPHYSICSINITIALIZATION_H
#define G4ATLASINTERFACES_IPHYSICSINITIALIZATION_H

#include <GaudiKernel/IAlgTool.h>

/**
 * Interface for tools requiring initialization
 * after Geant4 physics have been initialized.
 *  @author Julien Esseiva
 *  @date   2025-07-22
 */
class IPhysicsInitializationTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IPhysicsInitializationTool, 1, 0);
    virtual StatusCode initializePhysics() = 0;
};
#endif