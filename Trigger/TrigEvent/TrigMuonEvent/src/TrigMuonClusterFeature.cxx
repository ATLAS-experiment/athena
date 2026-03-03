/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*******************************************************

NAME:           TrigMuonClusterFeature.cxx
PACKAGE:        Trigger/TrigEvent/TrigMuonEvent
AUTHORS:        Antonio Policicchio <Antonio.Poliicchio@cern.ch>
AUTHORS:        Stefano Giagu <stefano.giagu@cern.ch>
PURPOSE:        Keep the important output variables
                from the muon ROI cluster trigger
DATE:           V1.0 January 13th, 2009

******************************************************/
// Local include(s):
#include "TrigMuonEvent/TrigMuonClusterFeature.h"

// Gaudi/Athena include(s):
#include "GaudiKernel/MsgStream.h"

// STL include(s):
#include <format>
#include <cmath>

// "Distance" used by the comparison operator(s):
static constexpr double DELTA = 0.001;

TrigMuonClusterFeature::TrigMuonClusterFeature	(float eta, float phi, int nroi, int njet, int ntrk) : P4PtEtaPhiMBase(), NavigableTerminalNode(),
  m_eta (eta),
  m_phi (phi),
  m_nroi (nroi),
  m_njet (njet),
  m_ntrk (ntrk)
{}
  

// Copy-by-pointer constructor (Note that also the base class is copied)
TrigMuonClusterFeature::TrigMuonClusterFeature( const TrigMuonClusterFeature * feat )
  : TrigMuonClusterFeature(*feat)
{}


//////////////////////////////////////////////////////////////////
// helper operators

std::string str ( const TrigMuonClusterFeature& d ) {
   return std::format(
    "  Eta:     {}; Phi:     {}; NRoI:    {}; NJET:    {}; NTRK:    {}",
    d.getEta(),
    d.getPhi(),
    d.getNRoi(),
    d.getNJet(),
    d.getNTRK()
  );
}
 
MsgStream& operator<< ( MsgStream& m, const TrigMuonClusterFeature& d ) {
  return (m << str(d));
}

bool operator== ( const TrigMuonClusterFeature& a, const TrigMuonClusterFeature& b ) {
  if ( std::abs(a.getEta() - b.getEta()) > DELTA )    return false;
  if ( std::abs(a.getPhi() - b.getPhi()) > DELTA )    return false;
  if ( a.getNRoi()   != b.getNRoi() )   return false;
  if ( a.getNJet()   != b.getNJet() )   return false;
  if ( a.getNTRK()   != b.getNTRK() )   return false;
  return true;
}
 
void diff( const TrigMuonClusterFeature& a, const TrigMuonClusterFeature& b,
           std::map<std::string, double>& variableChange ) {

   if( std::abs( a.getEta() - b.getEta() ) > DELTA ) {
      variableChange[ "Eta" ] = a.getEta() - b.getEta();
   }
   if( std::abs( a.getPhi() - b.getPhi() ) > DELTA ) {
      variableChange[ "Phi" ] = a.getPhi() - b.getPhi();
   }
   if( a.getNRoi() != b.getNRoi() ) {
      variableChange[ "NRoI" ] = static_cast< double >( a.getNRoi() - b.getNRoi() );
   }
   if( a.getNJet() != b.getNJet() ) {
      variableChange[ "NJet" ] = static_cast< double >( a.getNJet() - b.getNJet() );
   }
   if( a.getNTRK() != b.getNTRK() ) {
      variableChange[ "NTRK" ] = static_cast< double >( a.getNTRK() - b.getNTRK() );
   }

   return;
}


