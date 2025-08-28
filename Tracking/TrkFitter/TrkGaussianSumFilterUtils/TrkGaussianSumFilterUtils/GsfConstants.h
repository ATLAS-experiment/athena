/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @author Anthony Morley, Christos Anastopoulos
 * @brief  Collect constants we use
 * in GSF and their meaning in one place
 */
#ifndef GSFCONSTANTS_H
#define GSFCONSTANTS_H
#include "TrkGaussianSumFilterUtils/GsfFindIndexOfMinimum.h"
#include <cstddef>
#include <cstdint>
namespace GSFConstants {
/**
 * @brief Alignment used  for SIMD operations
 * internally to GSF.
 */
constexpr size_t alignment = vAlgs::alignmentForArray<256>();

/**
 * @brief The state is described by N Gaussian components
 * The Beth Heitler Material effect are also described
 * by M components.
 * Thee components are parametetrization
 * via polynomials  with C coeffiencts.
 *
 * The number of coefficients
 * is assumed to be fixed as all the parametrization
 * have the same number.
 *
 * The max number of Material Components
 * and maxNumberOfStateComponents are
 * more constraint by "reason".
 *
 * These numbers also mean we can use
 * std::array in places.
 */

/// Maximum number of Gaussian components for the
/// state description.
constexpr int8_t maxNumberofStateComponents = 12;
/// Maximum number of Gaussian components for the
/// material effects description
constexpr int8_t maxNumberofMatComponents = 6;
/// Number of coefficients for the polynomials,
/// parametrizing the mean,variace, weights of the
/// Gaussian components describing the material effects.
constexpr int8_t polynomialCoefficients = 6;
}
#endif
