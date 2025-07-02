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

    /// the mass of the charged pion (in MeV)
    constexpr double chargedPionMassInMeV = 139.57018;
  }

  namespace PDG2024
  {
    /// the mass of the electron (in MeV)
    constexpr double electronMassInMeV = 0.51099895000;

    /// the mass of the muon (in MeV)
    constexpr double muonMassInMeV = 105.6583755;

    /// the mass of the charged pion (in MeV)
    constexpr double chargedPionMassInMeV = 139.57039;
  }

  // Use the PDG2011 values for consistency with Gaudi
  using namespace PDG2011;
}

#endif
