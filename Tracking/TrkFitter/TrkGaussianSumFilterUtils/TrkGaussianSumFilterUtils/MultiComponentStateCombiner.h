/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   MultiComponentStateCombiner.h
 * @date   Monday 20th December 2004
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 * Methods that take a multi-component state and collapses
 * all components returning a single set of track
 * parameters with single mean and covariance matrix.
 */

#ifndef MultiComponentStateCombiner_H
#define MultiComponentStateCombiner_H

#include "TrkParameters/ComponentParameters.h"
namespace Trk {

namespace MultiComponentStateCombiner {

/** @bried Calculate combined state of many components */
std::unique_ptr<Trk::TrackParameters>
combineToSingle(const MultiComponentState&,
                const bool useMode = false);

/** @brief Combine parameters based on their relevant weigths*/
void
combineParametersWithWeight(AmgVector(5) & firstParameters,
                            double& firstWeight,
                            const AmgVector(5) & secondParameters,
                            const double secondWeight);

/** @brief Combine cov matrices based on their relevant weights*/
void
combineCovWithWeight(const AmgVector(5) & firstParameters,
                     AmgSymMatrix(5) & firstMeasuredCov,
                     const double firstWeight,
                     const AmgVector(5) & secondParameters,
                     const AmgSymMatrix(5) & secondMeasuredCov,
                     const double secondWeight);

/** @brief Helper to combine forward with  smoother MultiComponentStates*/
Trk::MultiComponentState
combineWithSmoother(const Trk::MultiComponentState& forwardsMultiState,
                    const Trk::MultiComponentState& smootherMultiState,
                    unsigned int maximumNumberOfComponents);

}//end of MultiComponentStateCombiner namespace
} // end Trk namespace
#endif
