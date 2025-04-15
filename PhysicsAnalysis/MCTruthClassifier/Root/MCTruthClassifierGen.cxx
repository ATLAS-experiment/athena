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
using std::abs;

#ifndef XAOD_ANALYSIS
std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const HepMcParticleLink& theLink, MCTruthPartClassifier::Info* info /*= nullptr*/) const {
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

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(HepMC::ConstGenParticlePtr theGenPart, MCTruthPartClassifier::Info* info /*= nullptr*/) const {
  ParticleType partType = Unknown;
  ParticleOrigin partOrig = NonDefined;

  if (!theGenPart) return std::make_pair(partType, partOrig);

  // Retrieve the links between HepMC and xAOD::TruthParticle
  const EventContext& ctx = info ? info->eventContext : Gaudi::Hive::currentContext();

  SG::ReadHandle<xAODTruthParticleLinkVector> truthParticleLinkVecReadHandle(m_truthLinkVecReadHandleKey, ctx);
  if (!truthParticleLinkVecReadHandle.isValid()) {
    ATH_MSG_WARNING( " Invalid ReadHandle for xAODTruthParticleLinkVector with key: " << truthParticleLinkVecReadHandle.key());
    return std::make_pair(partType, partOrig);
  }
  for (const auto *const entry : *truthParticleLinkVecReadHandle) {
    if (entry->first.isValid() && entry->second.isValid() && HepMC::is_same_particle(entry->first,theGenPart)) {
      const xAOD::TruthParticle* truthParticle = *entry->second;
      if (!theGenPart || !truthParticle ||
          theGenPart->pdg_id() != truthParticle->pdgId() ||
          theGenPart->status() != truthParticle->status() ||
          HepMC::is_same_particle(theGenPart,truthParticle)) {
        ATH_MSG_DEBUG(
            "HepMC::GenParticle and xAOD::TruthParticle do not match");
        return std::make_pair(partType, partOrig);
      }
      return particleTruthClassifier(truthParticle, info);
    }
  }
  return std::make_pair(partType, partOrig);
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
  bool isPartHadr = MC::isHadron(thePart) && !MC::isBeam(thePart);
  if (MC::isDecayed(thePart) && (!MC::isTau(thePart) && !isPartHadr)) return std::make_pair(GenParticle, partOrig);

  // SUSY datasets: tau(status==2)->tau(status==2)
  if (MC::isDecayed(thePart) && MC::isTau(thePart)) {
    const xAOD::TruthVertex* endVert = thePart->decayVtx();
    if (endVert != nullptr) {
      int numOfDaught = endVert->nOutgoingParticles();
      if (numOfDaught == 1 && MC::isTau(endVert->outgoingParticle(0))) {
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
  // don't consider  generator particles

  const xAOD::TruthVertex* partOriVert = thePart->hasProdVtx() ? thePart->prodVtx() : nullptr;

  const xAOD::TruthParticle* theMoth{};
  if (partOriVert != nullptr) {
    for (const auto& temp: partOriVert->particles_in()) {if (temp)  theMoth = temp;}
  }
  int motherPDG = theMoth?theMoth->pdg_id():0;
  info.setMotherProperties(theMoth);

  if (!partOriVert && HepMC::is_simulation_particle(thePart)) {
    return std::make_pair(NonPrimary, partOrig);
  }
  if (!partOriVert && MC::isElectron(thePart)) {
    // to define electron out come  status
    bool isPrompt = false;
    partOrig = defOrigOfElectron(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    return std::make_pair(UnknownElectron, partOrig);
  }
  if (!partOriVert && MC::isMuon(thePart)) {
    // to define electron out come  status
    bool isPrompt = false;
    partOrig = defOrigOfMuon(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    return std::make_pair(UnknownMuon, partOrig);
  }
  if (!partOriVert && MC::isTau(thePart)) {
    // to define electron out come  status
    partOrig = defOrigOfTau(*truthParticleContainerReadHandle, thePart, motherPDG, info);
    return std::make_pair(UnknownTau, partOrig);
  }
  if (!partOriVert && MC::isPhoton(thePart)) {
    // to define photon out come
    bool isPrompt = false;
    partOrig = defOrigOfPhoton(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    return std::make_pair(UnknownPhoton, partOrig);
  }
  if (!partOriVert && MC::isNeutrino(thePart)) {
    // to define neutrino outcome
    info.particleOutCome = NonInteract;
    return std::make_pair(Neutrino, partOrig);
  }

  if (thePart&& info.Mother() && HepMC::is_same_generator_particle(thePart,info.Mother()))
    return std::make_pair(NonPrimary, partOrig);

  if (isPartHadr) return std::make_pair(Hadron, partOrig);

  if (partOriVert  && motherPDG == 0 && partOriVert->nOutgoingParticles() == 1 &&
      partOriVert->nIncomingParticles() == 0) {
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

  if (motherPDG == thePart->pdg_id() &&  theMoth && theMoth->status() == 3 && MC::isDecayed(thePart)) return std::make_pair(GenParticle, partOrig);

  if (MC::isElectron(thePart)) {
    bool isPrompt = false;
    partOrig = defOrigOfElectron(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = defTypeOfElectron(partOrig, isPrompt);
  } else if (MC::isMuon(thePart)) {
    bool isPrompt = false;
    partOrig = defOrigOfMuon(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = defTypeOfMuon(partOrig, isPrompt);
  } else if (MC::isTau(thePart)) {
    partOrig = defOrigOfTau(*truthParticleContainerReadHandle, thePart, motherPDG, info);
    partType = defTypeOfTau(partOrig);
  } else if (MC::isPhoton(thePart)) {
    bool isPrompt = false;
    partOrig = defOrigOfPhoton(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = defTypeOfPhoton(partOrig);
  } else if (MC::isNeutrino(thePart)) {
    bool isPrompt = false;
    partOrig = defOrigOfNeutrino(*truthParticleContainerReadHandle, thePart, isPrompt, info);
    partType = Neutrino;
  }

  ATH_MSG_DEBUG("particleTruthClassifier  succeeded ");
  return std::make_pair(partType, partOrig);
}



ParticleOrigin MCTruthClassifier::defOrigOfElectron(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                     const xAOD::TruthParticle* thePart,
                                     bool& isPrompt,
                                     MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfElectron ");

  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isElectron(thePriPart)) return NonDefined;

  const xAOD::TruthVertex* partOriVert = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;

  //-- to define electron outcome status
  info.particleOutCome = defOutComeOfElectron(thePriPart);

  if (!partOriVert) return NonDefined;

  int numOfParents = -1;
  numOfParents = partOriVert->nIncomingParticles();
  if (numOfParents > 1) ATH_MSG_DEBUG("DefOrigOfElectron:: electron  has more than one mother ");

  const xAOD::TruthParticle* mother = MC::findMother(thePriPart);
  info.setMotherProperties(mother);
  if (!mother) {
    return NonDefined;
  }
  int motherPDG = mother->pdgId();
  info.setMotherProperties(mother);
  const xAOD::TruthVertex* mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;

  bool samePart = false;
  for (const auto& theDaug: partOriVert->particles_out()) {
    if (!theDaug)  continue;
    if (motherPDG == theDaug->pdgId() && info.Mother() && HepMC::is_same_generator_particle(theDaug, info.Mother())) samePart = true;
  }

  // to resolve Sherpa loop
  if (mothOriVert && HepMC::is_same_vertex(mothOriVert,partOriVert)) samePart = true;
  //

  if ((MC::isMuon(motherPDG) || MC::isTau(motherPDG) || MC::isW(motherPDG)) && mothOriVert != nullptr && !samePart) {
    int pPDG(0);
    const xAOD::TruthParticle* MotherParent(nullptr);
    do {
      pPDG = 0;
      MotherParent = MC::findMother(mother);
      // to prevent Sherpa loop
      const xAOD::TruthVertex* mother_prdVtx(nullptr);
      const xAOD::TruthVertex* mother_endVtx(nullptr);
      if (mother) {
        mother_prdVtx = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
        mother_endVtx = mother->decayVtx();
      }
      const xAOD::TruthVertex* parent_prdVtx(nullptr);
      const xAOD::TruthVertex* parent_endVtx(nullptr);
      if (MotherParent) {
        parent_prdVtx = MotherParent->hasProdVtx() ? MotherParent->prodVtx() : nullptr;
        parent_endVtx = MotherParent->decayVtx();
      }
      if (mother_endVtx == parent_prdVtx && mother_prdVtx == parent_endVtx) {
        MotherParent = mother;
        break;
      }
      //
      if (MotherParent) pPDG = MotherParent->pdgId();
      // to prevent Sherpa loop
      if (mother == MotherParent) break;
      if (MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)) mother = MotherParent;

    } while ((MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)));

    if (MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG) || MC::isZ(pPDG) || MC::isHiggs(pPDG) ||
        MC::isMSSMHiggs(pPDG) || MC::isHeavyBoson(pPDG) || MC::isTop(pPDG) || // MSSM Higgs bosons, Heavy bosons( Z', Z'', W'+)
        abs(pPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(pPDG) || // Left-right symmetric model WBoson || Right-handed neutrino (Pythia-specific)
        MC::isSUSY(pPDG))
      mother = MotherParent;
  }

  motherPDG = mother->pdgId();
  partOriVert = mother->decayVtx();
  mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
  numOfParents = partOriVert->nIncomingParticles();
  int numOfDaug = partOriVert->nOutgoingParticles();

  info.setMotherProperties(mother);

  int NumOfPhot(0);
  int NumOfEl(0);
  int NumOfPos(0);
  int NumOfNucFr(0);
  int NumOfquark(0);
  int NumOfgluon(0);
  int NumOfElNeut(0);
  int NumOfLQ(0);
  int NumOfMuPl(0);
  int NumOfMuMin(0);
  int NumOfMuNeut(0);
  int NumOfTau(0);
  int NumOfTauNeut(0);
  samePart = false;

  for (const auto& theDaug: partOriVert->particles_out()) {
    if (!theDaug) continue;
    int DaugType = theDaug->pdgId();
    if (MC::isSMQuark(DaugType)) NumOfquark++;
    else if (MC::isGluon(DaugType)) NumOfgluon++;
    else if (abs(DaugType) == MC::NU_E) NumOfElNeut++;
    else if (abs(DaugType) == MC::NU_MU) NumOfMuNeut++;
    else if (MC::isPhoton(DaugType)) NumOfPhot++;
    else if (DaugType == MC::ELECTRON) NumOfEl++;
    else if (DaugType == MC::POSITRON) NumOfPos++;
    else if (DaugType == MC::MUON) NumOfMuMin++;
    else if (DaugType == -MC::MUON) NumOfMuPl++;
    else if (MC::isTau(DaugType)) NumOfTau++;
    else if (abs(DaugType) == MC::NU_TAU) NumOfTauNeut++;
    else if (MC::isLeptoQuark(DaugType)) NumOfLQ++;
    if (abs(DaugType) == abs(motherPDG) && theDaug && info.Mother() && HepMC::is_same_generator_particle(theDaug, info.Mother() )) samePart = true;
    if (numOfParents == 1 &&
        (MC::isPhoton(motherPDG) || MC::isElectron(motherPDG) || MC::isMuon(motherPDG) || abs(motherPDG) == MC::PIPLUS) &&
        (MC::isNucleus(DaugType) || DaugType == 0 || DaugType == MC::PROTON || DaugType == MC::NEUTRON ||
         abs(DaugType) == MC::PIPLUS || abs(DaugType) == MC::PI0))
      NumOfNucFr++;
  }

  if (MC::isPhoton(motherPDG) && mothOriVert != nullptr) {
    for (const auto& theMother: mothOriVert->particles_in()) {
      if (!theMother) continue;
      info.photonMother = theMother;
    }
  }

  if ((MC::isPhoton(motherPDG) && numOfDaug == 2 && NumOfEl == 1 && NumOfPos == 1) || (MC::isPhoton(motherPDG) && numOfDaug == 1 && (NumOfEl == 1 || NumOfPos == 1))) return PhotonConv;

  // e,gamma,pi+Nuclear->NuclearFragments+nuclons+e
  if ((numOfParents == 1 && (MC::isPhoton(motherPDG) || MC::isElectron(motherPDG) || MC::isTau(motherPDG))) && numOfDaug > 1 && NumOfNucFr != 0) return ElMagProc;

  if (numOfParents == 1 && abs(motherPDG) == MC::PIPLUS && numOfDaug > 2 && NumOfNucFr != 0) return ElMagProc;

  // nuclear photo fission
  if (MC::isPhoton(motherPDG) && numOfDaug > 4 && NumOfNucFr != 0) return ElMagProc;

  // unknown process el(pos)->el+pos??
  if (MC::isElectron(motherPDG) && numOfDaug == 2 && NumOfEl == 1 && NumOfPos == 1) return ElMagProc;

  // unknown process el->el+el??
  if (motherPDG == MC::ELECTRON && numOfDaug == 2 && NumOfEl == 2 && NumOfPos == 0) return ElMagProc;

  // unknown process pos->pos+pos??
  if (motherPDG == MC::POSITRON && numOfDaug == 2 && NumOfEl == 0 && NumOfPos == 2) return ElMagProc;

  // unknown process pos/el->pos/el??
  if (MC::isElectron(motherPDG) && !MC::isDecayed(mother) && motherPDG == thePriPart->pdgId() && numOfDaug == 1 && !samePart) return ElMagProc;

  // pi->pi+e+/e-; mu->mu+e+/e- ;
  // gamma+ atom->gamma(the same) + e (compton scattering)
  if (numOfDaug == 2 && (NumOfEl == 1 || NumOfPos == 1) && !MC::isElectron(motherPDG) && samePart) return ElMagProc;

  if ((motherPDG == MC::PI0 && numOfDaug == 3 && NumOfPhot == 1 && NumOfEl == 1 && NumOfPos == 1) ||
      (motherPDG == MC::PI0 && numOfDaug == 4 && NumOfPhot == 0 && NumOfEl == 2 && NumOfPos == 2))
    return DalitzDec;

  // Quark weak decay
  if (MC::isSMQuark(motherPDG) && numOfParents == 1 && numOfDaug == 3 && NumOfquark == 1 && NumOfElNeut == 1) return QuarkWeakDec;

  if (MC::isMuon(motherPDG) && NumOfNucFr != 0) return ElMagProc;

  if (MC::isTop(motherPDG)) return top;

  if (MC::isW(motherPDG) && mothOriVert != nullptr && mothOriVert->nIncomingParticles() != 0) {

    const xAOD::TruthVertex* prodVert = mothOriVert;
    const xAOD::TruthParticle* ptrPart;
    do {
      ptrPart = prodVert->incomingParticle(0);
      prodVert = ptrPart->hasProdVtx() ? ptrPart->prodVtx() : nullptr;
    } while (MC::isW(ptrPart) && prodVert != nullptr);

    if (prodVert && prodVert->nIncomingParticles() == 1) {
      if (abs(ptrPart->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (abs(ptrPart->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (abs(ptrPart->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(motherPDG)) return WBoson;
  if (MC::isZ(motherPDG)) return ZBoson;

  // MadGraphPythia ZWW*->lllnulnu
  if (numOfParents == 1 && numOfDaug > 4 && (MC::isSMQuark(motherPDG) || MC::isGluon(motherPDG))) {

    const xAOD::TruthParticle* thePartToCheck = thePriPart;
    const xAOD::TruthParticle* theMother = thePriPart->hasProdVtx() ? thePriPart->prodVtx()->incomingParticle(0) : nullptr;
    if (theMother != nullptr && MC::isElectron(theMother) && MC::isDecayed(theMother)) thePartToCheck = theMother;

    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;

    for (unsigned int ipOut = 0; ipOut + 1 < partOriVert->nOutgoingParticles(); ++ipOut) {
      const xAOD::TruthParticle* theDaug = partOriVert->outgoingParticle(ipOut);
      if (!theDaug) continue;
      const xAOD::TruthParticle* theNextDaug = nullptr;
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partOriVert->nOutgoingParticles(); ipOut1++) {
        theNextDaug = partOriVert->outgoingParticle(ipOut1);
        if (theNextDaug != nullptr) break;
      }
      if (!theNextDaug) continue;
      if (skipnext) {
        skipnext = false;
        continue;
      }

      if (MC::isElectron(theDaug) && MC::isElectron(theNextDaug)) {
        // Zboson
        if (thePartToCheck == theDaug || thePartToCheck == theNextDaug) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isElectron(theDaug) && abs(theNextDaug->pdgId()) == MC::NU_E) {
        // WBoson
        if (thePartToCheck == theDaug || thePartToCheck == theNextDaug) {
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
    const int pdg1 = partOriVert->incomingParticle(0)->pdgId();
    const int pdg2 = partOriVert->incomingParticle(1)->pdgId();
  //--Sherpa Z->ee
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && NumOfEl == 1 && NumOfPos == 1) return ZBoson;

  //--Sherpa W->enu ??
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && (NumOfEl == 1 || NumOfPos == 1) && NumOfElNeut == 1) return WBoson;

  //--Sherpa ZZ,ZW
  if ((numOfDaug - NumOfquark - NumOfgluon) == 4 && (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
      (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

  //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
  if ((numOfDaug - NumOfquark - NumOfgluon - NumOfPhot) == 6 && (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
      (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }

  // New Sherpa Z->ee
  if (partOriVert == mothOriVert && partOriVert != nullptr) {
    int NumOfEleLoop = 0;
    int NumOfLepLoop = 0;
    int NumOfEleNeuLoop = 0;
    for (const auto *const pout: partOriVert->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partOriVert->particles_in()) {
        if (!pin) continue;
        if (!HepMC::is_same_particle(pout,pin)) continue;
        if (MC::isElectron(pout)) NumOfEleLoop++;
        if (std::abs(pin->pdgId()) == MC::NU_E) NumOfEleNeuLoop++;
        if (MC::isSMLepton(pout)) NumOfLepLoop++;
      }
    }
    if (NumOfEleLoop == 2 && NumOfEleNeuLoop == 0) return ZBoson;
    if (NumOfEleLoop == 1 && NumOfEleNeuLoop == 1) return WBoson;
    if ((NumOfEleLoop == 4 && NumOfEleNeuLoop == 0) || (NumOfEleLoop == 3 && NumOfEleNeuLoop == 1) ||
        (NumOfEleLoop == 2 && NumOfEleNeuLoop == 2))
      return DiBoson;
    if (NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(motherPDG)) return Higgs;

  if (MC::isMSSMHiggs(motherPDG)) return HiggsMSSM; // MSSM Higgs bosons

  if (MC::isHeavyBoson(motherPDG)) return HeavyBoson;  // Heavy bosons( Z', Z'', W'+)

  if (MC::isMuon(motherPDG)) return Mu;
  if (MC::isTau(motherPDG)) {
    ParticleOrigin tauOrig = defOrigOfTau(xTruthParticleContainer, mother, motherPDG, info);
    ParticleType tautype = defTypeOfTau(tauOrig);
    return (tautype == IsoTau)?tauOrig:TauLep;
  }

  if (abs(motherPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isLeptoQuark(motherPDG) || NumOfLQ != 0) return LQ;
  if (MC::isSUSY(motherPDG)) return SUSY;
  if (MC::isBSM(motherPDG)) return OtherBSM;
  ParticleType pType = defTypeOfHadron(motherPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && mothOriVert != nullptr && MC::isHardScatteringVertex(mothOriVert)) isPrompt = true;
  return convHadronTypeToOrig(pType, motherPDG);
}


ParticleOrigin MCTruthClassifier::defOrigOfMuon(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                 const xAOD::TruthParticle* thePart,
                                 bool& isPrompt,
                                 MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfMuon ");

  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isMuon(thePriPart)) return NonDefined;

  const xAOD::TruthVertex* partOriVert = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;

  //-- to define muon  outcome status
  info.particleOutCome = defOutComeOfMuon(thePriPart);

  if (!partOriVert) return NonDefined;

  int numOfParents = partOriVert->nIncomingParticles();
  if (numOfParents > 1) ATH_MSG_DEBUG("DefOrigOfMuon:: muon  has more than one mother ");

  const xAOD::TruthParticle* mother = MC::findMother(thePriPart);
  info.setMotherProperties(mother);
  if (!mother) {
    return NonDefined;
  }

  const xAOD::TruthVertex* mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
  int motherPDG = mother->pdgId();

  if ((MC::isTau(motherPDG)|| MC::isW(motherPDG)) && mothOriVert != nullptr) {
    int pPDG(0);
    const xAOD::TruthParticle* MotherParent(nullptr);
    do {
      //
      pPDG = 0;
      //
      const xAOD::TruthVertex* mother_prdVtx(nullptr);
      const xAOD::TruthVertex* mother_endVtx(nullptr);
      MotherParent = MC::findMother(mother);
      // to prevent Sherpa loop
      mother_prdVtx = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
      mother_endVtx = mother->decayVtx();
      //
      const xAOD::TruthVertex* parent_prdVtx(nullptr);
      const xAOD::TruthVertex* parent_endVtx(nullptr);
      if (MotherParent) {
        parent_prdVtx = MotherParent->hasProdVtx() ? MotherParent->prodVtx() : nullptr;
        parent_endVtx = MotherParent->decayVtx();
      }
      //
      if (mother_endVtx == parent_prdVtx && mother_prdVtx == parent_endVtx) {
        MotherParent = mother;
        break;
      }
      //
      // to prevent Sherpa loop
      if (mother == MotherParent) {
        break;
      }

      if (MotherParent) {
        pPDG = MotherParent->pdgId();
        if (MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)) {
          mother = MotherParent;
          info.setMotherProperties(mother);
        }
      }
    } while ((MC::isMuon(pPDG) || MC::isTau(pPDG) || MC::isW(pPDG)));

    if (MC::isTau(pPDG) || MC::isW(pPDG) || MC::isZ(pPDG) || MC::isHiggs(pPDG) ||
        MC::isMSSMHiggs(pPDG) || MC::isHeavyBoson(pPDG) || MC::isTop(pPDG) || // MSSM Higgs bosons, Heavy bosons( Z', Z'', W'+)
        abs(pPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(pPDG) ||  // Left-right symmetric model WBoson || Right-handed neutrino (Pythia-specific)
        MC::isSUSY(pPDG)) {
      info.setMotherProperties(mother);
    }
  }

  motherPDG = mother->pdgId();
  mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
  partOriVert = mother->decayVtx();
  numOfParents = partOriVert->nIncomingParticles();
  int numOfDaug = partOriVert->nOutgoingParticles();

  info.setMotherProperties(mother);
  auto DP = DecayProducts(partOriVert);
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
  if (std::abs(motherPDG) == MC::PIPLUS && numOfDaug == 2 && NumOfMuNeut == 1) return PionDecay;
  if (std::abs(motherPDG) == MC::KPLUS && numOfDaug == 2 && NumOfMuNeut == 1) return KaonDecay;
  if (MC::isTau(motherPDG)) {
    ParticleOrigin tauOrig = defOrigOfTau(xTruthParticleContainer, mother, motherPDG, info);
    ParticleType tautype = defTypeOfTau(tauOrig);
    return  (tautype == IsoTau)?tauOrig:TauLep;
  }

  if (MC::isTop(motherPDG)) return top;
  // Quark weak decay
  if (MC::isSMQuark(motherPDG) && numOfParents == 1 && numOfDaug == 3 && NumOfquark == 1 && NumOfMuNeut == 1) return QuarkWeakDec;

  if (MC::isW(motherPDG) && mothOriVert != nullptr && mothOriVert->nIncomingParticles() != 0) {
    const xAOD::TruthVertex* prodVert = mothOriVert;
    const xAOD::TruthParticle* itrP;
    do {
      itrP = prodVert->incomingParticle(0);
      prodVert = itrP->hasProdVtx() ? itrP->prodVtx() : nullptr;
    } while (MC::isW(itrP) && prodVert != nullptr);

    if (prodVert && prodVert->nIncomingParticles() == 1) {
      if (abs(itrP->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (abs(itrP->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (abs(itrP->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(motherPDG)) return WBoson;
  if (MC::isZ(motherPDG)) return ZBoson;
  if (MC::isPhoton(motherPDG) && numOfDaug == 2 && NumOfMuMin == 1 && NumOfMuPl == 1) return PhotonConv;
  //-- Exotics

  // MadGraphPythia ZWW*->lllnulnu

  if (numOfParents == 1 && numOfDaug > 4 && (MC::isSMQuark(motherPDG) || MC::isGluon(motherPDG))) {
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;
    for (unsigned int ipOut = 0; ipOut + 1 < partOriVert->nOutgoingParticles(); ipOut++) {
      if (skipnext) {
        skipnext = false;
        continue;
      }
      const xAOD::TruthParticle* theDaug = partOriVert->outgoingParticle(ipOut);
      if (!theDaug) continue;
      const xAOD::TruthParticle* theNextDaug = nullptr;
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partOriVert->nOutgoingParticles(); ipOut1++) {
        theNextDaug = partOriVert->outgoingParticle(ipOut1);
        if (theNextDaug != nullptr) break;
      }
      if (!theNextDaug) continue;
      if (MC::isMuon(theDaug) && MC::isMuon(theNextDaug)) {
        // Zboson
        if (thePriPart == theDaug || thePriPart == theNextDaug) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isMuon(theDaug) && abs(theNextDaug->pdgId()) == MC::NU_MU) {
        // WBoson
        if (thePriPart == theDaug || thePriPart == theNextDaug) {
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
    const int pdg1 = partOriVert->incomingParticle(0)->pdgId();
    const int pdg2 = partOriVert->incomingParticle(1)->pdgId();
  //--Sherpa Z->mumu
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && NumOfMuPl == 1 && NumOfMuMin == 1) return ZBoson;

  //--Sherpa W->munu ??
  // if(numOfParents==2&&(numOfDaug-NumOfquark-NumOfgluon)==2&&(NumOfEl==1||NumOfPos==1)&&NumOfElNeut==1) return WBoson;
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && (NumOfMuPl == 1 || NumOfMuMin == 1) && NumOfMuNeut == 1) return WBoson;

  //--Sherpa ZZ,ZW
  if ((numOfDaug - NumOfquark - NumOfgluon) == 4 &&
      (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
      (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

  //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
  if ((numOfDaug - NumOfquark - NumOfgluon - NumOfPhot) == 6 &&
      (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
      (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
}

  //--New Sherpa Z->mumu
  if (partOriVert == mothOriVert) {
    int NumOfMuLoop = 0;
    int NumOfMuNeuLoop = 0;
    int NumOfLepLoop = 0;
    for (const auto & pout: partOriVert->particles_out()) {
      for (const auto & pin: partOriVert->particles_in()) {
        if (!pout) continue;
        if (!pin) continue;
        if (HepMC::is_same_particle(pout,pin)) {
          if (MC::isMuon(pout)) NumOfMuLoop++;
          if (std::abs(pout->pdg_id()) == MC::NU_MU) NumOfMuNeuLoop++;
          if (MC::isSMLepton(pout)) NumOfLepLoop++;
        }
      }
    }
    if (NumOfMuLoop == 2 && NumOfMuNeuLoop == 0) return ZBoson;
    if (NumOfMuLoop == 1 && NumOfMuNeuLoop == 1) return WBoson;
    if ((NumOfMuLoop == 4 && NumOfMuNeuLoop == 0) || (NumOfMuLoop == 3 && NumOfMuNeuLoop == 1) || (NumOfMuLoop == 2 && NumOfMuNeuLoop == 2)) return DiBoson;
    if (NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(motherPDG)) return Higgs;

  if (MC::isMSSMHiggs(motherPDG)) return HiggsMSSM; // MSSM Higgs bosons

  if (MC::isHeavyBoson(motherPDG))  return HeavyBoson;  // Heavy bosons( Z', Z'', W'+)

  if (abs(motherPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isLeptoQuark(motherPDG) || NumOfLQ != 0) return LQ;
  if (MC::isSUSY(motherPDG)) return SUSY;
  if (MC::isBSM(motherPDG)) return OtherBSM;

  ParticleType pType = defTypeOfHadron(motherPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && mothOriVert != nullptr && MC::isHardScatteringVertex(mothOriVert)) isPrompt = true;

  return convHadronTypeToOrig(pType, motherPDG);
}

ParticleOrigin MCTruthClassifier::defOrigOfTau(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                const xAOD::TruthParticle* thePart,
                                int motherPDG,
                                MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfTau ");
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isTau(thePriPart)) return NonDefined;

  const xAOD::TruthVertex* partOriVert = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;

  //-- to define tau  outcome status
  if (MC::isPhysical(thePriPart)) info.particleOutCome = defOutComeOfTau(thePriPart);

  if (!partOriVert) return NonDefined;

  int numOfParents = partOriVert->nIncomingParticles();
  if (numOfParents > 1) ATH_MSG_DEBUG("DefOrigOfTau:: tau  has more than one mother ");

  const xAOD::TruthParticle* mother = MC::findMother(thePriPart);
  info.setMotherProperties(mother);
  if (!mother) {
    return NonDefined;
  }

  const xAOD::TruthVertex* mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;

  const xAOD::TruthParticle* MotherParent(nullptr);

  if (MC::isW(motherPDG) && mothOriVert != nullptr) {
    MotherParent = MC::findMother(mother);
    int pPDG(0);

    if (MotherParent) {//MotherParent checked here...
      pPDG = MotherParent->pdgId();
      if (MC::isTop(pPDG)) {
        mother = MotherParent; //...so mother cannot be nullptr
        info.setMotherProperties(mother);
      }
    }
  }

  motherPDG = mother->pdgId();
  info.setMotherProperties(mother);
  mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
  partOriVert = mother->decayVtx();
  if (!partOriVert) return NonDefined;

  numOfParents = partOriVert->nIncomingParticles();
  auto DP = DecayProducts(partOriVert);
  const int numOfDaug = DP.size();
  const int NumOfPhot = DP.pd(MC::PHOTON);
  const int NumOfEl = DP.pd(MC::ELECTRON);
  const int NumOfPos = DP.pd(MC::POSITRON);
  const int NumOfElNeut = DP.apd(MC::NU_E);
  const int NumOfMuNeut = DP.apd(MC::NU_MU);
  /* const int NumOfLQ = DP.apd(MC::LEPTOQUARK); */
  const int NumOfquark = DP.apd({MC::DQUARK,MC::UQUARK,MC::SQUARK,MC::CQUARK,MC::BQUARK,MC::TQUARK});
  const int NumOfgluon = DP.apd(MC::GLUON);
  const int NumOfMuPl = DP.pd(-MC::MUON);
  const int NumOfMuMin = DP.pd(MC::MUON);
  const int NumOfTau = DP.apd(MC::TAU);
  const int NumOfTauNeut = DP.apd(MC::NU_TAU);

  if (MC::isTop(motherPDG)) return top;
  if (MC::isW(motherPDG) && mothOriVert != nullptr && mothOriVert->nIncomingParticles() != 0) {
    const xAOD::TruthVertex* prodVert = mothOriVert;
    const xAOD::TruthParticle* itrP;
    do {
      itrP = prodVert->incomingParticle(0);
      prodVert = itrP->hasProdVtx() ? itrP->prodVtx() : nullptr;
    } while (MC::isW(itrP) && prodVert != nullptr);

    if (prodVert && prodVert->nIncomingParticles() == 1 ) {
      if (abs(itrP->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (abs(itrP->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (abs(itrP->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(motherPDG)) { return WBoson;}
  if (MC::isZ(motherPDG)) { return ZBoson;}
  if (numOfParents == 1 && numOfDaug > 4 && (MC::isSMQuark(motherPDG) || MC::isGluon(motherPDG))) {
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;
    for (unsigned int ipOut = 0; ipOut + 1 < partOriVert->nOutgoingParticles(); ipOut++) {
      if (skipnext) {
        skipnext = false;
        continue;
      }
      const xAOD::TruthParticle* theDaug = partOriVert->outgoingParticle(ipOut);
      if (!theDaug) continue;
      const xAOD::TruthParticle* theNextDaug = nullptr;
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partOriVert->nOutgoingParticles(); ipOut1++) {
        theNextDaug = partOriVert->outgoingParticle(ipOut1);
        if (theNextDaug != nullptr) break;
      }
      if (!theNextDaug) {
        continue;
      }
      if (MC::isTau(theDaug) && MC::isTau(theNextDaug)) {
        // Zboson
        if (thePriPart == theDaug || thePriPart == theNextDaug) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isTau(theDaug) && abs(theNextDaug->pdgId()) == MC::NU_TAU) {
        // WBoson
        if (thePriPart == theDaug || thePriPart == theNextDaug) {
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
  const int pdg1 = partOriVert->incomingParticle(0)->pdgId();
  const int pdg2 = partOriVert->incomingParticle(1)->pdgId();
  //--Sherpa Z->tautau
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && NumOfTau == 2  && (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return ZBoson;

  //--Sherpa W->taunu  new
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && NumOfTau == 1 && NumOfTauNeut == 1) return WBoson;

  //--Sherpa ZZ,ZW
  if ((numOfDaug - NumOfquark - NumOfgluon) == 4 &&
      (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
      (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

  //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
  if ((numOfDaug - NumOfquark - NumOfgluon - NumOfPhot) == 6 &&
      (NumOfEl + NumOfPos + NumOfMuPl + NumOfMuMin + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
      (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }
  // New Sherpa Z->tautau
  if (partOriVert == mothOriVert) {
    int NumOfTauLoop = 0;
    int NumOfTauNeuLoop = 0;
    int NumOfLepLoop = 0;
    for ( const auto *const pout: partOriVert->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partOriVert->particles_in()) {
        if (!pin) continue;
        if (!HepMC::is_same_particle(pout,pin)) continue;
        if (MC::isTau(pout)) NumOfTauLoop++;
        if (std::abs(pout->pdgId()) == MC::NU_TAU) NumOfTauNeuLoop++;
        if (MC::isSMLepton(pout)) NumOfLepLoop++;
      }
    }
    if (NumOfTauLoop == 2 && NumOfTauNeuLoop == 0) return ZBoson;
    if (NumOfTauLoop == 1 && NumOfTauNeuLoop == 1) return WBoson;
    if ((NumOfTauLoop == 4 && NumOfTauNeuLoop == 0) || (NumOfTauLoop == 3 && NumOfTauNeuLoop == 1) || (NumOfTauLoop == 2 && NumOfTauNeuLoop == 2)) return DiBoson;
    if (NumOfLepLoop == 4) return DiBoson;
  }

  if (MC::isHiggs(motherPDG)) return Higgs;
  if (MC::isMSSMHiggs(motherPDG)) return HiggsMSSM; // MSSM Higgs bosons
  if (MC::isHeavyBoson(motherPDG)) return HeavyBoson; // Heavy bosons( Z', Z'', W'+)
  if (abs(motherPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isSUSY(motherPDG)) return SUSY;
  if (MC::isBSM(motherPDG)) return OtherBSM;
  if (abs(motherPDG) == MC::JPSI) return JPsi;

  ParticleType pType = defTypeOfHadron(motherPDG);
  return convHadronTypeToOrig(pType, motherPDG);
}

ParticleOrigin MCTruthClassifier::defOrigOfPhoton(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                   const xAOD::TruthParticle* thePart,
                                   bool& isPrompt,
                                   MCTruthPartClassifier::Info& info) const
{
  if (!thePart) return NonDefined;
  ATH_MSG_DEBUG("Executing DefOrigOfPhoton ");

  info.resetMotherProperties();
  info.photonMother = nullptr;

  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (!MC::isPhoton(thePriPart)) return NonDefined;

  const xAOD::TruthVertex* partOriVert = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;

  //-- to define photon outcome status
  info.particleOutCome = defOutComeOfPhoton(thePriPart);
  if (!partOriVert) return NonDefined;

  int numOfParents = partOriVert->nIncomingParticles();
  if (partOriVert->nIncomingParticles() > 1) ATH_MSG_DEBUG("DefOrigOfPhoton:: photon  has more than one mother ");


  const xAOD::TruthParticle* mother = MC::findMother(thePriPart);
  info.setMotherProperties(mother);
  if (!mother) return NonDefined;
  int motherPDG = mother->pdgId();
  const xAOD::TruthVertex* mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
  info.setMotherProperties(mother);
  partOriVert = mother->decayVtx();
  numOfParents = partOriVert->nIncomingParticles();
  int numOfDaug = partOriVert->nOutgoingParticles();
  int NumOfNucFr(0);
  int NumOfEl(0);
  int NumOfPos(0);
  int NumOfMu(0);
  int NumOfTau(0);
  int NumOfPht(0);
  int NumOfLQ(0);
  int DaugType(0);
  long NumOfLep(0);
  long NumOfNeut(0);
  long NumOfPartons(0);
  const xAOD::TruthParticle* Daug = nullptr;
  for (const auto& pout: partOriVert->particles_out()) {
    if (!pout) continue;
    DaugType = pout->pdg_id();
    if (numOfParents == 1 && (MC::isPhoton(motherPDG) || MC::isElectron(motherPDG) || abs(motherPDG) == MC::PIPLUS) &&
        (MC::isNucleus(DaugType) || DaugType == 0 || DaugType == MC::PROTON || DaugType == MC::NEUTRON))
      NumOfNucFr++;
    if (DaugType == MC::PHOTON) NumOfPht++;
    else if (DaugType == MC::ELECTRON) NumOfEl++;
    else if (DaugType == MC::POSITRON) NumOfPos++;
    else if (MC::isMuon(DaugType)) NumOfMu++;
    else if (MC::isTau(DaugType)) NumOfTau++;
    else if (MC::isLeptoQuark(DaugType)) NumOfLQ++;
    else if (MC::isElectron(DaugType) || MC::isMuon(DaugType) || MC::isTau(DaugType)) NumOfLep++;
    else if (abs(DaugType) == MC::NU_E || abs(DaugType) == MC::NU_MU || abs(DaugType) == MC::NU_TAU) NumOfNeut++;
    if (abs(DaugType) < MC::ELECTRON || (abs(DaugType) > MC::NU_TAU && abs(DaugType) < 43 && !MC::isPhoton(DaugType))) NumOfPartons++;  // FIXME Too loose? This definition picks up 4th generation quarks and leptons as well as all gauge bosons and leptoquarks.
    if (DaugType == motherPDG) {
      Daug = pout;
     }
  }

  bool foundISR = false;
  bool foundFSR = false;
  if (numOfParents == 1 && numOfDaug == 2 &&  Daug && info.Mother() && HepMC::is_same_generator_particle(Daug, info.Mother())) return BremPhot;
  if (numOfParents == 1 && numOfDaug == 2 && MC::isElectron(motherPDG) && NumOfPht == 2) return ElMagProc;

  // decay of W,Z and Higgs to lepton with FSR generated by Pythia
  if (numOfParents == 1 && numOfDaug == 2 && (MC::isElectron(motherPDG) || MC::isMuon(motherPDG) || MC::isTau(motherPDG)) &&
      !(Daug && info.Mother() && HepMC::is_same_generator_particle(Daug, info.Mother())) && mothOriVert != nullptr &&
      mothOriVert->nIncomingParticles() == 1) {
    int itr = 0;
    int PartPDG = 0;
    const xAOD::TruthVertex* prodVert = mothOriVert;
    const xAOD::TruthVertex* Vert = nullptr;
    do {
      Vert = prodVert;
      for (const auto & pin: Vert->particles_in()) {
        if (!pin) continue;
        PartPDG = abs(pin->pdgId());
        prodVert = pin->prodVtx();
        if (MC::isZ(PartPDG) || MC::isW(PartPDG) || MC::isHiggs(PartPDG)) foundFSR = true;
      }
      itr++;
      if (itr > 100) { // FIXME Improve loop detection here?
        ATH_MSG_WARNING("DefOrigOfPhoton:: infinite while");
        break;
      }
    } while (prodVert != nullptr && abs(motherPDG) == PartPDG);

    if (foundFSR) return FSRPhot;
  }

  // Nucl reaction
  // gamma+Nuclear=>gamma+Nucl.Fr+Nuclons+pions
  // e+Nuclear=>e+gamma+Nucl.Fr+Nuclons+pions
  // pi+Nuclear=>gamma+Nucl.Fr+Nuclons+pions

  if ((numOfParents == 1 && (MC::isPhoton(motherPDG) || MC::isElectron(motherPDG)) && numOfDaug > 2 && NumOfNucFr != 0) ||
      (numOfParents == 1 && abs(motherPDG) == MC::PIPLUS && numOfDaug > 10 && NumOfNucFr != 0) ||
      (numOfParents == 1 && MC::isPhoton(motherPDG) && numOfDaug > 10 && MC::isStable(mother)) ||
      (numOfParents == 1 && MC::isNucleus(motherPDG) && std::abs(motherPDG) != MC::PROTON)) // FIXME vetoing protons here to preserve previous behaviour
    return NucReact;

  if (MC::isMuon(motherPDG) && NumOfMu == 0) return Mu;
  if (MC::isTau(motherPDG) && NumOfTau == 0) return TauLep;

  if (numOfParents == 1 && mother && mother->status() == 3) return (foundISR)? ISRPhot:UndrPhot;

  //-- to find initial and final state raiation and underline photons
  //-- SUSY
  if (numOfParents == 1 && (MC::isSMQuark(motherPDG) || MC::isGluon(motherPDG)) &&
      (numOfDaug != NumOfPht + NumOfPartons || !MC::Pythia8::isConditionA(mother))) {
    for (const auto& pout: partOriVert->particles_out()) {
      if (!pout) continue;
      if (motherPDG != pout->pdgId()) continue;
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
  if (numOfParents == 2 && ((MC::isElectron(motherPDG) && NumOfEl == 1 && NumOfPos == 1) || (MC::isMuon(motherPDG) && NumOfMu == 2) || (MC::isTau(motherPDG) && NumOfTau == 2))) {
    if (abs(partOriVert->incomingParticle(0)->pdgId()) == abs(partOriVert->incomingParticle(1)->pdgId())) return FSRPhot;
  }

  if (numOfParents == 2 && NumOfLep == 1 && NumOfNeut == 1 && (MC::isElectron(motherPDG) || abs(motherPDG) == MC::NU_E)) return FSRPhot;

  //-- Exotics - CompHep
  if (MC::isElectron(motherPDG) && numOfParents == 1 && numOfDaug == 2 && (NumOfEl == 1 || NumOfPos == 1) && NumOfPht == 1 &&
      !( Daug && info.Mother() && HepMC::is_same_generator_particle(Daug, info.Mother())) && !HepMC::is_simulation_particle(Daug) && !HepMC::is_simulation_particle(info.Mother()))
    return FSRPhot;

  // FSR  from Photos
  if (MC::isZ(motherPDG) && ((NumOfEl + NumOfPos == 2 || NumOfEl + NumOfPos == 4) || (NumOfMu == 2 || NumOfMu == 4) || (NumOfTau == 2 || NumOfTau == 4)) && NumOfPht > 0) return FSRPhot;

  if (NumOfPht > 0 && (abs(motherPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(motherPDG))) return FSRPhot; // Left-right symmetric model WBoson || Right-handed neutrinos (Pythia-specific)

  if (numOfParents == 2 && NumOfLQ == 1) return FSRPhot;

  //--- other process

  if (MC::isZ(motherPDG)) return ZBoson;
  if (MC::isW(motherPDG)) {

    if (NumOfLep == 1 && NumOfNeut == 1 && numOfDaug == NumOfLep + NumOfNeut + NumOfPht) return FSRPhot;

    if (mothOriVert != nullptr && mothOriVert->nIncomingParticles() != 0) {
      const xAOD::TruthVertex* prodVert = mothOriVert;
      const xAOD::TruthParticle* itrP;
      do {
        itrP = prodVert->incomingParticle(0);
        prodVert = itrP->hasProdVtx() ? itrP->prodVtx() : nullptr;
      } while (MC::isW(itrP) && prodVert != nullptr);

      if (prodVert && prodVert->nIncomingParticles() == 1 ) {
        if ( MC::isTau(itrP)) return TauLep;
        if ( MC::isMuon(itrP)) return Mu;
        if ( abs(itrP->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
        if ( abs(itrP->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
        if ( abs(itrP->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
      }
    } else
      return WBoson;
  }

  // MadGraphPythia ZWW*->lllnulnu
  if (numOfParents == 1 && numOfDaug > 4 && (MC::isSMQuark(motherPDG) || MC::isGluon(motherPDG))) {
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;
    for (unsigned int ipOut = 0; ipOut + 1 < partOriVert->nOutgoingParticles(); ipOut++) {
      if (skipnext) {
        skipnext = false;
        continue;
      }
      const xAOD::TruthParticle* theDaug = partOriVert->outgoingParticle(ipOut);
      if (!theDaug) continue;
      const xAOD::TruthParticle* theNextDaug = nullptr;
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partOriVert->nOutgoingParticles(); ipOut1++) {
        theNextDaug = partOriVert->outgoingParticle(ipOut1);
        if (theNextDaug != nullptr) break;
      }
      if (!theNextDaug) continue;
      if (MC::isTau(theDaug) && MC::isTau(theNextDaug)) {
        // Zboson
        if (thePriPart == theDaug || thePriPart == theNextDaug) {
          isZboson = true;
          break;
        }
        skipnext = true;
      } else if (MC::isTau(theDaug) && abs(theNextDaug->pdgId()) == MC::NU_TAU) {
        // WBoson
        if (thePriPart == theDaug || thePriPart == theNextDaug) {
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
  if (numOfParents == 4 && (numOfDaug - NumOfPht) == 4 && (NumOfLep + NumOfNeut == 4)) {
    if (MC::isSMLepton(partOriVert->incomingParticle(0))&&MC::isSMLepton(partOriVert->incomingParticle(1))
     && MC::isSMLepton(partOriVert->incomingParticle(2))&&MC::isSMLepton(partOriVert->incomingParticle(3)))
      return FSRPhot;
  }

  //--New Sherpa single photon
  if (partOriVert == mothOriVert && partOriVert != nullptr) {
    int NumOfPhtLoop = 0;
    for (const auto *const pout: partOriVert->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partOriVert->particles_in()) {
        if (!pin) continue;
        if (HepMC::is_same_particle(pout,pin) && MC::isPhoton(pout)) NumOfPhtLoop++;
        if (NumOfPhtLoop == 1) return SinglePhot;
      }
    }
  }

  if (MC::isHiggs(motherPDG)) return Higgs;
  if (abs(motherPDG) == MC::PI0) return PiZero;
  if (MC::isMSSMHiggs(motherPDG)) return HiggsMSSM; // MSSM Higgs bosons
  if (MC::isHeavyBoson(motherPDG) || std::abs(motherPDG) == 5100039 ) return HeavyBoson; // Heavy Bosons (Z' Z'' W'+) + KK excited graviton

  if (MC::isSUSY(motherPDG)) return SUSY;
  if (MC::isBSM(motherPDG)) return OtherBSM;

  // Pythia8 gamma+jet samples
  if (MC::Pythia8::isConditionA(mother) && MC::isStable(thePriPart) && NumOfPht == 1 && numOfDaug == (NumOfPht + NumOfPartons))  return PromptPhot;

  ParticleType pType = defTypeOfHadron(motherPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && mothOriVert != nullptr && MC::isHardScatteringVertex(mothOriVert)) isPrompt = true;
  return convHadronTypeToOrig(pType, motherPDG);
}

ParticleOrigin
MCTruthClassifier::defOrigOfNeutrino(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                     const xAOD::TruthParticle* thePart,
                                     bool& isPrompt,
                                     MCTruthPartClassifier::Info& info) const
{
  ATH_MSG_DEBUG("Executing DefOrigOfNeutrino ");

  const int nuFlav = abs(thePart->pdgId());
  const xAOD::TruthParticle* thePriPart = MC::findMatching(xTruthParticleContainer, thePart);
  if (!thePriPart) return NonDefined;
  if (abs(thePriPart->pdgId()) != nuFlav) return NonDefined;

  const xAOD::TruthVertex* partOriVert = thePriPart->hasProdVtx() ? thePriPart->prodVtx() : nullptr;

  //-- to define neutrino outcome status
  info.particleOutCome = NonInteract;

  if (!partOriVert) return NonDefined;

  int numOfParents = -1;
  numOfParents = partOriVert->nIncomingParticles();
  if (numOfParents > 1) ATH_MSG_DEBUG("DefOrigOfNeutrino:: neutrino  has more than one mother ");

  const xAOD::TruthParticle* mother = MC::findMother(thePriPart);
  info.mother = mother; // FIXME why isn't info.setMotherProperties(mother) used here??
  if (!mother) return NonDefined;
  int motherPDG = mother->pdgId();
  const xAOD::TruthVertex* mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;

  // to resolve Sherpa loop
  bool samePart = false;
  if (mothOriVert && HepMC::is_same_vertex(mothOriVert,partOriVert)) samePart = true;
  //
  if ((abs(motherPDG) == nuFlav || MC::isTau(motherPDG) || MC::isW(motherPDG)) && mothOriVert != nullptr &&
      !samePart) {
    int pPDG(0);
    const xAOD::TruthParticle* MotherParent(nullptr);
    do {
      pPDG = 0;
      MotherParent = MC::findMother(mother);
      // to prevent Sherpa loop
      const xAOD::TruthVertex* mother_prdVtx(nullptr);
      const xAOD::TruthVertex* mother_endVtx(nullptr);
      if (mother) {
        mother_prdVtx = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
        mother_endVtx = mother->decayVtx();
      }
      const xAOD::TruthVertex* parent_prdVtx(nullptr);
      const xAOD::TruthVertex* parent_endVtx(nullptr);
      if (MotherParent) {
        parent_prdVtx = MotherParent->hasProdVtx() ? MotherParent->prodVtx() : nullptr;
        parent_endVtx = MotherParent->decayVtx();
      }
      if (mother_endVtx == parent_prdVtx && mother_prdVtx == parent_endVtx) {
        MotherParent = mother;
        break;
      }
      //
      if (MotherParent) {
        pPDG = MotherParent->pdgId();
      }
      // to prevent Sherpa loop
      if (mother == MotherParent) {
        break;
      }
      if (abs(pPDG) == nuFlav || MC::isTau(pPDG) || MC::isW(pPDG) ) {
        mother = MotherParent;
        info.setMotherProperties(mother);
      }

    } while ((std::abs(pPDG) == nuFlav || MC::isTau(pPDG) || MC::isW(pPDG)));

    if (std::abs(pPDG) == nuFlav || MC::isTau(pPDG) || MC::isW(pPDG) || MC::isZ(pPDG) || MC::isHiggs(pPDG) ||
        MC::isMSSMHiggs(pPDG) || MC::isHeavyBoson(pPDG) || MC::isTop(pPDG) ||  // MSSM Higgs bosons, Heavy bosons( Z', Z'', W'+)
        std::abs(pPDG) == MC::WBOSON_LRSM || MC::isNeutrinoRH(pPDG) || // Left-right symmetric model WBoson || Right-handed neutrino (Pythia-specific)
        MC::isSUSY(pPDG)) {
      mother = MotherParent;
      info.setMotherProperties(mother);
    }
  }
  //if mother is still nullptr, we have a problem
  if (!mother) return NonDefined;

  motherPDG = mother->pdgId();
  partOriVert = mother->decayVtx();
  mothOriVert = mother->hasProdVtx() ? mother->prodVtx() : nullptr;
  numOfParents = partOriVert->nIncomingParticles();
  int numOfDaug = partOriVert->nOutgoingParticles();

  info.setMotherProperties(mother);

  int NumOfPhot(0);
  int NumOfquark(0);
  int NumOfgluon(0);
  int NumOfLQ(0);
  int NumOfElNeut(0);
  int NumOfMuNeut(0);
  int NumOfTauNeut(0);
  int NumOfEl(0);
  int NumOfMu(0);
  int NumOfTau(0);
  samePart = false;

  for (const auto& theDaug: partOriVert->particles_out()) {
    if (!theDaug) continue;
    int DaugType = theDaug->pdgId();
    if (MC::isSMQuark(DaugType)) NumOfquark++;
    else if (MC::isGluon(DaugType)) NumOfgluon++;
    else if (abs(DaugType) == MC::NU_E) NumOfElNeut++;
    else if (std::abs(DaugType) == MC::NU_MU) NumOfMuNeut++;
    else if (std::abs(DaugType) == MC::NU_TAU) NumOfTauNeut++;
    else if (MC::isPhoton(DaugType)) NumOfPhot++;
    else if (MC::isElectron(DaugType)) NumOfEl++;
    else if (MC::isMuon(DaugType)) NumOfMu++;
    else if (MC::isTau(DaugType)) NumOfTau++;
    else if (MC::isLeptoQuark(DaugType)) NumOfLQ++;
    if (std::abs(DaugType) == std::abs(motherPDG) && theDaug && info.Mother() && HepMC::is_same_generator_particle(theDaug,info.Mother())) samePart = true;
  }

  // Quark weak decay
  if (MC::isQuark(motherPDG) && numOfParents == 1 && numOfDaug == 3 && NumOfquark == 1 && (NumOfEl == 1 || NumOfMu == 1 || NumOfTau == 1)) return QuarkWeakDec;
  if (MC::isTop(motherPDG)) return top;

  if (MC::isW(motherPDG) && mothOriVert != nullptr && mothOriVert->nIncomingParticles() != 0) {
    const xAOD::TruthVertex* prodVert = mothOriVert;
    const xAOD::TruthParticle* ptrPart;
    do {
      ptrPart = prodVert->incomingParticle(0);
      prodVert = ptrPart->hasProdVtx() ? ptrPart->prodVtx() : nullptr;
    } while (MC::isW(ptrPart) && prodVert != nullptr);

    if (prodVert && prodVert->nIncomingParticles() == 1) {
      if (abs(ptrPart->pdgId()) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
      if (abs(ptrPart->pdgId()) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
      if (abs(ptrPart->pdgId()) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
    }
    return WBoson;
  }
  if (MC::isW(motherPDG)) return WBoson;
  if (MC::isZ(motherPDG)) return ZBoson;

  //-- Exotics

  // MadGraphPythia ZWW*->lllnulnu or ZWW*->nunulnulnu (don't even know if the latter is generated)
  if (numOfParents == 1 && numOfDaug > 4 && (MC::isSMQuark(motherPDG) || MC::isGluon(motherPDG))) {

    const xAOD::TruthParticle* thePartToCheck = thePriPart;
    const xAOD::TruthParticle* theMother = thePriPart->hasProdVtx() ? thePriPart->prodVtx()->incomingParticle(0) : nullptr;

    if (MC::isElectron(theMother) && MC::isDecayed(theMother)) thePartToCheck = theMother;
    bool isZboson = false;
    bool isWboson = false;
    bool skipnext = false;

    for (unsigned int ipOut = 0; ipOut + 1 < partOriVert->nOutgoingParticles(); ++ipOut) {
      const xAOD::TruthParticle* theDaug = partOriVert->outgoingParticle(ipOut);
      if (!theDaug) continue;
      const xAOD::TruthParticle* theNextDaug = nullptr;
      for (unsigned int ipOut1 = ipOut + 1; ipOut1 < partOriVert->nOutgoingParticles(); ipOut1++) {
        theNextDaug = partOriVert->outgoingParticle(ipOut1);
        if (theNextDaug != nullptr) break;
      }
      if (!theNextDaug) continue;

      if (skipnext) {
        skipnext = false;
        continue;
      }

      const int apdgID1 = abs(theDaug->pdgId());
      const int apdgID2 = abs(theNextDaug->pdgId());
      if (apdgID1 == apdgID2 && MC::isSMNeutrino(apdgID1)) {
        // Zboson
        if (thePartToCheck == theDaug || thePartToCheck == theNextDaug) {
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
        if (thePartToCheck == theDaug || thePartToCheck == theNextDaug) {
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
    int pdg1 = partOriVert->incomingParticle(0)->pdgId();
    int pdg2 = partOriVert->incomingParticle(1)->pdgId();
  //--Sherpa Z->nunu
    if ( (numOfDaug - NumOfquark - NumOfgluon) == 2 && (NumOfElNeut == 2 || NumOfMuNeut == 2 || NumOfTauNeut == 2)) return ZBoson;

  //--Sherpa W->enu ??
  if ((numOfDaug - NumOfquark - NumOfgluon) == 2 && ((NumOfEl == 1 && NumOfElNeut == 1) || (NumOfMu == 1 && NumOfMuNeut == 1) || (NumOfTau == 1 && NumOfTauNeut == 1))) return WBoson;

  //--Sherpa ZZ,ZW
  if ( (numOfDaug - NumOfquark - NumOfgluon) == 4 && (NumOfEl + NumOfMu + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 4) &&
       (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return DiBoson;

  //--Sherpa VVV -- Note, have to allow for prompt photon radiation or these get lost
  if ((numOfDaug - NumOfquark - NumOfgluon - NumOfPhot) == 6 && (NumOfEl + NumOfMu + NumOfTau + NumOfElNeut + NumOfMuNeut + NumOfTauNeut == 6) &&
     (MC::isQuark(pdg1)||MC::isGluon(pdg1)) && (MC::isQuark(pdg2)||MC::isGluon(pdg2))) return MultiBoson;
  }

  // New Sherpa Z->nunu
  if (partOriVert == mothOriVert && partOriVert != nullptr) {
    int NumOfLepLoop = 0;
    int NumOfNeuLoop = 0;
    for (const auto *const pout: partOriVert->particles_out()) {
      if (!pout) continue;
      for (const auto *const pin: partOriVert->particles_in()) {
        if (!pin) continue;
        if (HepMC::is_same_particle(pin,pout)) continue;
        const int apdgid = abs(pout->pdgId());
        if (MC::isSMLepton(apdgid)) {
          if (MC::isSMNeutrino(apdgid)) { NumOfNeuLoop++; }
          else { NumOfLepLoop++; }
        }
      }
    }
    if (NumOfNeuLoop == 2 && NumOfLepLoop == 0) return ZBoson;
    if (NumOfNeuLoop == 1 && NumOfLepLoop == 1) return WBoson;
    if (NumOfNeuLoop + NumOfLepLoop == 4) return DiBoson;
  }

  //-- McAtNLo

  if (MC::isHiggs(motherPDG)) return Higgs;
  if (MC::isMSSMHiggs(motherPDG)) return HiggsMSSM; // MSSM Higgs bosons
  if (MC::isHeavyBoson(motherPDG)) return HeavyBoson; // Heavy bosons( Z', Z'', W'+)

  if (MC::isTau(motherPDG)) {
    ParticleOrigin tauOrig = defOrigOfTau(xTruthParticleContainer, mother, motherPDG, info);
    ParticleType tautype = defTypeOfTau(tauOrig);
    return (tautype == IsoTau)?tauOrig:TauLep;
  }

  if (abs(motherPDG) == MC::WBOSON_LRSM) return WBosonLRSM; // Left-right symmetric model WBoson (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_E) return NuREle; // Right-handed NU_E (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_MU) return NuRMu; // Right-handed NU_MU (Pythia-specific)
  if (abs(motherPDG) == MC::RH_NU_TAU) return NuRTau; // Right-handed NU_TAU (Pythia-specific)
  if (MC::isLeptoQuark(motherPDG) || NumOfLQ != 0) return LQ;
  if (MC::isSUSY(motherPDG)) return SUSY;
  if (MC::isBSM(motherPDG)) return OtherBSM;

  ParticleType pType = defTypeOfHadron(motherPDG);
  if ((pType == BBbarMesonPart || pType == CCbarMesonPart) && mothOriVert != nullptr && MC::isHardScatteringVertex(mothOriVert)) isPrompt = true;

  return convHadronTypeToOrig(pType, motherPDG);
}
