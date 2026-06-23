/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4FASTSIMULATION_IFASTSIMDEDICATEDSD_H
#define LARG4FASTSIMULATION_IFASTSIMDEDICATEDSD_H

#include "LArG4Code/LArG4SimpleSD.h"

#include <utility>

class EnergySpot;
class StoreGateSvc;

/// This is the interface for the fast simulation dedicated sensitive detector.
class IFastSimDedicatedSD : public LArG4SimpleSD {

 public:

  /// Simple constructor and destructor
  IFastSimDedicatedSD(const std::string& name, StoreGateSvc* detStore,
                     std::string hitCollectionName)
    : LArG4SimpleSD(name, detStore, std::move(hitCollectionName)) {}

  ~IFastSimDedicatedSD() {}

  /// ProcessHitsMethod
  /** Process a single energy spot from a frozen shower.
      The appropriate region of the sensitive detector is calculated and a LArIdentifier is constructed*/
  virtual void ProcessSpot(const EnergySpot & spot, double weight) = 0;

};
#endif //LARG4FASTSIMULATION_IFASTSIMDEDICATEDSD_H
