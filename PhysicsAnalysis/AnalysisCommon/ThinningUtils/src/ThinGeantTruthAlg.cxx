///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ThinGeantTruthAlg.cxx
// Author: James Catmore <James.Catmore@cern.ch>
// based on similar code by Karsten Koeneke <karsten.koeneke@cern.ch>
// Uses thinning service to remove unwanted xAOD truth particles that
// can't be dropped earlier in the simulation chain.
// Not intended for use in derivations!
// - Keep all truth generator particles 
// - Keep all truth particles associated with reco photons, electrons
//   and muons, and their ancestors.
// - Drop any vertices that, after the above thinning, have neither
// incoming nor outgoing particles
// Unlike other algs in this package, no tool is used to select the
// objects for thinning - everything is done in this one class.
// Expression evaluation is also not used.
///////////////////////////////////////////////////////////////////

// EventUtils includes
#include "ThinGeantTruthAlg.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "xAODTruth/xAODTruthHelpers.h"
// STL includes
#include <algorithm>

// FrameWork includes
#include "Gaudi/Property.h"
#include "StoreGate/ThinningHandle.h"

// Standard includes
#include <cstdlib>

#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode
ThinGeantTruthAlg::initialize()
{
  if (m_streamName.empty()) {
    ATH_MSG_ERROR("StreamName property was not initialized.");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_truthParticlesKey.initialize(m_streamName));
  ATH_CHECK(m_truthVerticesKey.initialize(m_streamName));
  ATH_CHECK(m_electronsKey.initialize(m_keepEGamma));
  ATH_CHECK(m_fwdElectronsKey.initialize(m_keepEGamma && !m_fwdElectronsKey.empty()));
  ATH_CHECK(m_photonsKey.initialize(m_keepEGamma));
  ATH_CHECK(m_muonsKey.initialize(m_keepMuons));
  ATH_CHECK(m_egammaTruthKey.initialize(m_keepEGamma));
  if (m_keepEGamma) {
    m_readDecorKeys.emplace_back(m_electronsKey, m_truthLinkDecor);
    m_readDecorKeys.emplace_back(m_photonsKey, m_truthLinkDecor);
    if (!m_fwdElectronsKey.empty()){
      m_readDecorKeys.emplace_back(m_fwdElectronsKey, m_truthLinkDecor);
    }
  }
  if (!m_muonsKey.empty()){
    m_readDecorKeys.emplace_back(m_muonsKey, m_truthLinkDecor);
  }
  
  ATH_CHECK(m_readDecorKeys.initialize());


  return StatusCode::SUCCESS;
}

StatusCode
ThinGeantTruthAlg::finalize()
{
  ATH_MSG_INFO("Processed " << m_nEventsProcessed << " events containing "
                            << m_nParticlesProcessed << " truth particles and "
                            << m_nVerticesProcessed << " truth vertices ");
  ATH_MSG_INFO("Removed " << m_nParticlesThinned
                          << " Geant truth particles and " << m_nVerticesThinned
                          << " corresponding truth vertices ");
  return StatusCode::SUCCESS;
}

StatusCode
ThinGeantTruthAlg::execute(const EventContext& ctx) const
{
  // Increase the event counter
  ++m_nEventsProcessed;

  // Retrieve truth and vertex containers
  SG::ThinningHandle<xAOD::TruthParticleContainer> truthParticles(
    m_truthParticlesKey, ctx);
  SG::ThinningHandle<xAOD::TruthVertexContainer> truthVertices(
    m_truthVerticesKey, ctx);
  if (!truthParticles.isValid()) {
    ATH_MSG_FATAL("No TruthParticleContainer with key " << m_truthParticlesKey.key() << " found.");
    return StatusCode::FAILURE;
  }
  if (!truthVertices.isValid()) {
    ATH_MSG_FATAL("No TruthVertexContainer with key " << m_truthVerticesKey.key() << " found.");
    return StatusCode::FAILURE;
  }

  // Loop over photons, electrons and muons and get the associated truth
  // particles Retain the associated index number
  std::vector<int> recoParticleTruthIndices;
  std::vector<int> egammaTruthIndices{};

  // Muons
  if (m_keepMuons) {
    SG::ReadHandle<xAOD::MuonContainer> muons(m_muonsKey, ctx);
    // Retrieve muons, electrons and photons
    if (!muons.isValid()) {
      ATH_MSG_WARNING("No muon container with key " << m_muonsKey.key() << " found.");
    }
    for (const xAOD::Muon* muon : *muons) {
      const xAOD::TruthParticle* truthMuon = xAOD::TruthHelpers::getTruthParticle(*muon);
      if (truthMuon) {
        recoParticleTruthIndices.push_back(truthMuon->index());
      }
    }
  }

  // Electrons and photons
  if (m_keepEGamma) {

    // Electrons
    SG::ReadHandle<xAOD::ElectronContainer> electrons(m_electronsKey, ctx);
    if (!electrons.isValid()) {
      ATH_MSG_WARNING("No electron container with key " << m_electronsKey.key() << " found.");
    }
    for (const xAOD::Electron* electron : *electrons) {
      const xAOD::TruthParticle* truthElectron =
        xAOD::TruthHelpers::getTruthParticle(*electron);
      if (truthElectron) {
        recoParticleTruthIndices.push_back(truthElectron->index());
      }
    }

    // Forward Electrons
    if (!m_fwdElectronsKey.empty()) {
      SG::ReadHandle<xAOD::ElectronContainer> fwdElectrons(m_fwdElectronsKey,
                                                           ctx);
      if (!fwdElectrons.isValid()) {
        ATH_MSG_WARNING("No forward electron container with key "
                        << m_fwdElectronsKey.key() << " found.");
      }
      for (const xAOD::Electron* electron : *fwdElectrons) {
        const xAOD::TruthParticle* truthElectron =
          xAOD::TruthHelpers::getTruthParticle(*electron);
        if (truthElectron) {
          recoParticleTruthIndices.push_back(truthElectron->index());
        }
      }
    }

    // Photons
    SG::ReadHandle<xAOD::PhotonContainer> photons(m_photonsKey, ctx);
    if (!photons.isValid()) {
      ATH_MSG_WARNING("No photon container with key " << m_photonsKey.key() << " found.");
    }

    for (const xAOD::Photon* photon : *photons) {
      const xAOD::TruthParticle* truthPhoton =
        xAOD::TruthHelpers::getTruthParticle(*photon);
      if (truthPhoton) {
        recoParticleTruthIndices.push_back(truthPhoton->index());
      }
    }

    // egamma Truth Particles
    SG::ReadHandle<xAOD::TruthParticleContainer> egammaTruthParticles(
      m_egammaTruthKey, ctx);
    if (!egammaTruthParticles.isValid()) {
      ATH_MSG_WARNING("No e-gamma truth container with key " << m_egammaTruthKey.key() << " found.");
    }

    for (const xAOD::TruthParticle* egTruthParticle : *egammaTruthParticles) {

      static const SG::AuxElement::ConstAccessor<int> accType("truthType");

      if (!accType.isAvailable(*egTruthParticle) ||
          accType(*egTruthParticle) != MCTruthPartClassifier::IsoElectron ||
          std::abs(egTruthParticle->eta()) > m_etaMaxEgTruth) {
        continue;
      }
      // Only isolated true electrons
      using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
      static const SG::AuxElement::ConstAccessor<TruthLink_t> linkToTruth(
        "truthParticleLink");
      if (!linkToTruth.isAvailable(*egTruthParticle)) {
        continue;
      }

      const TruthLink_t& truthegamma = linkToTruth(*egTruthParticle);
      if (!truthegamma.isValid()) {
        continue;
      }
      egammaTruthIndices.push_back((*truthegamma)->index());
    }
  }

  // Set up masks
  std::vector<bool> particleMask, vertexMask;
  int nTruthParticles = truthParticles->size();
  int nTruthVertices = truthVertices->size();
  m_nParticlesProcessed.fetch_add(nTruthParticles, std::memory_order_relaxed);
  m_nVerticesProcessed.fetch_add(nTruthVertices, std::memory_order_relaxed);
  particleMask.assign(nTruthParticles, false);
  vertexMask.assign(nTruthVertices, false);

  // Vector of pairs keeping track of how many incoming/outgoing particles each
  // vertex has
  std::vector<std::pair<int, int>> vertexLinksCounts;
  for (const auto *vertex : *truthVertices) {
    std::pair<int, int> tmpPair;
    tmpPair.first = vertex->nIncomingParticles();
    tmpPair.second = vertex->nOutgoingParticles();
    vertexLinksCounts.push_back(tmpPair);
  }

  // Loop over truth particles and update mask
  std::unordered_set<int> encounteredUniqueIDs; // for loop protection
  for (int i = 0; i < nTruthParticles; ++i) {
    encounteredUniqueIDs.clear();
    const xAOD::TruthParticle* particle = (*truthParticles)[i];
    // Retain status 1 BSM particles and descendants
    if (MC::isBSM(particle) && MC::isStable(particle)) {
      descendants(particle, particleMask, encounteredUniqueIDs);
      encounteredUniqueIDs.clear();
    }
    // Retain children of longer-lived generator particles
    if (MC::isStable(particle)) {
      int pdgId = abs(particle->pdgId());
      if (std::find(m_longlived.begin(), m_longlived.end(), pdgId) !=
          m_longlived.end()) {
        const xAOD::TruthVertex* decayVtx(nullptr);
        if (particle->hasDecayVtx()) {
          decayVtx = particle->decayVtx();
        }
        int nChildren = 0;
        if (decayVtx)
          nChildren = decayVtx->nOutgoingParticles();
        for (int i = 0; i < nChildren; ++i) {
          particleMask[decayVtx->outgoingParticle(i)->index()] = true;
        }
      }
    }

    // Retain particles and their descendants/ancestors associated with the
    // reconstructed objects
    if (std::find(recoParticleTruthIndices.begin(),
                  recoParticleTruthIndices.end(),
                  i) != recoParticleTruthIndices.end()) {
      if (HepMC::is_simulation_particle(particle)) { // only need to do this for Geant particles since
                           // non-Geant are kept anyway
        ancestors(particle, particleMask, encounteredUniqueIDs);
        encounteredUniqueIDs.clear();
        descendants(particle, particleMask, encounteredUniqueIDs);
        encounteredUniqueIDs.clear();
      }
    }

    // Retain particles and their descendants  associated with the egamma Truth
    // Particles
    if (std::find(egammaTruthIndices.begin(), egammaTruthIndices.end(), i) !=
        egammaTruthIndices.end()) {
      descendants(particle, particleMask, encounteredUniqueIDs);
      encounteredUniqueIDs.clear();
    }

    if (!HepMC::is_simulation_particle(particle)) {
      particleMask[i] = true;
    }
    else {
      // deal with the products of interactions of quasi-stable
      // particles
      if (particle->hasProdVtx()) {
        const xAOD::TruthVertex* prodVtx = particle->prodVtx();
        const int nParents = prodVtx->nIncomingParticles();
        for (int parent = 0; parent < nParents; ++parent) {
          if (MC::isDecayed(prodVtx->incomingParticle(parent))) {
            // "simulation particle" with a parent with status==2 was
            // produced via the interaction of a quasi-stable particle
            // with the detector material. Such particles should be kept.
            particleMask[i] = true;
            break;
          }
        }
      }
    }
  }

  // Loop over the mask and update vertex association counters
  for (int i = 0; i < nTruthParticles; ++i) {
    if (!particleMask[i]) {
      ++m_nParticlesThinned;
      const xAOD::TruthParticle* particle = (*truthParticles)[i];
      if (particle->hasProdVtx()) {
        const auto *prodVertex = particle->prodVtx();
        --vertexLinksCounts[prodVertex->index()].second;
      }
      if (particle->hasDecayVtx()) {
        const auto *decayVertex = particle->decayVtx();
        --vertexLinksCounts[decayVertex->index()].first;
      }
    }
  }

  // Loop over truth vertices and update mask
  // Those for which all incoming and outgoing particles are to be thinned, will
  // be thinned as well
  unsigned int nVerticesThinned = 0;
  for (int i = 0; i < nTruthVertices; ++i) {
    if (vertexLinksCounts[i].first != 0 || vertexLinksCounts[i].second != 0) {
      vertexMask[i] = true;
    } else {
      ++nVerticesThinned;
    }
  }
  m_nVerticesThinned.fetch_add(nVerticesThinned, std::memory_order_relaxed);
  // Apply masks to thinning
  truthParticles.keep(particleMask);
  truthVertices.keep(vertexMask);

  return StatusCode::SUCCESS;
}

// Inline methods
//
// ==============================
// ancestors
// ==============================
// Updates particle mask such that particle and all ancestors are retained
void
ThinGeantTruthAlg::ancestors(const xAOD::TruthParticle* pHead,
                             std::vector<bool>& particleMask,
                             std::unordered_set<int>& encounteredUniqueIDs) const
{

  // Check that this uniqueID hasn't been seen before (e.g. we are in a loop)
  std::unordered_set<int>::const_iterator found =
    encounteredUniqueIDs.find(HepMC::uniqueID(pHead));
  if (found != encounteredUniqueIDs.end())
    return;
  encounteredUniqueIDs.insert(HepMC::uniqueID(pHead));

  // Save particle position in the mask
  int headIndex = pHead->index();
  particleMask[headIndex] = true;

  // Get the production vertex
  const xAOD::TruthVertex* prodVtx(nullptr);
  if (pHead->hasProdVtx()) {
    prodVtx = pHead->prodVtx();
  } else {
    return;
  }

  // Get children particles and self-call
  int nParents = prodVtx->nIncomingParticles();
  for (int i = 0; i < nParents; ++i)
    ancestors(prodVtx->incomingParticle(i), particleMask, encounteredUniqueIDs);
}

// ==============================
// descendants
// ==============================
// Updates particle mask such that particle and all descendants are retained
void
ThinGeantTruthAlg::descendants(
  const xAOD::TruthParticle* pHead,
  std::vector<bool>& particleMask,
  std::unordered_set<int>& encounteredUniqueIDs) const
{
  // Check that this unique ID hasn't been seen before (e.g. we are in a loop)
  std::unordered_set<int>::const_iterator found =
    encounteredUniqueIDs.find(HepMC::uniqueID(pHead));
  if (found != encounteredUniqueIDs.end())
    return;
  encounteredUniqueIDs.insert(HepMC::uniqueID(pHead));

  // Save the particle position in the mask
  int headIndex = pHead->index();
  particleMask[headIndex] = true;

  // Get the decay vertex
  const xAOD::TruthVertex* decayVtx(nullptr);
  if (pHead->hasDecayVtx()) {
    decayVtx = pHead->decayVtx();
  } else {
    return;
  }

  // Get children particles and self-call
  int nChildren = decayVtx->nOutgoingParticles();
  for (int i = 0; i < nChildren; ++i) {
    descendants(
      decayVtx->outgoingParticle(i), particleMask, encounteredUniqueIDs);
  }

  }


