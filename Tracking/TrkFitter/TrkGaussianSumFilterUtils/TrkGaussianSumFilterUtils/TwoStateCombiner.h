/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   TwoStateCombiner.h
 * @date   Monday 6th Junbe 2025
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 * "Track fitting with non-Gaussian noise"
 * R. Fruhwirth. Section 4
 */

#ifndef TwoStateCombiner_H
#define TwoStateCombiner_H

#include "TrkParameters/ComponentParameters.h"
namespace Trk {

namespace TwoStateCombiner {

/** @brief Helper to combine forward with  smoother MultiComponentStates*/
Trk::MultiComponentState combine(
    const Trk::MultiComponentState& forwardsMultiState,
    const Trk::MultiComponentState& smootherMultiState,
    unsigned int maximumNumberOfComponents);

}  // namespace GsfBayessianSmoother
}  // namespace Trk
#endif
