/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_G4CALOTRANSPORTTOOL_H
#define G4ATLASTOOLS_G4CALOTRANSPORTTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IG4CaloTransportTool.h"

#include "G4Navigator.hh"
#include "G4PropagatorInField.hh"

#include "G4AtlasTools/ThreadLocalHolder.h"


class G4Track;

/// @class G4CaloTransportTool
/// @brief A tool which transports particles through the Geant4 geometry.
///
/// @author Joshua Falco Beirer <joshua.falco.beirer@cern.ch>
///
class G4CaloTransportTool : virtual public extends1<AthAlgTool, IG4CaloTransportTool>
{

  public:

    G4CaloTransportTool(const std::string&, const std::string&, const IInterface*);

    // Algorithm initialize at begin of job
    virtual StatusCode initialize() override final;
    // Algorithm finalize at begin of job
    virtual StatusCode finalize() override final;
    // Initialize propagator for the current thread
    StatusCode initializePropagator() override final;
    // Transport input track through the geometry
    virtual std::vector<G4FieldTrack> transport(const G4Track& G4InputTrack) override final;

  private:
    // Create and return a new propagator
    G4PropagatorInField* makePropagator();
    // Advance track by single Geant4 step in geometry
    void doStep(G4FieldTrack& fieldTrack);
    // Pointer to the physical volume of the world (either simplified or full geometry)
    G4VPhysicalVolume* m_worldVolume;

    // Whether to use simplified geometry for particle transport
    Gaudi::Property<bool> m_useSimplifiedGeo{this, "UseSimplifiedGeo", true, "Use simplified geometry for particle transport"};
    // Name of the logical volume of the simplified world as defined in the loaded GDML file
    Gaudi::Property<std::string> m_simplifiedWorldLogName{this, "SimplifiedWorldLogName", "Name of the logical volume of the simplified world"};
    // Name of volume until which the particle is tracked in transport
    Gaudi::Property<std::string> m_transportLimitVolume{this, "TransportLimitVolume", "Name of the volume until which the particle is transported"};
    // Maximum number of steps in particle transport
    Gaudi::Property<unsigned int> m_maxSteps{this, "MaxSteps", 100, "Maximum number of steps in particle transport"};
    // Thread local holder for propagators
    thread_utils::ThreadLocalHolder<G4PropagatorInField> m_propagatorHolder;


}; // class G4CaloTransportTool


#endif // G4ATLASTOOLS_G4CALOTRANSPORTTOOL_H
