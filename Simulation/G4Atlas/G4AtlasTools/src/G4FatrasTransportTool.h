/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_G4FATRASTRANSPORTTOOL_H
#define G4ATLASTOOLS_G4FATRASTRANSPORTTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IG4FatrasTransportTool.h"
#include "G4PropagatorInField.hh"

#include <memory>
#include <vector>

class G4Track;
class G4VPhysicalVolume;
class G4FieldTrack;

/// @class G4FatrasTransportTool
/// @brief A tool which transports particles through the Geant4 geometry.
///
/// @author Georgios Christou <christog@cern.ch>
///
class G4FatrasTransportTool : virtual public extends<AthAlgTool, IG4FatrasTransportTool>
{

  public:

    G4FatrasTransportTool(const std::string&, const std::string&, const IInterface*);

    // Algorithm finalize at begin of job
    virtual StatusCode finalize() override final;
    // Initialize propagator for the current thread
    StatusCode initializePropagator() override final;
    // Transport input track through the geometry
    virtual std::vector<G4FieldTrack> transport(const G4Track& G4InputTrack) override final;

  private:
    // Deleter for the unique_ptr managing the G4PropagatorInField
    struct Deleter {
      void operator()(G4PropagatorInField*) const;
    };

    // Get the world volume in which the particle transport is performed
    G4VPhysicalVolume* getWorldVolume();
    // Create and return a new propagator
    G4PropagatorInField* makePropagator();
    // Advance track by single Geant4 step in geometry
    void doStep(G4FieldTrack& fieldTrack);
    // Pointer to the physical volume of the world (either simplified or full geometry)
    G4VPhysicalVolume* m_worldVolume{};

    // Whether to use simplified geometry for particle transport
    Gaudi::Property<bool> m_useSimplifiedGeo{this, "UseSimplifiedGeo", false, "Use simplified geometry for particle transport"};
    // Name of the logical volume of the simplified world as defined in the loaded GDML file
    Gaudi::Property<std::string> m_simplifiedWorldLogName{this, "SimplifiedWorldLogName", "Name of the logical volume of the simplified world"};
    // Name of volume until which the particle is tracked in transport
    Gaudi::Property<std::string> m_transportLimitVolume{this, "TransportLimitVolume", "Name of the volume until which the particle is transported"};
    // Maximum number of steps in particle transport
    Gaudi::Property<unsigned int> m_maxSteps{this, "MaxSteps", 1, "Maximum number of steps in particle transport"};
    // Thread local propagator
    static thread_local std::unique_ptr<G4PropagatorInField, Deleter> s_propagator;


}; // class G4FatrasTransportTool


#endif // G4ATLASTOOLS_G4FATRASTRANSPORTTOOL_H
