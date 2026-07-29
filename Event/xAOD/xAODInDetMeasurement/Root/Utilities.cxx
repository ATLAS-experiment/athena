/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/Utilities.h"
#include "AthContainers/JaggedVecAccessor.h"
#include <numeric>

namespace xAOD::xAODInDetMeasurement::Utilities {

  float computeTotalCharge( const SG::AuxElement& cluster) {
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<float> > chargesAcc("chargeList");
    assert( chargesAcc.isAvailable( cluster ) );
    const std::vector<float>& charges = chargesAcc(cluster);
    return std::accumulate(charges.begin(), charges.end(), 0.f);
  }

  int computeTotalToT( const SG::AuxElement& cluster) {
    static const SG::AuxElement::Accessor< SG::JaggedVecElt<int> > totsAcc("totList");
    assert( totsAcc.isAvailable( cluster ) );
    const std::vector<int>& tots = totsAcc(cluster);
    return std::accumulate(tots.begin(), tots.end(), 0);
  }

}
