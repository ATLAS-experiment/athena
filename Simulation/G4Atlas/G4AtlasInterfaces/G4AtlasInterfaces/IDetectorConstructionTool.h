/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4AtlasInterfaces_IDetectorConstructionTool_H
#define G4AtlasInterfaces_IDetectorConstructionTool_H

// Include files
#include <string>
#include <vector>
// from Gaudi
#include "GaudiKernel/IAlgTool.h"

class G4VPhysicalVolume;
class G4VUserDetectorConstruction;

/** @class IDetectorConstructionTool IDetectorConstructionTool.h "G4AtlasInterfaces/IDetectorConstructionTool.h"
 *
 *  Abstract interface to a detector construction tool.
 *
 *  @author ADA
 *  @date   2015-02-20
 */

class IDetectorConstructionTool : virtual public IAlgTool {
 public:
  // When using the default deleter, std::unique_ptr requires a complete type at
  // the point where the destructor is called (i.e. when calling std::unique_ptr
  // destructor, move assignment, and reset()). By having implementers of this
  // interface provide a custom deleter, clients don't need to have a complete
  // type for G4VUserDetectorConstruction
  using Deleter = std::function<void(G4VUserDetectorConstruction*)>;
  using UPDetectorConstruction =
      std::unique_ptr<G4VUserDetectorConstruction, Deleter>;
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(IDetectorConstructionTool, 1, 0);

  virtual UPDetectorConstruction GetDetectorConstruction() = 0;

  virtual std::vector<std::string>& GetParallelWorldNames()  = 0;
};
#endif
