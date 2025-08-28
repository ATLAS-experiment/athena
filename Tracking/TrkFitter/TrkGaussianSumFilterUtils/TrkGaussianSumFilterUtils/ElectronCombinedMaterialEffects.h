/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   ElectronCombinedMaterialEffects.h
 * @date   Friday 11th January 2005
 * @author Tom Athkinson, Anthony Morley, Christos Anastopoulos
 * Class definition for consideration of multiple scatter and energy
 * loss effects from material simultaneously
 */

#ifndef TrkElectronCombinedMaterialEffects_H
#define TrkElectronCombinedMaterialEffects_H

#include "TrkGaussianSumFilterUtils/GsfConstants.h"
#include "TrkGaussianSumFilterUtils/GsfMaterial.h"
//
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkParameters/ComponentParameters.h"

namespace Trk {

class ElectronCombinedMaterialEffects
{
public:
  /** Helper Struct for multiple Gaussian components*/
  struct ComponentValues
  {
    double weight;
    double mean;
    double variance;
  };
  using MixtureParameters = std::array<ComponentValues, GSFConstants::maxNumberofMatComponents>;
  using Polynomial = std::array<double, GSFConstants::polynomialCoefficients>;

  // ctor with arguments
  ElectronCombinedMaterialEffects(
      const std::string& parameterisationFileName,
      const std::string& parameterisationFileNameHighX0);

  void compute(GsfMaterial::Combined&,
               const Trk::ComponentParameters&,
               const Trk::MaterialProperties&,
               double,
               Trk::PropDirection = anyDirection) const;

private:
  /* brief Electron energy loss due to Bremsstrahlung*/
  void BetheHeitler(GsfMaterial::EnergyLoss& cache,
                    const ComponentParameters& componentParameters,
                    const MaterialProperties& materialProperties,
                    double pathLenght,
                    PropDirection direction = anyDirection) const;

  //Actual number of Material components
  int m_BHnumberOfComponents{};
  int m_BHnumberOfComponentsHighX0{};
  //These are nested std arrays.
  std::array<Polynomial, GSFConstants::maxNumberofMatComponents> m_BHpolynomialWeights{};
  std::array<Polynomial, GSFConstants::maxNumberofMatComponents> m_BHpolynomialMeans{};
  std::array<Polynomial, GSFConstants::maxNumberofMatComponents> m_BHpolynomialVariances{};
  std::array<Polynomial, GSFConstants::maxNumberofMatComponents> m_BHpolynomialWeightsHighX0{};
  std::array<Polynomial, GSFConstants::maxNumberofMatComponents> m_BHpolynomialMeansHighX0{};
  std::array<Polynomial, GSFConstants::maxNumberofMatComponents> m_BHpolynomialVariancesHighX0{};
};

} // end Trk namespace

#endif
