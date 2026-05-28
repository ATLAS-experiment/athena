/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class LeadTracksRoISeedTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "ZWindowRoISeedTool/LeadTracksRoISeedTool.h"

#include "GaudiKernel/EventContext.h"
#include "TrkTrackSummary/TrackSummary.h"

#include "TVector2.h"

#include <map>
#include <cmath>


///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::LeadTracksRoISeedTool::LeadTracksRoISeedTool
(const std::string& t,const std::string& n,const IInterface* p)
  : base_class(t,n,p)
{
}

///////////////////////////////////////////////////////////////////
// Initialization
///////////////////////////////////////////////////////////////////

StatusCode InDet::LeadTracksRoISeedTool::initialize()
{
  StatusCode sc = AlgTool::initialize();   
  
  ATH_CHECK( m_inputTracksCollectionKey.initialize() );
  ATH_CHECK( m_beamSpotKey.initialize() );

  ATH_CHECK( m_trackToVertex.retrieve() );
  
  return sc;
}

/////////////////////////////////////////////////////////////////////
// Compute RoI
/////////////////////////////////////////////////////////////////////

std::vector<InDet::IZWindowRoISeedTool::ZWindow> InDet::LeadTracksRoISeedTool::getRoIs(const EventContext& ctx) const
{

  static const float nonZeroInvP = 1e-9;
  
  // prepare output
  std::vector<InDet::IZWindowRoISeedTool::ZWindow> listRoIs;
  InDet::IZWindowRoISeedTool::ZWindow RoI;
  listRoIs.clear();

  //select tracks, then order by pT
  SG::ReadHandle<TrackCollection> tracks(m_inputTracksCollectionKey, ctx); 
  if ( not tracks.isValid() ) {
    ATH_MSG_ERROR("Could not find TrackCollection " << m_inputTracksCollectionKey.key() << " in StoreGate.");
    return listRoIs;    
  }
  ATH_MSG_DEBUG("Input track collection size "<<tracks->size());
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};

  std::vector<Trk::Track*> selectedTracks;
  for ( Trk::Track* trk : tracks->stdcont() ) {
    float theta = trk->perigeeParameters()->parameters()[Trk::theta];
    float ptinv = std::abs(trk->perigeeParameters()->parameters()[Trk::qOverP]) / std::sin(theta);    
    if (ptinv < 0.001) //1 GeV tracks
      ATH_MSG_VERBOSE("Examining track");
    if ( std::abs(ptinv) > nonZeroInvP ) {
      float pt = 1. / ptinv;      
      if (pt > 1000.) //1 GeV tracks for printout
	ATH_MSG_VERBOSE("- pT = " << pt << " MeV");
      if ( pt < m_trkSubLeadingPt ) continue;
    }
    float eta = -std::log( std::tan( 0.5*theta ) );
    ATH_MSG_VERBOSE("- eta = " << eta);
    if ( std::abs(eta) > m_trkEtaMax ) continue;
    float d0 = trk->perigeeParameters()->parameters()[Trk::d0];
    ATH_MSG_VERBOSE("- d0 = " << d0 << "mm");
    if ( std::abs(d0) > m_trkD0Max ) continue;
    ATH_MSG_VERBOSE("- Passed all selections");
    selectedTracks.push_back(trk);
  }

  std::sort(selectedTracks.begin(), selectedTracks.end(), tracksPtGreaterThan);
  ATH_MSG_DEBUG("Selected track collection size "<<selectedTracks.size());

  //create all pairs that satisfy leading pT and delta z0 requirements
  typedef std::vector<Trk::Track*>::iterator iteratorTracks;
  for ( Trk::Track *trkLeading : selectedTracks ) {
    //kinematic requirements
    float thetaLeading = trkLeading->perigeeParameters()->parameters()[Trk::theta];
    float ptInvLeading = std::abs(trkLeading->perigeeParameters()->parameters()[Trk::qOverP]) / std::sin(thetaLeading);
    ATH_MSG_VERBOSE("Examining selected track pairs");
    if (std::abs(ptInvLeading) > nonZeroInvP) {
      float pt = 1. / ptInvLeading;
      ATH_MSG_VERBOSE("- pT_leading = " << pt << " MeV");
      if ( pt < m_trkLeadingPt ) break; //tracks ordered by pT
    }
    //loop over sub-leading track
    for ( Trk::Track*  trk : selectedTracks ) {
      //kinematic requirements
      float z0Leading = trkLeading->perigeeParameters()->parameters()[Trk::z0];
      float z0 = trk->perigeeParameters()->parameters()[Trk::z0];
      ATH_MSG_VERBOSE("- z0Leading = " << z0Leading << " mm");
      ATH_MSG_VERBOSE("- z0_sublead = " << z0 << " mm");

      auto leadAtBeam = m_trackToVertex->perigeeAtBeamline(ctx, *trkLeading, *beamSpotHandle);
      auto subleadAtBeam = m_trackToVertex->perigeeAtBeamline(ctx, *trk, *beamSpotHandle);
      float z0LeadingBeam = leadAtBeam->parameters()[Trk::z0];
      float z0Beam = subleadAtBeam->parameters()[Trk::z0];

      if ( std::abs(z0LeadingBeam - z0Beam) > m_maxDeltaZ ) continue;
      //create the pair in global coordinates 
      float z0TrkReference = subleadAtBeam->associatedSurface().center().z();
      float z0TrkLeadingReference = leadAtBeam->associatedSurface().center().z();
      RoI.zReference = (z0Beam + z0TrkReference + z0LeadingBeam + z0TrkLeadingReference) / 2;
      RoI.zWindow[0] = RoI.zReference - m_z0Window; 
      RoI.zWindow[1] = RoI.zReference + m_z0Window; 
      RoI.zPerigeePos[0] = z0LeadingBeam; 
      RoI.zPerigeePos[1] = z0Beam; 
      ATH_MSG_DEBUG("New RoI created [mm]: " << RoI.zWindow[0] << " - " << RoI.zWindow[1] << " (z-ref: " << RoI.zReference << ")");
      listRoIs.push_back(RoI);
    }
  }


  if( listRoIs.empty() ){
    for( Trk::Track* trkLeading : selectedTracks ){
      //kinematic requirements
      float thetaLeading = trkLeading->perigeeParameters()->parameters()[Trk::theta];
      float ptInvLeading = std::abs(trkLeading->perigeeParameters()->parameters()[Trk::qOverP]) / std::sin(thetaLeading);
      ATH_MSG_VERBOSE("Examining selected track pairs");
      if (std::abs(ptInvLeading) > nonZeroInvP) {
	float pt = 1. / ptInvLeading;
	ATH_MSG_VERBOSE("- pT_leading = " << pt << " MeV");
	if ( pt < m_trkLeadingPt ) break; //tracks ordered by pT
	
	auto leadAtBeam = m_trackToVertex->perigeeAtBeamline(ctx, *trkLeading, *beamSpotHandle);
	float z0LeadingBeam = leadAtBeam->parameters()[Trk::z0];
	
	//create the pair in global coordinates 
	float z0TrkLeadingReference = leadAtBeam->associatedSurface().center().z();
	RoI.zReference = z0LeadingBeam + z0TrkLeadingReference;
	RoI.zWindow[0] = RoI.zReference - m_z0Window; 
	RoI.zWindow[1] = RoI.zReference + m_z0Window; 
	RoI.zPerigeePos[0] = z0LeadingBeam; 
	ATH_MSG_DEBUG("New RoI created [mm]: " << RoI.zWindow[0] << " - " << RoI.zWindow[1] << " (z-ref: " << RoI.zReference << ")");
	listRoIs.push_back(RoI);
      }
      

    }
  }

  return listRoIs;
  
}

