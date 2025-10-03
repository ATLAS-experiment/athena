/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Implementation file that mainly contains the code logic
 * dealing with Truth - record classification
 * Contributors:  Pierre-Antoine Delsart
 *                Andrii Verbytskyi <andrii.verbytskyi@mpp.mpg.de>
 */

#include "MCTruthClassifier/MCTruthClassifier.h"
#include "AsgDataHandles/ReadHandle.h"
#include "TruthUtils/MagicNumbers.h"
#ifndef XAOD_ANALYSIS
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#endif
#include "TruthUtils/DecayProducts.h"
using namespace MCTruthPartClassifier;

#ifndef XAOD_ANALYSIS
std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleHepMCTruthClassifier(const HepMcParticleLink& theLink, MCTruthPartClassifier::Info* info /*= nullptr*/) const {
  // Retrieve the links between HepMC and xAOD::TruthParticle
  const EventContext& ctx = info ? info->eventContext : Gaudi::Hive::currentContext();
  SG::ReadHandle<xAODTruthParticleLinkVector> truthParticleLinkVecReadHandle(m_truthLinkVecReadHandleKey, ctx);
  if (!truthParticleLinkVecReadHandle.isValid()) {
    ATH_MSG_WARNING(" Invalid ReadHandle for xAODTruthParticleLinkVector with key: " << truthParticleLinkVecReadHandle.key());
    return std::make_pair(Unknown, NonDefined);
  }
  ElementLink<xAOD::TruthParticleContainer> tplink = truthParticleLinkVecReadHandle->find (theLink);
  if (tplink.isValid()) {
    return particleTruthClassifier (*tplink, info);
  }
  return std::make_pair(Unknown, NonDefined);
}
#endif

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::TruthParticle* thePart, MCTruthPartClassifier::Info* infoin /*= nullptr*/) const {
  MCTruthPartClassifier::Info tmpinfo;
  MCTruthPartClassifier::Info& info = (infoin) ? *infoin : tmpinfo;

  ATH_MSG_DEBUG("Executing particleTruthClassifier");

  ParticleType partType = Unknown;
  ParticleOrigin partOrig = NonDefined;
  if (!thePart) {
    return std::make_pair(partType, partOrig);
  }
  info.genPart = thePart;

  // retrieve collection and get a pointer
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainerReadHandle(m_truthParticleContainerKey,info.eventContext);
  if (!truthParticleContainerReadHandle.isValid()) {
    ATH_MSG_WARNING( " Invalid ReadHandle for xAOD::TruthParticleContainer with key: " << truthParticleContainerReadHandle.key());
    return std::make_pair(partType, partOrig);
  }

  ATH_MSG_DEBUG("xAODTruthParticleContainer with key  " << truthParticleContainerReadHandle.key() << " has valid ReadHandle ");

  if (!MC::isStable(thePart) && !MC::isDecayed(thePart)) {
    return std::make_pair(GenParticle, partOrig);
  }
  const bool isPartHadr = MC::isHadron(thePart) && !MC::isBeam(thePart);
  if (MC::isDecayed(thePart) && (!MC::isTau(thePart) && !isPartHadr)) return std::make_pair(GenParticle, partOrig);

  // SUSY datasets: tau(status==2)->tau(status==2)
  if (MC::isDecayed(thePart) && MC::isTau(thePart)) {
    const xAOD::TruthVertex* endVert = thePart->decayVtx();
    if (endVert) {
      if (endVert->nOutgoingParticles() == 1 && MC::isTau(endVert->outgoingParticle(0))) {
        return std::make_pair(GenParticle, partOrig);
      }
    }
  }

  if (MC::isStable(thePart) && MC::isSUSY(thePart)) return std::make_pair(SUSYParticle, partOrig);

  if (MC::isStable(thePart) && MC::isBSM(thePart)) return std::make_pair(OtherBSMParticle, partOrig);

  if (MC::isDecayed(thePart) &&
      (!MC::isElectron(thePart) && !MC::isMuon(thePart) &&
       !MC::isTau(thePart) && !MC::isPhoton(thePart)) &&
      !isPartHadr)
    return std::make_pair(GenParticle, partOrig);

  // FIXME vetoing protons here to preserve previous behaviour
  if (MC::isNucleus(thePart) && std::abs(thePart->pdgId()) != MC::PROTON) return std::make_pair(NuclFrag, partOrig);

  if ( !MC::isSMLepton(thePart) && !MC::isPhoton(thePart)  && !isPartHadr) return std::make_pair(partType, partOrig);
  // don't consider generator particles

  const xAOD::TruthVertex* partProdVtx = thePart->hasProdVtx() ? thePart->prodVtx() : nullptr;

  const xAOD::TruthParticle* parent{};
  if (partProdVtx) {
    for (const auto& temp: partProdVtx->particles_in()) {if (temp)  parent = temp;}
  }
  const int parentPDG = parent?parent->pdg_id():0;
  info.setMotherProperties(parent);

  if (!partProdVtx && HepMC::is_simulation_particle(thePart)) {
    return std::make_pair(NonPrimary, partOrig);
  }
  if (!partProdVtx && MC::isElectron(thePart)) {
    // to define electron outcome status
    bool isPrompt = false; // updated by defOrigOfElectron
    partOrig = defOrigOfElectron(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    return std::make_pair(UnknownElectron, partOrig);
  }
  if (!partProdVtx && MC::isMuon(thePart)) {
    // to define electron outcome status
    bool isPrompt = false; // updated by defOrigOfMuon
    partOrig = defOrigOfMuon(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    return std::make_pair(UnknownMuon, partOrig);
  }
  if (!partProdVtx && MC::isTau(thePart)) {
    // to define electron outcome status
    partOrig = defOrigOfTau(*truthParticleContainerReadHandle, thePart, parentPDG, info);
    return std::make_pair(UnknownTau, partOrig);
  }
  if (!partProdVtx && MC::isPhoton(thePart)) {
    // to define photon outcome
    bool isPrompt = false; // updated by defOrigOfPhoton
    partOrig = defOrigOfPhoton(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    return std::make_pair(UnknownPhoton, partOrig);
  }
  if (!partProdVtx && MC::isNeutrino(thePart)) {
    // to define neutrino outcome
    info.particleOutCome = NonInteract;
    return std::make_pair(Neutrino, partOrig);
  }

  if (thePart && info.Mother() && HepMC::is_same_generator_particle(thePart,info.Mother()))
    return std::make_pair(NonPrimary, partOrig);

  if (isPartHadr) return std::make_pair(Hadron, partOrig);

  if (partProdVtx && parentPDG == 0 && partProdVtx->nOutgoingParticles() == 1 &&
      partProdVtx->nIncomingParticles() == 0) {
    if (MC::isElectron(thePart)) {
      info.particleOutCome = defOutComeOfElectron(thePart);
      return std::make_pair(IsoElectron, SingleElec);
    }
    if (MC::isMuon(thePart)) {
      info.particleOutCome = defOutComeOfMuon(thePart);
      return std::make_pair(IsoMuon, SingleMuon);
    }
    if (MC::isTau(thePart)) {
      info.particleOutCome = defOutComeOfTau(thePart);
      return std::make_pair(IsoTau, SingleTau);
    }
    if (MC::isPhoton(thePart)) {
      info.particleOutCome = defOutComeOfPhoton(thePart);
      return std::make_pair(IsoPhoton, SinglePhot);
    }
  }

  if (parentPDG == thePart->pdg_id() && parent && parent->status() == 3 && MC::isDecayed(thePart)) return std::make_pair(GenParticle, partOrig);

  if (MC::isElectron(thePart)) {
    bool isPrompt = false; // updated by defOrigOfElectron
    partOrig = defOrigOfElectron(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = defTypeOfElectron(partOrig, isPrompt);
  } else if (MC::isMuon(thePart)) {
    bool isPrompt = false; // updated by defOrigOfMuon
    partOrig = defOrigOfMuon(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = defTypeOfMuon(partOrig, isPrompt);
  } else if (MC::isTau(thePart)) {
    partOrig = defOrigOfTau(*truthParticleContainerReadHandle, thePart, parentPDG, info);
    partType = defTypeOfTau(partOrig);
  } else if (MC::isPhoton(thePart)) {
    bool isPrompt = false; // updated by defOrigOfPhoton
    partOrig = defOrigOfPhoton(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = defTypeOfPhoton(partOrig);
  } else if (MC::isNeutrino(thePart)) {
    bool isPrompt = false; // updated by defOrigOfNeutrino
    partOrig = defOrigOfNeutrino(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = Neutrino;
  }

  ATH_MSG_DEBUG("particleTruthClassifier  succeeded ");
  return std::make_pair(partType, partOrig);
}


bool MCTruthClassifier::TruthLoopDetectionMethod1(const xAOD::TruthVertex * childOrigVtx, const xAOD::TruthParticle* parent) const
{
  // Start of method 1 of protecting against loops
  const int parentPDG = parent->pdgId();
  for (const auto& aChild: childOrigVtx->particles_out()) {
    if (!aChild)  continue;
    if (parentPDG == aChild->pdgId() && HepMC::is_same_generator_particle(aChild, parent)) {
      // One of the children produced in the decay of parent is
      // actually the same particle.  NB In the case of multiple
      // children this child may not necessarily be
      // thePriPart. Does this matter?
      return true;
    }
  }

  // to resolve Sherpa loop
  return TruthLoopDetectionMethod3(childOrigVtx, parent);
  // End of method 1 of protecting against loops
}


bool MCTruthClassifier::TruthLoopDetectionMethod2(const xAOD::TruthParticle* child, const xAOD::TruthParticle* parent) const
{
  // Start of method 2 of protecting against loops
  // to prevent Sherpa loop
  const xAOD::TruthVertex* child_prdVtx{};
  const xAOD::TruthVertex* child_endVtx{};
  if (child) {
    child_prdVtx = child->hasProdVtx() ? child->prodVtx() : nullptr;
    child_endVtx = child->decayVtx();
  }
  const xAOD::TruthVertex* parent_prdVtx{};
  const xAOD::TruthVertex* parent_endVtx{};
  if (parent) {
    parent_prdVtx = parent->hasProdVtx() ? parent->prodVtx() : nullptr;
    parent_endVtx = parent->decayVtx();
  }
  // V0->parent->V1-> ...->V2->child->V3
  // V3 == V0 && V1 == V2
  return (child_endVtx == parent_prdVtx && child_prdVtx == parent_endVtx);
}


bool MCTruthClassifier::TruthLoopDetectionMethod3(const xAOD::TruthVertex * childOrigVtx, const xAOD::TruthParticle* parent) const
{
  // Start of method 3 of protecting against loops
  // to resolve Sherpa loop
  const xAOD::TruthVertex* parentOrigVtx = parent->hasProdVtx() ? parent->prodVtx() : nullptr;
  if (parentOrigVtx && HepMC::is_same_vertex(parentOrigVtx,childOrigVtx)) {
    // The "parent" and the "child" have the same production vertex.
    return true;
  }
  return false;
  // End of method 3 of protecting against loops
}


ParticleOrigin MCTruthClassifier::defOrigOfElectron(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                    const xAOD::TruthParticle* thePart,
                                                    bool& isPrompt,
                                                    MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfElectron ");

  // Find the first copy of this particle stored in the xAOD::TruthParticleContainer (i.e. the particle prior to any interactions)
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isElectron(thePriPart)) return NonDefined;

  //-- to define electron outcome status
  info.particleOutCome = defOutComeOfElectron(thePriPart);

  const xAOD::TruthVertex* partProdVtx = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;
  if (!partProdVtx) return NonDefined;

  if (partProdVtx->nIncomingParticles() > 1) ATH_MSG_DEBUG("DefOrigOfElectron:: electron has more than one parent.");

  const xAOD::TruthParticle* ancestor = MC::findMother(thePriPart);
  info.setMotherProperties(ancestor);
  if (!ancestor) { return NonDefined; } // After this point "ancestor" cannot be nullptr

  // Start of method 1 of protecting against loops
  bool samePart = TruthLoopDetectionMethod1(partProdVtx, ancestor);
  // to resolve Sherpa loop
  // End of method 1 of protecting against loops

  if ((MC::isMuon(ancestor) || MC::isTau(ancestor) || MC::isW(ancestor)) && ancestor->hasProdVtx() && !samePart) {
    int pPDG(0);
    const xAOD::TruthParticle* ancestorParent{};
    do {
      pPDG = 0; // reset pPDG
      ancestorParent = MC::findMother(ancestor);
      // Start of method 2 of protecting against loops
      // to prevent Sherpa loop
      if (ancestor == ancestorParent) { break; }
      if (TruthLoopDetectionMethod2(ancestor,ancestorParent)) {
        ancestorParent = ancestor;
        break;
      }
      // End of method 2 of protecting against loops
      // FIXME why are slightly different criteria used in method 1 and method 2???
      if (ancestorParent) {
        pPDG = ancestorParent->pdgId(); // Only set pPDG in the case that we aren't in a loop.
        if (MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)) { // There will be another iteration so set ancestor to ancestorParent
          ancestor = ancestorParent; // ancestorParent is not nullptr here
        }
      }
    } while ((MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)));

    if (MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG) || MC::isZ(pPDG) || MC::isHiggs(pPDG) ||
        MC::isMSSMHiggs(pPDG) || MC::isHeavyBoson(pPDG) || MC::isTop(pPDG) || // MSSM Higgs bosons, Heavy bosons( Z', Z'', W'+)
        std::abs(pPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(pPDG) || // Left-right symmetric model WBoson || Right-handed neutrino (Pythia-specific)
        MC::isSUSY(pPDG)) {
      ancestor = ancestorParent; // ancestorParent is not nullptr here
    }
  }

  info.setMotherProperties(ancestor);
  const int ancestorPDG = ancestor->pdgId();
  const xAOD::TruthVertex* ancestorProdVtx = ancestor->hasProdVtx() ? ancestor->prodVtx() : nullptr;
  partProdVtx = ancestor->decayVtx();
  const int numOfParents = partProdVtx->nIncomingParticles();
  const int numberOfChildren = partProdVtx->nOutgoingParticles();

  // Determine decay products
  auto DP = DecayProducts(partProdVtx);
  const int NumOfPhot = DP.pd(MC::PHOTON);
  const int NumOfEl = DP.pd(MC::ELECTRON);
  const int NumOfPos = DP.pd(MC::POSITRON);
  const int NumOfquark = DP.apd({MC::DQUARK,MC::UQUARK,MC::SQUARK,MC::CQUARK,MC::BQUARK,MC::TQUARK});
  const int NumOfgluon = DP.apd(MC::GLUON);
  const int NumOfElNeut = DP.apd(MC::NU_E);
  const int NumOfLQ = DP.apd(MC::LEPTOQUARK);
  const int NumOfMuPl = DP.pd(-MC::MUON);
  const int NumOfMuMin = DP.pd(MC::MUON);
  const int NumOfMuNeut = DP.apd(MC::NU_MU);
  const int NumOfTau = DP.apd(MC::TAU);
  const int NumOfTauNeut = DP.apd(MC::NU_TAU);

  samePart = false;
  int NumOfNucFr(0);
  const bool possibleNuclearFragment = (numOfParents == 1 && (MC::isPhoton(ancestorPDG) || MC::isElectron(ancestorPDG) || MC::isMuon(ancestorPDG) || std::abs(ancestorPDG) == MC::PIPLUS));
  for (const auto& aChild: partProdVtx->particles_out()) {
    if (!aChild) continue;
    const int childPDG = aChild->pdgId();
    if (std::abs(childPDG) == std::abs(ancestorPDG) && HepMC::is_same_generator_particle(aChild, ancestor )) samePart = true;
    if (possibleNuclearFragment &&
        (MC::isNucleus(childPDG) || childPDG == 0 || childPDG == MC::PROTON || childPDG == MC::NEUTRON || // FIXME Do we really expect particles with PDG_ID = 0 in the truth record?
         std::abs(childPDG) == MC::PIPLUS || std::abs(childPDG) == MC::PI0))
      NumOfNucFr++;
  }
  // End of section determining decay products

  if (MC::isPhoton(ancestorPDG) && ancestorProdVtx) {
    if (ancestorProdVtx->nIncomingParticles() > 1) { ATH_MSG_DEBUG("DefOrigOfElectron:: photon has more than one parent.");  }
    for (const auto& photonParent: ancestorProdVtx->particles_in()) {
      if (!photonParent) continue;
      info.photonMother = photonParent;  // FIXME Just taking the first valid particle...
    }
  }

  if ((MC::isPhoton(ancestorPDG) && numberOfChildren == 2 && NumOfEl == 1 && NumOfPos == 1) || (MC::isPhoton(ancestorPDG) && numberOfChildren == 1 && (NumOfEl == 1 || NumOfPos == 1))) return PhotonConv;

  // e,gamma,pi+Nuclear->NuclearFragments+nuclons+e
  if ((numOfParents == 1 && (MC::isPhoton(ancestorPDG) || MC::isElectron(ancestorPDG) || MC::isTau(ancestorPDG))) && numberOfChildren > 1 && NumOfNucFr != 0) return ElMagProc;

  if (numOfParents == 1 && std::abs(ancestorPDG) == MC::PIPLUS && numberOfChildren > 2 && NumOfNucFr != 0) return ElMagProc;

  // nuclear photo fission
  if (MC::isPhoton(ancestorPDG) && numberOfChildren > 4 && NumOfNucFr != 0) return ElMagProc;

  // unknown process el(pos)->el+pos??
  if (MC::isElectron(ancestorPDG) && numberOfChildren == 2 && NumOfEl == 1 && NumOfPos == 1) return ElMagProc;

  // unknown process el->el+el??
  if (ancestorPDG == MC::ELECTRON && numberOfChildren == 2 && NumOfEl == 2 && NumOfPos == 0) return ElMagProc;

  // unknown process pos->pos+pos??
  if (ancestorPDG == MC::POSITRON && numberOfChildren == 2 && NumOfEl == 0 && NumOfPos == 2) return ElMagProc;

  // unknown process pos/el->pos/el??
  if (MC::isElectron(ancestorPDG) && !MC::isDecayed(ancestor) && ancestorPDG == thePriPart->pdgId() && numberOfChildren == 1 && !samePart) return ElMagProc;

  // pi->pi+e+/e-; mu->mu+e+/e- ;
  // gamma+ atom->gamma(the same) + e (compton scattering)
  if (numberOfChildren == 2 && (NumOfEl == 1 || NumOfPos == 1) && !MC::isElectron(ancestorPDG) && samePart) return ElMagProc;

  if ((ancestorPDG == MC::PI0 && numberOfChildren == 3 && NumOfPhot == 1 && NumOfEl == 1 && NumOfPos == 1) ||
      (ancestorPDG == MC::PI0 && numberOfChildren == 4 && NumOfPhot == 0 && NumOfEl == 2 && NumOfPos == 2))
    return DalitzDec;

  // Quark weak decay
  if (MC::isSMQuark(ancestorPDG) && numOfParents == 1 && numberOfChildren == 3 && NumOfquark == 1 && NumOfElNeut == 1) return QuarkWeakDec;

  if (MC::isMuon(ancestorPDG) && NumOfNucFr != 0) return ElMagProc;

  if (MC::isTop(ancestorPDG)) return top;

  if (MC::isW(ancestorPDG) && ancestorProdVtx && ancestorProdVtx->nIncomingParticles() != 0) {

    const xAOD::TruthVertex* prodVert = ancestorProdVtx;
    const xAOD::TruthParticle* ptrPart{};
    do {
      ptrPart = prodVert->incomingParticle(0); // FIXME just taking the first one
      prodVert = ptrPart->hasProdVtx() ? ptrPart->prodVtx() : nullptr;
    } while (MC::isW(ptrPart) && prodVert);

    if (prodVert && prodVert->nIncomingParticles() == 1) {
      if (std::abs(ptrPart->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (std::abs(ptrPart->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (std::abs(ptrPart->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(ancestorPDG)) return WBoson;
  if (MC::isZ(ancestorPDG)) return ZBoson;

  // MadGraphPythia ZWW*->lllnulnu
  if (numOfParents == 1 && numberOfChildren > 4 && (MC::isSMQuark(ancestorPDG) || MC::isGluon(ancestorPDG))) {

    const xAOD::TruthParticle* thePartToCheck = thePriPart;
    const xAOD::TruthParticle* theParent = thePriPart->hasProdVtx() ? thePriPart->prodVtx()->incomingParticle(0) : nullptr; // FIXME just taking the first one
    if (theParent && MC::isElectron(theParent) && MC::isDecayed(theParent)) { thePartToCheck = theParent; }

    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;

    for (unsigned int ipOut = 0; ipOut + 1 < partProdVtx->nOutgoingParticles(); ++ipOut) {
      const xAOD::TruthParticle* aChild = partProdVtx->outgoingParticle(ipOut);
      if (!aChild) continue;
      const xAOD::TruthParticle* theNextChild = nullptr;
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partProdVtx->nOutgoingParticles(); ipOut1++) {
        theNextChild = partProdVtx->outgoingParticle(ipOut1);
        if (theNextChild) break;
      }
      if (!theNextChild) continue;
      if (skipnext) {
        skipnext = false;
        continue;
      }

      if (MC::isElectron(aChild) && MC::isElectron(theNextChild)) {
        // Zboson
        if (thePartToCheck == aChild || thePartToCheck == theNextChild) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isElectron(aChild) && std::abs(theNextChild->pdgId()) == MC::NU_E) {
        // WBoson
        if (thePartToCheck == aChild || thePartToCheck == theNextChild) {
          isWboson = true;
          break;
        }
        skipnext = true;
      }
    }
    if (isWboson) return WBoson;
    if (isZboson) return ZBoson;
  }
  if (numOfParents == 2) {
    //--Sherpa Z->ee
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && NumOfEl == 1 && NumOfPos == 1) return ZBoson;

    //--Sherpa W->enu ??
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && (NumOfEl == 1 || NumOfPos == 1) && NumOfElNeut == 1) return WBoson;

    const int pdg1 = partProdVtx->incomingParticle(0)->pdgId();
    const int pdg2 = partProdVtx->incomingParticle(1)->pdgId();
    //--Sherpa ZZ,ZW
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 4 &&
        (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

    //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
    if ((numberOfChildren - NumOfquark - NumOfgluon - NumOfPhot) == 6 &&
        (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }

  // New Sherpa Z->ee
  if (partProdVtx == ancestorProdVtx) {
    int NumOfEleLoop = 0;
    int NumOfLepLoop = 0;
    int NumOfEleNeuLoop = 0;
    for (const auto *const pout: partProdVtx->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partProdVtx->particles_in()) {
        if (!pin) continue;
        if (!HepMC::is_same_particle(pout,pin)) continue;
        if (MC::isElectron(pout)) NumOfEleLoop++;
        if (std::abs(pout->pdgId()) == MC::NU_E) NumOfEleNeuLoop++;
        if (MC::isSMLepton(pout)) NumOfLepLoop++;
        break; // break out of inner loop after having found two matching particles
      }
    }
    if (NumOfEleLoop == 2 && NumOfEleNeuLoop == 0) return ZBoson;
    if (NumOfEleLoop == 1 && NumOfEleNeuLoop == 1) return WBoson;
    if ((NumOfEleLoop == 4 && NumOfEleNeuLoop == 0) || (NumOfEleLoop == 3 && NumOfEleNeuLoop == 1) ||
        (NumOfEleLoop == 2 && NumOfEleNeuLoop == 2)) return DiBoson;
    if (NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(ancestorPDG)) return Higgs;

  if (MC::isMSSMHiggs(ancestorPDG)) return HiggsMSSM; // MSSM Higgs bosons

  if (MC::isHeavyBoson(ancestorPDG)) return HeavyBoson;  // Heavy bosons( Z', Z'', W'+)

  if (MC::isMuon(ancestorPDG)) return Mu;
  if (MC::isTau(ancestorPDG)) {
    const ParticleOrigin tauOrig = defOrigOfTau(xTruthParticleContainer, ancestor, ancestorPDG, info);
    const ParticleType tautype = defTypeOfTau(tauOrig);
    return (tautype == IsoTau)?tauOrig:TauLep;
  }

  if (std::abs(ancestorPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isLeptoQuark(ancestorPDG) || NumOfLQ != 0) return LQ;
  if (MC::isSUSY(ancestorPDG)) return SUSY;
  if (MC::isBSM(ancestorPDG)) return OtherBSM;

  const ParticleType pType = defTypeOfHadron(ancestorPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && ancestorProdVtx && MC::isHardScatteringVertex(ancestorProdVtx)) isPrompt = true;
  return convHadronTypeToOrig(pType, ancestorPDG);
}


ParticleOrigin MCTruthClassifier::defOrigOfMuon(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                const xAOD::TruthParticle* thePart,
                                                bool& isPrompt,
                                                MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfMuon ");

  // Find the first copy of this particle stored in the xAOD::TruthParticleContainer (i.e. the particle prior to any interactions)
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isMuon(thePriPart)) return NonDefined;

  //-- to define muon  outcome status
  info.particleOutCome = defOutComeOfMuon(thePriPart);

  const xAOD::TruthVertex* partProdVtx = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;
  if (!partProdVtx) return NonDefined;

  if (partProdVtx->nIncomingParticles() > 1) ATH_MSG_DEBUG("DefOrigOfMuon:: muon has more than one parent.");

  const xAOD::TruthParticle* ancestor = MC::findMother(thePriPart);
  info.setMotherProperties(ancestor);
  if (!ancestor) { return NonDefined; } // ancestor is not a nullptr beyond this point

  // "method 1" for finding Sherpa loops from defOrigOfElectron not used here. Why?

  if ((MC::isTau(ancestor)|| MC::isW(ancestor)) && ancestor->hasProdVtx()) {
    int pPDG(0);
    const xAOD::TruthParticle* ancestorParent{};
    do {
      pPDG = 0;
      ancestorParent = MC::findMother(ancestor);
      // Start of method 2 of protecting against loops
      // to prevent Sherpa loop
      if (ancestor == ancestorParent) { break; }
      if (TruthLoopDetectionMethod2(ancestor,ancestorParent)) {
        ancestorParent = ancestor;
        break;
      }
      // End of method 2 of protecting against loops

      if (ancestorParent) {
        pPDG = ancestorParent->pdgId();// Only set pPDG in the case that we aren't in a loop.
        if (MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)) { // FIXME should this be (MC::isTau(pPDG) || MC::isW(pPDG)) ???
          // There will be another iteration so set ancestor to ancestorParent
          ancestor = ancestorParent; // ancestorParent is not nullptr here
        }
      }
    } while ((MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG))); // FIXME should this be (MC::isTau(pPDG) || MC::isW(pPDG)) ???

    if (MC::isTau(pPDG) || MC::isW(pPDG) || MC::isZ(pPDG) || MC::isHiggs(pPDG) ||
        MC::isMSSMHiggs(pPDG) || MC::isHeavyBoson(pPDG) || MC::isTop(pPDG) || // MSSM Higgs bosons, Heavy bosons( Z', Z'', W'+)
        std::abs(pPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(pPDG) ||  // Left-right symmetric model WBoson || Right-handed neutrino (Pythia-specific)
        MC::isSUSY(pPDG)) {
      ancestor = ancestorParent; // ancestorParent is not nullptr here
    }
  }

  info.setMotherProperties(ancestor);
  const int ancestorPDG = ancestor->pdgId();
  const xAOD::TruthVertex* ancestorProdVtx = ancestor->hasProdVtx() ? ancestor->prodVtx() : nullptr;
  partProdVtx = ancestor->decayVtx();
  const int numOfParents = partProdVtx->nIncomingParticles();
  const int numberOfChildren = partProdVtx->nOutgoingParticles();

  // Determine decay products
  auto DP = DecayProducts(partProdVtx);
  const int NumOfPhot = DP.pd(MC::PHOTON);
  const int NumOfEl = DP.pd(MC::ELECTRON);
  const int NumOfPos = DP.pd(MC::POSITRON);
  const int NumOfElNeut = DP.apd(MC::NU_E);
  const int NumOfMuNeut = DP.apd(MC::NU_MU);
  const int NumOfLQ = DP.apd(MC::LEPTOQUARK);
  const int NumOfquark = DP.apd({MC::DQUARK,MC::UQUARK,MC::SQUARK,MC::CQUARK,MC::BQUARK,MC::TQUARK});
  const int NumOfgluon = DP.apd(MC::GLUON);
  const int NumOfMuPl = DP.pd(-MC::MUON);
  const int NumOfMuMin = DP.pd(MC::MUON);
  const int NumOfTau = DP.apd(MC::TAU);
  const int NumOfTauNeut = DP.apd(MC::NU_TAU);
  // End of section determining decay products

  if (std::abs(ancestorPDG) == MC::PIPLUS && numberOfChildren == 2 && NumOfMuNeut == 1) return PionDecay;
  if (std::abs(ancestorPDG) == MC::KPLUS && numberOfChildren == 2 && NumOfMuNeut == 1) return KaonDecay;
  if (MC::isTau(ancestorPDG)) {
    const ParticleOrigin tauOrig = defOrigOfTau(xTruthParticleContainer, ancestor, ancestorPDG, info);
    const ParticleType tautype = defTypeOfTau(tauOrig);
    return  (tautype == IsoTau)?tauOrig:TauLep;
  }

  if (MC::isTop(ancestorPDG)) return top;
  // Quark weak decay
  if (MC::isSMQuark(ancestorPDG) && numOfParents == 1 && numberOfChildren == 3 && NumOfquark == 1 && NumOfMuNeut == 1) return QuarkWeakDec;

  if (MC::isW(ancestorPDG) && ancestorProdVtx && ancestorProdVtx->nIncomingParticles() != 0) {
    const xAOD::TruthVertex* prodVert = ancestorProdVtx;
    const xAOD::TruthParticle* itrP;
    do {
      itrP = prodVert->incomingParticle(0); // FIXME just taking the first one
      prodVert = itrP->hasProdVtx() ? itrP->prodVtx() : nullptr;
    } while (MC::isW(itrP) && prodVert);

    if (prodVert && prodVert->nIncomingParticles() == 1) {
      if (std::abs(itrP->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (std::abs(itrP->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (std::abs(itrP->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(ancestorPDG)) return WBoson;
  if (MC::isZ(ancestorPDG)) return ZBoson;
  if (MC::isPhoton(ancestorPDG) && numberOfChildren == 2 && NumOfMuMin == 1 && NumOfMuPl == 1) return PhotonConv;
  //-- Exotics

  // MadGraphPythia ZWW*->lllnulnu
  if (numOfParents == 1 && numberOfChildren > 4 && (MC::isSMQuark(ancestorPDG) || MC::isGluon(ancestorPDG))) {
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;
    for (unsigned int ipOut = 0; ipOut + 1 < partProdVtx->nOutgoingParticles(); ipOut++) {
      if (skipnext) {
        skipnext = false;
        continue;
      }
      const xAOD::TruthParticle* aChild = partProdVtx->outgoingParticle(ipOut);
      if (!aChild) continue;
      const xAOD::TruthParticle* theNextChild{};
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partProdVtx->nOutgoingParticles(); ipOut1++) {
        theNextChild = partProdVtx->outgoingParticle(ipOut1);
        if (theNextChild) break;
      }
      if (!theNextChild) continue;
      if (MC::isMuon(aChild) && MC::isMuon(theNextChild)) {
        // Zboson
        if (thePriPart == aChild || thePriPart == theNextChild) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isMuon(aChild) && std::abs(theNextChild->pdgId()) == MC::NU_MU) {
        // WBoson
        if (thePriPart == aChild || thePriPart == theNextChild) {
          isWboson = true;
          break;
        }
        skipnext = true;
      }
    }
    if (isWboson) return WBoson;
    if (isZboson) return ZBoson;
  }
  if (numOfParents == 2 ) {
    //--Sherpa Z->mumu
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && NumOfMuPl == 1 && NumOfMuMin == 1) return ZBoson;

    //--Sherpa W->munu ??
    // if(numOfParents==2&&(numberOfChildren-NumOfquark-NumOfgluon)==2&&(NumOfEl==1||NumOfPos==1)&&NumOfElNeut==1) return WBoson;
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && (NumOfMuPl == 1 || NumOfMuMin == 1) && NumOfMuNeut == 1) return WBoson;

    const int pdg1 = partProdVtx->incomingParticle(0)->pdgId();
    const int pdg2 = partProdVtx->incomingParticle(1)->pdgId();
    //--Sherpa ZZ,ZW
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 4 &&
        (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

    //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
    if ((numberOfChildren - NumOfquark - NumOfgluon - NumOfPhot) == 6 &&
        (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }

  //--New Sherpa Z->mumu
  if (partProdVtx == ancestorProdVtx) {
    int NumOfMuLoop = 0;
    int NumOfMuNeuLoop = 0;
    int NumOfLepLoop = 0;
    for (const auto & pout: partProdVtx->particles_out()) {
      if (!pout) continue;
      for (const auto & pin: partProdVtx->particles_in()) {
        if (!pin) continue;
        if (HepMC::is_same_particle(pout,pin)) {
          if (MC::isMuon(pout)) NumOfMuLoop++;
          if (std::abs(pout->pdg_id()) == MC::NU_MU) NumOfMuNeuLoop++;
          if (MC::isSMLepton(pout)) NumOfLepLoop++;
          break; // break out of inner loop after having found two matching particles
        }
      }
    }
    if (NumOfMuLoop == 2 && NumOfMuNeuLoop == 0) return ZBoson;
    if (NumOfMuLoop == 1 && NumOfMuNeuLoop == 1) return WBoson;
    if ((NumOfMuLoop == 4 && NumOfMuNeuLoop == 0) || (NumOfMuLoop == 3 && NumOfMuNeuLoop == 1) ||
        (NumOfMuLoop == 2 && NumOfMuNeuLoop == 2)) return DiBoson;
    if (NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(ancestorPDG)) return Higgs;

  if (MC::isMSSMHiggs(ancestorPDG)) return HiggsMSSM; // MSSM Higgs bosons

  if (MC::isHeavyBoson(ancestorPDG))  return HeavyBoson;  // Heavy bosons( Z', Z'', W'+)

  if (std::abs(ancestorPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isLeptoQuark(ancestorPDG) || NumOfLQ != 0) return LQ;
  if (MC::isSUSY(ancestorPDG)) return SUSY;
  if (MC::isBSM(ancestorPDG)) return OtherBSM;

  const ParticleType pType = defTypeOfHadron(ancestorPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && ancestorProdVtx && MC::isHardScatteringVertex(ancestorProdVtx)) isPrompt = true;

  return convHadronTypeToOrig(pType, ancestorPDG);
}


ParticleOrigin MCTruthClassifier::defOrigOfTau(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                               const xAOD::TruthParticle* thePart,
                                               int ancestorPDGin,
                                               MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfTau ");

  // Find the first copy of this particle stored in the xAOD::TruthParticleContainer (i.e. the particle prior to any interactions)
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isTau(thePriPart)) return NonDefined;

  //-- to define tau  outcome status
  if (MC::isPhysical(thePriPart)) info.particleOutCome = defOutComeOfTau(thePriPart); // FIXME why do we need the additional check on MC::isPhysical here c.f. defOrigOfElectron and defOrigOfMuon?

  const xAOD::TruthVertex* partProdVtx = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;
  if (!partProdVtx) return NonDefined;

  if (partProdVtx->nIncomingParticles() > 1) ATH_MSG_DEBUG("DefOrigOfTau:: tau has more than one parent.");

  const xAOD::TruthParticle* ancestor = MC::findMother(thePriPart);
  info.setMotherProperties(ancestor);
  if (!ancestor) { return NonDefined; } // ancestor is not a nullptr beyond this point

  // "method 1" for finding Sherpa loops from defOrigOfElectron not used here. Why?

  // Difference from defOrigOfElectron and defOrigOfMuon - no loop through ancestor particles

  if (MC::isW(ancestorPDGin) && ancestor->hasProdVtx()) { // FIXME ancestorPDGin here could in principle be inconsistent with ancestorProdVtx
    const xAOD::TruthParticle* ancestorParent = MC::findMother(ancestor);
    if (ancestorParent && MC::isTop(ancestorParent->pdgId())) {
      ancestor = ancestorParent; //...so ancestor cannot be nullptr
    }
  }

  const int ancestorPDG = ancestor->pdgId();
  info.setMotherProperties(ancestor);
  const xAOD::TruthVertex* ancestorProdVtx = ancestor->hasProdVtx() ? ancestor->prodVtx() : nullptr;
  partProdVtx = ancestor->decayVtx();
  if (!partProdVtx) return NonDefined; // FIXME not sure this could ever be true?
  const int numOfParents = partProdVtx->nIncomingParticles();

  // Determine decay products
  auto DP = DecayProducts(partProdVtx);
  const int numberOfChildren = DP.size();
  const int NumOfPhot = DP.pd(MC::PHOTON);
  const int NumOfEl = DP.pd(MC::ELECTRON);
  const int NumOfPos = DP.pd(MC::POSITRON);
  const int NumOfElNeut = DP.apd(MC::NU_E);
  const int NumOfMuNeut = DP.apd(MC::NU_MU);
  /* const int NumOfLQ = DP.apd(MC::LEPTOQUARK); */ // FIXME Leptoquarks not an option?
  const int NumOfquark = DP.apd({MC::DQUARK,MC::UQUARK,MC::SQUARK,MC::CQUARK,MC::BQUARK,MC::TQUARK});
  const int NumOfgluon = DP.apd(MC::GLUON);
  const int NumOfMuPl = DP.pd(-MC::MUON);
  const int NumOfMuMin = DP.pd(MC::MUON);
  const int NumOfTau = DP.apd(MC::TAU);
  const int NumOfTauNeut = DP.apd(MC::NU_TAU);
  // End of section determining decay products

  if (MC::isTop(ancestorPDG)) return top;
  if (MC::isW(ancestorPDG) && ancestorProdVtx && ancestorProdVtx->nIncomingParticles() != 0) {
    const xAOD::TruthVertex* prodVert = ancestorProdVtx;
    const xAOD::TruthParticle* itrP;
    do {
      itrP = prodVert->incomingParticle(0); // FIXME just taking the first one
      prodVert = itrP->hasProdVtx() ? itrP->prodVtx() : nullptr;
    } while (MC::isW(itrP) && prodVert);

    if (prodVert && prodVert->nIncomingParticles() == 1 ) {
      if (std::abs(itrP->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (std::abs(itrP->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (std::abs(itrP->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(ancestorPDG)) { return WBoson;}
  if (MC::isZ(ancestorPDG)) { return ZBoson;}
  if (numOfParents == 1 && numberOfChildren > 4 && (MC::isSMQuark(ancestorPDG) || MC::isGluon(ancestorPDG))) {
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;
    for (unsigned int ipOut = 0; ipOut + 1 < partProdVtx->nOutgoingParticles(); ipOut++) {
      if (skipnext) {
        skipnext = false;
        continue;
      }
      const xAOD::TruthParticle* aChild = partProdVtx->outgoingParticle(ipOut);
      if (!aChild) continue;
      const xAOD::TruthParticle* theNextChild{};
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partProdVtx->nOutgoingParticles(); ipOut1++) {
        theNextChild = partProdVtx->outgoingParticle(ipOut1);
        if (theNextChild) break;
      }
      if (!theNextChild) {
        continue;
      }
      if (MC::isTau(aChild) && MC::isTau(theNextChild)) {
        // Zboson
        if (thePriPart == aChild || thePriPart == theNextChild) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isTau(aChild) && std::abs(theNextChild->pdgId()) == MC::NU_TAU) {
        // WBoson
        if (thePriPart == aChild || thePriPart == theNextChild) {
          isWboson = true;
          break;
        }
        skipnext = true;
      }
    }
    if (isWboson) return WBoson;
    if (isZboson) return ZBoson;
  }
  if (numOfParents == 2 ) {
    const int pdg1 = partProdVtx->incomingParticle(0)->pdgId();
    const int pdg2 = partProdVtx->incomingParticle(1)->pdgId();
    //--Sherpa Z->tautau
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && NumOfTau == 2  && (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return ZBoson; // FIXME Why the extra checks on incoming particles compared to Z->ee, Z->mumu and Z->nunu?

    //--Sherpa W->taunu  new
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && NumOfTau == 1 && NumOfTauNeut == 1) return WBoson;

    //--Sherpa ZZ,ZW
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 4 &&
        (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

    //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
    if ((numberOfChildren - NumOfquark - NumOfgluon - NumOfPhot) == 6 &&
        (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }

  // New Sherpa Z->tautau
  if (partProdVtx == ancestorProdVtx) {
    int NumOfTauLoop = 0;
    int NumOfTauNeuLoop = 0;
    int NumOfLepLoop = 0;
    for ( const auto *const pout: partProdVtx->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partProdVtx->particles_in()) {
        if (!pin) continue;
        if (!HepMC::is_same_particle(pout,pin)) continue;
        if (MC::isTau(pout)) NumOfTauLoop++;
        if (std::abs(pout->pdgId()) == MC::NU_TAU) NumOfTauNeuLoop++;
        if (MC::isSMLepton(pout)) NumOfLepLoop++;
        break; // break out of inner loop after having found two matching particles
      }
    }
    if (NumOfTauLoop == 2 && NumOfTauNeuLoop == 0) return ZBoson;
    if (NumOfTauLoop == 1 && NumOfTauNeuLoop == 1) return WBoson;
    if ((NumOfTauLoop == 4 && NumOfTauNeuLoop == 0) || (NumOfTauLoop == 3 && NumOfTauNeuLoop == 1) || (NumOfTauLoop == 2 && NumOfTauNeuLoop == 2)) return DiBoson;
    if (NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(ancestorPDG)) return Higgs;
  if (MC::isMSSMHiggs(ancestorPDG)) return HiggsMSSM; // MSSM Higgs bosons
  if (MC::isHeavyBoson(ancestorPDG)) return HeavyBoson; // Heavy bosons( Z', Z'', W'+)
  if (std::abs(ancestorPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isSUSY(ancestorPDG)) return SUSY;
  if (MC::isBSM(ancestorPDG)) return OtherBSM;
  if (std::abs(ancestorPDG) == MC::JPSI) return JPsi;

  const ParticleType pType = defTypeOfHadron(ancestorPDG);
  return convHadronTypeToOrig(pType, ancestorPDG);
}


ParticleOrigin MCTruthClassifier::defOrigOfPhoton(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                  const xAOD::TruthParticle* thePart,
                                                  bool& isPrompt,
                                                  MCTruthPartClassifier::Info& info) const
{
  if (!thePart) return NonDefined; // FIXME Why is this extra protection needed for this function and not the others?
  ATH_MSG_DEBUG("Executing DefOrigOfPhoton ");

  info.resetMotherProperties();
  info.photonMother = nullptr;

  // Find the first copy of this particle stored in the xAOD::TruthParticleContainer (i.e. the particle prior to any interactions)
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isPhoton(thePriPart)) return NonDefined;

  const xAOD::TruthVertex* partProdVtx = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;

  //-- to define photon outcome status
  info.particleOutCome = defOutComeOfPhoton(thePriPart);

  if (!partProdVtx) return NonDefined;

  int numOfParents = partProdVtx->nIncomingParticles();
  if (partProdVtx->nIncomingParticles() > 1) ATH_MSG_DEBUG("DefOrigOfPhoton:: photon has more than one parent.");

  const xAOD::TruthParticle* ancestor = MC::findMother(thePriPart);
  info.setMotherProperties(ancestor);
  if (!ancestor) { return NonDefined; } // ancestor is not a nullptr beyond this point

  int ancestorPDG = ancestor->pdgId();
  // "method 1" for finding Sherpa loops from defOrigOfElectron not used here. Why?
  const xAOD::TruthVertex* ancestorProdVtx = ancestor->hasProdVtx() ? ancestor->prodVtx() : nullptr;

  partProdVtx = ancestor->decayVtx(); // FIXME how often does this line actually change the pointer???
  numOfParents = partProdVtx->nIncomingParticles();

  const int numberOfChildren = partProdVtx->nOutgoingParticles();

  // Determine decay products
  auto DP = DecayProducts(partProdVtx);
  const int NumOfEl = DP.pd(MC::ELECTRON);
  const int NumOfPos = DP.pd(MC::POSITRON);
  const int NumOfMu = DP.apd(MC::MUON);
  const int NumOfTau = DP.apd(MC::TAU);
  const int NumOfLQ = DP.apd(MC::LEPTOQUARK);
  const int NumOfLep = NumOfEl + NumOfPos + NumOfMu + NumOfTau;
  const int NumOfNeut = DP.apd({MC::NU_E,MC::NU_MU,MC::NU_TAU});
  const int NumOfPht = DP.pd(MC::PHOTON);

  int childPDG(0);
  int NumOfPartons(0);
  int NumOfNucFr(0);
  const bool possibleNuclearFragment = (numOfParents == 1 && (MC::isPhoton(ancestorPDG) || MC::isElectron(ancestorPDG) || std::abs(ancestorPDG) == MC::PIPLUS));
  const xAOD::TruthParticle* child{};
  for (const auto& pout: partProdVtx->particles_out()) {
    if (!pout) continue;
    childPDG = pout->pdg_id();
    if (possibleNuclearFragment &&
        (MC::isNucleus(childPDG) || childPDG == 0 || childPDG == MC::PROTON || childPDG == MC::NEUTRON)) { // FIXME Do we really expect particles with PDG_ID = 0 in the truth record?
      NumOfNucFr++;
    }
    if (std::abs(childPDG) < MC::ELECTRON ||
        (std::abs(childPDG) > MC::NU_TAU && std::abs(childPDG) < 43 && !MC::isPhoton(childPDG))) {
      // FIXME Too loose? This definition picks up 4th generation quarks and leptons as well as all gauge bosons and leptoquarks.
      // Suggest MC::isSMQuark(childPDG) || (MC::isBoson(childPDG) && !MC::isPhoton(childPDG))
      // or maybe even MC::isSMQuark(childPDG) || MC::isGluon(childPDG)
      // AKA const int NumOfPartons = DP.apd({MC::DQUARK,MC::UQUARK,MC::SQUARK,MC::CQUARK,MC::BQUARK,MC::TQUARK,MC::GLUON});
      NumOfPartons++;
    }
    if (childPDG == ancestorPDG) {
      child = pout;
    }
  }
  // End of section determining decay products

  bool foundISR = false;
  bool foundFSR = false;
  if (numOfParents == 1 && numberOfChildren == 2 &&  child && HepMC::is_same_generator_particle(child, ancestor)) return BremPhot;
  if (numOfParents == 1 && numberOfChildren == 2 && MC::isElectron(ancestorPDG) && NumOfPht == 2) return ElMagProc;

  // decay of W,Z and Higgs to lepton with FSR generated by Pythia
  if (numOfParents == 1 && numberOfChildren == 2 && (MC::isElectron(ancestorPDG) || MC::isMuon(ancestorPDG) || MC::isTau(ancestorPDG)) &&
      !(child && HepMC::is_same_generator_particle(child, ancestor)) && ancestorProdVtx &&
      ancestorProdVtx->nIncomingParticles() == 1) {
    int itr = 0;
    int PartPDG = 0;
    const xAOD::TruthVertex* prodVert = ancestorProdVtx;
    const xAOD::TruthVertex* Vert{};
    do {
      Vert = prodVert;
      for (const auto & pin: Vert->particles_in()) {
        if (!pin) continue;
        PartPDG = std::abs(pin->pdgId());
        prodVert = pin->prodVtx();
        if (MC::isZ(PartPDG) || MC::isW(PartPDG) || MC::isHiggs(PartPDG)) foundFSR = true;
      }
      itr++;
      if (itr > 100) { // FIXME Improve loop detection here?
        ATH_MSG_WARNING("DefOrigOfPhoton:: infinite while");
        break;
      }
    } while (prodVert && std::abs(ancestorPDG) == PartPDG);

    if (foundFSR) return FSRPhot;
  }

  // Nucl reaction
  // gamma+Nuclear=>gamma+Nucl.Fr+Nuclons+pions
  // e+Nuclear=>e+gamma+Nucl.Fr+Nuclons+pions
  // pi+Nuclear=>gamma+Nucl.Fr+Nuclons+pions

  if ((numOfParents == 1 && (MC::isPhoton(ancestorPDG) || MC::isElectron(ancestorPDG)) && numberOfChildren > 2 && NumOfNucFr != 0) ||
      (numOfParents == 1 && std::abs(ancestorPDG) == MC::PIPLUS && numberOfChildren > 10 && NumOfNucFr != 0) ||
      (numOfParents == 1 && MC::isPhoton(ancestorPDG) && numberOfChildren > 10 && MC::isStable(ancestor)) ||
      (numOfParents == 1 && MC::isNucleus(ancestorPDG) && std::abs(ancestorPDG) != MC::PROTON)) // FIXME vetoing protons here to preserve previous behaviour
    return NucReact;

  if (MC::isMuon(ancestorPDG) && NumOfMu == 0) return Mu;
  if (MC::isTau(ancestorPDG) && NumOfTau == 0) return TauLep;

  if (numOfParents == 1 && ancestor->status() == 3) return (foundISR)? ISRPhot:UndrPhot; // FIXME foundISR is always false at this point

  //-- to find initial and final state raiation and underline photons
  //-- SUSY
  if (numOfParents == 1 && (MC::isSMQuark(ancestorPDG) || MC::isGluon(ancestorPDG)) &&
      (numberOfChildren != NumOfPht + NumOfPartons || !MC::Pythia8::isConditionA(ancestor))) {
    for (const auto& pout: partProdVtx->particles_out()) {
      if (!pout) continue;
      if (ancestorPDG != pout->pdgId()) continue;
      const xAOD::TruthVertex* Vrtx = pout->decayVtx();
      if (!Vrtx) continue;
      if (Vrtx->nOutgoingParticles() != 1 && Vrtx->nIncomingParticles() == 1) continue;
      if (!Vrtx->outgoingParticle(0)) continue;
      if (Vrtx->outgoingParticle(0)->pdgId() == 91) foundISR = true; // Herwig "cluster"
    }
    return (foundISR)?ISRPhot:UndrPhot;
  }

  //-- to find final  state radiation
  //-- Exotics

  // FSR  from Photos
  //-- Exotics- CompHep
  if (numOfParents == 2 && ((MC::isElectron(ancestorPDG) && NumOfEl == 1 && NumOfPos == 1) || (MC::isMuon(ancestorPDG) && NumOfMu == 2) || (MC::isTau(ancestorPDG) && NumOfTau == 2))) {
    if (std::abs(partProdVtx->incomingParticle(0)->pdgId()) == std::abs(partProdVtx->incomingParticle(1)->pdgId())) return FSRPhot;
  }

  if (numOfParents == 2 && NumOfLep == 1 && NumOfNeut == 1 && (MC::isElectron(ancestorPDG) || std::abs(ancestorPDG) == MC::NU_E)) return FSRPhot;

  //-- Exotics - CompHep
  if (MC::isElectron(ancestorPDG) && numOfParents == 1 && numberOfChildren == 2 && (NumOfEl == 1 || NumOfPos == 1) && NumOfPht == 1 &&
      !( child && HepMC::is_same_generator_particle(child, ancestor)) && !HepMC::is_simulation_particle(child) && !HepMC::is_simulation_particle(ancestor))
    return FSRPhot;

  // FSR  from Photos
  if (MC::isZ(ancestorPDG) && ((NumOfEl + NumOfPos == 2 || NumOfEl + NumOfPos == 4) || (NumOfMu == 2 || NumOfMu == 4) || (NumOfTau == 2 || NumOfTau == 4)) && NumOfPht > 0) return FSRPhot;

  if (NumOfPht > 0 && (std::abs(ancestorPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(ancestorPDG))) return FSRPhot; // Left-right symmetric model WBoson || Right-handed neutrinos (Pythia-specific)

  if (numOfParents == 2 && NumOfLQ == 1) return FSRPhot;

  //--- other process

  if (MC::isZ(ancestorPDG)) return ZBoson;
  if (MC::isW(ancestorPDG)) {

    if (NumOfLep == 1 && NumOfNeut == 1 && numberOfChildren == NumOfLep + NumOfNeut + NumOfPht) return FSRPhot;

    if (ancestorProdVtx && ancestorProdVtx->nIncomingParticles() != 0) {
      const xAOD::TruthVertex* prodVert = ancestorProdVtx;
      const xAOD::TruthParticle* itrP;
      do {
        itrP = prodVert->incomingParticle(0); // FIXME just taking the first one
        prodVert = itrP->hasProdVtx() ? itrP->prodVtx() : nullptr;
      } while (MC::isW(itrP) && prodVert);

      if (prodVert && prodVert->nIncomingParticles() == 1 ) {
        if ( MC::isTau(itrP)) return TauLep;
        if ( MC::isMuon(itrP)) return Mu;
        if ( std::abs(itrP->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
        if ( std::abs(itrP->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
        if ( std::abs(itrP->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
      }
    } else
      return WBoson;
  }

  // MadGraphPythia ZWW*->lllnulnu
  if (numOfParents == 1 && numberOfChildren > 4 && (MC::isSMQuark(ancestorPDG) || MC::isGluon(ancestorPDG))) {
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;
    for (unsigned int ipOut = 0; ipOut + 1 < partProdVtx->nOutgoingParticles(); ipOut++) {
      if (skipnext) {
        skipnext = false;
        continue;
      }
      const xAOD::TruthParticle* aChild = partProdVtx->outgoingParticle(ipOut);
      if (!aChild) continue;
      const xAOD::TruthParticle* theNextChild{};
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partProdVtx->nOutgoingParticles(); ipOut1++) {
        theNextChild = partProdVtx->outgoingParticle(ipOut1);
        if (theNextChild) break;
      }
      if (!theNextChild) continue;
      if (MC::isTau(aChild) && MC::isTau(theNextChild)) {
        // Zboson
        if (thePriPart == aChild || thePriPart == theNextChild) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isTau(aChild) && std::abs(theNextChild->pdgId()) == MC::NU_TAU) {
        // WBoson
        if (thePriPart == aChild || thePriPart == theNextChild) {
          isWboson = true;
          break;
        }
        skipnext = true;
      }
    }
    if (isWboson) return WBoson;
    if (isZboson) return ZBoson;
  }

  //--Sherpa ZZ,ZW+FSR
  if (numOfParents == 4 && (numberOfChildren - NumOfPht) == 4 && (NumOfLep + NumOfNeut == 4)) {
    if (MC::isSMLepton(partProdVtx->incomingParticle(0))&&MC::isSMLepton(partProdVtx->incomingParticle(1))
        && MC::isSMLepton(partProdVtx->incomingParticle(2))&&MC::isSMLepton(partProdVtx->incomingParticle(3)))
      return FSRPhot;
  }

  //--New Sherpa single photon
  if (partProdVtx == ancestorProdVtx) {
    for (const auto *const pout: partProdVtx->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partProdVtx->particles_in()) {
        if (!pin) continue;
        if (!HepMC::is_same_particle(pout,pin)) continue;
        if (MC::isPhoton(pout)) return SinglePhot;
        break; // break out of inner loop after having found two matching particles
      }
    }
  }

  if (MC::isHiggs(ancestorPDG)) return Higgs;
  if (std::abs(ancestorPDG) == MC::PI0) return PiZero;
  if (MC::isMSSMHiggs(ancestorPDG)) return HiggsMSSM; // MSSM Higgs bosons
  if (MC::isHeavyBoson(ancestorPDG) || std::abs(ancestorPDG) == 5100039 ) return HeavyBoson; // Heavy Bosons (Z' Z'' W'+) + KK excited graviton

  if (MC::isSUSY(ancestorPDG)) return SUSY;
  if (MC::isBSM(ancestorPDG)) return OtherBSM;

  // Pythia8 gamma+jet samples
  if (MC::Pythia8::isConditionA(ancestor) && MC::isStable(thePriPart) && NumOfPht == 1 && numberOfChildren == (NumOfPht + NumOfPartons))  return PromptPhot;

  const ParticleType pType = defTypeOfHadron(ancestorPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && ancestorProdVtx && MC::isHardScatteringVertex(ancestorProdVtx)) isPrompt = true;
  return convHadronTypeToOrig(pType, ancestorPDG);
}

ParticleOrigin
MCTruthClassifier::defOrigOfNeutrino(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                     const xAOD::TruthParticle* thePart,
                                     bool& isPrompt,
                                     MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfNeutrino ");

  const int nuFlav = std::abs(thePart->pdgId());
  // Find the first copy of this particle stored in the xAOD::TruthParticleContainer (i.e. the particle prior to any interactions)
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (std::abs(thePriPart->pdgId()) != nuFlav) return NonDefined; // FIXME should this be if (!MC::isSMNeutrino(thePriPart) || abs(thePriPart->pdgId()) != nuFlav) return NonDefined; // (Use MC::isNeutrino if 4th generation neutrinos OK)

  //-- to define neutrino outcome status
  info.particleOutCome = NonInteract;

  const xAOD::TruthVertex* partProdVtx = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;
  if (!partProdVtx) return NonDefined;

  if (partProdVtx->nIncomingParticles() > 1) ATH_MSG_DEBUG("DefOrigOfNeutrino:: neutrino has more than one parent.");

  const xAOD::TruthParticle* ancestor = MC::findMother(thePriPart);
  info.setMotherProperties(ancestor);
  if (!ancestor) { return NonDefined; } // ancestor is not a nullptr beyond this point

  // Start of method 3 of protecting against loops
  // to resolve Sherpa loop
  bool samePart = TruthLoopDetectionMethod1(partProdVtx, ancestor);
  // End of method 3 of protecting against loops

  if ((std::abs(ancestor->pdgId()) == nuFlav || MC::isTau(ancestor) || MC::isW(ancestor)) && ancestor->hasProdVtx() && !samePart) {
    int pPDG(0);
    const xAOD::TruthParticle* ancestorParent{};
    do {
      pPDG = 0;
      ancestorParent = MC::findMother(ancestor);
      // Start of method 2 of protecting against loops
      // to prevent Sherpa loop
      if (TruthLoopDetectionMethod2(ancestor,ancestorParent)) {
        ancestorParent = ancestor;
        break;
      }
      //
      if (ancestorParent) {
        pPDG = ancestorParent->pdgId(); // FIXME difference in behaviour compared to defOrigOfElectron/Muon pPDG set even if we are in a loop
      }
      // to prevent Sherpa loop
      if (ancestor == ancestorParent) { break; }
      // End of method 2 of protecting against Sherpa loops
      if (std::abs(pPDG) == nuFlav || MC::isTau(pPDG) || MC::isW(pPDG) ) {
        // There will be another iteration so set ancestor to ancestorParent
        ancestor = ancestorParent; // ancestorParent is not a nullptr
        info.setMotherProperties(ancestor); // FIXME difference in behaviour compared to MCTruthClassifier::defOrigOfElectron/Muon
      }

    } while ((std::abs(pPDG) == nuFlav || MC::isTau(pPDG) || MC::isW(pPDG)));

    if (std::abs(pPDG) == nuFlav || MC::isTau(pPDG) || MC::isW(pPDG) || MC::isZ(pPDG) || MC::isHiggs(pPDG) ||
        MC::isMSSMHiggs(pPDG) || MC::isHeavyBoson(pPDG) || MC::isTop(pPDG) ||  // MSSM Higgs bosons, Heavy bosons( Z', Z'', W'+)
        std::abs(pPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(pPDG) || // Left-right symmetric model WBoson || Right-handed neutrino (Pythia-specific)
        MC::isSUSY(pPDG)) {
      ancestor = ancestorParent; // ancestorParent is not nullptr here
      info.setMotherProperties(ancestor);
    }
  }
  //if ancestor is still nullptr, we have a problem
  if (!ancestor) return NonDefined; // FIXME it should not be possible for ancestor to be nullptr at this point???

  info.setMotherProperties(ancestor);
  const int ancestorPDG = ancestor->pdgId();
  partProdVtx = ancestor->decayVtx();
  const xAOD::TruthVertex* ancestorProdVtx = ancestor->hasProdVtx() ? ancestor->prodVtx() : nullptr;
  const int numOfParents = partProdVtx->nIncomingParticles();
  const int numberOfChildren = partProdVtx->nOutgoingParticles();

  // Determine decay products
  auto DP = DecayProducts(partProdVtx);
  const int NumOfPhot = DP.pd(MC::PHOTON);
  const int NumOfquark = DP.apd({MC::DQUARK,MC::UQUARK,MC::SQUARK,MC::CQUARK,MC::BQUARK,MC::TQUARK});
  const int NumOfgluon = DP.apd(MC::GLUON);
  const int NumOfLQ = DP.apd(MC::LEPTOQUARK);
  const int NumOfElNeut = DP.apd(MC::NU_E);
  const int NumOfMuNeut = DP.apd(MC::NU_MU);
  const int NumOfTauNeut = DP.apd(MC::NU_TAU);
  const int NumOfEl = DP.apd(MC::ELECTRON);
  const int NumOfMu = DP.apd(MC::MUON);
  const int NumOfTau = DP.apd(MC::TAU);

  samePart = false;
  for (const auto& aChild: partProdVtx->particles_out()) {
    if (!aChild) continue;
    if (aChild->pdgId() == ancestorPDG && HepMC::is_same_generator_particle(aChild,ancestor)) {
      samePart = true;
      break;
    }
  }
  // End of section determining decay products

  // Quark weak decay
  if (MC::isQuark(ancestorPDG) && numOfParents == 1 && numberOfChildren == 3 && NumOfquark == 1 && (NumOfEl == 1 || NumOfMu == 1 || NumOfTau == 1)) return QuarkWeakDec;
  if (MC::isTop(ancestorPDG)) return top;

  if (MC::isW(ancestorPDG) && ancestorProdVtx && ancestorProdVtx->nIncomingParticles() != 0) {
    const xAOD::TruthVertex* prodVert = ancestorProdVtx;
    const xAOD::TruthParticle* ptrPart;
    do {
      ptrPart = prodVert->incomingParticle(0); // FIXME just taking the first one
      prodVert = ptrPart->hasProdVtx() ? ptrPart->prodVtx() : nullptr;
    } while (MC::isW(ptrPart) && prodVert);

    if (prodVert && prodVert->nIncomingParticles() == 1) {
      if (std::abs(ptrPart->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (std::abs(ptrPart->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (std::abs(ptrPart->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(ancestorPDG)) return WBoson;
  if (MC::isZ(ancestorPDG)) return ZBoson;

  //-- Exotics

  // MadGraphPythia ZWW*->lllnulnu or ZWW*->nunulnulnu (don't even know if the latter is generated)
  if (numOfParents == 1 && numberOfChildren > 4 && (MC::isSMQuark(ancestorPDG) || MC::isGluon(ancestorPDG))) {

    const xAOD::TruthParticle* thePartToCheck = thePriPart;
    const xAOD::TruthParticle* theParent = thePriPart->hasProdVtx() ? thePriPart->prodVtx()->incomingParticle(0) : nullptr; // FIXME just taking the first one

    if (MC::isElectron(theParent) && MC::isDecayed(theParent)) { thePartToCheck = theParent; }
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;

    for (unsigned int ipOut = 0; ipOut + 1 < partProdVtx->nOutgoingParticles(); ++ipOut) {
      const xAOD::TruthParticle* aChild = partProdVtx->outgoingParticle(ipOut);
      if (!aChild) continue;
      const xAOD::TruthParticle* theNextChild{};
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partProdVtx->nOutgoingParticles(); ipOut1++) {
        theNextChild = partProdVtx->outgoingParticle(ipOut1);
        if (theNextChild) break;
      }
      if (!theNextChild) continue;

      if (skipnext) {
        skipnext = false;
        continue;
      }

      const int apdgID1 = std::abs(aChild->pdgId());
      const int apdgID2 = std::abs(theNextChild->pdgId());
      if (apdgID1 == apdgID2 && MC::isSMNeutrino(apdgID1)) {
        // Zboson
        if (thePartToCheck == aChild || thePartToCheck == theNextChild) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if ((apdgID1 == MC::ELECTRON && apdgID2 == MC::NU_E) ||
                 (apdgID1 == MC::NU_E && apdgID2 == MC::ELECTRON) ||
                 (apdgID1 == MC::MUON && apdgID2 == MC::NU_MU) ||
                 (apdgID1 == MC::NU_MU && apdgID2 == MC::MUON) ||
                 (apdgID1 == MC::TAU && apdgID2 == MC::NU_TAU) ||
                 (apdgID1 == MC::NU_TAU && apdgID2 == MC::TAU)
                 ) {
        // WBoson
        if (thePartToCheck == aChild || thePartToCheck == theNextChild) {
          isWboson = true;
          break;
        }
        skipnext = true;
      }
    }
    if (isWboson) return WBoson;
    if (isZboson) return ZBoson;
  }

  if (numOfParents == 2) {
    //--Sherpa Z->nunu
    if ( (numberOfChildren - NumOfquark - NumOfgluon) == 2 && (NumOfElNeut == 2 || NumOfMuNeut == 2 || NumOfTauNeut == 2)) return ZBoson;

    //--Sherpa W->enu ??
    if ((numberOfChildren - NumOfquark - NumOfgluon) == 2 && ((NumOfEl == 1 && NumOfElNeut == 1) || (NumOfMu == 1 && NumOfMuNeut == 1) || (NumOfTau == 1 && NumOfTauNeut == 1))) return WBoson;

    const int pdg1 = partProdVtx->incomingParticle(0)->pdgId();
    const int pdg2 = partProdVtx->incomingParticle(1)->pdgId();
    //--Sherpa ZZ,ZW
    if ( (numberOfChildren - NumOfquark - NumOfgluon) == 4 && (NumOfEl + NumOfMu + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
         (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

    //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
    if ((numberOfChildren - NumOfquark - NumOfgluon - NumOfPhot) == 6 && (NumOfEl + NumOfMu + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
        (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }

  // New Sherpa Z->nunu
  if (partProdVtx == ancestorProdVtx) {
    int NumOfLepLoop = 0;
    int NumOfNeuLoop = 0;
    for (const auto *const pout: partProdVtx->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partProdVtx->particles_in()) {
        if (!pin) continue;
        if (HepMC::is_same_particle(pin,pout)) continue;
        const int apdgid = std::abs(pout->pdgId());
        if (MC::isSMLepton(apdgid)) {
          if (MC::isSMNeutrino(apdgid)) { NumOfNeuLoop++; }
          else { NumOfLepLoop++; }
        }
        break; // break out of inner loop after having found two matching particles
      }
    }
    if (NumOfNeuLoop == 2 && NumOfLepLoop == 0) return ZBoson;
    if (NumOfNeuLoop == 1 && NumOfLepLoop == 1) return WBoson;
    if (NumOfNeuLoop + NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(ancestorPDG)) return Higgs;
  if (MC::isMSSMHiggs(ancestorPDG)) return HiggsMSSM; // MSSM Higgs bosons
  if (MC::isHeavyBoson(ancestorPDG)) return HeavyBoson; // Heavy bosons( Z', Z'', W'+)

  if (MC::isTau(ancestorPDG)) {
    const ParticleOrigin tauOrig = defOrigOfTau(xTruthParticleContainer, ancestor, ancestorPDG, info);
    const ParticleType tautype = defTypeOfTau(tauOrig);
    return (tautype == IsoTau)?tauOrig:TauLep;
  }

  if (std::abs(ancestorPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
  if (std::abs(ancestorPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isLeptoQuark(ancestorPDG) || NumOfLQ != 0) return LQ;
  if (MC::isSUSY(ancestorPDG)) return SUSY;
  if (MC::isBSM(ancestorPDG)) return OtherBSM;

  const ParticleType pType = defTypeOfHadron(ancestorPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && ancestorProdVtx && MC::isHardScatteringVertex(ancestorProdVtx)) isPrompt = true;

  return convHadronTypeToOrig(pType, ancestorPDG);
}
