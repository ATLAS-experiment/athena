/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Calibration/src/HGTD_TimeResolutionTool.cxx
 *
 * @author Jernej Debevc <jernej.debevc@cern.ch>
 */

#include "HGTD_Calibration/HGTD_TimeResolutionTool.h"

#include <algorithm>
#include <cmath>

#include "AthenaKernel/Units.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Math/Polynomial.h"

HGTD_TimeResolutionTool::HGTD_TimeResolutionTool(const std::string &type,
                                                 const std::string &name,
                                                 const IInterface *parent)
    : AthAlgTool(type, name, parent) {}

float HGTD_TimeResolutionTool::timeResolution(
    const double depositedCharge, const double radius,
    const double integratedLumi) const {

  const double receivedFluence{sensorFluence(
      replacementCorrectedLuminosity(integratedLumi, radius), radius)};

  // Multiple processes contribute to the total time resolution. They are added
  // here in quadrature one-by-one.
  double timeResVariance{0.0};
  timeResVariance += std::pow(sigmaLandau(receivedFluence), 2.0);
  timeResVariance +=
      std::pow(sigmaALTIROCJitter(depositedCharge, receivedFluence), 2.0);
  timeResVariance += std::pow(sigmaTDC(), 2.0);
  timeResVariance += std::pow(sigmaClock(), 2.0);

  return std::sqrt(timeResVariance);
}

double HGTD_TimeResolutionTool::replacementCorrectedLuminosity(
    const double integratedLumi, const double radius) const {

  constexpr double innerRingRadius{230.0 * Athena::Units::mm};
  constexpr double middleRingRadius{470.0 * Athena::Units::mm};

  const std::vector<double> innerRingReplacementLumis{
      // Unit is fb^-1
      1000.0,
      2000.0,
      3000.0,
  };

  const std::vector<double> middleRingReplacementLumis{
      // Unit is fb^-1
      2000.0,
  };

  double replacementLumi{0.0};

  if (radius < innerRingRadius) {
    replacementLumi =
        latestReplacementLuminosity(integratedLumi, innerRingReplacementLumis);
  } else if (radius < middleRingRadius) {
    replacementLumi =
        latestReplacementLuminosity(integratedLumi, middleRingReplacementLumis);
  }
  // Outer ring does not have replacements - replacement lumi is 0.0

  return integratedLumi - replacementLumi;
}

double HGTD_TimeResolutionTool::latestReplacementLuminosity(
    const double integratedLumi,
    const std::vector<double> &replacementLumis) const {

  auto it = std::lower_bound(replacementLumis.begin(), replacementLumis.end(),
                             integratedLumi);

  return it == replacementLumis.begin() ? 0.0 : *(--it);
}

double HGTD_TimeResolutionTool::sensorFluence(
    const double sensorAccumulatedLumi, const double radius) const {

  // Maximum luminosity expected by sensors in fb^-1
  constexpr double maxSensorLuminosity{4000.0};

  // 4th order polynomial parametrization of the neutron fluence as function of
  // radius in cm.
  ROOT::Math::Polynomial neutral(2.82428e+08, -5.22843e+10, 3.62182e+12,
                                 -1.4085e+14, 4.08821e+15);

  // Equivalent as line above, but for charged hadrons.
  ROOT::Math::Polynomial charged(1.03139e+09, -2.06558e+11, 1.53897e+13,
                                 -5.18627e+14, 7.17046e+15);

  return (neutral(radius / Athena::Units::cm) +
          m_chargedNeutralRatio * charged(radius / Athena::Units::cm)) *
         sensorAccumulatedLumi / maxSensorLuminosity;
}

double HGTD_TimeResolutionTool::sigmaLandau(const double fluence) const {

  // Parametrization of sensor bias voltage as a function of fluence in 1e14
  // neq/cm^2. Returns voltage in V.
  ROOT::Math::Polynomial biasVoltageVsFluence(20.59, 96.26);

  // LGADs can be biased at max 550 V due to single-event burnout. Unit V
  const double operatingBiasVoltage{
      std::min(biasVoltageVsFluence(fluence / 1.0e14), 550.0)};

  constexpr double sensorActiveThickness{50.0 * Athena::Units::um};
  constexpr double gainLayerVoltage{25.0};  // Unit V

  const double averageElectricField{
      (operatingBiasVoltage - gainLayerVoltage) /
      (sensorActiveThickness / Athena::Units::um)};  // Unit V/um

  // Parametrization of the Landau fluctuation contribution to time resolution
  // as a function of average electric field in V/um:  c1 + c2 / <E>
  return (24.441 + 20.2311 / averageElectricField) * Athena::Units::picosecond;
}

double HGTD_TimeResolutionTool::sigmaALTIROCJitter(const double depositedCharge,
                                                   const double fluence) const {

  constexpr double maxJitter{999.9 * Athena::Units::ns};

  // Handle the case where insufficient charge is collected at high fluence and
  // jitter goes to infinity.
  if (fluence > 3.313e15 || depositedCharge == 0.0) {
    return maxJitter;
  }

  // Convert deposited charge to fC
  const double chargeInfC{depositedCharge / (1.0e-15 * Gaudi::Units::coulomb)};

  // Parametrization of the collected charge as a function of fluence in 1e14
  // neq/cm^2 for a reference 0.56 fC deposited charge.
  ROOT::Math::Polynomial referenceCollectedChargeVsFluence(-0.01598, -0.0752,
                                                           20.04);

  const double collectedCharge{
      referenceCollectedChargeVsFluence(fluence / 1.0e14) * chargeInfC / 0.56};

  return std::min((11.5201 + 36576.2 * std::pow(collectedCharge, -3.87335)) *
                      Athena::Units::picosecond,
                  maxJitter);
}

double HGTD_TimeResolutionTool::sigmaTDC() const {

  return 20.0 / std::sqrt(12.0) * Athena::Units::picosecond;
}

double HGTD_TimeResolutionTool::sigmaClock() const {

  return 14.0 * Athena::Units::picosecond;
}
