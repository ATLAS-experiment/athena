/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATIONBASE
#include "AsgDataHandles/ReadHandle.h"
#include "MCTruthClassifier/MCTruthClassifier.h"
using namespace MCTruthPartClassifier;

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::TrackParticle* trkPtr, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  ATH_MSG_DEBUG("Executing trackClassifier");
  ParticleType parttype = Unknown;
  ParticleOrigin partorig = NonDefined;
  const xAOD::TruthParticle* genPart = getGenPart(trkPtr);
  if (info) info->genPart = genPart;
  if (!genPart) return std::make_pair(parttype, partorig);
  ATH_MSG_DEBUG("trackClassifier  succeeded ");
  return particleTruthClassifier(genPart, info);
}

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::Electron* elec, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  ATH_MSG_DEBUG("Executing egamma electron Classifier");
  ParticleType parttype = Unknown;
  ParticleOrigin partorig = NonDefined;
  const xAOD::TruthParticle* genPart = nullptr;
  const xAOD::TrackParticle* trkPtr = elec->trackParticle();
  if (elec->author() != xAOD::EgammaParameters::AuthorFwdElectron ||trkPtr) {
    // Central electron or forward electron with track (when reco
    // implemented in the future)
    if (!trkPtr){
      return std::make_pair(parttype, partorig);
    }
    genPart = getGenPart(trkPtr);
  } else {
#ifndef XAOD_ANALYSIS // cluster matching available only in Athena
    const xAOD::CaloCluster* clus = elec->caloCluster();
    genPart = egammaClusMatch(clus, true, info);
#else
    ATH_MSG_WARNING("Forward Electron classification using extrapolation to Calo is available only in Athena , check your enviroment. ");
#endif
  }

  if (info)  info->genPart = genPart;
  if (!genPart) return std::make_pair(parttype, partorig);
  ATH_MSG_DEBUG("egamma electron Classifier  succeeded ");
  return particleTruthClassifier(genPart, info);
}

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::Photon* phot, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  ATH_MSG_DEBUG("Executing egamma photon Classifier");
  ParticleType parttype = Unknown;
  ParticleOrigin partorig = NonDefined;
  const xAOD::CaloCluster* clus = phot->caloCluster();
  if (!clus) return std::make_pair(parttype, partorig);
  if (std::fabs(clus->eta()) > 10.0 || std::fabs(clus->phi()) > 6.28 || (clus->et()) <= 0.)  return std::make_pair(parttype, partorig);

  const xAOD::Vertex* VxCvPtr = phot->vertex();
  if (VxCvPtr != nullptr) {
    for (int itrk = 0; itrk < (int)VxCvPtr->nTrackParticles(); itrk++) {
      if (itrk > 1) continue;
      const xAOD::TrackParticle* trkPtr = VxCvPtr->trackParticle(itrk);
      if (!trkPtr) continue;
      const xAOD::TruthParticle* thePart = getGenPart(trkPtr);
      std::pair<ParticleType, ParticleOrigin> classif = particleTruthClassifier(thePart, info);
      if (info) {
        info->cnvPhotTrkPtr.push_back(trkPtr);
        info->cnvPhotTrkToTruthPart.push_back(thePart);
        info->cnvPhotPartType.push_back(classif.first);
        info->cnvPhotPartOrig.push_back(classif.second);
      }
    }
  }

  const xAOD::TruthParticle* genPart = nullptr;
#ifndef XAOD_ANALYSIS // Fwd electron available only in Athena
  genPart = egammaClusMatch(clus, false, info);
#else
  ATH_MSG_WARNING("Photon  Classification using extrapolation to Calo is  available only in Athena , check your enviroment. ");
#endif
  if (!genPart) return std::make_pair(parttype, partorig);
  if (info) info->genPart = genPart;
  ATH_MSG_DEBUG("egamma photon  Classifier  succeeded ");
  return particleTruthClassifier(genPart, info);
}

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::Muon* mu, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  ATH_MSG_DEBUG("Executing muon  Classifier");
  ParticleType parttype = Unknown;
  ParticleOrigin partorig = NonDefined;
  const xAOD::TrackParticle* trkPtr = nullptr;
  if (mu->primaryTrackParticleLink().isValid()) trkPtr = *mu->primaryTrackParticleLink();
  else if (mu->combinedTrackParticleLink().isValid()) trkPtr = *mu->combinedTrackParticleLink();
  else if (mu->inDetTrackParticleLink().isValid()) trkPtr = *mu->combinedTrackParticleLink();
  else if (mu->muonSpectrometerTrackParticleLink().isValid()) trkPtr = *mu->muonSpectrometerTrackParticleLink();

  if (!trkPtr) return std::make_pair(parttype, partorig);

  const xAOD::TruthParticle* genPart = getGenPart(trkPtr);
  if (!genPart) return std::make_pair(parttype, partorig);
  if (info) info->genPart = genPart;
  ATH_MSG_DEBUG("muon Classifier  succeeded ");
  return particleTruthClassifier(genPart, info);
}

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::CaloCluster* clus, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  ATH_MSG_DEBUG("Executing egamma photon Classifier with cluster Input");
  ParticleType parttype = Unknown;
  ParticleOrigin partorig = NonDefined;
  if (!clus) return std::make_pair(parttype, partorig);
  if (std::fabs(clus->eta()) > 10.0 || std::fabs(clus->phi()) > M_PI || (clus->et()) <= 0.) return std::make_pair(parttype, partorig);
  const xAOD::TruthParticle* genPart = nullptr;
#ifndef XAOD_ANALYSIS // Fwd electron available only in Athena
  genPart = egammaClusMatch(clus, false, info);
#else
  ATH_MSG_WARNING("Cluster  Classification using extrapolation to Calo is available only in Athena , check your enviroment. ");
#endif
  if (!genPart) return std::make_pair(parttype, partorig);
  ATH_MSG_DEBUG("Calo Cluster  Classifier  succeeded ");
  if (info) info->genPart = genPart;
  return particleTruthClassifier(genPart, info);
}

std::pair<ParticleType, ParticleOrigin>
MCTruthClassifier::particleTruthClassifier(const xAOD::Jet* jet, bool DR, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  ATH_MSG_DEBUG("Executing Classifier with jet Input");
  ParticleType parttype = UnknownJet;
  ParticleOrigin partorig = NonDefined;
  ParticleType tempparttype = UnknownJet;
  std::set<const xAOD::TruthParticle*> allJetMothers;
  std::set<const xAOD::TruthParticle*> constituents;
  if (!jet) return std::make_pair(parttype, partorig);
  allJetMothers.clear();
  constituents.clear();
  findJetConstituents(jet, constituents, DR);
  // AV: Jet type is the type of hadron with "heaviest" flavour among the jet constituents.
  // AV: No hadrons in the jet -- the flavour is unknown.
  // AV: The algorithm will fail on 4/5 quark hadrons and probably on nonBSM hadrons. To be fixed.
  for (const auto& thePart: constituents) {
    MC::findParticleAncestors(thePart, allJetMothers);
    //AV: probably skip ME particles
    if (!MC::isPhysical(thePart)) continue;
    // determine if hadron and its type
    tempparttype = particleTruthClassifier(thePart, info).first;
    if (tempparttype != Hadron) continue;
    tempparttype = defTypeOfHadron(thePart->pdgId());
    // classify the jet
    if (tempparttype == BBbarMesonPart || tempparttype == BottomMesonPart || tempparttype == BottomBaryonPart) {
      parttype = BJet;
      continue;
    }
    if (tempparttype == CCbarMesonPart || tempparttype == CharmedMesonPart || tempparttype == CharmedBaryonPart) {
      if (parttype != BJet) parttype = CJet;
      continue;
    }
    if (tempparttype == StrangeBaryonPart || tempparttype == LightBaryonPart || tempparttype == StrangeMesonPart || tempparttype == LightMesonPart) {
      if (parttype != BJet && parttype != CJet) parttype = LJet;
      continue;
    }
  }

  // clasify the jet origin
  partorig = defJetOrig(allJetMothers);
  allJetMothers.clear();
  constituents.clear();
  ATH_MSG_DEBUG(" jet Classifier succeeded");
  return std::make_pair(parttype, partorig);
}

const xAOD::TruthParticle*
MCTruthClassifier::getGenPart(const xAOD::TrackParticle* trk, MCTruthPartClassifier::Info* info /*= nullptr*/) const
{
  // return GenParticle corresponding to given TrackParticle
  ATH_MSG_DEBUG("Executing getGenPart ");
  if (!trk) return nullptr;
  if (info) {
    info->deltaRMatch = -999.;
    info->deltaPhi = -999.;
    info->probTrkToTruth = 0;
    info->numOfSiHits = 0;
  }

  uint8_t NumOfPixHits = 0;
  uint8_t NumOfSCTHits = 0;
  typedef ElementLink<xAOD::TruthParticleContainer> TruthLink_t;

  static const SG::AuxElement::Accessor<TruthLink_t> tPL("truthParticleLink");
  if (!tPL.isAvailable(*trk)) {
    ATH_MSG_DEBUG("Track particle is not associated to truth particle");
    return nullptr;
  }

  const auto& truthLink = tPL(*trk);
  if (!truthLink.isValid()) {
    ATH_MSG_DEBUG("Invalid link to truth particle");
    return nullptr;
  }

  const xAOD::TruthParticle* theGenParticle = (*truthLink);
  if (!theGenParticle) {
    ATH_MSG_DEBUG("Could not find truth matching for track");
    return nullptr;
  }

  if (info) {
    static const SG::AuxElement::Accessor<float> tMP("truthMatchProbability");
    if (tMP.isAvailable(*trk)) {
      info->probTrkToTruth = tMP(*trk);
    } else {
      ATH_MSG_DEBUG("Truth match probability not available");
    }
  }

  if (theGenParticle->status() == 3) {
    ATH_MSG_WARNING("track matched to the truth with status " << theGenParticle->status());
  }
  else if (MC::isDecayed(theGenParticle)) {
    // Matching to status == 2 particles is to be expected if
    // quasi-stable particle simulation was used.
    ATH_MSG_DEBUG("track matched to the truth with status " << theGenParticle->status());
  }

  if (MC::isDecayed(theGenParticle) && (MC::isElectron(theGenParticle) || MC::isMuon(theGenParticle))) {
    const xAOD::TruthVertex* EndVrtx = theGenParticle->decayVtx();
    const xAOD::TruthParticle* theGenPartTmp(nullptr);

    if (EndVrtx != nullptr) {
      int itr = 0;
      do {
        theGenPartTmp = nullptr;
        for (const auto & theDaugt: EndVrtx->particles_out()) {
          if (!theDaugt) continue;
          if (theDaugt->pdgId() == theGenParticle->pdgId()) theGenPartTmp = theDaugt;
          if (theDaugt->pdgId() != theGenParticle->pdgId() && !MC::isPhoton(theDaugt)) theGenPartTmp = nullptr;
        }
        itr++;
        if (itr > 100) {
          ATH_MSG_WARNING("getGenPart infinite while");
          break;
        }
        EndVrtx = theGenPartTmp ? theGenPartTmp->decayVtx() : nullptr;
      } while (theGenPartTmp && theGenPartTmp->pdgId() == theGenParticle->pdgId() && MC::isDecayed(theGenPartTmp) && EndVrtx != nullptr);

      if (theGenPartTmp && theGenPartTmp->pdgId() == theGenParticle->pdgId()) theGenParticle = theGenPartTmp;
    }
  }

  if (!trk->summaryValue(NumOfSCTHits, xAOD::numberOfSCTHits)) ATH_MSG_DEBUG("Could not retrieve number of SCT hits");
  if (!trk->summaryValue(NumOfPixHits, xAOD::numberOfPixelHits))  ATH_MSG_DEBUG("Could not retrieve number of Pixel hits");

  uint8_t NumOfSiHits = NumOfSCTHits + NumOfPixHits;

  float deltaPhi = detPhi(theGenParticle->phi(), trk->phi());
  float deteta = detEta(theGenParticle->eta(), trk->eta());
  float deltaRMatch = std::hypot(deltaPhi, deteta);
  if ((NumOfSiHits > m_NumOfSiHitsCut && deltaRMatch > m_deltaRMatchCut) ||
      (NumOfSiHits <= m_NumOfSiHitsCut && deltaPhi > m_deltaPhiMatchCut)) theGenParticle = nullptr;

  if (info) {
    info->deltaRMatch = deltaRMatch;
    info->deltaPhi = deltaPhi;
    info->numOfSiHits = NumOfSiHits;
  }
  ATH_MSG_DEBUG("getGenPart  succeeded ");
  return (theGenParticle);
}

void MCTruthClassifier::findJetConstituents(const xAOD::Jet* jet,
                                       std::set<const xAOD::TruthParticle*>& constituents,
                                       bool DR) const
{
  if (DR) {
    // use a DR matching scheme (default)
    // retrieve collection and get a pointer
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainerReadHandle(m_truthParticleContainerKey);

    if (!truthParticleContainerReadHandle.isValid()) {
      ATH_MSG_WARNING(" Invalid ReadHandle for xAOD::TruthParticleContainer with key: " << truthParticleContainerReadHandle.key());
      return;
    }
    ATH_MSG_DEBUG("xAODTruthParticleContainer with key  " << truthParticleContainerReadHandle.key() << " has valid ReadHandle ");
    // find the matching truth particles
    for (const auto *const thePart : *truthParticleContainerReadHandle) {
      // match truth particles to the jet
      if (MC::isStable(thePart) && thePart->p4().DeltaR(jet->p4()) < m_jetPartDRMatch) {
        constituents.insert(thePart);
      }
    }
  }
  else {
    xAOD::JetConstituentVector vec = jet->getConstituents();
    for (const auto *particle0 : vec) {
      const xAOD::TruthParticle* thePart = dynamic_cast<const xAOD::TruthParticle*>(particle0->rawConstituent());
      if (MC::isStable(thePart)) {
        constituents.insert(thePart);
      }
    }
  }
}
double MCTruthClassifier::fracParticleInJet(const xAOD::TruthParticle* thePart, const xAOD::Jet* jet, bool DR, bool nparts) const
{
  std::set<const xAOD::TruthParticle*> constituents;
  std::set<const xAOD::TruthParticle*> daughters;
  std::set<const xAOD::TruthParticle*> intersect;

  findJetConstituents(jet, constituents, DR);
  MC::findParticleStableDescendants(thePart, daughters);
  if (daughters.empty()) daughters.insert(thePart);
  // Get the intersection of constituents and daughters
  std::set_intersection(constituents.begin(),
                        constituents.end(),
                        daughters.begin(),
                        daughters.end(),
                        std::inserter(intersect, intersect.begin()));

  if (nparts) return 1.0*intersect.size() / daughters.size();
  double frac = 0;
  double tot = 0;
  for (const auto *daughter : daughters) { tot += daughter->pt();}
  for (const auto *particle : intersect) { frac += particle->pt();}
  return frac/tot;
}

#endif
