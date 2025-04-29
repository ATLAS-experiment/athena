/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASINTERFACES_IPHYSICSOPTIONTOOL_H
#define G4ATLASINTERFACES_IPHYSICSOPTIONTOOL_H

#include <memory>
#include "GaudiKernel/IAlgTool.h"
class G4String;
class G4VPhysicsConstructor;

/** @class IPhysicsOptionTool IPhysicsOptionTool.h "G4AtlasInterfaces/IPhysicsOptionTool.h"
 *
 *  Abstract interface to Geant4 Physics list classes
 *
 *  @author Edoardo Farina
 *  @date   2015-05-15
 */

namespace G4AtlasPhysicsOption {
  enum Type {
    BSMPhysics = 1, // Adds new BSM particles and potentially decays
    QS_ExtraParticles = 2, // Adds particles from the PDG Table not currently known to Geant4
    QS_ExtraProc = 3, // Adds MSC and Ionisation processes for specific particles (possibly merge with the next one?)
    GlobalProcesses = 4, // Adds a new physics process for all particles meeting certain criteria
    UnknownType = 5 // Not set
  };
}

class IPhysicsOptionTool : virtual public IAlgTool
{
public:
 using UPPhysicsConstructor = std::unique_ptr<G4VPhysicsConstructor>;

 IPhysicsOptionTool() {}
 virtual ~IPhysicsOptionTool() {}
 /// Creates the InterfaceID and interfaceID() method
 DeclareInterfaceID(IPhysicsOptionTool, 1, 0);

 // Method needed to register G4VPhysicsConstructor into G4VmodularPhysicsList
 virtual UPPhysicsConstructor GetPhysicsOption() = 0;

 virtual G4AtlasPhysicsOption::Type GetOptionType() const {
   return m_physicsOptionType; };

protected:

  G4AtlasPhysicsOption::Type m_physicsOptionType{G4AtlasPhysicsOption::Type::UnknownType};

};
#endif
