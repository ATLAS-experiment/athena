/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/HGTD_TimeResolutionTool.h
 *
 * @author Jernej Debevc <jernej.debevc@cern.ch>
 *
 * @brief Time resolution interface for HGTD LGAD pixels.
 */

#ifndef HGTD_TIMERESOLUTIONTOOL_H
#define HGTD_TIMERESOLUTIONTOOL_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"

class HGTD_TimeResolutionTool : public AthAlgTool {

 public:
  HGTD_TimeResolutionTool(const std::string& type, const std::string& name,
                          const IInterface* parent);

  /**
   * @brief Returns the time resolution of an HGTD pixel.
   *
   * This function computes the expected time resolution of a hit on a single
   * HGTD pixel based on the deposited charge, the radial position within the
   * detector, and integrated luminosity.
   *
   * @param depositedCharge The charge deposited in the pixel.
   * @param radius The radial distance of the pixel from the beam pipe.
   * @param integratedLumi The total integrated luminosity since the start of
   * Run 4. Has to be given in units of fb^-1.
   */
  float timeResolution(const double depositedCharge, const double radius,
                       const double integratedLumi) const;

 private:
  Gaudi::Property<double> m_chargedNeutralRatio{
      this, "ChargedNeutralRatio", 3.0,
      "Ratio of radiation damage from charged hadrons vs neutrons"};

  /**
   * @brief Corrects the integrated luminosity for possible module replacements.
   *
   * This function takes the total integrated luminosity since the start of Run
   * 4 and corrects it by accounting for any replacements of HGTD modules done
   * at a given radius.
   *
   * @param integratedLumi The total integrated luminosity since the start of
   * Run 4. Has to be given in units of fb^-1.
   * @param radius The radial distance of the pixel from the beam pipe.
   *
   * @return The integrated luminosity the sensor was exposed to.
   */
  double replacementCorrectedLuminosity(const double integratedLumi,
                                        const double radius) const;

  /**
   * @brief Helper function to determine the latest replacement luminosity.
   *
   * Given a list of integrated luminosities at which replacements of HGTD
   * modules occured, this function finds and returns the most recent
   * replacement luminosity.
   *
   * @param integratedLumi The total integrated luminosity since the start of
   * Run 4.
   * @param replacementLumis A vector containing luminosities at which
   * replacements occured (sorted low-to-high).
   */
  double latestReplacementLuminosity(
      const double integratedLumi,
      const std::vector<double>& replacementLumis) const;

  /**
   * @brief Returns the fluence received by the sensor in neq/cm^2.
   *
   * @param sensorAccumulatedLumi The integrated luminosity the sensor was
   * exposed to. Has to be given in units of fb^-1.
   * @param radius The radial distance of the pixel from the beam pipe.
   */
  double sensorFluence(const double sensorAccumulatedLumi,
                       const double radius) const;

  /**
   * @brief Returns the time resolution contribution from Landau fluctuations.
   *
   * This function computes the standard deviation of the time resolution
   * contribution coming from Landau fluctuations, which arise from the
   * variablity in charge deposition and collection in the sensor active region.
   *
   * @param fluence The radiation fluence in neq/cm^2 the sensor was exposed to.
   */
  double sigmaLandau(const double fluence) const;

  /**
   * @brief Returns the time resolution contribution from the electronics jitter
   * of ALTIROC.
   *
   * This function computes the standard deviation of the time resolution
   * contribution coming from the jitter due to electronics noise of the ALTIROC
   * ASIC.
   *
   * @param depositedCharge The charge deposited in the sensor (in unit of
   * electrons).
   * @param fluence The radiation fluence in neq/cm^2 the sensor was exposed to.
   */
  double sigmaALTIROCJitter(const double depositedCharge,
                            const double fluence) const;

  /**
   * @brief Returns the time resolution contribution from TDC digitization
   * smearing.
   *
   * This function computes the standard deviation of the time resolution
   * contribution coming from the time smearing introduced by the TDC
   * digitization, mainly due to the finite TDC bin size, but also including
   * effects such as TDC bin size smearing etc.
   */
  double sigmaTDC() const;

  /**
   * @brief Returns the time resolution contribution from the LHC clock.
   */
  double sigmaClock() const;
};

#endif  // HGTD_TIMERESOLUTIONTOOL_H
