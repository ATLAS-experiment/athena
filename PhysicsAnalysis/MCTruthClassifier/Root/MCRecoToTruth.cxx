/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATIONBASE
#include "AsgDataHandles/ReadHandle.h"
#include "MCTruthClassifier/MCTruthClassifier.h"
using namespace MCTruthPartClassifier;

IMCTruthClassifier::Result
MCTruthClassifier::particleTruthClassifier(const EventContext& ctx, const xAOD::TrackParticle* trkPtr) const
{
  ATH_MSG_DEBUG("Executing trackClassifier");
  const xAOD::TruthParticle* genPart = getGenPart(trkPtr);
  if (!genPart) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  ATH_MSG_DEBUG("trackClassifier  succeeded ");
  return particleTruthClassifier(ctx, genPart);
}

IMCTruthClassifier::Result
MCTruthClassifier::particleTruthClassifier(const EventContext& ctx, const xAOD::Electron* elec) const
{
  ATH_MSG_DEBUG("Executing egamma electron Classifier");
  const xAOD::TruthParticle* genPart = nullptr;
  const xAOD::TrackParticle* trkPtr = elec->trackParticle();
  if (elec->author() != xAOD::EgammaParameters::AuthorFwdElectron ||trkPtr) {
    // Central electron or forward electron with track (when reco
    // implemented in the future)
    if (!trkPtr){
      return {Unknown, NonDefined, nullptr, UnknownOutCome};
    }
    genPart = getGenPart(trkPtr);
  } else {
#ifndef XAOD_ANALYSIS // cluster matching available only in Athena
    const xAOD::CaloCluster* clus = elec->caloCluster();
    genPart = egammaClusMatch(ctx, clus, true);
#else
    ATH_MSG_WARNING("Forward Electron classification using extrapolation to Calo is available only in Athena , check your enviroment. ");
#endif
  }

  if (!genPart) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  ATH_MSG_DEBUG("egamma electron Classifier  succeeded ");
  return particleTruthClassifier(ctx, genPart);
}

IMCTruthClassifier::Result
MCTruthClassifier::particleTruthClassifier(const EventContext& ctx, const xAOD::Photon* phot) const
{
  ATH_MSG_DEBUG("Executing egamma photon Classifier");
  const xAOD::CaloCluster* clus = phot->caloCluster();
  if (!clus) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  if (std::fabs(clus->eta()) > 10.0 || std::fabs(clus->phi()) > 6.28 || (clus->et()) <= 0.)  return {Unknown, NonDefined, nullptr, UnknownOutCome};

  const xAOD::TruthParticle* genPart = nullptr;
#ifndef XAOD_ANALYSIS // Fwd electron available only in Athena
  genPart = egammaClusMatch(ctx, clus, false);
#else
  ATH_MSG_WARNING("Photon  Classification using extrapolation to Calo is  available only in Athena , check your enviroment. ");
#endif
  if (!genPart) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  ATH_MSG_DEBUG("egamma photon  Classifier  succeeded ");
  return particleTruthClassifier(ctx, genPart);
}

IMCTruthClassifier::Result
MCTruthClassifier::particleTruthClassifier(const EventContext& ctx, const xAOD::Muon* mu) const
{
  ATH_MSG_DEBUG("Executing muon  Classifier");
  const xAOD::TrackParticle* trkPtr = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  
  if (!trkPtr) return {Unknown, NonDefined, nullptr, UnknownOutCome};

  const xAOD::TruthParticle* genPart = getGenPart(trkPtr);
  if (!genPart) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  ATH_MSG_DEBUG("muon Classifier  succeeded ");
  return particleTruthClassifier(ctx, genPart);
}

IMCTruthClassifier::Result
MCTruthClassifier::particleTruthClassifier(const EventContext& ctx, const xAOD::CaloCluster* clus) const
{
  ATH_MSG_DEBUG("Executing egamma photon Classifier with cluster Input");
  if (!clus) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  if (std::fabs(clus->eta()) > 10.0 || std::fabs(clus->phi()) > M_PI || (clus->et()) <= 0.) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  const xAOD::TruthParticle* genPart = nullptr;
#ifndef XAOD_ANALYSIS // Fwd electron available only in Athena
  genPart = egammaClusMatch(ctx, clus, false);
#else
  ATH_MSG_WARNING("Cluster  Classification using extrapolation to Calo is available only in Athena , check your enviroment. ");
#endif
  if (!genPart) return {Unknown, NonDefined, nullptr, UnknownOutCome};
  ATH_MSG_DEBUG("Calo Cluster  Classifier  succeeded ");
  return particleTruthClassifier(ctx, genPart);
}

const xAOD::TruthParticle*
MCTruthClassifier::getGenPart(const xAOD::TrackParticle* trk) const
{
  // return GenParticle corresponding to given TrackParticle
  ATH_MSG_DEBUG("Executing getGenPart ");
  if (!trk) return nullptr;
  uint8_t NumOfPixHits = 0;
  uint8_t NumOfSCTHits = 0;
  typedef ElementLink<xAOD::TruthParticleContainer> TruthLink_t;

  static const SG::Accessor<TruthLink_t> tPL("truthParticleLink");
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

  float deltaPhi = std::abs(std::remainder(theGenParticle->phi() - trk->phi(), 2*std::numbers::pi));
  float deteta = std::abs(theGenParticle->eta() - trk->eta());
  float deltaRMatch = std::hypot(deltaPhi, deteta);
  if ((NumOfSiHits > m_NumOfSiHitsCut && deltaRMatch > m_deltaRMatchCut) ||
      (NumOfSiHits <= m_NumOfSiHitsCut && deltaPhi > m_deltaPhiMatchCut)) theGenParticle = nullptr;

  ATH_MSG_DEBUG("getGenPart  succeeded ");
  return (theGenParticle);
}

#endif
