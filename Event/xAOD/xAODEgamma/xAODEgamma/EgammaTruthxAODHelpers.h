// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id: EgammaTruthxAODHelpers.h 768706 2016-08-18 23:55:25Z christos $
#ifndef XAOD_EGAMMATRUTHXAODHELPERS_H
#define XAOD_EGAMMATRUTHXAODHELPERS_H

#include "xAODEgamma/EgammaFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/PhotonFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticle.h"

#include <cstddef>
#include <set>

namespace xAOD {

  namespace EgammaHelpers{

    ///@brief return the reco electron associated to the given TruthParticle (if any)
    const xAOD::Electron* getRecoElectron(const xAOD::TruthParticle*);

    ///@brief return the reco photon associated to the given TruthParticle (if any)
    const xAOD::Photon* getRecoPhoton(const xAOD::TruthParticle* particle);

    ///@brief is the object matched to a true converted photon with R < maxRadius
    bool isTrueConvertedPhoton(const xAOD::Photon* ph, float maxRadius = 800.);

    ///@brief is the true object a converted photon with R < maxRadius
    bool isTrueConvertedPhoton(const xAOD::TruthParticle* truePh, float maxRadius = 800.);

    ///@brief Helper function for getting the truth lineage of an electron that is arising
    /// from material interactions in simulation (so called Bkg Electron)
    /// There are cases when an electron has  a  photon (or electron) mother, that in turn comes
    /// from another electron of photon.
    /// The allTheWayBack (default true) will return the full lineage of the electron going back
    /// to the earliest generator e/gamma truth particle available.
    /// When this is set to false we stop at the generator e/gamma particle that
    /// interacted with material in simulation
    /// (meaning we will not go at the beginning of the generator record).
    /// The 0th entry of the returned vector is the passed e/gamma particle, while the last
    /// is the last e/gamma particle we have seen in the lineage.
    std::vector<const xAOD::TruthParticle*>
    getBkgElectronLineage(const xAOD::TruthParticle* truthel,
			  const bool allTheWayBack = true);

    ///@brief Helper wrapper function for calling the function above
    /// extracting the  truth from a reco electron.
    const xAOD::TruthParticle*
    getBkgElectronMother(const xAOD::Electron* el,
			 const bool allTheWayBack = true);

    ///@brief Helper function for getting the "Mother" electron for an existing electron.
    /// return the last entry in the lineage vector
    const xAOD::TruthParticle*
    getBkgElectronMother(const xAOD::TruthParticle* truthel,
			 const bool allTheWayBack = true);

    ///@brief Helper wrapper function for calling the function below that accepts truth input.
    std::vector<const xAOD::TruthParticle*>
    getBkgElectronLineage(const xAOD::Electron* el,
			  const bool allTheWayBack = true);

  }// EgammaHelpers

} // namespace xAOD

#endif // XAOD_EGAMMAXAODHELPERS_H
