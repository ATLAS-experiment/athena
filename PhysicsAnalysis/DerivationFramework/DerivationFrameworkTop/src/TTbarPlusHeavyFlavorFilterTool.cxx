/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTop/TTbarPlusHeavyFlavorFilterTool.h"
#include "TruthUtils/HepMCHelpers.h"

namespace DerivationFramework{

TTbarPlusHeavyFlavorFilterTool::TTbarPlusHeavyFlavorFilterTool(const std::string& t, const std::string& n, const IInterface* p)
  : AthAlgTool(t,n,p)
{

  declareProperty("MCCollectionName",m_mcName="TruthEvents");
  declareProperty("UsePileUp",m_usePileUp=false); /// of course doesn't work if pu is not available e.g. generation level
  declareProperty("UseFinalStateHadrons",m_useFinalStateHadrons=false);
  declareProperty("BpTMinCut",m_bPtMinCut=5000.); /// MeV
  declareProperty("BetaMaxCut",m_bEtaMaxCut=3.);
  declareProperty("CpTMinCut",m_cPtMinCut=5000.); /// MeV
  declareProperty("CetaMaxCut",m_cEtaMaxCut=3.);
  declareProperty("BMultiplicityCut",m_bMultiCut=1); /// >=
  declareProperty("CMultiplicityCut",m_cMultiCut=1); /// >=
  declareProperty("ExcludeBFromTTbar",m_excludeBFromTop=true);  /// use IS quark
  declareProperty("ExcludeCFromTTbar",m_excludeCFromTop=true); /// use IS quark

}

 TTbarPlusHeavyFlavorFilterTool::~TTbarPlusHeavyFlavorFilterTool(){}

StatusCode TTbarPlusHeavyFlavorFilterTool::initialize() {
  ATH_MSG_INFO("Initialize " );
  return StatusCode::SUCCESS;
}

StatusCode TTbarPlusHeavyFlavorFilterTool::finalize() {
  return StatusCode::SUCCESS;
}


int TTbarPlusHeavyFlavorFilterTool::filterFlag() const{

  int nB=0;
  int nC=0;

  int nBtop=0;
  int nCtop=0;

  const xAOD::TruthEventContainer* xTruthEventContainer = 0;
  if (evtStore()->retrieve(xTruthEventContainer,m_mcName).isFailure()) {
    ATH_MSG_WARNING("could not retrieve TruthEventContainer " <<m_mcName);
    return -1;
  }

  for ( const auto* truthevent : *xTruthEventContainer ) {

    for(unsigned int i = 0; i < truthevent->nTruthParticles(); i++){

      const xAOD::TruthParticle* part = truthevent->truthParticle(i);
      // In release 21 we'll have a thinned truth record in the AODs.
      // Specifically, geant particle are removed in most cases. The subsequent
      // nullptr check is supposed to catch these truth particles,
      // unfortunately however, there's no way to check whether this truth
      // particle would have had an impact on what we do further down.
      if ( !part){
          // We could possibly also use break since the thinned truth particles
          // in principle should have no simulation particles.
          continue;
      }

      if (HepMC::is_simulation_particle(part)) break;

      bool isbquark=false;
      bool iscquark=false;

      bool isbhadron=false;
      bool ischadron=false;

      int pdgid = std::abs(part->pdgId());

      //// don't loose time checking all if one found
      if ( MC::isBottom(pdgid) ) {
        isbquark=true;
      }
      else if ( MC::isCharm(pdgid) ) {
        iscquark=true;
      }
      else if ( MC::isBottomHadron(part) && !HepMC::is_simulation_particle(part) ) {
        isbhadron=true;
      }
      else if ( MC::isCharmHadron(part) && !HepMC::is_simulation_particle(part) ) {
        ischadron=true;
      }
      else {
        continue;
      }

      if( (isbquark || isbhadron) && !passBSelection(part) ) continue;
      if( (iscquark || ischadron) && !passCSelection(part) ) continue;

      if(isbhadron || ischadron){
        if(!isInitialHadron(part) && !m_useFinalStateHadrons) continue;
        if(!isFinalHadron(part) && m_useFinalStateHadrons) continue;
      }

      if(m_excludeBFromTop && isbquark){
        bool islooping = isLooping(part);
        if (isDirectlyFromTop(part)) ++nBtop;
        if (isDirectlyFromWTop(part, islooping)) ++nBtop;
      }
      if(m_excludeCFromTop && iscquark){
        bool islooping = isLooping(part);
        if(isDirectlyFromTop(part))++nCtop;
        if(isDirectlyFromWTop(part, islooping))++nCtop;
      }

      bool ischadronfromb = isCHadronFromB(part);

      if(isbhadron) ++nB;
      if(ischadron && !ischadronfromb) ++nC;

    } /// loop on particles

    if(!m_usePileUp){
      break;  /// only one hard scatter break
    }

  } /// loop on events (only one at evgen - no PU)


  int nAddB=nB;
  if(m_excludeBFromTop){
    nAddB-=nBtop;
  }

  int nAddC=nC;
  if(m_excludeCFromTop){
    nAddC-=nCtop;
  }

  int flavortype=0;

  if(nAddC>=m_cMultiCut){
    flavortype=4;
  }

  if(nAddB>=m_bMultiCut){
    flavortype=5;
  }

  return flavortype;

}


bool TTbarPlusHeavyFlavorFilterTool::passBSelection(const xAOD::TruthParticle* part) const{

  double pt = part->pt();
  double eta = fabs(part->eta());

  if(pt <m_bPtMinCut) return false;
  if(eta > m_bEtaMaxCut) return false;

  return true;

}

bool TTbarPlusHeavyFlavorFilterTool::passCSelection(const xAOD::TruthParticle* part) const{

  double pt = part->pt();
  double eta = fabs(part->eta());
  if (pt < m_cPtMinCut) return false;
  if (eta > m_cEtaMaxCut) return false;
  return true;
}

bool TTbarPlusHeavyFlavorFilterTool::isInitialHadron(const xAOD::TruthParticle* part) const{

  int qtype = std::abs(MC::leadingQuark(part));
  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
      if (HepMC::is_same_particle(part,parent) ) continue; /// protection for sherpa 
      int mothertype = std::abs(MC::leadingQuark(parent));
      if (mothertype == qtype ){
        return false;
      }
    }

  return true;
}


bool TTbarPlusHeavyFlavorFilterTool::isFinalHadron(const xAOD::TruthParticle* part) const {

  int qtype = std::abs(MC::leadingQuark(part));
  for(unsigned j = 0; j < part->nChildren(); j++){
    const xAOD::TruthParticle* child = part->child(j);
    if (HepMC::is_same_particle(part,child) ) continue; /// protection for sherpa
    int childtype = std::abs(MC::leadingQuark(child));
    if (childtype == qtype ){
      return false;
    }
  }
  return true;
}

bool TTbarPlusHeavyFlavorFilterTool::isQuarkFromHadron(const xAOD::TruthParticle* part) const {

  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
    if (HepMC::is_same_particle(part,parent) ) continue; /// protection for sherpa
    int mothertype = std::abs(MC::leadingQuark(parent));
    if ( MC::isCharm(mothertype) || MC::isBottom(mothertype) ){
      return true;
    }
    if (isQuarkFromHadron(parent)) return true;
  }
  return false;
}

bool TTbarPlusHeavyFlavorFilterTool::isCHadronFromB(const xAOD::TruthParticle* part) const {

  if(!MC::isCharmHadron(part)||HepMC::is_simulation_particle(part)) return false;

  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
    if (HepMC::is_same_particle(part,parent) ) continue; /// protection for sherpa
    if (MC::isBottomHadron(parent)&&!HepMC::is_simulation_particle(parent) ){
      return true;
    }
    if (MC::isCharmHadron(parent)&&!HepMC::is_simulation_particle(parent)){
      if(isCHadronFromB(parent)) return true;
    }
  }

  return false;
}

bool TTbarPlusHeavyFlavorFilterTool::isLooping(const xAOD::TruthParticle* part, std::set<const xAOD::TruthParticle*> init_part) const {
  init_part.insert(part);
  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
    if (init_part.find(parent) != init_part.end()) return true;
    if (isLooping(parent, init_part)) return true;
  }
  return false;
}

const xAOD::TruthParticle*  TTbarPlusHeavyFlavorFilterTool::findInitial(const xAOD::TruthParticle* part, bool looping) const {

  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
    if (HepMC::is_same_particle(part,parent)) continue; /// protection for sherpa
    if (part->pdgId() == parent->pdgId() ){
      return findInitial(parent, looping);
    }
  }
  return part;
}

bool TTbarPlusHeavyFlavorFilterTool::isFromTop(const xAOD::TruthParticle* part, bool looping) const{
  const xAOD::TruthParticle* initpart = findInitial(part, looping);
  return isDirectlyFromTop(initpart);
}

bool TTbarPlusHeavyFlavorFilterTool::isDirectlyFromTop(const xAOD::TruthParticle* part) const{

  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
    if (HepMC::is_same_particle(part,parent) ) continue; /// protection for sherpa
    if ( MC::isTop(parent->pdgId()) ) return true;
  }
  return false;
}

bool TTbarPlusHeavyFlavorFilterTool::isDirectlyFromWTop(const xAOD::TruthParticle * part, bool looping) const{

  for(unsigned int i=0; i<part->nParents(); ++i){
    const xAOD::TruthParticle* parent = part->parent(i);
    if (HepMC::is_same_particle(part,parent)) continue; /// protection for sherpa
    if (MC::isW(parent) ){
      if (isFromTop(parent, looping) ) return true;
    }
  }
  return false;
}


} ///namespace

