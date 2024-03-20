/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE) // Can only be used in Athena
#include "MCTruthClassifier/MCTruthClassifier.h"
// xAOD EDM includes
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
// Athena only includes
#include "AthenaKernel/Units.h"
#include "AtlasHepMC/GenParticle.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"

#include <cmath>

using namespace MCTruthPartClassifier;

namespace {
std::unique_ptr<Trk::CurvilinearParameters> extractParamFromTruth(const xAOD::TruthParticle& particle) {
  // get start parameters
  const xAOD::TruthVertex* pvtx = particle.prodVtx();
  if (pvtx == nullptr) return nullptr;
  double charge = particle.charge();
  Amg::Vector3D pos(pvtx->x(), pvtx->y(), pvtx->z());
  Amg::Vector3D mom(particle.px(), particle.py(), particle.pz());
  // Aproximate neutral particles as charged with infinite momentum
  if (particle.isNeutral()) {
    charge = 1.;
    mom.normalize();
    mom *= 1e10;
  }
  return std::make_unique<Trk::CurvilinearParameters>(pos, mom, charge);
}
}

// Methods using directly the extrapolator usable only from Athena
const xAOD::TruthParticle* MCTruthClassifier::egammaClusMatch(const xAOD::CaloCluster* clus, bool isFwrdEle, MCTruthPartClassifier::Info* info) const
{
  ATH_MSG_DEBUG("Executing egammaClusMatch ");
  const xAOD::TruthParticle* theMatchPart = nullptr;
  const EventContext& ctx = info ? info->eventContext : Gaudi::Hive::currentContext();
  // retrieve collection and get a pointer
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainerReadHandle(m_truthParticleContainerKey, ctx);

  if (!truthParticleContainerReadHandle.isValid()) {
    ATH_MSG_WARNING( " Invalid ReadHandle for xAOD::TruthParticleContainer with key: " << truthParticleContainerReadHandle.key());
    return theMatchPart;
  }

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{ m_caloMgrKey, ctx };
  if (!caloMgrHandle.isValid()) {
    ATH_MSG_WARNING(" Invalid ReadCondHandle for CaloDetDescrManager with key: " << m_caloMgrKey.key());
    return theMatchPart;
  }

  const CaloDetDescrManager* caloDDMgr = *caloMgrHandle;
  const xAOD::TruthParticle* theEgamma(nullptr);
  const xAOD::TruthParticle* theLeadingPartInCone(nullptr);
  const xAOD::TruthParticle* theBestPartOutCone(nullptr);
  const xAOD::TruthParticle* theBestPartdR(nullptr);
  double LeadingPhtPT(0);
  double LeadingPartPT(0);
  double LeadingPhtdR(999.);
  double LeadingPartdR(999.);
  double BestPartdR(999.);
  double etaClus = clus->etaBE(2);
  double phiClus = clus->phiBE(2);
  if (etaClus < -900) {
    etaClus = clus->eta();
  }
  if (phiClus < -900) {
    phiClus = clus->phi();
  }
  std::vector<const xAOD::TruthParticle*> tps;
  if (!m_truthInConeTool->particlesInCone(ctx, etaClus, phiClus, 0.5, tps)) {
    ATH_MSG_WARNING("Truth Particle in Cone failed");
    return theMatchPart;
  }

  for (const auto* const thePart : tps) {
    // loop over the stable particle
    if (!MC::isStable(thePart)) continue;
    // excluding G4 particle
    if ((!isFwrdEle || (isFwrdEle && m_FwdElectronUseG4Sel)) && HepMC::is_simulation_particle(thePart)) continue;
    long iParticlePDG = thePart->pdgId();
    // excluding neutrino
    if (std::abs(iParticlePDG) == 12 || std::abs(iParticlePDG) == 14 || std::abs(iParticlePDG) == 16) continue;
    double pt = thePart->pt() / Athena::Units::GeV;
    double q = thePart?thePart->charge():0.0;
    // exclude charged particles with pT<1 GeV
    if (q != 0 && pt < m_pTChargePartCut) continue;
    if (q == 0 && pt < m_pTNeutralPartCut) continue;

    // eleptical cone  for extrapolations m_partExtrConePhi X m_partExtrConeEta
    if (!isFwrdEle && m_ROICone && std::hypot( detPhi(phiClus, thePart->phi())/m_partExtrConePhi, detEta(etaClus, thePart->eta())/m_partExtrConeEta) > 1.0) {
      continue;
    }
    // Also check if the clus and true have different sign , i they need both to be <0 or >0
    if (isFwrdEle && // It is forward and
        (((etaClus < 0) - (thePart->eta() < 0) != 0)
         // The truth eta has different sign wrt to the fwd electron
         || (std::fabs(thePart->eta()) < m_FwdElectronTruthExtrEtaCut) // or the truth is less than 2.4 (default cut)
         || (std::fabs(thePart->eta() - etaClus) > m_FwdElectronTruthExtrEtaWindowCut) // or if the delta Eta between el and truth is  > 0.15
         ) // then do no extrapolate this truth Particle for this fwd electron
    ) {
      continue;
    }
    double dR(-999.);
    bool isNCone = false;
    bool isExt = genPartToCalo(ctx, clus, thePart, isFwrdEle, dR, isNCone, *caloDDMgr);
    if (!isExt) continue;
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(), thePart);
    if (info) {
      info->egPartPtr.push_back(thePart);
      info->egPartdR.push_back(dR);
      info->egPartClas.push_back(particleTruthClassifier(theMatchPart, info));
    }
    // Gen particles Not forward
    if (!isFwrdEle) {
      // the leading photon or electron  inside narrow eleptical cone
      // m_phtClasConePhi  X m_phtClasConeEta
      if ((iParticlePDG == 22 || std::abs(iParticlePDG) == 11) && isNCone && pt > LeadingPhtPT) {
        theEgamma = thePart;
        LeadingPhtPT = pt;
        LeadingPhtdR = dR;
      }
      // leading particle (excluding photon and electron) inside narrow eleptic
      // cone m_phtClasConePhi  X m_phtClasConeEta
      if ((iParticlePDG != 22 && std::abs(iParticlePDG) != 11) && isNCone && pt > LeadingPartPT) {
        theLeadingPartInCone = thePart;
        LeadingPartPT = pt;
        LeadingPartdR = dR;
      };
      // the best dR matched particle outside  narrow eleptic cone cone
      // m_phtClasConePhi  X m_phtClasConeEta
      if (!isNCone && dR < BestPartdR) {
        theBestPartOutCone = thePart;
        BestPartdR = dR;
      };
    } else {
      if (dR < BestPartdR) {
        theBestPartdR = thePart;
        BestPartdR = dR;
      };
    }
  }

  if (theEgamma != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(), theEgamma);
    if (info) info->deltaRMatch = LeadingPhtdR;
  } else if (theLeadingPartInCone != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),theLeadingPartInCone);
    if (info) info->deltaRMatch = LeadingPartdR;
  } else if (theBestPartOutCone != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),theBestPartOutCone);
    if (info) info->deltaRMatch = BestPartdR;
  } else if (isFwrdEle && theBestPartdR != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),theBestPartdR );
    if (info) info->deltaRMatch = BestPartdR;
  } else {
    theMatchPart = nullptr;
  }
  if (isFwrdEle || theMatchPart != nullptr || !m_inclG4part) return theMatchPart;

  // additional loop over G4 particles,
  for (const auto* const thePart : tps) {
    if (!MC::isStable(thePart)) continue;
    if (!HepMC::is_simulation_particle(thePart)) continue;
    long iParticlePDG = thePart->pdgId();
    // exclude neutrino
    if (std::abs(iParticlePDG) == 12 || std::abs(iParticlePDG) == 14 || std::abs(iParticlePDG) == 16) continue;
    if (thePart->decayVtx() != nullptr) continue;
    if (std::hypot( detPhi(phiClus, thePart->phi())/m_partExtrConePhi, detEta(etaClus, thePart->eta())/m_partExtrConeEta ) > 1.0) continue;

    double pt = thePart->pt() / Athena::Units::GeV;
    double q = thePart->charge();
    // exclude charged particles with pT<1 GeV
    if (q != 0 && pt < m_pTChargePartCut) continue;
    if (q == 0 && pt < m_pTNeutralPartCut) continue;

    double dR(-999.);
    bool isNCone = false;
    bool isExt = genPartToCalo(ctx, clus, thePart, isFwrdEle, dR, isNCone, *caloDDMgr);
    if (!isExt) continue;

    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),thePart);
    if (info) {
      info->egPartPtr.push_back(thePart);
      info->egPartdR.push_back(dR);
      info->egPartClas.push_back(particleTruthClassifier(theMatchPart, info));
    }
    // the leading photon or electron  inside narrow eleptical cone
    // m_phtClasConePhi  X m_phtClasConeEta
    if ((iParticlePDG == 22 || std::abs(iParticlePDG) == 11) && isNCone && pt > LeadingPhtPT) {
      theEgamma = thePart;
      LeadingPhtPT = pt;
      LeadingPhtdR = dR;
    }
    // leading particle (excluding photon or electron) inside narrow eleptic
    // cone m_phtClasConePhi  X m_phtClasConeEta
    if ((iParticlePDG != 22 && std::abs(iParticlePDG) != 11) && isNCone && pt > LeadingPartPT) {
      theLeadingPartInCone = thePart;
      LeadingPartPT = pt;
      LeadingPartdR = dR;
    }
    // the best dR matched particle outside  narrow eleptic cone cone
    // m_phtClasConePhi  X m_phtClasConeEta
    if (!isNCone && dR < BestPartdR) {
      theBestPartOutCone = thePart;
      BestPartdR = dR;
    }
  }

  if (theEgamma != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),theEgamma);
    if (info) info->deltaRMatch = LeadingPhtdR;
  } else if (theLeadingPartInCone != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),theLeadingPartInCone);
    if (info)  info->deltaRMatch = LeadingPartdR;
  } else if (theBestPartOutCone != nullptr) {
    theMatchPart = MC::find_matching(truthParticleContainerReadHandle.ptr(),theBestPartOutCone);
    if (info) info->deltaRMatch = BestPartdR;
  } else {
    theMatchPart = nullptr;
  }
  ATH_MSG_DEBUG("succeeded  egammaClusMatch ");
  return theMatchPart;
}

bool MCTruthClassifier::genPartToCalo(const EventContext& ctx,
                                 const xAOD::CaloCluster* clus,
                                 const xAOD::TruthParticle* thePart,
                                 bool isFwrdEle,
                                 double& dRmatch,
                                 bool& isNarrowCone,
                                 const CaloDetDescrManager& caloDDMgr) const
{
  dRmatch = -999.;
  isNarrowCone = false;
  if (thePart == nullptr) return false;
  double phiClus = clus->phiBE(2);
  double etaClus = clus->etaBE(2);
  if (etaClus < -900) {
    etaClus = clus->eta();
  }
  if (phiClus < -900) {
    phiClus = clus->phi();
  }
  //--FixMe
  if (isFwrdEle || (etaClus == 0. && phiClus == 0.)) {
    phiClus = clus->phi();
    etaClus = clus->eta();
  }
  // define calo sample
  CaloSampling::CaloSample sample = CaloSampling::EMB2;
  if ((clus->inBarrel() && !clus->inEndcap()) ||
      (clus->inBarrel() && clus->inEndcap() && clus->eSample(CaloSampling::EMB2) >= clus->eSample(CaloSampling::EME2))) {
    // Barrel
    sample = CaloSampling::EMB2;
  } else if ((!clus->inBarrel() && clus->inEndcap() && !isFwrdEle) ||
             (clus->inBarrel() && clus->inEndcap() && clus->eSample(CaloSampling::EME2) > clus->eSample(CaloSampling::EMB2))) {
    // End-cap
    sample = CaloSampling::EME2;
  } else if (isFwrdEle && clus->inEndcap()) {
    // FCAL
    sample = CaloSampling::FCAL2;
  } else {
    return false;
  }
  std::unique_ptr<Trk::CurvilinearParameters> params = extractParamFromTruth(*thePart);
  if (!params) return false;
  // create extension to sample
  std::vector<CaloSampling::CaloSample> samples = { sample };
  auto  extension = m_caloExtensionTool->layersCaloExtension(ctx, *params, samples, etaClus, caloDDMgr);
  bool extensionOK = (!extension.empty());
  if (!extensionOK) {
    ATH_MSG_WARNING("extrapolation of Truth Particle with eta  " << thePart->eta() << " , charge " << thePart->charge() << " , Pt " << thePart->pt() << " to calo failed");
    return false;
  }
  double etaCalo = extension[0].second->position().eta();
  double phiCalo = extension[0].second->position().phi();

  double dPhi = detPhi(phiCalo, phiClus);
  double dEta = detEta(etaCalo, etaClus);
  dRmatch = std::hypot(dPhi, dEta);

  if ((!isFwrdEle && dRmatch > m_phtdRtoTrCut) || (isFwrdEle && dRmatch > m_fwrdEledRtoTrCut)) return false;
  if (!isFwrdEle && std::hypot( dPhi/m_phtClasConePhi, dEta/m_phtClasConeEta ) <= 1.0) isNarrowCone = true;
  return true;
}
#endif
