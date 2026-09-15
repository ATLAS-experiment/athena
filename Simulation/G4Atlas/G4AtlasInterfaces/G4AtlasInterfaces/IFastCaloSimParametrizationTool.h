/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4AtlasInterfaces_IFastCaloSimParametrizationTool_H
#define G4AtlasInterfaces_IFastCaloSimParametrizationTool_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// Geant4
#include "G4FieldTrack.hh"
#include "G4Track.hh"
// FastCaloSim
#include "FastCaloSim/Core/TFCSExtrapolationState.h"
#include "FastCaloSim/Core/TFCSParametrizationBase.h"
#include "FastCaloSim/Core/TFCSSimulationState.h"
#include "FastCaloSim/Core/TFCSTruthState.h"

class IFastCaloSimParametrizationTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IFastCaloSimParametrizationTool, 1, 0);

  /// Create the shared transport world on the master thread.
  virtual StatusCode initializeTransportGeometry() = 0;

  /// Create the thread-local propagator after the transport world exists.
  virtual StatusCode initializeTransportPropagator() = 0;

  /// Transport a particle through the calorimeter and record its steps.
  virtual std::vector<G4FieldTrack> transport(const G4Track &G4InputTrack) = 0;

  /// Extrapolate transport steps to the ID-Calo boundary and calorimeter layers.
  virtual void extrapolate(TFCSExtrapolationState &result,
                           const TFCSTruthState *truth,
                           const std::vector<G4FieldTrack> &caloSteps) = 0;

  /// Simulate calorimeter cell energy deposits.
  virtual FCSReturnCode simulate(TFCSSimulationState &simulstate,
                                 const TFCSTruthState *truth,
                                 const TFCSExtrapolationState *extrapol) = 0;
};

#endif  // G4AtlasInterfaces_IFastCaloSimParametrizationTool_H
