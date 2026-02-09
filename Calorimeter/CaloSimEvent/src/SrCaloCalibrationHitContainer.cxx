/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// 09-Feb-2004 WGS: The following code was duplicated from
// CaloHitContainer.cxx and modified for CaloCalibrationHits.

#include "CaloSimEvent/SrCaloCalibrationHitContainer.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <utility>

#include "GaudiKernel/System.h"

SrCaloCalibrationHitContainer::SrCaloCalibrationHitContainer(
    const std::string& collectionName)
    : AthenaHitsVector<CaloCalibrationHit>(collectionName) {}

SrCaloCalibrationHitContainer::~SrCaloCalibrationHitContainer() = default;

SrCaloCalibrationHitContainer::operator std::string() const {
  std::string newline("\n");
  std::ostringstream ss;
  ss << System::typeinfoName(typeid(*this));
  ss << ": content ";
  ss << newline;

  int counter = 0;

  for (const CaloCalibrationHit* hit : *this) {  // Loop over Hits
    ss << "CaloCalibrationHit[";
    ss << counter;
    ss << "] ";

    ss << " ID = " << std::hex << hit->cellID().get_identifier32().get_compact()
       << std::dec << " ;  ";

    ss << " E=(" << hit->energy(0) << "," << hit->energy(1) << ","
       << hit->energy(2) << "," << hit->energy(3) << ") MeV";

    ss << newline;

    counter++;
  }

  ss << newline;
  ss << "Number of Hits in this container : ";
  ss << counter;

  return ss.str();
}
