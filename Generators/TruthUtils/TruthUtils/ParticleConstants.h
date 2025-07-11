/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHUTILS_PARTICLECONSTANTS_H
#define TRUTHUTILS_PARTICLECONSTANTS_H

/// @file ParticleConstants.h
///
/// A number of `constexpr` particle constants to avoid hardcoding them
/// directly in various places.  This is not meant ot be a complete list
/// of everything that could be needed, please look that up from the PDG
/// database, this is just for numbers that would otherwise have to be
/// hard-coded because it can't (practically) do that lookup.
///
/// Please note that while you should feel free to add new constants
/// here, you need to also add them to the testParticleConstants.cxx
/// test, so that we can ensure that the values here match the values
/// in the ParticleDataTable.

namespace ParticleConstants
{
  namespace PDG2011
  {
    /// the mass of the electron (in MeV)
    constexpr double electronMassInMeV = 0.510998910;

    /// the mass of the muon (in MeV)
    constexpr double muonMassInMeV = 105.658367;

    /// the mass of the tau (in MeV)
    constexpr double tauMassInMeV = 1776.82;

    /// the mass of the Z0 boson (in MeV)
    constexpr double ZMassInMeV = 91187.6;

    /// the mass of the pi zero (in MeV)
    constexpr double piZeroMassInMeV = 134.9766;

    /// the mass of the charged pion (in MeV)
    constexpr double chargedPionMassInMeV = 139.57018;

    /// the mass of the eta meson (in MeV)
    constexpr double etaMassInMeV = 547.853;

    /// the mass of the neutral kaon (K0) (in MeV)
    constexpr double KZeroMassInMeV = 497.614;

    /// the mass of the charged kaon (in MeV)
    constexpr double chargedKaonMassInMeV = 493.677;

    /// the mass of the D0 meson (in MeV)
    constexpr double DZeroMassInMeV = 1864.8;

    // the mass of the J/psi meson (in MeV)
    constexpr double JpsiMassInMeV = 3096.916;

    /// the mass of the neutral B0 meson (in MeV)
    constexpr double BZeroMassInMeV = 5279.5;

    /// the mass of the charged B meson (in MeV)
    constexpr double BPlusMassInMeV = 5279.17;

    /// the mass of the Bs meson (in MeV)
    constexpr double BsMassInMeV = 5366.3;

    /// the mass of the neutron (in MeV)
    constexpr double neutronMassInMeV = 939.565346;

    /// the mass of the proton (in MeV)
    constexpr double protonMassInMeV = 938.272013;

    /// the mass of the lambda baryon (in MeV)
    constexpr double lambdaMassInMeV = 1115.683;
  }

  namespace PDG2024
  {
    /// the mass of the electron (in MeV)
    constexpr double electronMassInMeV = 0.51099895000;

    /// the mass of the muon (in MeV)
    constexpr double muonMassInMeV = 105.6583755;

    /// the mass of the tau (in MeV)
    constexpr double tauMassInMeV = 1776.93;

    /// the mass of the Z0 boson (in MeV)
    constexpr double ZZeroMassInMeV = 91188.0;

    /// the mass of the pi zero (in MeV)
    constexpr double piZeroMassInMeV = 134.9768;

    /// the mass of the charged pion (in MeV)
    constexpr double chargedPionMassInMeV = 139.57039;

    /// the mass of the eta meson (in MeV)
    constexpr double etaMassInMeV = 547.862;

    /// the mass of the neutral kaon (K0) (in MeV)
    constexpr double KZeroMassInMeV = 497.611;

    /// the mass of the charged kaon (in MeV)
    constexpr double chargedKaonMassInMeV = 493.677;

    /// the mass of the D0 meson (in MeV)
    constexpr double DZeroMassInMeV = 1864.84;

    // the mass of the J/psi meson (in MeV)
    constexpr double JpsiMassInMeV = 3096.900;

    /// the mass of the neutral B0 meson (in MeV)
    constexpr double BZeroMassInMeV = 5279.72;

    /// the mass of the charged B meson (in MeV)
    constexpr double BPlusMassInMeV = 5279.41;

    /// the mass of the neutron (in MeV)
    constexpr double neutronMassInMeV = 939.5654205;

    /// the mass of the proton (in MeV)
    constexpr double protonMassInMeV = 938.27208816;

    /// the mass of the lambda baryon (in MeV)
    constexpr double lambdaMassInMeV = 1115.683;
  }

  // Use the PDG2011 values for consistency with Gaudi
  using namespace PDG2011;


  /// various mass-less particles
  ///
  /// This exists just to allow using a symbolic name to indicate what a
  /// given value `0.0` means.
  constexpr double photonMassInMeV = 0.0;
  constexpr double electronNeutrinoMassInMeV = 0.0;
  constexpr double muonNeutrinoMassInMeV = 0.0;
  constexpr double tauNeutrinoMassInMeV = 0.0;
}

#endif
