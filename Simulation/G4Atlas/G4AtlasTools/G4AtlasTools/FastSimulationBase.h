/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_FASTSIMULATIONBASE_H
#define G4ATLASTOOLS_FASTSIMULATIONBASE_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IFastSimulation.h"

// Members
#include <G4Region.hh>

// STL library
#include <string>

/// @class FastSimulationBase
/// Lightweight Gaudi tool base class for Geant4 fast-simulation models.
/// It takes care of the per-thread creation of the concrete
/// fast-simulation model and registering it for automatic Geant4 cleanup.
/// Derived tools are responsible for implementing `makeFastSimModel()` and
/// for assigning the returned model to the desired Geant4 regions, which can
/// be accessed via the configured `RegionName` property or the `getRegion()`
/// helper. Multi-threaded jobs will invoke `initializeFastSim` on every worker
/// during detector construction, ensuring each thread instantiates its own model.
class FastSimulationBase : public extends<AthAlgTool, IFastSimulation> {
 public:
  FastSimulationBase(const std::string& type, const std::string& name,
                     const IInterface *parent);

  /// @brief Construct and setup the fast simulation model.
  ///
  /// This method invokes the makeFastSimModel of the derived concrete tool type.
  /// It is the derived class's responsibility to assign the fast simulation model
  /// to the correct regions. The fast simulation model is registered for deletion.
  /// In multi-threaded jobs, this method is called once on each geant4 worker thread during
  /// detector construction (ConstructSDandField).
  StatusCode initializeFastSim() override;

  /** Begin of an athena event - do anything that needs to be done at the beginning of each *athena* event. */
  virtual StatusCode BeginOfAthenaEvent() override { return StatusCode::SUCCESS; }

  /** End of an athena event - do any tidying up required at the end of each *athena* event. */
  virtual StatusCode EndOfAthenaEvent() override { return StatusCode::SUCCESS; }

 protected:
  // Helper to retrieve the region to which this fast simulation is assigned from the region store.
  G4Region* getRegion() const;

  /// The region to which this fast sim is assigned.
  Gaudi::Property<std::string> m_regionName{this, "RegionName", ""};
  /// This Fast Simulation has no regions associated with it.
  Gaudi::Property<bool> m_noRegions{this, "NoRegions", false};
};

#endif
