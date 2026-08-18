/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASINTERFACES_IPHYSICSLISTSVC_H
#define G4ATLASINTERFACES_IPHYSICSLISTSVC_H

#include "GaudiKernel/IService.h"

class G4VUserPhysicsList;

/** @class IPhysicsListSvc IPhysicsListSvc.h "G4AtlasInterfaces/IPhysicsListSvc.h"
 *
 *  Abstract interface to Geant4 Physics list classes
 *
 *  @author Andrea Dell'Acqua
 *  @date   2014-10-01
 */

class IPhysicsListSvc : virtual public IService
{
public:
  IPhysicsListSvc() {}
  virtual ~IPhysicsListSvc() {}
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(IPhysicsListSvc, 1, 0);

  /** Build the physics list.  This method should be called when G4 is ready
      for the initialization of the physics list */
  virtual void CreatePhysicsList () = 0 ;

  /** Simple function to get the physics list pointer, in case it is needed */
  virtual G4VUserPhysicsList* GetPhysicsList() = 0 ;

  /** Get the run manager out and assign the physics list to it.  This should
      be called once the run manager exists, before the physics list initialization
      needs to happen. */
  virtual void SetPhysicsList() = 0 ;

  /** Set all physics options with the ordering expected by legacy run managers. */
  virtual void SetPhysicsOptions() = 0 ;

  /** Set global physics-list options before run-manager initialization. */
  virtual void SetPhysicsListOptions() = 0 ;

  /** Set options requiring processes to have already been constructed. */
  virtual void SetPhysicsProcessOptions() = 0 ;

};
#endif // G4ATLASINTERFACES_IPHYSICSLISTSVC_H
