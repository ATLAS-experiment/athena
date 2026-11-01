/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4HepEmOptions_h
#define G4HepEmOptions_h 1

#include <string>
#include <vector>

/// Configuration of the G4HepEm tracking manager, used by the
/// FTFP_BERT_ATL_HepEm and FTFP_BERT_ATL_AdePT physics lists.
/// Filled from the PhysicsListSvc properties.
struct G4HepEmOptions
{
  /// Allow multiple steps in the combined MSC+transportation
  bool multipleStepsInMSCTransport{false};
  /// G4Regions where Woodcock tracking of photons is applied
  std::vector<std::string> woodcockTrackingRegions{"EMEC", "EMB"};
  /// Apply the e+ correction to theta0 in the Urban MSC
  bool mscPositronCorrection{true};
  /// Enable e-/e+ energy loss fluctuation (all regions)
  bool energyLossFluctuation{true};
  /// G4Regions where e-/e+ energy loss fluctuation is disabled
  std::vector<std::string> noEnergyLossFluctuationRegions{};
};

#endif
