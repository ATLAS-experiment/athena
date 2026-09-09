/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASINTERFACES_IG4RunTool_H
#define G4ATLASINTERFACES_IG4RunTool_H

#include <GaudiKernel/IAlgTool.h>
#include <memory>

// Forward declarations
class AtlasG4SyncEventUserInfo;

/** @class IG4RunTool IG4RunTool.h "G4AtlasInterfaces/IG4RunTool.h"
 *  
 *  Provides an interface to interact with the Geant4 run
 * 
 *  @author Julien Esseiva
 *  @date   2025-07-10
 */

class IG4RunTool : virtual public IAlgTool {
 public:
  // type alias
  using UPEvent = std::unique_ptr<AtlasG4SyncEventUserInfo>;

  DeclareInterfaceID(IG4RunTool, 1, 0);

  // Synchronization methods
  virtual void NotifyBeginRun() = 0;
  /// Wait for BeginOfRun, or return failure if the Geant4 thread stops first.
  virtual StatusCode WaitBeginRun() = 0;
  
  // Event queue management
  virtual size_t Size() const = 0;
  // Push a non-null event to the queue. Null events are rejected; queue
  // shutdown is controlled internally by G4RunTool::finalize().
  virtual void PushEvent(UPEvent) = 0;
  // pop the event from the queue and return it
  virtual UPEvent GetEvent() = 0;
};

#endif
