/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorFilters/Common.h"
#include "TruthUtils/TruthClassifiers.h"
namespace Common {
bool prompt( const xAOD::TruthParticle* part,ToolHandle<IMCTruthClassifier>& classif ) {
    MCTruthPartClassifier::ParticleOrigin orig = classif->particleTruthClassifier( part ).second;
      
    switch(orig) {
    case MCTruthPartClassifier::NonDefined:
    case MCTruthPartClassifier::PhotonConv:
    case MCTruthPartClassifier::DalitzDec:
    case MCTruthPartClassifier::ElMagProc:
    case MCTruthPartClassifier::Mu:
    case MCTruthPartClassifier::LightMeson:
    case MCTruthPartClassifier::StrangeMeson:
    case MCTruthPartClassifier::CharmedMeson:
    case MCTruthPartClassifier::BottomMeson:
    case MCTruthPartClassifier::CCbarMeson:
    case MCTruthPartClassifier::JPsi:
    case MCTruthPartClassifier::BBbarMeson:
    case MCTruthPartClassifier::LightBaryon:
    case MCTruthPartClassifier::StrangeBaryon:
    case MCTruthPartClassifier::CharmedBaryon:
    case MCTruthPartClassifier::BottomBaryon:
    case MCTruthPartClassifier::PionDecay:
    case MCTruthPartClassifier::KaonDecay: 
      return false;
    default:
      break;
    }
    return true;
  }


static bool fromWZ(const HepMC::ConstGenParticlePtr& part )
{
  // Loop through parents
  // Hit a hadron -> return false
  // Hit a parton -> return true
  //   This catch is important - we *cannot* look explicitly for the W or Z, because some
  //    generators do not include the W or Z in the truth record (like Sherpa)
  //   This code, like the code before it, really assumes one incoming particle per vertex...
  if (!part->production_vertex()) return false;
  for (const auto&  iter: part->production_vertex()->particles_in()){
    int parent_pdgid = iter->pdg_id();
    if ( MC::isW(parent_pdgid) || MC::isZ(parent_pdgid) ) return true;
    if ( MC::isHadron( parent_pdgid ) ) return false;
    if ( MC::isQuark( parent_pdgid ) ) return true;
    if ( parent_pdgid == part->pdg_id() ) return fromWZ( iter );
  }
  return false;
}

static bool fromTau(const HepMC::ConstGenParticlePtr& part )
{
  // Loop through parents
  // Find a tau -> return true
  // Find a hadron or parton -> return false
  //   This code, like the code before it, really assumes one incoming particle per vertex...
  if (!part->production_vertex()) return false;
  for (const auto& iter: part->production_vertex()->particles_in()){
    int parent_pdgid = iter->pdg_id();
    if ( MC::isTau( parent_pdgid ) ) return true;
    if ( MC::isHadron( parent_pdgid ) || MC::isQuark( parent_pdgid ) ) return false;
    if ( parent_pdgid == part->pdg_id() ) return fromTau( iter );
  }
  return false;
}

bool fromWZorTau(const HepMC::ConstGenParticlePtr& part ) { return fromWZ(part)||fromTau(part); }

 
void classify(ToolHandle<IMCTruthClassifier>& classif,
        const xAOD::TruthParticle * theParticle,
        unsigned int& particleOutCome,
        unsigned int& result,
        int& hadron_pdg,
        unsigned int& particleType,
        unsigned int& particleOrigin )
  {
       std::tie(particleType, particleOrigin, std::ignore, particleOutCome) = classif->particleTruthClassifier_full(theParticle);
       auto tpl = MCTruthPartClassifier::defOrigOfParticle(theParticle);
       result = (unsigned int)std::get<0>(tpl);
       hadron_pdg = std::get<1>(tpl)?std::get<1>(tpl)->pdg_id():0;
  }
}
