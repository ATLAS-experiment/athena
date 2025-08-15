/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EnergyLossUpdator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk include
#include "TrkExTools/EnergyLossUpdator.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"


#include <cmath>


// constructor
Trk::EnergyLossUpdator::EnergyLossUpdator(const std::string& t,
                                          const std::string& n,
                                          const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<Trk::IEnergyLossUpdator>(this);
}

// public interface method
double
Trk::EnergyLossUpdator::dEdX(const MaterialProperties& mat,
                             double p,
                             ParticleHypothesis particle) const
{
  if (particle == Trk::undefined || particle == Trk::nonInteracting) {
    return 0.;
  }

  // preparation of kinetic constants
  double m = Trk::ParticleMasses::mass[particle];
  double E = std::sqrt(p * p + m * m);
  double beta = p / E;
  double gamma = E / m;

  // add ionization and radiation
  double dEdX =
    Trk::MaterialInteraction::dEdXBetheBloch(mat, beta, gamma, particle) +
    Trk::MaterialInteraction::dEdXBetheHeitler(mat, E, particle);

  // add e+e- pair production and photonuclear effect for muons at energies
  // above 8 GeV
  if ((particle == Trk::muon) && (E > 8000.)) {
    if (E < 1.e6) {
      dEdX += -0.5345 / mat.x0() + 6.803e-5 * E / mat.x0() +
              2.278e-11 * E * E / mat.x0() -
              9.899e-18 * E * E * E / mat.x0(); // E below 1 TeV
    } else {
      dEdX += -2.986 / mat.x0() + 9.253e-5 * E / mat.x0(); // E above 1 TeV
    }
  }
  return dEdX;
}

// public interface method
Trk::EnergyLoss
Trk::EnergyLossUpdator::energyLoss(const MaterialProperties& mat,
                                   double p,
                                   double pathcorrection,
                                   PropDirection dir,
                                   ParticleHypothesis particle,
                                   bool useMPV) const
{
  if (particle == Trk::undefined) {
    ATH_MSG_WARNING(
      "undefined ParticleHypothesis, energy loss calculation cancelled");
    return {};
  }

  if (useMPV) {
    return ionizationEnergyLoss(mat, p, pathcorrection, dir, particle);
  }

  double deltaE = 0.;
  // preparation
  double sign = (dir == Trk::oppositeMomentum) ? -1. : 1.;

  double pathLength = pathcorrection * mat.thicknessInX0() * mat.x0();

  double sigIoni = 0.;
  double sigRad = 0.;
  double kazL = 0.;
  double meanIoni = Trk::MaterialInteraction::dEdl_ionization(
    p, (mat.material()), particle, sigIoni, kazL);
  double meanRad = Trk::MaterialInteraction::dEdl_radiation(
    p, (mat.material()), particle, sigRad);

  meanIoni = sign * pathLength * meanIoni;
  meanRad = sign * pathLength * meanRad;
  sigIoni = pathLength * sigIoni;
  sigRad = pathLength * sigRad;
  kazL = pathLength * kazL;

  //
  // include pathlength dependence of Landau ionization
  //
  sigIoni = sigIoni - kazL * std::log(pathLength);

  deltaE = meanIoni + meanRad;

  double sigmaDeltaE = std::sqrt(sigIoni * sigIoni + sigRad * sigRad);
  ATH_MSG_DEBUG(" Energy loss updator deltaE "
                << deltaE << " meanIoni " << meanIoni << " meanRad " << meanRad
                << " sigIoni " << sigIoni << " sigRad " << sigRad << " sign "
                << sign << " pathLength " << pathLength);
  return (!m_detailedEloss ? Trk::EnergyLoss(deltaE, sigmaDeltaE)
                           : Trk::EnergyLoss(deltaE, sigmaDeltaE, sigmaDeltaE,
                                             sigmaDeltaE, meanIoni, sigIoni,
                                             meanRad, sigRad, pathLength));
}

// public interface method

Trk::EnergyLoss
Trk::EnergyLossUpdator::updateEnergyLoss(Trk::EnergyLoss& eLoss,
                                         double caloEnergy,
                                         double caloEnergyError,
                                         double pCaloEntry,
                                         double momentumError,
                                         int& elossFlag) const
{
  //
  // Input: the detailed EnergyLoss object in the Calorimeter that contains the
  // different Eloss terms and their uncertainties; caloEnergy and error; and
  // the muon momentumError (all in MeV)
  //
  // For use in the MuonSystem
  // Input: caloEnergy = 0. caloEnergyError = 0. and pCaloEntry = pMuonEntry
  // momentum at MuonEntry
  //
  // Output: an updated Energy loss values deltaE()
  //         that can be used in the track fit and corresponds to the Most
  //         Probable EnergyLoss value taking into account the ionization,
  //         radiation and smearing due to the errors including the
  //         momentumError (in MeV)
  //
  //         elossFlag = false if Calorimeter Energy is NOT stored (and later
  //         fitted) on the Eloss object
  //                   = true  Calorimeter Energy is stored and will be fitted
  //
  //  deltaE is used in the final fit
  //

  elossFlag = 0;

  int isign = 1;
  if (eLoss.deltaE() < 0) {
    isign = -1;
  }

  double deltaE = eLoss.deltaE();
  double sigmaDeltaE = eLoss.sigmaDeltaE();
  // Detailed Eloss
  double deltaE_ioni = eLoss.meanIoni();
  double sigmaDeltaE_ioni = 0.45 * eLoss.sigmaIoni(); // sigma Landau
  double deltaE_rad = eLoss.meanRad();
  double sigmaDeltaE_rad =
    eLoss.sigmaRad(); // rms and mean of steep exponential
  double depth = eLoss.length();

  // Eloss radiative protection

  if (eLoss.meanRad() > 100000.) {
    deltaE_rad = 100000.;
    sigmaDeltaE_rad = eLoss.sigmaRad() * 100000. / eLoss.meanRad();
  }

  double sigmaPlusDeltaE = eLoss.sigmaPlusDeltaE();
  double sigmaMinusDeltaE = eLoss.sigmaMinusDeltaE();

  double MOP = deltaE_ioni - isign * 3.59524 * sigmaDeltaE_ioni;

  //
  // MOP shift due to ionization and radiation
  //
  double MOPshift =
    isign * 50 * 10000. / pCaloEntry +
    isign * 0.75 * std::sqrt(sigmaDeltaE_ioni * sigmaDeltaE_rad);
  double MOPshiftNoRad = isign * 50 * 10000. / pCaloEntry;
  //
  // define sigmas for Landau convoluted with exponential
  //
  double fracErad = sigmaDeltaE_rad +
                    std::abs(deltaE_rad) * pCaloEntry / (800000. + pCaloEntry);
  double sigmaL = sigmaDeltaE_ioni + 0.8 * fracErad / 3.59524;
  //  double sigmaLNoRad = sigmaDeltaE_ioni;
  double sigmaMinus = 1.02 * sigmaDeltaE_ioni + 0.08 * sigmaDeltaE_rad;
  double sigmaPlus = 4.65 * sigmaDeltaE_ioni + 1.16 * sigmaDeltaE_rad;
  //  double sigmaMinusNoRad = 1.02 * sigmaDeltaE_ioni;
  //  double sigmaPlusNoRad = 4.65 * sigmaDeltaE_ioni;
  double xc = momentumError / (sigmaL > 0. ? sigmaL : 1.);
  double correction =
    (0.3849 * xc * xc + 7.76672e-03 * xc * xc * xc) /
    (1 + 2.8631 * xc + 0.3849 * xc * xc + 7.76672e-03 * xc * xc * xc);

  //
  // Case where the measured Calorimeter energy is not available (e.g. low pT or
  // not isolated)
  //

  if (caloEnergyError <= 0) {
    //
    // Shift of MOP due to momentum resolution
    //
    double MOPreso = isign * 3.59524 * sigmaL * correction;

    deltaE = MOP + MOPshift + MOPreso;
    sigmaMinusDeltaE = sigmaMinus;
    sigmaPlusDeltaE = sigmaPlus;
    sigmaDeltaE = std::sqrt(0.5 * sigmaMinusDeltaE * sigmaMinusDeltaE +
                            0.5 * sigmaPlusDeltaE * sigmaPlusDeltaE);
    //
    if (m_optimalRadiation && std::abs(deltaE) < caloEnergy &&
        pCaloEntry > 100000) {
      //
      // Calorimeter measurement can be used as veto to say there was no
      // significant radiation
      //
      // In that case the Eloss is taken as the ionization Eloss
      // Use MOP after correction for landau tail (MOPshiftNoRad) and momentum
      // resolution smearing (MOPreso)
      //
      sigmaL = sigmaDeltaE_ioni + 0.3 * fracErad / 3.59524;
      xc = momentumError / (sigmaL > 0. ? sigmaL : 1.);
      correction =
        (0.3849 * xc * xc + 7.76672e-03 * xc * xc * xc) /
        (1 + 2.8631 * xc + 0.3849 * xc * xc + 7.76672e-03 * xc * xc * xc);

      MOPreso = isign * 3.59524 * sigmaL * correction;
      deltaE = MOP + MOPshift + MOPreso;
      sigmaMinusDeltaE = sigmaMinus;
      sigmaPlusDeltaE = sigmaPlus;
      sigmaDeltaE = std::sqrt(0.5 * sigmaMinusDeltaE * sigmaMinusDeltaE +
                              0.5 * sigmaPlusDeltaE * sigmaPlusDeltaE);
    }
  } else {
    double sigmaPlusTot =
      std::sqrt(sigmaPlus * sigmaPlus + caloEnergyError * caloEnergyError);
    if (m_optimalRadiation) {
      sigmaPlusTot =
        std::sqrt(4.65 * sigmaDeltaE_ioni * 4.65 * sigmaDeltaE_ioni +
                  caloEnergyError * caloEnergyError);
    }
    double MOPtot = std::abs(MOP + MOPshift);
    if (m_optimalRadiation) {
      MOPtot = std::abs(MOP + MOPshiftNoRad);
    }

    if (caloEnergy > MOPtot + 2 * sigmaPlusTot) {
      //
      // Use measured Calorimeter energy
      //
      //
      // take into account the tail in the Measured Eloss
      //
      double MOPreso = isign * 3.59524 * sigmaL * correction;
      deltaE = isign * caloEnergy + MOPreso;
      sigmaMinusDeltaE = caloEnergyError + 0.08 * sigmaDeltaE_rad;
      sigmaPlusDeltaE = caloEnergyError + 1.16 * sigmaDeltaE_rad;
      sigmaDeltaE = std::sqrt(0.5 * sigmaMinusDeltaE * sigmaMinusDeltaE +
                              0.5 * sigmaPlusDeltaE * sigmaPlusDeltaE);
      elossFlag = 1;
    } else {
      // Use MOP after corrections

      //
      // Shift of MOP due to momentum resolution smearing
      //
      sigmaL = sigmaDeltaE_ioni + 0.3 * fracErad / 3.59524;
      xc = momentumError / (sigmaL > 0. ? sigmaL : 1);
      correction =
        (0.3849 * xc * xc + 7.76672e-03 * xc * xc * xc) /
        (1 + 2.8631 * xc + 0.3849 * xc * xc + 7.76672e-03 * xc * xc * xc);
      double MOPreso = isign * 3.59524 * sigmaL * correction;
      //
      // Use MOP after correction for landau tail (MOPshiftNoRad) and radiation
      // (MOPshift) and momentum resolution smearing (MOPreso)
      //
      deltaE = MOP + MOPshift + MOPreso;
      sigmaMinusDeltaE = sigmaMinus;
      sigmaPlusDeltaE = sigmaPlus;
      sigmaDeltaE = std::sqrt(0.5 * sigmaMinusDeltaE * sigmaMinusDeltaE +
                              0.5 * sigmaPlusDeltaE * sigmaPlusDeltaE);
    }
  }

  return {deltaE,
                    sigmaDeltaE,
                    sigmaMinusDeltaE,
                    sigmaPlusDeltaE,
                    deltaE_ioni,
                    sigmaDeltaE_ioni,
                    deltaE_rad,
                    sigmaDeltaE_rad,
                    depth};
}

// public interface method
void
Trk::EnergyLossUpdator::getX0ElossScales(int icalo,
                                         double eta,
                                         double phi,
                                         double& X0Scale,
                                         double& ElossScale) const
{
  //
  // for Calorimeter icalo = 1
  //     Muon System icalo = 0
  // convention eta, phi is at Calorimeter Exit (or Muon Entry)
  //            eta and phi are from the position (not direction)
  //
  // input   X0 and ElossScale = 1
  // output  updated X0Scale and ElossScale
  //

  double X0CaloGirder[4] = {
    -1.02877e-01, -2.74322e-02, 8.12989e-02, 9.73551e-01
  };

  // R2012 to R2015 determined on ATLAS-R2-2015-03-01-00 to be used in rel 21
  constexpr double X0CaloScale[60] = {
    1.01685,  1.02092,  1.01875,  1.01812,  1.01791,  1.01345,  1.01354,
    1.02145,  1.01645,  1.01585,  1.0172,   1.02262,  1.01464,  0.990931,
    0.971953, 0.99845,  1.01433,  0.982143, 0.974015, 0.978742, 0.960029,
    0.966766, 0.980199, 0.989586, 0.997144, 1.00169,  0.994166, 0.966332,
    0.93671,  0.935656, 0.921994, 0.901489, 0.897799, 0.89638,  0.905629,
    0.903374, 0.925922, 0.941203, 0.956273, 0.968618, 0.976883, 0.988349,
    0.99855,  1.00212,  1.01456,  1.01541,  1.02532,  1.03238,  1.03688,
    1.03783,  1.02078,  1.01529,  1.0156,   1.02212,  1.02226,  1.02406,
    1.02188,  1.00661,  1.00661,  1.00661
  };

  // R2012 to R2015 determined on ATLAS-R2-2015-03-01-00 to be used in rel 21
  constexpr double ElossCaloScale[30] = {
    1.06921, 1.06828, 1.06734, 1.06092, 1.06638, 1.06335, 1.07421, 1.05885,
    1.07351, 1.07435, 1.06902, 1.07704, 1.08782, 1.09844, 1.115,   1.07609,
    1.08233, 1.08764, 1.08209, 1.08255, 1.08008, 1.07573, 1.077,   1.07271,
    1.07343, 1.07769, 1.07794, 1.08377, 1.08377, 1.08377
  };

  //
  constexpr double X0MuonScale[60] = {
    -0.0320612, -0.0320612, -0.0320612, -0.0320612,  -0.0693796,  -0.0389677,
    -0.0860891, -0.124606,  -0.0882329, -0.100014,   -0.0790912,  -0.0745538,
    -0.099088,  -0.0933711, -0.0618782, -0.0619762,  -0.0658361,  -0.109704,
    -0.129547,  -0.143364,  -0.0774768, -0.0739859,  -0.0417835,  -0.022119,
    0.00308797, 0.0197657,  -0.0137871, -0.036848,   -0.0643794,  -0.0514949,
    -0.0317105, 0.016539,   0.0308435,  -0.00056883, -0.00756813, -0.00760612,
    -0.0234571, -0.0980915, -0.101175,  -0.102354,   -0.0920337,  -0.100337,
    -0.0887628, 0.0660931,  0.228999,   0.260675,    0.266301,    0.267907,
    0.281668,   0.194433,   0.132954,   0.20707,     0.220466,    0.20936,
    0.191441,   0.191441,   0.191441,   0.191441,    0.191441,    0.191441
  };

  int i60 = std::abs(eta) * 20.;

  if (i60 < 0) {
    i60 = 0;
  }
  if (i60 > 59) {
    i60 = 59;
  }

  if (icalo == 1) {
    //
    // Girder parametrization
    //
    double x =
      phi + 3.1416 - 3.1416 / 32. * int((3.1416 + phi) / (3.1416 / 32.));
    double scale = 0.;
    if (x > M_PI / 64.) {
      x = M_PI / 32. - x;
    }

    if (x < 0.005) {
      scale =
        X0CaloGirder[0] * (1 - x / 0.005) + X0CaloGirder[1] + X0CaloGirder[3];
    } else if (x < 0.017) {
      scale = X0CaloGirder[1] + X0CaloGirder[3];
    } else if (x < 0.028) {
      scale = X0CaloGirder[2] + X0CaloGirder[3];
    } else {
      scale = X0CaloGirder[3];
    }

    if (std::abs(eta) > 1.3) {
      scale = 1.;
    }
    //
    // eta dependence of X0
    //
    scale *= X0CaloScale[i60];
    X0Scale = scale;
    //
    // eta dependence of Eloss
    //
    int i30 = std::abs(eta) * 10.;
    if (i30 < 0) {
      i30 = 0;
    }
    if (i30 > 29) {
      i30 = 29;
    }

    double nfactor = 0.987363 / 1.05471;

    ElossScale = nfactor * ElossCaloScale[i30] * scale;
  } else {
    //
    // Muon system
    //
    // eta dependence of X0
    //
    double scale = 1. + X0MuonScale[i60];
    //
    // Muon scale is now 1 with MuonTrackingGeometry and TrkDetDescrGeoModelCnv
    // fixes
    //
    scale = 1.0;
    X0Scale = scale;
    ElossScale = 0.93 * scale;
  }
}

Trk::EnergyLoss
Trk::EnergyLossUpdator::ionizationEnergyLoss(const MaterialProperties& mat,
                                             double p,
                                             double pathcorrection,
                                             PropDirection dir,
                                             ParticleHypothesis particle) const
{
  // preparation
  double sign = (dir == Trk::oppositeMomentum) ? -1. : 1.;
  double pathLength = pathcorrection * mat.thicknessInX0() * mat.x0();

  double sigIoni = 0.;
  double kazL = 0.;

  double meanIoni =
    sign * Trk::MaterialInteraction::dE_MPV_ionization(
             p, (mat.material()), particle, sigIoni, kazL, pathLength);

  return (!m_detailedEloss
            ? Trk::EnergyLoss(meanIoni, sigIoni)
            : Trk::EnergyLoss(meanIoni,
                              sigIoni,
                              sigIoni,
                              sigIoni,
                              meanIoni,
                              sigIoni,
                              0.,
                              0.,
                              pathLength));
}
