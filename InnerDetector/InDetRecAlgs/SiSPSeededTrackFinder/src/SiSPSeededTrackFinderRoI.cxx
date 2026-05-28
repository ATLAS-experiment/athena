/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiSPSeededTrackFinder/SiSPSeededTrackFinderRoI.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "SiSPSeededTrackFinderData/SiSpacePointsSeedMakerEventData.h"
#include "SiSPSeededTrackFinderData/SiTrackMakerEventData_xk.h"
#include "TrkPatternParameters/PatternTrackParameters.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"

#include <set>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiSPSeededTrackFinderRoI::SiSPSeededTrackFinderRoI
(const std::string& name,ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator)
{
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSPSeededTrackFinderRoI::initialize() 
{

  // Initialize read/write handles
  ATH_CHECK( m_outputTracksKey.initialize() );
  ATH_CHECK( m_vxOutputKey.initialize() );
  ATH_CHECK( m_SpacePointsPixelKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_SpacePointsSCTKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_prdToTrackMap.initialize( !m_prdToTrackMap.key().empty() ) );
  ATH_CHECK( m_beamSpotKey.initialize() );
  
  // Retrieve Tools
  ATH_CHECK( m_ZWindowRoISeedTool.retrieve() );
  ATH_CHECK( m_RandomRoISeedTool.retrieve( DisableTool{(not m_doRandomSpot) or m_RandomRoISeedTool.name().empty()} ) );
  ATH_CHECK( m_seedsmaker.retrieve() );
  ATH_CHECK( m_trackmaker.retrieve() );
  ATH_CHECK( m_trackSummaryTool.retrieve( DisableTool{ m_trackSummaryTool.name().empty()} ));
  if (not m_etaDependentCutsSvc.name().empty()) ATH_CHECK(m_etaDependentCutsSvc.retrieve());
  magneticFieldInit(); //init magnetic field for simplified propagation
 
  /// Statistics and Debug
  if (msgLvl(MSG::DEBUG)) {
    dump(MSG::DEBUG, nullptr);
  }
  m_neventsTotal   = 0;
  m_problemsTotal  = 0;
  
  return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////
// Extended local EDM
///////////////////////////////////////////////////////////////////

namespace InDet {
  class ExtendedSiTrackMakerEventData_xk : public InDet::SiTrackMakerEventData_xk
  {
  public:
    ExtendedSiTrackMakerEventData_xk(const SG::ReadHandleKey<Trk::PRDtoTrackMap> &key) { 
      if (!key.key().empty()) {
        m_prdToTrackMap = SG::ReadHandle<Trk::PRDtoTrackMap>(key);
        setPRDtoTrackMap(m_prdToTrackMap.cptr());
      }
    }
  private:
    void dummy() {}
    SG::ReadHandle<Trk::PRDtoTrackMap> m_prdToTrackMap;
  };
}

///////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////
 
StatusCode InDet::SiSPSeededTrackFinderRoI::execute(const EventContext& ctx) const
{
  SG::WriteHandle<TrackCollection> outputTracks{m_outputTracksKey, ctx};
  ATH_CHECK(outputTracks.record(std::make_unique<TrackCollection>()));

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};
  ATH_CHECK( beamSpotHandle.isValid() );
  Trk::PerigeeSurface beamPosPerigee(beamSpotHandle->beamPos());

  bool PIX = true ;
  bool SCT = true ;
  bool ERR = false;

  //Create container for RoI information
  auto theVertexContainer = std::make_unique<xAOD::VertexContainer>();
  auto theVertexAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
  theVertexContainer->setStore( theVertexAuxContainer.get() );

  // Find reference point of the event and create z boundary region
  //
  std::vector<InDet::IZWindowRoISeedTool::ZWindow> listRoIs;
  listRoIs =  m_ZWindowRoISeedTool->getRoIs(ctx);
  double ZBoundary[2];
  //if no RoI found; no need to go further
  if ( listRoIs.empty() ) {
    ATH_MSG_DEBUG("no selectedRoIs " );
    SG::WriteHandle<xAOD::VertexContainer> vxOut_h (m_vxOutputKey, ctx);
    CHECK( vxOut_h.record ( std::move(theVertexContainer), std::move(theVertexAuxContainer) ) );
    return StatusCode::SUCCESS;
  }
  ZBoundary[0] = listRoIs[0].zWindow[0];
  ZBoundary[1] = listRoIs[0].zWindow[1];
  //listRoIs[0].zReference is the midpoint
  ATH_MSG_DEBUG("selectedRoIs " << ZBoundary[0] <<" " << ZBoundary[1]);
  
  //Store RoI information in a xAOD::Vertex object
  static const SG::AuxElement::Accessor<float> vtxDecor_boundaryLow("boundaryLow");
  static const SG::AuxElement::Accessor<float> vtxDecor_boundaryHigh("boundaryHigh");
  static const SG::AuxElement::Accessor<float> vtxDecor_perigeeZ0Lead("perigeeZ0Lead");
  static const SG::AuxElement::Accessor<float> vtxDecor_perigeeZ0Sublead("perigeeZ0Sublead");
  static const SG::AuxElement::Accessor<float> vtxDecor_isHS("isHS");

  for( size_t r = 0; r < listRoIs.size(); r++ ){

    theVertexContainer->push_back( new xAOD::Vertex() );

    theVertexContainer->back()->setZ( listRoIs[r].zReference );
    vtxDecor_boundaryLow(*theVertexContainer->back()) = listRoIs[r].zWindow[0];;
    vtxDecor_boundaryHigh(*theVertexContainer->back()) = listRoIs[r].zWindow[1];
    vtxDecor_perigeeZ0Lead(*theVertexContainer->back()) = listRoIs[r].zPerigeePos[0];
    vtxDecor_perigeeZ0Sublead(*theVertexContainer->back()) = listRoIs[r].zPerigeePos[1];
    vtxDecor_isHS(*theVertexContainer->back()) = 1;
  }
  
  //Analyses that want to run low-pt tracking with a region of interest care about the beam conditions near a collision of interest.  Validation of the beam conditions elsewhere in the beamspot (regarding low-pt tracks) will be needed to establish meaningful uncertainties.  Choosing a random position allows for this check.  Run with RAWtoESD section of postexec: ToolSvc.InDetSiSpTrackFinder_LowPtRoI.doRandomSpot = True
  double RandZBoundary[2];
  std::vector<InDet::IZWindowRoISeedTool::ZWindow> listRandRoIs;
  if(m_doRandomSpot){
    //Finding Random Spot in beamspot
    listRandRoIs =  m_RandomRoISeedTool->getRoIs(ctx);

    while( std::abs( listRoIs[0].zReference - listRandRoIs[0].zReference ) < 5. || std::abs(listRandRoIs[0].zReference) > 250.0 ){
      listRandRoIs.clear();
      listRandRoIs =  m_RandomRoISeedTool->getRoIs(ctx);
    }

    RandZBoundary[0] = listRandRoIs[0].zWindow[0];
    RandZBoundary[1] = listRandRoIs[0].zWindow[1];
    for( size_t r = 0; r < listRandRoIs.size(); r++ ){

      theVertexContainer->push_back( new xAOD::Vertex() );
      
      theVertexContainer->back()->setZ( listRandRoIs[r].zReference );
      vtxDecor_boundaryLow(*theVertexContainer->back()) = listRoIs[r].zWindow[0];;
      vtxDecor_boundaryHigh(*theVertexContainer->back()) = listRoIs[r].zWindow[1];
      vtxDecor_perigeeZ0Lead(*theVertexContainer->back()) = listRoIs[r].zPerigeePos[0];
      vtxDecor_perigeeZ0Sublead(*theVertexContainer->back()) = listRoIs[r].zPerigeePos[1];
      vtxDecor_isHS(*theVertexContainer->back()) = 0;      
    }
  }
  
  // Record the RoI information
  SG::WriteHandle<xAOD::VertexContainer> vxOut_h (m_vxOutputKey, ctx);
  CHECK( vxOut_h.record ( std::move(theVertexContainer), std::move(theVertexAuxContainer) ) );

  // Find seeds that point within the RoI region in z
  //  
  SiSpacePointsSeedMakerEventData seedEventData;
  m_seedsmaker->newEvent(ctx, seedEventData, -1); 
  std::list<Trk::Vertex> VZ; 
  if(m_RoIWidth >= 0.) m_seedsmaker->find3Sp(ctx, seedEventData, VZ, ZBoundary); 
  //If you want to disable the RoI but still have a separate container for low-pt tracks, 
  // make the RoI input width a negative value.  The RoI "vertex" container will still be 
  // there in case you want to use that information for whatever reason (ie where the RoI 
  // would have been centered).
  if(m_RoIWidth < 0.) m_seedsmaker->find3Sp(ctx, seedEventData, VZ); 
  if(m_doRandomSpot) m_seedsmaker->find3Sp(ctx, seedEventData, VZ, RandZBoundary);

  InDet::ExtendedSiTrackMakerEventData_xk trackEventData(m_prdToTrackMap);
  m_trackmaker->newEvent(ctx, trackEventData, PIX, SCT);

  // Loop through all seed and create track candidates
  //
  ERR = false;
  Counter_t counter{};
  std::multimap<double,Trk::Track*>    qualitySortedTrackCandidates;
  const InDet::SiSpacePointsSeed* seed = 0;

  while((seed = m_seedsmaker->next(ctx, seedEventData))) {
    ++counter[kNSeeds];
    const std::list<Trk::Track*> trackList = m_trackmaker->getTracks(ctx, trackEventData, seed->spacePoints()); 
    for(Trk::Track* t: trackList) {
      qualitySortedTrackCandidates.insert(std::make_pair( -trackQuality(t), t ));
    }
    if( counter[kNSeeds] >= m_maxNumberSeeds) {
      ERR = true; 
      ++m_problemsTotal;  
      break;
    }
  }
  
  m_trackmaker->endEvent(trackEventData);

  // Remove shared tracks with worse quality
  //
  filterSharedTracks(qualitySortedTrackCandidates);

  // Save good tracks in track collection
  //
  for (const std::pair<const double, Trk::Track*> & qualityAndTrack: qualitySortedTrackCandidates) {
    ++counter[kNTracks];
    if (m_trackSummaryTool.isEnabled()) {
       m_trackSummaryTool->computeAndReplaceTrackSummary(*(qualityAndTrack.second),
                                                         false /* DO NOT suppress hole search*/);
    }
    outputTracks->push_back(qualityAndTrack.second);
  }
  m_counterTotal[kNSeeds] += counter[kNSeeds];
  ++m_neventsTotal;

  // In case of errors, clear output tracks
  //
  if (ERR) { 
    outputTracks->clear(); 
  } else {
    m_counterTotal[kNTracks] += counter[kNTracks];
  }

  // Print common event information
  //
  if (msgLvl(MSG::DEBUG)) {
    dump(MSG::DEBUG, &counter);
  }

  return StatusCode::SUCCESS;

}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSPSeededTrackFinderRoI::finalize() 
{
  dump(MSG::INFO, &m_counterTotal);

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSPSeededTrackFinderRoI::dump(MSG::Level assign_level, const InDet::SiSPSeededTrackFinderRoI::Counter_t* counter) const
{
  msg(assign_level) <<std::endl;
  MsgStream& out_msg=msg();
  if (counter) dumpevent(out_msg ,*counter);
  else dumptools(out_msg);
  out_msg << endmsg;
  return out_msg;
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSPSeededTrackFinderRoI::dumptools( MsgStream& out ) const
{
  int n;
  n     = 65-m_seedsmaker.type().size();
  std::string s2; for(int i=0; i<n; ++i) s2.append(" "); s2.append("|");
  n     = 65-m_trackmaker.type().size();
  std::string s3; for(int i=0; i<n; ++i) s3.append(" "); s3.append("|");
  n     = 65-m_outputTracksKey.key().size();
  std::string s4; for(int i=0; i<n; ++i) s4.append(" "); s4.append("|");
  
  out<<"|----------------------------------------------------------------"
     <<"----------------------------------------------------|"
     <<std::endl;
  out<<"| Tool for space points seeds             finding | "<<m_seedsmaker.type()<<s2
     <<std::endl;
  out<<"| Tool for space points seeded track      finding | "<<m_trackmaker.type()<<s3
     <<std::endl;
  out<<"| Location of output tracks                       | "<<m_outputTracksKey.key()<<s4
     <<std::endl;
  out<<"|----------------------------------------------------------------"
     <<"----------------------------------------------------|"
     <<std::endl;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the ostream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSPSeededTrackFinderRoI::dumpevent( MsgStream& out, const InDet::SiSPSeededTrackFinderRoI::Counter_t& counter) const
{
  int ns = counter[kNSeeds];
  int nt = counter[kNTracks];

  out<<"|-------------------------------------------------------------------|" <<std::endl;
  out<<"|  Investigated "
     <<std::setw(9)<<ns<<" space points seeds and found ";
  out<<std::setw(9)<<nt<<" tracks using RoI-z strategy           |"<<std::endl;

  out<<"|-------------------------------------------------------------------|" <<std::endl;
  if(m_problemsTotal > 0) {
    out<<"|  Events       "
       <<std::setw(7)<<m_neventsTotal   <<" |"
       <<std::endl;
    out<<"|  Problems     "
       <<std::setw(7)<<m_problemsTotal  <<" |"
       <<std::endl;
    out<<"|-------------------------------------------------------------------|" <<std::endl;
  }
  return out;
}

///////////////////////////////////////////////////////////////////
// Track quality calculation
///////////////////////////////////////////////////////////////////

double InDet::SiSPSeededTrackFinderRoI::trackQuality(const Trk::Track* Tr) const
{
 double quality = 0. ;
 double baseScorePerHit       = 17.;

  /// check all hits on the track
 for (const Trk::TrackStateOnSurface* m: *(Tr->trackStateOnSurfaces())) {
   /// exclude anything which is not an actual hit 
   if (not m->type(Trk::TrackStateOnSurface::Measurement)) continue;
  /// retrieve the fit quality for a given hit
   const Trk::FitQualityOnSurface fq = m->fitQualityOnSurface();
   if (!fq) continue;
  
   double x2 = fq.chiSquared();
   double hitQualityScore;
   /// score the hit based on the technology (pixels get higher score) and 
   /// the local chi2 for the hit 
   if (fq.numberDoF() == 2) hitQualityScore = (1.2*(baseScorePerHit-x2*.5));   // pix
   else                      hitQualityScore =      (baseScorePerHit-x2    );   // sct
   if (hitQualityScore < 0.) hitQualityScore = 0.;    // do not allow a bad hit to decrement the overall score 
   quality += hitQualityScore;
 }
  /// penalise brem tracks 
 if (Tr->info().trackProperties(Trk::TrackInfo::BremFit)) quality *= 0.7;

 return quality;
}

///////////////////////////////////////////////////////////////////
// Filer shared tracks
///////////////////////////////////////////////////////////////////

void InDet::SiSPSeededTrackFinderRoI::filterSharedTracks
(std::multimap<double,Trk::Track*>& qualitySortedTracks) const
{
  std::set<const Trk::PrepRawData*> clusters;
  
  std::vector<const Trk::PrepRawData*> freeClusters;
  freeClusters.reserve(15);    
  
  std::multimap<double, Trk::Track*>::iterator it_qualityAndTrack = qualitySortedTracks.begin();

  /// loop over all track candidates, sorted by quality
  while (it_qualityAndTrack!=qualitySortedTracks.end()) {   
    freeClusters.clear();

    std::set<const Trk::PrepRawData*>::iterator it_clustersEnd = clusters.end();

    int nClusters = 0; 
    int nPixels = 0;

    /// loop over measurements on the track candidate 
    for (const Trk::TrackStateOnSurface* tsos: *((*it_qualityAndTrack).second->trackStateOnSurfaces())) {
      /// get the PRD from the measurement
      if(!tsos->type(Trk::TrackStateOnSurface::Measurement)) continue;
      const Trk::FitQualityOnSurface fq =  tsos->fitQualityOnSurface();
      if(!fq) continue;
      if(fq.numberDoF() == 2) ++nPixels;
      
      const Trk::MeasurementBase* mb = tsos->measurementOnTrack();
      const Trk::RIO_OnTrack*     ri = dynamic_cast<const Trk::RIO_OnTrack*>(mb);
      if(!ri) continue;
      const Trk::PrepRawData* pr = ri->prepRawData();
      if (not pr) continue;
      /// increase cluster count
      ++nClusters;
      /// and check if the cluster was already used in a previous ( = higher quality) track 
      if (clusters.find(pr)==it_clustersEnd) {
	/// if not, record as a free (not prevously used) cluster 
	freeClusters.push_back(pr); 
      }
    }

    /// check if the track has the minimum number of free clusters or if it has no shared clusters 
    int nFreeClusters = static_cast<int>(freeClusters.size()); 
    if (nFreeClusters >= m_nfreeCut || nFreeClusters==nClusters) {
      /// if this is fulfilled, we keep the candidate 
      /// add the free clusters to our cluster set 
      clusters.insert(freeClusters.begin(), freeClusters.end());

      if (m_etaDependentCutsSvc.name().empty()) {
	++it_qualityAndTrack;
      } else {
	//eta-dependent cuts
	int nFreeClusters = static_cast<int>(freeClusters.size());      
	if( passEtaDepCuts( (*it_qualityAndTrack).second, nClusters, nFreeClusters, nPixels) ){
	  /// if this is fulfilled, we keep the candidate
	  ++it_qualityAndTrack;
	} else {
	  /// if we do not keep the track, clean up candidate
	  delete (*it_qualityAndTrack).second;
	  qualitySortedTracks.erase(it_qualityAndTrack++);
	}      
      } //eta-dependent cuts
    } else {
      /// if we do not keep the track, clean up candidate 
      delete (*it_qualityAndTrack).second;
      qualitySortedTracks.erase(it_qualityAndTrack++);
    }
  }
}

///////////////////////////////////////////////////////////////////
// Callback function - get the magnetic field /
///////////////////////////////////////////////////////////////////

void InDet::SiSPSeededTrackFinderRoI::magneticFieldInit() 
{
  // Build MagneticFieldProperties 
  //
  if(m_fieldmode == "NoField") {
    m_fieldprop = Trk::MagneticFieldProperties(Trk::NoField);
  } else {
    m_fieldprop = Trk::MagneticFieldProperties(Trk::FastField);
  }
}

bool InDet::SiSPSeededTrackFinderRoI::passEtaDepCuts(const Trk::Track* track,
						     int nClusters,
						     int nFreeClusters,
						     int nPixels) const
{
  DataVector<const Trk::TrackStateOnSurface>::const_iterator  m = track->trackStateOnSurfaces()->begin();
  const Trk::TrackParameters* par = (*m)->trackParameters();
  if(!par) return false;

  double eta = std::abs(par->eta());
  if(nClusters               < m_etaDependentCutsSvc->getMinSiHitsAtEta(eta)) return false;
  if(nFreeClusters           < m_etaDependentCutsSvc->getMinSiNotSharedAtEta(eta)) return false;
  if(nClusters-nFreeClusters > m_etaDependentCutsSvc->getMaxSharedAtEta(eta)) return false;
  if(nPixels                 < m_etaDependentCutsSvc->getMinPixelHitsAtEta(eta)) return false;

  if(par->pT() < m_etaDependentCutsSvc->getMinPtAtEta(eta)) return false;
  if(!(*m)->type(Trk::TrackStateOnSurface::Perigee)) return true ;
  if(std::abs(par->localPosition()[0]) > m_etaDependentCutsSvc->getMaxPrimaryImpactAtEta(eta)) return false;
  return true;
}



