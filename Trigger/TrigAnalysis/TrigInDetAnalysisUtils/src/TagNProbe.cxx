/**
 **     @file    TagNProbe.cxx
 **
 **     @author  mark sutton
 **     @date    Sat Apr  9 12:55:17 CEST 2022
 **
 **     Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 **/


#include "TrigInDetAnalysisUtils/TagNProbe.h"
#include "TruthUtils/ParticleConstants.h"


TagNProbe::TagNProbe( const std::string& refName0, 
		      const std::string& refName1, 
		      double massMin, double massMax, bool unique_flag ) :
  m_particleType0(refName0),
  m_particleType1(refName1),
  m_mass0(0),
  m_mass1(0),
  m_massMin(massMin),
  m_massMax(massMax),
  m_unique(unique_flag)
{
  construct();
}

TagNProbe::TagNProbe( const std::string& refName, 
		      double massMin, double massMax, bool unique_flag ) :
  m_particleType0(refName),
  m_particleType1(refName),
  m_mass0(0),
  m_mass1(0),
  m_massMin(massMin),
  m_massMax(massMax),
  m_unique(unique_flag)
{
  construct();
}


void TagNProbe::construct() { 
  const double muonMass     = ParticleConstants::muonMassInMeV/1000.; // Convert MeV to GeV
  const double electronMass = ParticleConstants::electronMassInMeV/1000.; // Convert MeV to GeV
  const double tauMass      = ParticleConstants::tauMassInMeV/1000.; // Convert MeV to GeV

  if      ( m_particleType0.find("Muon")!=std::string::npos )      m_mass0 = muonMass;
  else if ( m_particleType0.find("Electron")!=std::string::npos )  m_mass0 = electronMass;
  else if ( m_particleType0.find("Tau")!=std::string::npos )       m_mass0 = tauMass;

  if      ( m_particleType1.find("Muon")!=std::string::npos )      m_mass1 = muonMass;
  else if ( m_particleType1.find("Electron")!=std::string::npos )  m_mass1 = electronMass;
  else if ( m_particleType1.find("Tau")!=std::string::npos )       m_mass1 = tauMass;
}



double TagNProbe::mass_obj( const TIDA::Track* t1, const TIDA::Track* t2, TrigObjectMatcher* tom ) const {

  if ( tom!=0 && tom->status() ) { 
    return mass( tom->object(t1->id()), tom->object(t2->id()) );
  }
  else {
    return mass( t1, t2 );
  }
  
}


double TagNProbe::mass_obj( const TIDA::Track* t1, const TIDA::Track* t2, TrigObjectMatcher* tom_tag, TrigObjectMatcher* tom_probe ) const {

  if ( tom_tag!=0 && tom_tag->status() && tom_probe!=0 && tom_probe->status() ) {
    /// different toms for each leg - eg if we wanted electron + tau 
    return mass( tom_tag->object(t1->id()), tom_probe->object(t2->id()) );
  }
  else if ( tom_tag!=0 && tom_tag->status() ) { 
    /// only tag tom ...
    return mass( tom_tag->object(t1->id()), t2 );
  }
  else if ( tom_probe!=0 && tom_probe->status() ) {
    /// only probe tom ...
    return mass( t1, tom_probe->object(t2->id()) );
  }
  else {
    /// just tracks ...
    return mass( t1, t2 );
  }
  
}


TIDA::Chain* TagNProbe::findChain( const std::string& chainname, std::vector<TIDA::Chain>& chains ) const {
  for ( size_t i=chains.size() ; i-- ; ) {
    if ( chains[i].name() == chainname ) return &chains[i];
  }
  return 0;
}


