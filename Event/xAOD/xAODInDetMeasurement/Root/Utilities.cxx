/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 
*/

#include "xAODInDetMeasurement/Utilities.h"

namespace xAOD::xAODInDetMeasurement::Utilities {

  float computeTotalCharge( const SG::AuxElement& cluster) {
    static const SG::AuxElement::Accessor<std::vector<float> > chargesAcc("chargeList");
    assert( chargesAcc.isAvailable( cluster ) );
    return xAOD::xAODInDetMeasurement::Utilities::computeTotalCharge( chargesAcc(cluster) );
  }

  float computeTotalCharge( const std::vector<float>& charges) {
    float totalCharge = 0.f;
    for (auto& charge : charges)
      totalCharge += charge;
    return totalCharge;
  }

  int computeTotalToT( const SG::AuxElement& cluster) {
    static const SG::AuxElement::Accessor< std::vector<int> > totsAcc("totList");
    assert( totsAcc.isAvailable( cluster ) );
    return xAOD::xAODInDetMeasurement::Utilities::computeTotalToT( totsAcc(cluster) );
  }

  int computeTotalToT( const std::vector<int>& tots) {
    int totalToT = 0;
    for (auto& tot : tots)
      totalToT += tot;
    return totalToT;
  }

}
