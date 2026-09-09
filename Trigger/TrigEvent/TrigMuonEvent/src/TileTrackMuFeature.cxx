/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "TrigMuonEvent/TileTrackMuFeature.h"



/** Distance used by the comparison operators */
namespace{
  constexpr double DELTA = 0.001;
} 



TileTrackMuFeature::TileTrackMuFeature(
        float PtTR_Trk,
        float EtaTR_Trk, float PhiTR_Trk, int Typ_IDTrk,
        const ElementLink< TileMuFeatureContainer>& TileMuOutput,
        const ElementLink< TrigInDetTrackCollection>& IDScanOutput) 
  :  P4PtEtaPhiMBase()
  , m_PtTR_Trk(PtTR_Trk)
  , m_EtaTR_Trk(EtaTR_Trk)
  , m_PhiTR_Trk(PhiTR_Trk)
  , m_Typ_IDTrk(Typ_IDTrk)
  , m_TileMuOutput(TileMuOutput)
  , m_IDScanOutput(IDScanOutput)
{
}


TileTrackMuFeature::TileTrackMuFeature(const TileTrackMuFeature& muon_feature)
  : m_PtTR_Trk(muon_feature.m_PtTR_Trk)
  , m_EtaTR_Trk(muon_feature.m_EtaTR_Trk)
  , m_PhiTR_Trk(muon_feature.m_PhiTR_Trk)
  , m_Typ_IDTrk(muon_feature.m_Typ_IDTrk)
  , m_TileMuOutput(muon_feature.m_TileMuOutput)
  , m_IDScanOutput(muon_feature.m_IDScanOutput)
{
}

TileTrackMuFeature&
TileTrackMuFeature::operator=(TileTrackMuFeature&& muon_feature) noexcept
{
  if (this != &muon_feature) {
    m_PtTR_Trk     = muon_feature.m_PtTR_Trk;
    m_EtaTR_Trk    = muon_feature.m_EtaTR_Trk;
    m_PhiTR_Trk    = muon_feature.m_PhiTR_Trk;
    m_Typ_IDTrk    = muon_feature.m_Typ_IDTrk;
    m_TileMuOutput = std::move(muon_feature.m_TileMuOutput);
    m_IDScanOutput = std::move(muon_feature.m_IDScanOutput);
  }
  return *this;
}


/** Assignement operator */
TileTrackMuFeature&
TileTrackMuFeature::operator=(const TileTrackMuFeature& muon_feature) {
  if (this != &muon_feature) {
    m_PtTR_Trk     = muon_feature.m_PtTR_Trk;
    m_EtaTR_Trk    = muon_feature.m_EtaTR_Trk;
    m_PhiTR_Trk    = muon_feature.m_PhiTR_Trk;
    m_Typ_IDTrk    = muon_feature.m_Typ_IDTrk;
    m_TileMuOutput = muon_feature.m_TileMuOutput;
    m_IDScanOutput = muon_feature.m_IDScanOutput;
  }
  return *this;
}

/** Helper operators */

std::string str ( const TileTrackMuFeature& d )
{
  std::stringstream ss;

  ss << "EtaTR_Trk: " << d.EtaTR_Trk()
     << "; PhiTR_Trk: " << d.PhiTR_Trk()
     << "; PtTR_Trk: " << d.PtTR_Trk();

  return ss.str();
}

const TileMuFeature*  
TileTrackMuFeature::TileMuOutput() const { return *m_TileMuOutput; }

const TrigInDetTrack* 
TileTrackMuFeature::IDScanOutput() const { return *m_IDScanOutput; }
 const ElementLink<TileMuFeatureContainer>&   
TileTrackMuFeature::TileMuLink() const{ 
  return m_TileMuOutput;
}
  const ElementLink<TrigInDetTrackCollection>& 
TileTrackMuFeature::IDScanLink() const{ 
  return m_IDScanOutput;
}
MsgStream& operator<< ( MsgStream& m, const TileTrackMuFeature& d )
{
  return ( m << str( d ) );
}

bool operator== ( const TileTrackMuFeature& a, const TileTrackMuFeature& b )
{
  if( std::abs( a.EtaTR_Trk() - b.EtaTR_Trk() ) > DELTA ) return false;
  if( std::abs( a.PhiTR_Trk() - b.PhiTR_Trk() ) > DELTA ) return false;
  if( std::abs( a.PtTR_Trk() - b.PtTR_Trk() ) > DELTA ) return false;

  return true;
}

void diff( const TileTrackMuFeature& a, const TileTrackMuFeature& b, std::map< std::string, double >& variableChange )
{
  if( std::abs( a.EtaTR_Trk() - b.EtaTR_Trk() ) > DELTA ) {
    variableChange[ "EtaTR_Trk" ] = a.EtaTR_Trk() - b.EtaTR_Trk();
  }
  if( std::abs( a.PhiTR_Trk() - b.PhiTR_Trk() ) > DELTA ) {
    variableChange[ "PhiTR_Trk" ] = a.PhiTR_Trk() - b.PhiTR_Trk();
  }
  if( std::abs( a.PtTR_Trk() - b.PtTR_Trk() ) > DELTA ) {
    variableChange[ "PtTR_Trk" ] = a.PtTR_Trk() - b.PtTR_Trk();
  }

  return;
}
