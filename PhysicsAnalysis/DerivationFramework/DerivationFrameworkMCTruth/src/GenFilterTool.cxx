/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Class header file
#include "GenFilterTool.h"

// EDM includes
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "StoreGate/ReadDecorHandle.h"

// Tool handle interface
#include "MCTruthClassifier/IMCTruthClassifier.h"

namespace DerivationFramework {

  using namespace MCTruthPartClassifier;

  static bool isFromWZTau(unsigned int orig) {
    switch(orig) {
    case ParticleOrigin::WBoson:
    case ParticleOrigin::ZBoson:
    case ParticleOrigin::TauLep:
      return true;
    default:
      return false;
    }
    return false;
  }


  GenFilterTool::GenFilterTool(const std::string& t, const std::string& n, const IInterface* p)
    : base_class(t,n,p) {


  }


  GenFilterTool::~GenFilterTool() = default;

  bool GenFilterTool::isPrompt( const xAOD::TruthParticle* tp ) const
  {
    ParticleOrigin orig = m_classif->particleTruthClassifier( tp ).second;
    ATH_MSG_VERBOSE("Particle has origin " << orig);

    switch(orig) {
    case NonDefined:
    case PhotonConv:
    case DalitzDec:
    case ElMagProc:
    case Mu:
    case LightMeson:
    case StrangeMeson:
    case CharmedMeson:
    case BottomMeson:
    case CCbarMeson:
    case JPsi:
    case BBbarMeson:
    case LightBaryon:
    case StrangeBaryon:
    case CharmedBaryon:
    case BottomBaryon:
    case PionDecay:
    case KaonDecay:
      return false;
    default:
      break;
    }
    return true;
  }
  StatusCode GenFilterTool::initialize() {
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_mcKey.initialize());
    ATH_CHECK(m_truthJetsKey.initialize());
    ATH_CHECK(m_truthFatJetsKey.initialize());
    ATH_CHECK(m_decorKeys.initialize());
    ATH_CHECK(m_dec_genFiltHTKey.initialize());
    ATH_CHECK(m_dec_genFiltHTinclNuKey.initialize());
    ATH_CHECK(m_dec_genFiltMETKey.initialize());
    ATH_CHECK(m_dec_genFiltPTZKey.initialize());
    ATH_CHECK(m_dec_genFiltFatJKey.initialize());
    ATH_CHECK(m_mcReadDecor.initialize());
    return StatusCode::SUCCESS;
  }
  StatusCode GenFilterTool::addBranches() const{
    ATH_MSG_VERBOSE("GenFilterTool::addBranches()");
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoKey, ctx};
    if (!eventInfo.isValid()) {
      ATH_MSG_ERROR("could not retrieve event info " <<m_eventInfoKey.fullKey());
      return StatusCode::FAILURE;
    }

   
    float genFiltHT{0.f}, genFiltHTinclNu{0.f}, genFiltMET{0.f}, genFiltPTZ{0.f}, genFiltFatJ{0.f};
    ATH_CHECK( getGenFiltVars(ctx, genFiltHT, genFiltHTinclNu, genFiltMET, genFiltPTZ, genFiltFatJ) );

    ATH_MSG_DEBUG("Computed generator filter quantities: HT " << genFiltHT/1e3 << ", HTinclNu " << genFiltHTinclNu/1e3 << ", MET " << genFiltMET/1e3 << ", PTZ " << genFiltPTZ/1e3 << ", FatJ " << genFiltFatJ/1e3 );

    SG::makeHandle<float>(m_dec_genFiltHTKey, ctx)(*eventInfo) = genFiltHT;
    SG::makeHandle<float>(m_dec_genFiltHTinclNuKey, ctx)(*eventInfo) = genFiltHTinclNu;
    SG::makeHandle<float>(m_dec_genFiltMETKey, ctx)(*eventInfo) = genFiltMET;
    SG::makeHandle<float>(m_dec_genFiltPTZKey, ctx)(*eventInfo) = genFiltPTZ;
    SG::makeHandle<float>(m_dec_genFiltFatJKey, ctx)(*eventInfo) = genFiltFatJ;

    return StatusCode::SUCCESS;
  }

  StatusCode GenFilterTool::getGenFiltVars(const EventContext& ctx, float& genFiltHT, float& genFiltHTinclNu, float& genFiltMET, float& genFiltPTZ, float& genFiltFatJ) const {
    // Get jet container out
    
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int> mcParticleOrigin{m_mcReadDecor, ctx} ;
    if (!mcParticleOrigin.isValid()) {
      ATH_MSG_ERROR("WARNING could not retrieve TruthParticleContainer " <<m_mcKey.fullKey());
      return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::JetContainer> truthjets{m_truthJetsKey, ctx};
    if (!truthjets.isValid()){
      ATH_MSG_ERROR( "No xAOD::JetContainer found in StoreGate with key " << m_truthJetsKey );
      return StatusCode::FAILURE;
    }

    // Get HT
    genFiltHT = 0.;
    genFiltHTinclNu = 0.; // this HT definition includes neutrinos from W/Z/tau
    for (const auto *const tj : *truthjets) {
      if ( tj->pt()>m_MinJetPt && std::abs(tj->eta())<m_MaxJetEta ) {
        ATH_MSG_VERBOSE("Adding truth jet with pt " << tj->pt()
                        << ", eta " << tj->eta()
                        << ", phi " << tj->phi()
                        << ", nconst = " << tj->numConstituents());
        genFiltHT += tj->pt();
        genFiltHTinclNu += tj->pt();
      }
    }

    // Get MET and add leptons to HT
    float MEx(0.), MEy(0.);
    for (const auto *const tp : *mcParticleOrigin){
      int pdgid = tp->pdgId();
      if (HepMC::is_simulation_particle(tp)) continue; // Particle is from G4
      if (MC::isZeroEnergyPhoton(tp)) continue; // Work around for an old generator bug
      if ( !MC::isStable(tp)) continue; // Stable!

      if ((MC::isElectron(pdgid) || MC::isMuon(pdgid)) && tp->pt()>m_MinLepPt && std::abs(tp->eta())<m_MaxLepEta) {
        if( isPrompt(tp) ) {
          ATH_MSG_VERBOSE("Adding prompt lepton " << tp);
          genFiltHT += tp->pt();
          genFiltHTinclNu += tp->pt();
        }
      }

      // include neutrinos from W/Z/Tau in one of the HT definitions
      // this corresponds to a subset of HT-sliced samples where the HT filter
      // was configured to include these particles
      unsigned int orig = mcParticleOrigin (*tp);
      if (tp->isNeutrino() && isFromWZTau(orig)) {
        ATH_MSG_VERBOSE("Adding neutrino from W/Z/Tau with pt " << tp->pt()
                        << ", eta " << tp->eta()
                        << ", phi " << tp->phi()
                        << ", status " << tp->status()
                        << ", pdgId " << pdgid);
        genFiltHTinclNu += tp->pt();
      }

      if (MC::isSpecialNonInteracting(tp) && isPrompt(tp) ) {
        ATH_MSG_VERBOSE("Found prompt nonInteracting particle with pt " << tp->pt()
                        << ", eta " << tp->eta()
                        << ", phi " << tp->phi()
                        << ", status " << tp->status()
                        << ", pdgId " << pdgid);
        MEx += tp->px();
        MEy += tp->py();
      }
    }
    genFiltMET = sqrt(MEx*MEx+MEy*MEy);

    // Get PTZ
    float PtZ(.0);
    float MinPt_PTZ(5000.), MaxEta_PTZ(5.0), MinMass_PTZ(20000.), MaxMass_PTZ(14000000.);
    bool AllowElecMu_PTZ = false;
    bool AllowSameCharge_PTZ = false;
    for (const xAOD::TruthParticle* pitr1 : *mcParticleOrigin){
      const int pdgId1 = pitr1->pdgId();
      if (HepMC::is_simulation_particle(pitr1)) continue;
      if (!MC::isStable(pitr1)) continue;
      // Pick electrons or muons with Pt > MinPt_PTZ and |eta| < m_maxEta
      if (MC::isElectron(pdgId1) || MC::isMuon(pdgId1)) {
        if (pitr1->pt() >= MinPt_PTZ && std::abs(pitr1->eta()) <= MaxEta_PTZ){
          for (const xAOD::TruthParticle* pitr2 : *mcParticleOrigin){
            if (pitr2==pitr1) continue;
            if (HepMC::is_simulation_particle(pitr2)) continue;
            if (!MC::isStable(pitr2)) continue;
            const int pdgId2 = pitr2->pdgId();
            // Pick electrons or muons with Pt > MinPt_PTZ and |eta| < MaxEta_PTZ
            // If AllowSameCharge_PTZ is not true only pick those with opposite charge to the first particle
            // If AllowElecMu_PTZ is true allow also Z -> emu compinations (with charge requirements as above)
            if (!(MC::isElectron(pdgId2) || MC::isMuon(pdgId2))) continue;
            if ( ( AllowSameCharge_PTZ  && ( AllowElecMu_PTZ || std::abs(pdgId2) == std::abs(pdgId1) ) ) ||
                 ( !AllowSameCharge_PTZ && ( pdgId2 == -1*pdgId1 || (AllowElecMu_PTZ && MC::charge3(pdgId1) == MC::charge3(pdgId2) ) ) )
                 ) {
              if (pitr2->pt() >= MinPt_PTZ && std::abs(pitr2->eta()) <= MaxEta_PTZ){
                double invMass = (pitr1->p4()+pitr2->p4()).M();
                double dilepPt = (pitr1->p4()+pitr2->p4()).Pt();
                // Only consider pair that fall in the mass window
                if (MinMass_PTZ < invMass && invMass < MaxMass_PTZ) {
                  if (dilepPt > PtZ) PtZ = dilepPt;
                }
              }
            }
          }
        }
      }
    }
    genFiltPTZ = PtZ;

   //Get FatJ
   // Get correct jet container
   SG::ReadHandle<xAOD::JetContainer> truthjets10{m_truthFatJetsKey, ctx} ;
   if ( !truthjets10.isValid()){
     ATH_MSG_ERROR( "No xAOD::JetContainer found in StoreGate with key "<<m_truthFatJetsKey.fullKey() );
     return StatusCode::FAILURE;
   }
   genFiltFatJ=0.;
   for (const auto *const j : *truthjets10) {
     if (j->pt()>genFiltFatJ) genFiltFatJ=j->pt();
   }


    return StatusCode::SUCCESS;
  }


} /// namespace
