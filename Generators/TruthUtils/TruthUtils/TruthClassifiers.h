/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_TRUTHCLASSIFIERS_H
#define TRUTHUTILS_TRUTHCLASSIFIERS_H
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/DecayProducts.h"
#include <utility>
#include <bitset>
#include <vector>
#include <string>
namespace MCTruthPartClassifier {
#include "TruthUtils/TruthClasses.h"

inline ParticleOrigin convHadronTypeToOrig(ParticleType pType, int motherPDG)
{
  if (pType == CCbarMesonPart && abs(motherPDG) == MC::JPSI) return JPsi;
  if (pType == BBbarMesonPart) return BBbarMeson;
  if (pType == BottomMesonPart) return BottomMeson;
  if (pType == BottomBaryonPart) return BottomBaryon;
  if (pType == CCbarMesonPart) return CCbarMeson;
  if (pType == CharmedMesonPart) return CharmedMeson;
  if (pType == CharmedBaryonPart) return CharmedBaryon;
  if (pType == StrangeBaryonPart) return StrangeBaryon;
  if (pType == StrangeMesonPart) return StrangeMeson;
  if (pType == LightBaryonPart) return LightBaryon;
  if (pType == LightMesonPart) return LightMeson;
  return NonDefined;
}

inline ParticleOrigin defHadronType(int pdg) {
  if (abs(pdg) == MC::JPSI) return JPsi;
  if (MC::isBBbarMeson(pdg)) return BBbarMeson;
  if (MC::isCCbarMeson(pdg)) return CCbarMeson;
  if (MC::isBottomMeson(pdg)) return BottomMeson;
  if (MC::isCharmMeson(pdg)) return CharmedMeson;
  if (MC::isBottomBaryon(pdg)) return BottomBaryon;
  if (MC::isCharmBaryon(pdg)) return CharmedBaryon;
  if (MC::isStrangeBaryon(pdg)) return StrangeBaryon;
  if (MC::isLightBaryon(pdg)) return LightBaryon;
  if (MC::isStrangeMeson(pdg)) return StrangeMeson;
  if (MC::isLightMeson(pdg)) return LightMeson;
  return NonDefined;
}

inline ParticleType defTypeOfHadron(int pdg) {
  if (MC::isBBbarMeson(pdg)) return BBbarMesonPart;
  if (MC::isCCbarMeson(pdg)) return CCbarMesonPart;
  if (MC::isBottomMeson(pdg)) return BottomMesonPart;
  if (MC::isCharmMeson(pdg)) return CharmedMesonPart;
  if (MC::isBottomBaryon(pdg)) return BottomBaryonPart;
  if (MC::isCharmBaryon(pdg)) return CharmedBaryonPart;
  if (MC::isStrangeBaryon(pdg)) return StrangeBaryonPart;
  if (MC::isLightBaryon(pdg)) return LightBaryonPart;
  if (MC::isStrangeMeson(pdg)) return StrangeMesonPart;
  if (MC::isLightMeson(pdg)) return LightMesonPart;
  return Unknown;
}


inline ParticleType defTypeOfElectron(ParticleOrigin EleOrig, bool isPrompt) {

  if (EleOrig == NonDefined)
    return UnknownElectron;

  if (EleOrig == WBoson || EleOrig == ZBoson || EleOrig == top || EleOrig == SingleElec || EleOrig == Higgs ||
      EleOrig == HiggsMSSM || EleOrig == HeavyBoson || EleOrig == WBosonLRSM || EleOrig == NuREle || EleOrig == NuRMu ||
      EleOrig == NuRTau || EleOrig == LQ || EleOrig == SUSY || EleOrig == DiBoson || EleOrig == ZorHeavyBoson ||
      EleOrig == OtherBSM || EleOrig == MultiBoson || isPrompt) {
    return IsoElectron;
  }
  if (EleOrig == JPsi || EleOrig == BottomMeson || EleOrig == CharmedMeson || EleOrig == BottomBaryon ||
      EleOrig == CharmedBaryon || EleOrig == TauLep || EleOrig == Mu || EleOrig == QuarkWeakDec) {
    return NonIsoElectron;
  }
  return BkgElectron;
}


inline ParticleType defTypeOfMuon(ParticleOrigin MuOrig, bool isPrompt) {

  if (MuOrig == NonDefined) return UnknownMuon;

  if (MuOrig == WBoson || MuOrig == ZBoson || MuOrig == top || MuOrig == SingleMuon || MuOrig == Higgs ||
      MuOrig == HiggsMSSM || MuOrig == HeavyBoson || MuOrig == WBosonLRSM || MuOrig == NuREle || MuOrig == NuRMu ||
      MuOrig == NuRTau || MuOrig == LQ || MuOrig == SUSY || MuOrig == DiBoson || MuOrig == ZorHeavyBoson ||
      MuOrig == OtherBSM || MuOrig == MultiBoson || isPrompt) {
    return IsoMuon;
  }
  if (MuOrig == JPsi || MuOrig == BottomMeson || MuOrig == CharmedMeson || MuOrig == BottomBaryon ||
      MuOrig == CharmedBaryon || MuOrig == TauLep || MuOrig == QuarkWeakDec) {
    return NonIsoMuon;
  }
  //  if (MuOrig == Pion  || MuOrig == Kaon ) return  DecayMuon;
  return BkgMuon;
}

inline ParticleType defTypeOfTau(ParticleOrigin TauOrig) {
  if (TauOrig == NonDefined) return UnknownTau;

  if (TauOrig == WBoson || TauOrig == ZBoson || TauOrig == top || TauOrig == SingleMuon || TauOrig == Higgs ||
      TauOrig == HiggsMSSM || TauOrig == HeavyBoson || TauOrig == WBosonLRSM || TauOrig == NuREle || TauOrig == NuRMu ||
      TauOrig == NuRTau || TauOrig == SUSY || TauOrig == DiBoson || TauOrig == ZorHeavyBoson || TauOrig == OtherBSM ||
      TauOrig == MultiBoson)
    return IsoTau;

  if (TauOrig == JPsi || TauOrig == BottomMeson || TauOrig == CharmedMeson || TauOrig == BottomBaryon ||
      TauOrig == CharmedBaryon || TauOrig == QuarkWeakDec)
    return NonIsoTau;

  return BkgTau;
}


inline ParticleType defTypeOfPhoton(ParticleOrigin PhotOrig) 
{
  if (PhotOrig == NonDefined) return UnknownPhoton;

  if (PhotOrig == WBoson || PhotOrig == ZBoson || PhotOrig == SinglePhot || PhotOrig == Higgs ||
      PhotOrig == HiggsMSSM || PhotOrig == HeavyBoson || PhotOrig == PromptPhot || PhotOrig == SUSY ||
      PhotOrig == OtherBSM)
    return IsoPhoton;

  if (PhotOrig == ISRPhot || PhotOrig == FSRPhot || PhotOrig == TauLep || PhotOrig == Mu || PhotOrig == NuREle ||
      PhotOrig == NuRMu || PhotOrig == NuRTau)
    return NonIsoPhoton;

  return BkgPhoton;
}

template <class T> ParticleOrigin defJetOrig(const T& allJetMothers) {
  ParticleOrigin partOrig = NonDefined;
  for (const auto& it: allJetMothers) {
    int pdg = abs(it->pdg_id());
    if (MC::isTop(pdg)) partOrig = top;
    if (MC::isZ(pdg))   partOrig = ZBoson;
    if (MC::isW(pdg) && !(partOrig == top)) partOrig = WBoson;
    if ((MC::isQuark(pdg) || MC::isGluon(pdg)) && partOrig != top && partOrig != ZBoson && partOrig != WBoson) partOrig = QCD;
    if (MC::isHiggs(pdg)) return Higgs;
    if (pdg == 35 || pdg == 36 || pdg == 37) return HiggsMSSM;
    if (pdg == 32 || pdg == 33 || pdg == 34) return HeavyBoson;
    if (pdg == 42) return LQ;
    if (MC::isSUSY(pdg)) return SUSY;
    if (MC::isBSM(pdg)) return OtherBSM;
  }
  return partOrig;
}
  enum MCTC_bits : unsigned int { HadTau=0, Tau, hadron, frombsm, uncat, isbsm, isgeant, stable, totalBits };

template <class T>
std::tuple<unsigned int, T> defOrigOfParticle(T thePart) {

  T parent_hadron_ptr = nullptr;

  bool uncat = 0, fromHad = 0, fromTau = 0;
  bool isPhysical = MC::isPhysical(thePart);
  bool isGeant = HepMC::is_simulation_particle(thePart);
  bool isBSM = MC::isBSM(thePart);
  bool fromBSM = isBSM; // just to initialise

  auto prodVtx = thePart->production_vertex();
  if (isPhysical && prodVtx && !isGeant) {
    fromHad = MC::fromHadron(thePart, parent_hadron_ptr, fromTau, fromBSM); 
  }
  else  uncat = 1;

  std::bitset<MCTC_bits::totalBits> classifier;
  classifier[MCTC_bits::stable]  = isPhysical;
  classifier[MCTC_bits::isgeant] = isGeant;
  classifier[MCTC_bits::isbsm]   = isBSM;
  classifier[MCTC_bits::uncat]   = uncat;
  classifier[MCTC_bits::frombsm] = fromBSM;
  classifier[MCTC_bits::hadron]  = fromHad;
  classifier[MCTC_bits::Tau]     = fromTau;
  classifier[MCTC_bits::HadTau]  = fromHad && fromTau;
  unsigned int outputvalue = static_cast<unsigned int>(classifier.to_ulong());

  return std::make_tuple(outputvalue,parent_hadron_ptr);
}
 inline int isPrompt(const unsigned int classify, bool allow_prompt_tau_decays = true) {
    std::bitset<MCTC_bits::totalBits> res(classify);
    if (res.test(MCTC_bits::uncat)) return -1;
    bool fromPromptTau = res.test(MCTC_bits::Tau) && !res.test(MCTC_bits::HadTau);
    if (fromPromptTau) return int(allow_prompt_tau_decays);
    return !res.test(MCTC_bits::hadron);
  }

template <class T> 
ParticleOutCome defOutComeOfElectron(T thePart) {
  ParticleOutCome PartOutCome = UnknownOutCome;
  auto EndVert = MC::findEndVert(thePart);
  if (EndVert == nullptr) return NonInteract;

  int ElecOutNumOfNucFr(0);
  int ElecOutNumOfElec(0);
  int NumOfHadr(0);
  auto outgoing = EndVert->particles_out();
  int NumOfElecDaug = outgoing.size();
  for (const auto& p: outgoing) {
    if (!p) continue;
    int EndDaugType = p->pdg_id();
    if (MC::isElectron(EndDaugType)) ElecOutNumOfElec++;
    if (MC::isHadron(p) && !MC::isBeam(p)) NumOfHadr++;
    if (EndDaugType > 1000000000 || EndDaugType == 0 || abs(EndDaugType) == 2212 || abs(EndDaugType) == 2112) ElecOutNumOfNucFr++;
  }

  if (ElecOutNumOfNucFr != 0 || NumOfHadr != 0)  PartOutCome = NuclInteraction;
  if (ElecOutNumOfElec == 1 && NumOfElecDaug == 1) PartOutCome = ElectrMagInter;

  return PartOutCome;
}
template <class T> 
ParticleOutCome defOutComeOfMuon(T thePart) {
  ParticleOutCome PartOutCome = UnknownOutCome;
  auto EndVert = MC::findEndVert(thePart);
  if (EndVert == nullptr) return NonInteract;
  int MuOutNumOfNucFr(0);
  int NumOfHadr(0);
  int NumOfEleNeutr(0);
  int NumOfMuonNeutr(0);
  int NumOfElec(0);
  auto outgoing = EndVert->particles_out();
  int NumOfMuDaug = outgoing.size();
  for (const auto& p: outgoing) {
    if (!p) continue;
    int EndDaugType = p->pdg_id();
    if (MC::isElectron(EndDaugType)) NumOfElec++;
    if (abs(EndDaugType) == 12) NumOfEleNeutr++;
    if (abs(EndDaugType) == 14) NumOfMuonNeutr++;
    if (MC::isHadron(p) && !MC::isBeam(p)) NumOfHadr++;
    if (EndDaugType > 1000000000 || EndDaugType == 0 || abs(EndDaugType) == 2212 || abs(EndDaugType) == 2112) MuOutNumOfNucFr++;
  }

  if (MuOutNumOfNucFr != 0 || NumOfHadr != 0) PartOutCome = NuclInteraction;
  if (NumOfMuDaug == 3 && NumOfElec == 1 && NumOfEleNeutr == 1 && NumOfMuonNeutr == 1) PartOutCome = DecaytoElectron;

  return PartOutCome;
}
template <class T> 
ParticleOutCome defOutComeOfTau(T thePart) {
  ParticleOutCome PartOutCome = UnknownOutCome;
  auto EndVert = MC::findEndVert(thePart);
  if (EndVert == nullptr) return NonInteract;
  int NumOfTauDaug = EndVert->nOutgoingParticles();
  auto tauFinalStatePart = MC::findFinalStatePart(EndVert);
  auto PD = DecayProducts(tauFinalStatePart);
  int NumOfElec = PD.apd(11);
  int NumOfMuon = PD.apd(13);
  int NumOfElecNeut = PD.apd(12);
  int NumOfMuonNeut = PD.apd(14);
  int NumOfPhot = PD.apd(22);
  int NumOfPi = PD.apd(211);
  int NumOfKaon = PD.apd(321);
  int NumOfNucFr = PD.apd(0) + PD.apd(1000000000, std::numeric_limits<int>::max());

  if (NumOfNucFr != 0) PartOutCome = NuclInteraction;
  if ((NumOfTauDaug == 3 && NumOfElec == 1 && NumOfElecNeut == 1) || (NumOfTauDaug == (3 + NumOfPhot) && NumOfElecNeut == 1)) PartOutCome = DecaytoElectron;
  if ((NumOfTauDaug == 3 && NumOfMuon == 1 && NumOfMuonNeut == 1) || (NumOfTauDaug == (3 + NumOfPhot) && NumOfMuonNeut == 1)) PartOutCome = DecaytoMuon;

  if (NumOfPi == 1 || NumOfKaon == 1) PartOutCome = OneProng;
  if (NumOfPi + NumOfKaon == 3) PartOutCome = ThreeProng;
  if (NumOfPi + NumOfKaon == 5) PartOutCome = FiveProng;


  return PartOutCome;
}
template <class T> 
ParticleOutCome defOutComeOfPhoton(T thePart) {
  ParticleOutCome PartOutCome = UnknownOutCome;
  auto EndVert = MC::findEndVert(thePart);
  if (EndVert == nullptr) return UnConverted;

  int PhtOutNumOfNucFr(0);
  int PhtOutNumOfEl(0);
  int PhtOutNumOfPos(0);
  int PhtOutNumOfHadr(0);

  auto outgoing = EndVert->particles_out();
  int NumOfPhtDaug = outgoing.size();
  for (const auto& p: outgoing) {
    if (!p) continue;
    int EndDaugType = p->pdg_id();
    if (EndDaugType > 1000000000 || EndDaugType == 0 || abs(EndDaugType) == 2212 || abs(EndDaugType) == 2112) PhtOutNumOfNucFr++;
    if (EndDaugType == 11)  PhtOutNumOfEl++;
    if (EndDaugType == -11) PhtOutNumOfPos++;
    if (MC::isHadron(p)&& !MC::isBeam(p) ) PhtOutNumOfHadr++;
  }

  if (PhtOutNumOfEl == 1 && PhtOutNumOfPos == 1 && NumOfPhtDaug == 2) PartOutCome = Converted;
  if ((NumOfPhtDaug > 1 && PhtOutNumOfNucFr != 0) || PhtOutNumOfHadr > 0) PartOutCome = NuclInteraction;

  return PartOutCome;
}



}
#endif