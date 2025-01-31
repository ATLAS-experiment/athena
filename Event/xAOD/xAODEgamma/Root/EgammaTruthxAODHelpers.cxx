/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODEgamma/EgammaTruthxAODHelpers.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/Electron.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"

#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

// ==================================================================

/// Accessor for the "recoElectronLink" dynamic variable
///
/// It is declared outside of the @c xAOD::EgammaHelpers::getRecoElectron(...)
/// call to make sure that the auxiliary ID registry would know about this type
/// as soon as the library holding this code is loaded.
///
static const SG::AuxElement::Accessor< ElementLink< xAOD::ElectronContainer > >
   recoElectronLinkAcc( "recoElectronLink" );

const xAOD::Electron*
xAOD::EgammaHelpers::getRecoElectron( const xAOD::TruthParticle* particle ) {

   if( ! recoElectronLinkAcc.isAvailable( *particle ) ) {
      return nullptr;
   }
   const ElementLink< xAOD::ElectronContainer >& link =
      recoElectronLinkAcc( *particle );
   if( ! link.isValid() ) {
      return nullptr;
   }
   return *link;
}

/// Accessor for the "recoPhotonLink" dynamic variable
///
/// It is declared outside of the @c xAOD::EgammaHelpers::getRecoPhoton(...)
/// call to make sure that the auxiliary ID registry would know about this type
/// as soon as the library holding this code is loaded.
///
static const SG::AuxElement::Accessor< ElementLink< xAOD::PhotonContainer > >
   recoPhotonLinkAcc( "recoPhotonLink" );

const xAOD::Photon*
xAOD::EgammaHelpers::getRecoPhoton( const xAOD::TruthParticle* particle ) {

   if( ! recoPhotonLinkAcc.isAvailable( *particle ) ) {
      return nullptr;
   }
   const ElementLink< xAOD::PhotonContainer >& link =
      recoPhotonLinkAcc( *particle );
   if( ! link.isValid() ) {
      return nullptr;
   }
   return *link;
}
// ==================================================================

//Is the object matched to a true converted photon with R < maxRadius
bool xAOD::EgammaHelpers::isTrueConvertedPhoton(const xAOD::Photon* ph, float maxRadius /* = 800. */){
  const xAOD::TruthParticle *truthPh = xAOD::TruthHelpers::getTruthParticle(*ph);
  if (!truthPh) {return false;}
  // In older versions egammaTruthParticles did not have a decay vertex associated
  // so we look for the original truth particle
  if (truthPh->hasDecayVtx()){
    return isTrueConvertedPhoton(truthPh, maxRadius);
  }
  const xAOD::TruthParticle *orgTruthPh = xAOD::TruthHelpers::getTruthParticle(*truthPh);
  if (!orgTruthPh){ return false;}
  return xAOD::EgammaHelpers::isTrueConvertedPhoton(orgTruthPh, maxRadius);
}

//Is the true object a converted photon with R < maxRadius
bool xAOD::EgammaHelpers::isTrueConvertedPhoton(const xAOD::TruthParticle* truthPh, float maxRadius /*= 800.*/){
  return (MC::isPhoton(truthPh) && truthPh->hasDecayVtx()
	  && truthPh->decayVtx()->perp() < maxRadius);
}

//Lineage methods
std::vector<const xAOD::TruthParticle*>
xAOD::EgammaHelpers::getBkgElectronLineage(const xAOD::TruthParticle* truthel,
					   const bool allTheWayBack/*=true*/) {
  std::vector<const xAOD::TruthParticle*> vec;
  //Truth must exist and be an electron
  if (!truthel || !MC::isElectron(truthel)){
    return vec;
  }
  vec.push_back(truthel); //push its self back as first entry

  // The first parent has to exist
  if (!truthel->nParents()) {
    return vec;
  }

  if (!HepMC::is_simulation_particle(truthel) && !allTheWayBack)  {
    return vec;
  }

  //And has to be a photon or electron
  const xAOD::TruthParticle* parent = nullptr;
  for (size_t p = 0; p < truthel->nParents(); ++p) {
    if ( !MC::isPhoton(truthel->parent(p)) && !MC::isElectron(truthel->parent(p)) )  return vec;
    parent = truthel->parent(p); //AV: note, here is an ambiguity for the case of multiple parents.
  }

  if (!parent)  return vec;
  vec.push_back(parent); //push in the parent as the second entry

  //Loop over the generations
  while (parent->nParents() &&
	 (HepMC::is_simulation_particle(parent) || allTheWayBack)) {
    //Find the next parent
    const xAOD::TruthParticle* tmp = nullptr; 
    //You want to see an electron or a photon
    for (size_t p = 0; p < parent->nParents(); ++p) {
      if (MC::isPhoton(parent->parent(p)) || MC::isElectron(parent->parent(p))) tmp = parent->parent(p); //AV: note some ambiguity for multiple parents passing the selection
    }
    if (tmp) {
      parent = tmp;
    } else { // if we do not see any more electron and photons we stop
      break;
    }
    vec.push_back(parent); //push in the parent
  }
  return vec;
}
const xAOD::TruthParticle*
xAOD::EgammaHelpers::getBkgElectronMother(const xAOD::Electron* el,
					  const bool allTheWayBack/*=true*/){
  const xAOD::TruthParticle *truthel =
    xAOD::TruthHelpers::getTruthParticle(*el);
  return getBkgElectronMother(truthel,allTheWayBack);
}

const xAOD::TruthParticle*
xAOD::EgammaHelpers::getBkgElectronMother(const xAOD::TruthParticle* truthel,
					  const bool allTheWayBack/*=true*/) {
  std::vector<const xAOD::TruthParticle*>  vec =
    xAOD::EgammaHelpers::getBkgElectronLineage(truthel,allTheWayBack);
  if (!vec.empty()) {
    return vec.back();
  }
  return nullptr;
}


std::vector<const xAOD::TruthParticle*>
xAOD::EgammaHelpers::getBkgElectronLineage(const xAOD::Electron* el,
					   const bool allTheWayBack/*=true*/) {
  const xAOD::TruthParticle *truthel = xAOD::TruthHelpers::getTruthParticle(*el);
  return getBkgElectronLineage(truthel,allTheWayBack);
}
