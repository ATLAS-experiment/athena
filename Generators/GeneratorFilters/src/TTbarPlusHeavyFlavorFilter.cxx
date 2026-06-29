/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/TTbarPlusHeavyFlavorFilter.h"
#include "AtlasHepMC/Relatives.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GaudiKernel/MsgStream.h"

//--------------------------------------------------------------------------
TTbarPlusHeavyFlavorFilter::TTbarPlusHeavyFlavorFilter(const std::string& fname, 
						 ISvcLocator* pSvcLocator)
  : GenFilter(fname,pSvcLocator)
 {

  declareProperty("UseFinalStateHadrons",m_useFinalStateHadrons=true);
  declareProperty("SelectB",m_selectB=false);
  declareProperty("SelectC",m_selectC=false);
  declareProperty("SelectL",m_selectL=false);
  declareProperty("BpTMinCut",m_bPtMinCut=5000.); /// MeV
  declareProperty("BetaMaxCut",m_bEtaMaxCut=3.);
  declareProperty("CpTMinCut",m_cPtMinCut=5000.); /// MeV
  declareProperty("CetaMaxCut",m_cEtaMaxCut=3.);
  declareProperty("BMultiplicityCut",m_bMultiCut=1); /// >=
  declareProperty("CMultiplicityCut",m_cMultiCut=1); /// >=
  declareProperty("ExcludeBFromTTbar",m_excludeBFromTop=true);  /// use IS quark
  declareProperty("ExcludeCFromTTbar",m_excludeCFromTop=true); /// use IS quark

}

//--------------------------------------------------------------------------
 TTbarPlusHeavyFlavorFilter::~TTbarPlusHeavyFlavorFilter(){
   /////
}

//---------------------------------------------------------------------------
StatusCode TTbarPlusHeavyFlavorFilter::filterInitialize() {
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode TTbarPlusHeavyFlavorFilter::filterFinalize() {
  ATH_MSG_INFO( m_nPass  << " Events out of " << m_nPass+m_nFail << " passed the filter"  );
  return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode TTbarPlusHeavyFlavorFilter::filterEvent(const EventContext& ctx) {
//---------------------------------------------------------------------------

  bool pass = false;

  int nB=0;
  int nC=0;

  int nBtop=0;
  int nCtop=0;

  McEventCollection::const_iterator itr;
  for (itr = events()->begin(); itr!=events()->end(); ++itr) {

    const HepMC::GenEvent* genEvt = (*itr);

    // Loop over all truth particles in the event
    // ===========================================
    for(const auto&  part: *genEvt) {

      if(HepMC::is_simulation_particle(part)) break; // FIXME This is dangerous as it assumes a certain ordering of particles which does not properly consider quasi-stable particles

      bool isbquark=false;
      bool iscquark=false;

      bool isbhadron=false;
      bool ischadron=false;

      //// don't loose time checking all if one found
      if( MC::isBottom(part) ){
	isbquark=true;
      }
      else if( MC::isCharm(part) ){
	iscquark=true;
      }
      else if ( isBHadron(part) ){
	isbhadron=true;
      }
      else if ( isCHadron(part) ){
	ischadron=true;
      }
      else{
	continue;
      }

 
      if( (isbquark || isbhadron) && !passBSelection(part) ) continue;
      if( (iscquark || ischadron) && !passCSelection(part) ) continue;

      if(isbhadron || ischadron){
	if(!isInitialHadron(part) && !m_useFinalStateHadrons) continue;
	if(!isFinalHadron(part) && m_useFinalStateHadrons) continue;
      }

      if(m_excludeBFromTop && isbquark){
	if(isDirectlyFromTop(part))++nBtop;
	if(isDirectlyFromWTop(part))++nBtop;
      }
      if(m_excludeCFromTop && iscquark){
	if(isDirectlyFromTop(part))++nCtop;
	if(isDirectlyFromWTop(part))++nCtop;
      }

      bool ischadronfromb = isCHadronFromB(part);
      
      if(isbhadron) ++nB;
      if(ischadron && !ischadronfromb) ++nC;
      

    } /// loop on particles

    // Coverity dislikes this break as it means the loop is poitless, commentted out in line with other filters
    //break;

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

  if(m_selectB && 5 == flavortype) pass=true;
  if(m_selectC && 4 == flavortype) pass=true;
  if(m_selectL && 0 == flavortype) pass=true;

  setFilterPassed(pass, ctx);
  return StatusCode::SUCCESS;

}


bool TTbarPlusHeavyFlavorFilter::passBSelection(const HepMC::ConstGenParticlePtr& part) const{

  const HepMC::FourVector& p4 = part->momentum();
  double pt = p4.perp();
  double eta = std::abs(p4.eta());

  if(pt<m_bPtMinCut) return false;
  if(eta>m_bEtaMaxCut) return false;

  return true;
  
}

bool TTbarPlusHeavyFlavorFilter::passCSelection(const HepMC::ConstGenParticlePtr& part) const{

  const HepMC::FourVector& p4 = part->momentum();
  double pt = p4.perp();
  double eta = std::abs(p4.eta());

  if(pt<m_cPtMinCut) return false;
  if(eta>m_cEtaMaxCut) return false;

  return true;
}


int TTbarPlusHeavyFlavorFilter::hadronType(int pdgid) const{
  const int leadingQuark = MC::leadingQuark(pdgid);
  return (leadingQuark < 4 || leadingQuark > 5) ? 0 : leadingQuark; // Only interested in b/c hadrons
}


bool TTbarPlusHeavyFlavorFilter::isBHadron(const HepMC::ConstGenParticlePtr& part) const{
  return (!HepMC::is_simulation_particle(part) && MC::isBottomHadron(part));
}


bool TTbarPlusHeavyFlavorFilter::isCHadron(const HepMC::ConstGenParticlePtr& part) const{
  return (!HepMC::is_simulation_particle(part) && MC::isCharmHadron(part));
}



bool TTbarPlusHeavyFlavorFilter::isInitialHadron(const HepMC::ConstGenParticlePtr& part) const{

    auto prod = part->production_vertex();
    if(!prod) return true;
    const int type = hadronType(part->pdg_id());
    for(const auto& firstParent: prod->particles_in()){
      if( hadronType( firstParent->pdg_id() ) == type ){
	return false;
      }
    }

  return true;
}


bool TTbarPlusHeavyFlavorFilter::isFinalHadron(const HepMC::ConstGenParticlePtr& part) const{

    auto end = part->end_vertex();
    if(!end) return true;
    const int type = hadronType(part->pdg_id());
    for(const auto& firstChild: end->particles_out()){
      if( hadronType( firstChild->pdg_id() ) == type ){
	return false;
      }
    }
  return true;

}



bool TTbarPlusHeavyFlavorFilter::isQuarkFromHadron(const HepMC::ConstGenParticlePtr& part) const{

  auto prod = part->production_vertex();
  if(!prod) return false;
    for(const auto& firstParent: HepMC::ancestor_particles(prod)){
      const int mothertype = hadronType( firstParent->pdg_id() );
      if( MC::isCharm(mothertype) || MC::isBottom(mothertype) ){
	return true;
      }
    }
  return false;

}

bool TTbarPlusHeavyFlavorFilter::isCHadronFromB(const HepMC::ConstGenParticlePtr& part) const{

  if(!isCHadron(part)) return false;

  auto prod = part->production_vertex();
  if(!prod) return false;
    for(const auto& firstParent:HepMC::ancestor_particles(prod)){
      if( isBHadron(firstParent) ){
	return true;
      }
    }
  return false;
}





HepMC::ConstGenParticlePtr  TTbarPlusHeavyFlavorFilter::findInitial(const HepMC::ConstGenParticlePtr& part) const{

  auto prod = part->production_vertex();

  if(!prod) return part;
  for(const auto& firstParent: prod->particles_in()){
    if( part->pdg_id() == firstParent->pdg_id() ){
      return findInitial(firstParent);
    }
  }
   
  return part;

}

bool TTbarPlusHeavyFlavorFilter::isFromTop(const HepMC::ConstGenParticlePtr& part) const{

  auto initpart = findInitial(part);
  return isDirectlyFromTop(initpart);
 
}

bool TTbarPlusHeavyFlavorFilter::isDirectlyFromTop(const HepMC::ConstGenParticlePtr& part) const{

 auto prod = part->production_vertex();

  if(!prod) return false;
  for (auto firstParent: prod->particles_in()){
    if( MC::isTop(firstParent) ) return true;
  }
   
  return false;
}



bool TTbarPlusHeavyFlavorFilter::isDirectlyFromWTop(const HepMC::ConstGenParticlePtr& part) const{

  auto prod = part->production_vertex();

  if(!prod) return false;
  for(const auto& firstParent: prod->particles_in()){
    if( MC::isW(firstParent) ){
      if( isFromTop(firstParent) ) return true;
    }
  }
   
  return false;


}


