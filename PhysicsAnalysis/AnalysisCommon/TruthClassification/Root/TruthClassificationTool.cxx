/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <set>

#include <MCTruthClassifier/MCTruthClassifierDefs.h>
#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/xAODTruthHelpers.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include "TruthClassification/TruthClassificationTool.h"
#include "TruthUtils/HepMCHelpers.h"

namespace
{
  bool isInSet(int origin, const std::set<int> &s)
  {
    return s.find(origin) != s.end();
  }
}


TruthClassificationTool::TruthClassificationTool(const std::string &type)
  : asg::AsgTool(type)
{
  declareProperty ("separateChargeFlipElectrons", m_separateChargeFlipElectrons, "separate prompt charge-flipped electrons");
  declareProperty ("separateChargeFlipMuons",     m_separateChargeFlipMuons,     "separate prompt charge-flipped muons");
  declareProperty ("useTruthParticleDecorations", m_useTruthParticleDecorations, "use truth particle decorations");
}


StatusCode TruthClassificationTool::classify(const xAOD::IParticle &particle,
                                             unsigned int &classification) const
{
  Truth::Type type = Truth::Type::Unknown;
  ANA_CHECK(classify(particle, type));
  classification = static_cast<int>(type);
  return StatusCode::SUCCESS;
}


StatusCode TruthClassificationTool::classify(const xAOD::IParticle &particle,
                                             Truth::Type &classification) const
{
  const xAOD::TruthParticle *truthParticle = dynamic_cast<const xAOD::TruthParticle *> (&particle);
  if (dynamic_cast<const xAOD::Electron *> (&particle) || (truthParticle != nullptr && MC::isElectron(truthParticle)))
  {
    ANA_CHECK(classifyElectron(particle, classification));
  }
  else if (dynamic_cast<const xAOD::Muon *> (&particle) || (truthParticle != nullptr && MC::isMuon(truthParticle)))
  {
    ANA_CHECK(classifyMuon(particle, classification));
  }
  else
  {
    ANA_MSG_ERROR("Only electrons and muons are supported.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


StatusCode TruthClassificationTool::classifyElectron(const xAOD::IParticle &electron,
                                                     Truth::Type &classification) const
{
  

  // Check if xAOD::TruthParticle or if not if it has the TruthParticleLink
  const xAOD::TruthParticle *truthParticle = dynamic_cast<const xAOD::TruthParticle *> (&electron);
  bool isTruthParticle{};
  if (truthParticle == nullptr)
  {
    // need to find the truth particle
    truthParticle = xAOD::TruthHelpers::getTruthParticle(electron);
  }
  else
  {
    isTruthParticle = true;
  }

  if (!m_truthPdgId.isAvailable(electron) && !isTruthParticle)
  {
    ANA_MSG_ERROR("Electron does not have the 'truthPdgId' decoration.");
    return StatusCode::FAILURE;
  }

  if (!m_firstMotherTruthType.isAvailable(electron)
    || !m_firstMotherTruthOrigin.isAvailable(electron)
    || !m_firstMotherPdgId.isAvailable(electron))
  {
    ANA_MSG_ERROR("Electron does not have one or more 'firstEgMother' decorations.");
    return StatusCode::FAILURE;
  }

  int type = isTruthParticle ? m_classifierParticleType(electron) : m_truthType(electron);
  int origin = isTruthParticle ? m_classifierParticleOrigin(electron) : m_truthOrigin(electron);
  int pdgId = isTruthParticle ? truthParticle->pdgId() : m_truthPdgId(electron);
  if (m_useTruthParticleDecorations && !isTruthParticle)
  {
    type = m_classifierParticleType(*truthParticle);
    origin = m_classifierParticleOrigin(*truthParticle);
  }

  int firstMotherType = m_firstMotherTruthType(electron);
  int firstMotherOrigin = m_firstMotherTruthOrigin(electron);
  int firstMotherPdgId = m_firstMotherPdgId(electron);
  // not in the smart slimming list, thus only in few derivations
  int lastMotherType = m_lastMotherTruthType.isAvailable(electron) ? m_lastMotherTruthType(electron) : -1;
  int lastMotherOrigin = m_lastMotherTruthOrigin.isAvailable(electron) ? m_lastMotherTruthOrigin(electron) : -1;
  int lastMotherPdgId = m_lastMotherPdgId.isAvailable(electron) ? m_lastMotherPdgId(electron) : -1;
  // fallback recorations
  int fallbackType{-1};
  if (m_fallbackTruthType.isAvailable(electron) && m_fallbackDR.isAvailable(electron))
  {
    fallbackType = m_fallbackDR(electron) < 0.05 ? m_fallbackTruthType(electron) : -1;
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Prompt Photon Conversions
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // gamma -> e+ e-
  if (type == MCTruthPartClassifier::BkgElectron
    && (origin == MCTruthPartClassifier::PhotonConv || origin == MCTruthPartClassifier::ElMagProc)
    && firstMotherType == MCTruthPartClassifier::IsoPhoton && firstMotherOrigin == MCTruthPartClassifier::PromptPhot)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // H -> gamma gamma, gamma -> e+ e-
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::PhotonConv
    && firstMotherType == MCTruthPartClassifier::IsoPhoton && firstMotherOrigin == MCTruthPartClassifier::Higgs)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // bkg electrons from bkg photons
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::PhotonConv
    && firstMotherType == MCTruthPartClassifier::UnknownPhoton && firstMotherOrigin == MCTruthPartClassifier::NonDefined)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // bkg photon from UndrPhot; (Here there is a generator level photon (not gen electron ) that later converts)
  if (type == MCTruthPartClassifier::BkgElectron
    && (origin == MCTruthPartClassifier::PhotonConv || origin == MCTruthPartClassifier::ElMagProc )
    && firstMotherType == MCTruthPartClassifier::BkgPhoton && firstMotherOrigin == MCTruthPartClassifier::UndrPhot)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // type = 16 and origin = 38 (again, this is a photon)
  if (type == MCTruthPartClassifier::BkgPhoton && origin == MCTruthPartClassifier::UndrPhot)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // Is an isolated photon
  if (type == MCTruthPartClassifier::IsoPhoton && pdgId == 22)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // electrons from ElMagProc
  // when FSR, a better classification can be made with the fall back vars
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::ElMagProc
    && firstMotherType == MCTruthPartClassifier::UnknownPhoton && firstMotherOrigin == MCTruthPartClassifier::NonDefined)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::ElMagProc
    && firstMotherType == MCTruthPartClassifier::NonIsoPhoton && firstMotherOrigin == MCTruthPartClassifier::FSRPhot)
  {
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // TODO: Message from Otilia: """
  // but it's not clear if these electrons are really
  // "fakes" or they should go in the real category (we don't know from where
  // this photon is coming...). I would say more truth studies should be done.
  // """
  // Hence the warning message...
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::PhotonConv
    && firstMotherType == MCTruthPartClassifier::Unknown && firstMotherOrigin == MCTruthPartClassifier::ZBoson)
  {
    ANA_MSG_WARNING("Electron identified as from a PromptPhotonConversion, "
                    "but this type of electron needs further study!");
    classification = Truth::Type::PromptPhotonConversion;
    return StatusCode::SUCCESS;
  }

  // when always a photon (last mum is a photon, even if the truth PDG is 11 and first mum PDG is 11 ):
  // very likely these are internal conversions;  last_mum_pdgId == 22 important as the cases with last_mum_pdgId == 11 were found to be quite often close to a true electron
  if (type == MCTruthPartClassifier::BkgElectron && firstMotherType == MCTruthPartClassifier::BkgElectron
    && origin == MCTruthPartClassifier::PhotonConv && firstMotherOrigin == MCTruthPartClassifier::PhotonConv
    && std::abs(firstMotherPdgId) == 11 && std::abs(pdgId) == 11)
  {  // electron
    if(lastMotherType == -1 || (lastMotherType == MCTruthPartClassifier::GenParticle && (lastMotherPdgId == 22 || std::abs(lastMotherPdgId) == 11)))
    {
      // lastMotherType == -1 ==>  when the last mother info is not stored in the derivations
      classification = Truth::Type::PromptPhotonConversion;
      return StatusCode::SUCCESS;
    }
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Is muon reco as electron or ele radiated by muons
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if ((type == MCTruthPartClassifier::NonIsoElectron || type == MCTruthPartClassifier::NonIsoPhoton)
    && (origin == MCTruthPartClassifier::Mu || firstMotherOrigin == MCTruthPartClassifier::Mu))
  {
    classification = Truth::Type::ElectronFromMuon;
    return StatusCode::SUCCESS;
  }

  if (type == MCTruthPartClassifier::BkgElectron && firstMotherOrigin == MCTruthPartClassifier::Mu)
  {
    classification = Truth::Type::ElectronFromMuon;
    return StatusCode::SUCCESS;
  }

  if (type == MCTruthPartClassifier::IsoMuon || type == MCTruthPartClassifier::NonIsoMuon || type == MCTruthPartClassifier::BkgMuon || type == MCTruthPartClassifier::UnknownMuon) {
    classification = Truth::Type::ElectronFromMuon;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Tau decays
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // Non-isolated electron/photon from tau decay
  if ((type == MCTruthPartClassifier::NonIsoElectron || type == MCTruthPartClassifier::NonIsoPhoton)
    && origin == MCTruthPartClassifier::TauLep)
  {
    classification = Truth::Type::TauDecay;
    return StatusCode::SUCCESS;
  }

  // tau -> tau gamma, gamma -> e+ e-, etc
  if ((firstMotherType == MCTruthPartClassifier::NonIsoElectron || firstMotherType == MCTruthPartClassifier::NonIsoPhoton)
    && firstMotherOrigin == MCTruthPartClassifier::TauLep)
  {
    classification = Truth::Type::TauDecay;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Light hadron sources
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (type == MCTruthPartClassifier::Hadron || fallbackType == MCTruthPartClassifier::Hadron)
  {
    classification = Truth::Type::LightFlavorDecay;
    return StatusCode::SUCCESS;
  }

  if (firstMotherType == MCTruthPartClassifier::BkgElectron)
  {
    if ((origin == MCTruthPartClassifier::DalitzDec || origin == MCTruthPartClassifier::ElMagProc)
      && (hasLightHadronOrigin(origin) || hasLightHadronOrigin(firstMotherOrigin)))
    {
      classification = Truth::Type::LightFlavorDecay;
      return StatusCode::SUCCESS;
    }
  }

  if (type == MCTruthPartClassifier::BkgElectron)
  {
    if (origin == MCTruthPartClassifier::DalitzDec || firstMotherOrigin == MCTruthPartClassifier::DalitzDec) {
      classification = Truth::Type::LightFlavorDecay;
      return StatusCode::SUCCESS;
    }
    if (hasLightHadronOrigin(origin) || hasLightHadronOrigin(firstMotherOrigin)) {
      classification = Truth::Type::LightFlavorDecay;
      return StatusCode::SUCCESS;
    }
  }

  if (type == MCTruthPartClassifier::BkgPhoton
    && (hasLightHadronOrigin(origin) || hasLightHadronOrigin(firstMotherOrigin)))
  {
    classification = Truth::Type::LightFlavorDecay;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // From B hadron
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (hasBHadronOrigin(origin) || hasBHadronOrigin(firstMotherOrigin))
  {
    classification = Truth::Type::BHadronDecay;
    return StatusCode::SUCCESS;
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // From C hadron
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (type != MCTruthPartClassifier::IsoElectron && (hasCHadronOrigin(origin) || hasCHadronOrigin(firstMotherOrigin)))
  {
    classification = Truth::Type::CHadronDecay;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Prompt / Isolated electrons
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (isPromptElectron(electron, isTruthParticle, truthParticle))
  {
    if (m_separateChargeFlipElectrons && isChargeFlipElectron(electron, isTruthParticle, truthParticle))
    {
      classification = Truth::Type::ChargeFlipIsoElectron;
      return StatusCode::SUCCESS;
    }

    classification = Truth::Type::IsoElectron;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Unknown & known Unknown
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // TODO: See if we want this or not. Now we check if this is something we are
  // able to classify or not. Note that this might be a bit dangerous because
  // the reasons for not having origin and status codes might be complex. The
  // main idea is to weed out things we don't have a hope of classifying due to
  // missing or unknown information.
  static const SG::ConstAccessor<int> statusAcc("status");
  bool stable = (truthParticle != nullptr && statusAcc.isAvailable(*truthParticle)) ? MC::isStable(truthParticle) : false;


  if (origin == MCTruthPartClassifier::NonDefined && firstMotherOrigin == MCTruthPartClassifier::NonDefined)
  {
    if (!stable)
    {
      if ((type == MCTruthPartClassifier::Unknown || type == MCTruthPartClassifier::UnknownPhoton) && firstMotherType == MCTruthPartClassifier::Unknown)
      {
        classification = Truth::Type::KnownUnknown;
        return StatusCode::SUCCESS;
      }
    } else {
      if ((type == MCTruthPartClassifier::Unknown && firstMotherType == MCTruthPartClassifier::Unknown)
        || (type == MCTruthPartClassifier::UnknownElectron && firstMotherType == MCTruthPartClassifier::UnknownElectron))
      {
        classification = Truth::Type::KnownUnknown;
        return StatusCode::SUCCESS;
      }

      if (type == MCTruthPartClassifier::UnknownPhoton)
      {
        classification = Truth::Type::KnownUnknown;
        return StatusCode::SUCCESS;
      }
    }
  }

  // non-iso photons with no info available to classify
  if (type == MCTruthPartClassifier::NonIsoPhoton && origin == MCTruthPartClassifier::FSRPhot && pdgId == 22
    && firstMotherType == 0 && firstMotherOrigin == 0 && firstMotherPdgId == 0)
  {
    if (lastMotherType == -1 || (lastMotherType == 0 && lastMotherOrigin == 0 && lastMotherPdgId == 0))
    { // last_firstMotherType == -1 ==>  when the last_mum info is not stored in the derivations
      classification = Truth::Type::KnownUnknown;
      return StatusCode::SUCCESS;
    }
  }

  ANA_MSG_WARNING("Electron type unknown: type = " << type << ", origin = " << origin);

  // debug printout
  if (truthParticle != nullptr)
  {
    const xAOD::TruthParticle *parent = truthParticle;
    ATH_MSG_DEBUG("Unknown particle decay chain (approximate):");
    std::string out = "\t";
    while (parent != nullptr)
    {
      out.append(std::to_string(parent->pdgId()));
      parent = parent->parent(0); //AV: Note the ambiguity: only the first particle is considered.
      if (parent) out.append(" -> ");
    }
    ATH_MSG_DEBUG(out);
  }

  classification = Truth::Type::Unknown;
  return StatusCode::SUCCESS;
}


StatusCode TruthClassificationTool::classifyMuon(const xAOD::IParticle &muon,
                                                 Truth::Type &classification) const
{
  

  // Check if xAOD::TruthParticle or if not if it has the TruthParticleLink
  const xAOD::TruthParticle *truthParticle
    = dynamic_cast<const xAOD::TruthParticle *> (&muon);
  bool isTruthParticle{};
  if (truthParticle == nullptr)
  {
    // need to find the truth particle
    truthParticle = xAOD::TruthHelpers::getTruthParticle(muon);
  }
  else
  {
    isTruthParticle = true;
  }

  int type = isTruthParticle ? m_classifierParticleType(muon) : m_truthType(muon);
  int origin = isTruthParticle ? m_classifierParticleOrigin(muon) : m_truthOrigin(muon);
  if (m_useTruthParticleDecorations && !isTruthParticle && truthParticle != nullptr)
  {
    type = m_classifierParticleType(*truthParticle);
    origin = m_classifierParticleOrigin(*truthParticle);
  }

  // fallback recorations
  int fallbackType{-1};
  if (m_fallbackTruthType.isAvailable(muon) && m_fallbackDR.isAvailable(muon))
  {
    fallbackType = m_fallbackDR(muon) < 0.05 ? m_fallbackTruthType(muon) : -1;
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // muons from taus
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (type == MCTruthPartClassifier::NonIsoMuon && origin == MCTruthPartClassifier::TauLep)
  {
    classification = Truth::Type::TauDecay;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Light hadron sources
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (type == MCTruthPartClassifier::BkgMuon && hasLightHadronOrigin(origin))
  {
    classification = Truth::Type::LightFlavorDecay;
    return StatusCode::SUCCESS;
  }

  if (type == MCTruthPartClassifier::Hadron || fallbackType == MCTruthPartClassifier::Hadron )
  {
    classification = Truth::Type::LightFlavorDecay;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // From B hadron
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (hasBHadronOrigin(origin))
  {
    classification = Truth::Type::BHadronDecay;
    return StatusCode::SUCCESS;
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // From C hadron
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (type != MCTruthPartClassifier::IsoMuon && hasCHadronOrigin(origin))
  {
    classification = Truth::Type::CHadronDecay;
    return StatusCode::SUCCESS;
  }
  // TODO:: There is a comment in the example code about J/psi but there is a
  // separate origin code for that: `MCTruthPartClassifier::JPsi == 28.` --> this might not be in all samples/generators?


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // prompt muons
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Check if the type of muon is IsoMuon(6) and whether the origin
  // of the muon is from a prompt source
  static const std::set<int> promptOrigin({
      MCTruthPartClassifier::SingleMuon,  // Single muon (origin = 2) from muon twiki
      MCTruthPartClassifier::top,
      MCTruthPartClassifier::WBoson,
      MCTruthPartClassifier::ZBoson,
      MCTruthPartClassifier::Higgs,
      MCTruthPartClassifier::HiggsMSSM,
      MCTruthPartClassifier::SUSY,
      MCTruthPartClassifier::DiBoson,
      MCTruthPartClassifier::CCbarMeson, // PromptQuarkoniumDecay
      MCTruthPartClassifier::BBbarMeson,
      MCTruthPartClassifier::HeavyBoson
  });
  if (type == MCTruthPartClassifier::IsoMuon && isInSet(origin, promptOrigin))
  {
    //separate charge-flip muons
    if (m_separateChargeFlipMuons && isChargeFlipMuon(muon, isTruthParticle, truthParticle))
    {
      classification = Truth::Type::ChargeFlipMuon;
      return StatusCode::SUCCESS;
    }
    classification = Truth::Type::PromptMuon;
    return StatusCode::SUCCESS;
  }


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Known Unknown
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  if (type == MCTruthPartClassifier::UnknownMuon && origin == MCTruthPartClassifier::NonDefined)
  {
    classification = Truth::Type::KnownUnknown;
    return StatusCode::SUCCESS;
  }
  
  static const SG::ConstAccessor<int> statusAcc("status");
  bool stable = (truthParticle != nullptr && statusAcc.isAvailable(*truthParticle)) ? MC::isStable(truthParticle) : false;

  if (!stable) {
    if (type == MCTruthPartClassifier::Unknown && origin == MCTruthPartClassifier::NonDefined)
    { // Data
      classification = Truth::Type::KnownUnknown;
      return StatusCode::SUCCESS;
    }
    if (type == -99999 && origin == -99999)
    { // MC - no status = 1 truth particle associated with the primary track
      classification = Truth::Type::KnownUnknown;
      return StatusCode::SUCCESS;
    }
  }
  
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // NonMuonlike
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Check if the type of muon is related to electrons, ie IsoElectron (2) or NonIsoElectron (3) or BkgElectron (4)
  if (type == MCTruthPartClassifier::IsoElectron || type == MCTruthPartClassifier::BkgElectron || type == MCTruthPartClassifier::NonIsoElectron)
  {
    classification = Truth::Type::NonMuonlike;
    return StatusCode::SUCCESS;
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TauLike, BHadLike, CHadLike
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Check if the matched parent of truth muon fall into these 3 xxxLike
  // Only truth muon with one matched parent is considered
  // If parent is tau, the muons is TauLike
  // If parent is bhadron/b quark, the muons is BHadLike
  // If parent is chadron/c quark, the muons is CHadLike
  if (type == MCTruthPartClassifier::Unknown && origin == MCTruthPartClassifier::NonDefined){
    if( truthParticle != nullptr && truthParticle->nParents() == 1 && truthParticle->parent(0) != nullptr){
      const xAOD::TruthParticle *parent = truthParticle->parent(0);
      int parent_pdgid = parent->pdgId(); 
      if(parent->isTau()){
        classification = Truth::Type::TauDecayLike;
        ATH_MSG_WARNING("Muon type taudecaylike: type = " << type << ", origin = " << origin << ", parent = " << parent_pdgid) ; 
        return StatusCode::SUCCESS;
      }
      if(parent->isBottomHadron() || parent->hasBottom()){
        classification = Truth::Type::BHadronDecayLike;
        ATH_MSG_WARNING("Muon type bhadrondecaylike: type = " << type << ", origin = " << origin << ", parent = " << parent_pdgid) ; 
        return StatusCode::SUCCESS;
      }
      if(parent->isCharmHadron() || parent->hasCharm()){ 
        if( (parent_pdgid / 1000) % 10 != 0 || (parent_pdgid / 100) % 10 != 4 || (parent_pdgid / 10) % 10 != 4){ // to exclude ccbarmeson 
          classification = Truth::Type::CHadronDecayLike;
          ATH_MSG_WARNING("Muon type chadrondecaylike: type = " << type << ", origin = " << origin << ", parent = " << parent_pdgid) ; 
          return StatusCode::SUCCESS;
        } 
      }
    }
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Unknown
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  classification = Truth::Type::Unknown;
  ATH_MSG_WARNING("Muon type unknown: type = " << type << ", origin = " << origin) ; 

  return StatusCode::SUCCESS;
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
}

bool TruthClassificationTool::isPromptElectron(const xAOD::IParticle &electron,
                                               bool isTruthParticle,
                                               const xAOD::TruthParticle *truthParticle) const
{
  

  int type = isTruthParticle ? m_classifierParticleType(electron) : m_truthType(electron);
  int origin = isTruthParticle ? m_classifierParticleOrigin(electron) : m_truthOrigin(electron);
  if (m_useTruthParticleDecorations && !isTruthParticle && truthParticle != nullptr)
  {
    type = m_classifierParticleType(*truthParticle);
    origin = m_classifierParticleOrigin(*truthParticle);
  }

  // Electron is IsoElectron - return true
  if (type == MCTruthPartClassifier::IsoElectron)
  {
    return true;
  }

  int pdgId = isTruthParticle ? truthParticle->pdgId() : m_truthPdgId(electron);
  int firstMotherType = m_firstMotherTruthType(electron);
  int firstMotherOrigin = m_firstMotherTruthOrigin(electron);

  // Adding these cases from ElectronEfficiencyHelpers
  if (firstMotherType == MCTruthPartClassifier::IsoElectron && std::abs(m_firstMotherPdgId(electron)) == 11)
  {
    return true;
  }

  // FSR photons from electrons
  if (origin == MCTruthPartClassifier::FSRPhot && type == MCTruthPartClassifier::NonIsoPhoton && std::abs(pdgId) == 11)
  {
    return true;
  }

  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::PhotonConv
    && firstMotherType == MCTruthPartClassifier::NonIsoPhoton && firstMotherOrigin == MCTruthPartClassifier::FSRPhot
    && std::abs(pdgId) == 11)
  {
    return true;
  }

  // If we reach here then it is not a prompt electron
  return false;
}


bool TruthClassificationTool::isChargeFlipElectron(const xAOD::IParticle &electron,
                                                   bool isTruthParticle,
                                                   const xAOD::TruthParticle *truthParticle) const
{
  

  int type = isTruthParticle ? m_classifierParticleType(electron) : m_truthType(electron);
  int origin = isTruthParticle ? m_classifierParticleOrigin(electron) : m_truthOrigin(electron);
  int pdgId = isTruthParticle && truthParticle ? truthParticle->pdgId() : m_truthPdgId(electron);
  if (m_useTruthParticleDecorations && !isTruthParticle && truthParticle != nullptr)
  {
    type = m_classifierParticleType(*truthParticle);
    origin = m_classifierParticleOrigin(*truthParticle);
  }

  int firstMotherType = m_firstMotherTruthType(electron);
  int firstMotherOrigin = m_firstMotherTruthOrigin(electron);
  int firstMotherPdgId = m_firstMotherPdgId(electron);

  // not consider FSR photons from electrons (the photon has no charge)
  if (origin == MCTruthPartClassifier::FSRPhot && type == MCTruthPartClassifier::NonIsoPhoton && std::abs(pdgId) == 11)
  {
    return false;
  }
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::PhotonConv
    && firstMotherType == MCTruthPartClassifier::NonIsoPhoton && firstMotherOrigin == MCTruthPartClassifier::FSRPhot
    && std::abs(pdgId) == 11)
  {
    return false;
  }

  // bkg electrons with no additional info to help us classify them FSR -- not in the charge flip category
  if (type == MCTruthPartClassifier::BkgElectron && origin == MCTruthPartClassifier::PhotonConv
    && firstMotherType == MCTruthPartClassifier::BkgElectron && firstMotherOrigin == MCTruthPartClassifier::PhotonConv
    && std::abs(pdgId) == 11)
  {
    return false;
  }

  if (isTruthParticle)
  {
    if (truthParticle->charge() != 0)
    {
      return (firstMotherPdgId * truthParticle->charge()) > 0;
    }
  }
  else
  {
    const xAOD::Electron &xAODElectron = *dynamic_cast<const xAOD::Electron *> (&electron);
    if (xAODElectron.charge() != 0)
    {
      return (firstMotherPdgId * xAODElectron.charge()) > 0;
    }
  }

  return (firstMotherPdgId * (-pdgId)) > 0;
}


bool TruthClassificationTool::isChargeFlipMuon(const xAOD::IParticle &muon,
                                               bool isTruthParticle,
                                               const xAOD::TruthParticle *truthParticle) const
{
  if (isTruthParticle)
  {
    return false;
  }

  if (truthParticle != nullptr && xAOD::P4Helpers::isInDeltaR(*truthParticle, muon, 0.025))
  {
    const xAOD::Muon &xAODMuon = *dynamic_cast<const xAOD::Muon *> (&muon);
    return (truthParticle->charge() * xAODMuon.charge()) < 0;
  }

  ANA_MSG_DEBUG("Cannot find associated truth-particle... assuming muon has correct charge");
  return false;
}


bool TruthClassificationTool::hasBHadronOrigin(int origin) const
{
  static const std::set<int> b_hadrons({
    MCTruthPartClassifier::BottomMeson,
    MCTruthPartClassifier::BBbarMeson,
    MCTruthPartClassifier::BottomBaryon,
  });
  return isInSet(origin, b_hadrons);
}


bool TruthClassificationTool::hasCHadronOrigin(int origin) const {
  static const std::set<int> c_hadrons({
    MCTruthPartClassifier::CharmedMeson,
    MCTruthPartClassifier::CCbarMeson,
    MCTruthPartClassifier::CharmedBaryon,
  });
  return isInSet(origin, c_hadrons);
}


bool TruthClassificationTool::hasLightHadronOrigin(int origin) const {
  static const std::set<int> light_source({
    MCTruthPartClassifier::PiZero,
    MCTruthPartClassifier::LightMeson,
    MCTruthPartClassifier::StrangeMeson,
    MCTruthPartClassifier::LightBaryon,
    MCTruthPartClassifier::StrangeBaryon,
    MCTruthPartClassifier::PionDecay,
    MCTruthPartClassifier::KaonDecay,
  });
  return isInSet(origin, light_source);
}
