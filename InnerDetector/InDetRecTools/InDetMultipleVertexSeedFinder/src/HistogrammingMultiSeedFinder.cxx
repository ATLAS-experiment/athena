/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMultipleVertexSeedFinder/HistogrammingMultiSeedFinder.h"
#include "TrkTrack/Track.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "InDetMultipleVertexSeedFinderUtils/InDetTrackClusterCleaningTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "xAODTracking/Vertex.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include <optional>

namespace InDet
{
 StatusCode HistogrammingMultiSeedFinder::initialize()
 {
 
  if(m_trkFilter.retrieve().isFailure())
  {
   ATH_MSG_ERROR(" Unable to retrieve "<<m_trkFilter);
   return StatusCode::FAILURE;
  }
   
  if(m_cleaningTool.retrieve().isFailure())
  {
   ATH_MSG_ERROR(" Unable to retrieve "<<m_cleaningTool);
   return StatusCode::FAILURE;
  }
   
  ATH_CHECK(m_beamSpotKey.initialize());
  if(m_vtxSeedFinder.retrieve().isFailure())
  {
   ATH_MSG_ERROR("Unable to retrieve " << m_vtxSeedFinder);
   return StatusCode::FAILURE;
  }
 
  if ( m_extrapolator.retrieve().isFailure() ) 
  {                              
   ATH_MSG_ERROR("Failed to retrieve tool " << m_extrapolator);
   return StatusCode::FAILURE;                                                  
  }
                
  return StatusCode::SUCCESS;
 }//end of initialize mtehod


 HistogrammingMultiSeedFinder::HistogrammingMultiSeedFinder(const std::string& t, const std::string& n, const IInterface*p):
         AthAlgTool(t,n,p), 
         m_sepNBins(2),
         m_nBins(500),
         m_nRemaining(1), 
         m_histoRange(200.), 
         m_ignoreBeamSpot(false),
         m_vtxSeedFinder("Trk::CrossDistancesSeedFinder"),
         m_extrapolator("Trk::Extrapolator") 
 {
  declareInterface<IMultiPVSeedFinder>(this); 
  declareProperty("maxSeparNumBins",     m_sepNBins);
  declareProperty("nBins",                  m_nBins);
  declareProperty("nRemainTracks",     m_nRemaining);
  declareProperty("HistoRange",        m_histoRange);
  declareProperty("IgnoreBeamSpot",	m_ignoreBeamSpot);


  //track filter
  declareProperty("TrackSelector", m_trkFilter );

  //cleaning tool
  declareProperty("CleaningTool",  m_cleaningTool);

  //vertex finder tool (needed when no beam spot is available)
  declareProperty("VertexSeedFinder",m_vtxSeedFinder);

  //extrapolator
  declareProperty("Extrapolator",m_extrapolator); 
 }//end of constructor

 HistogrammingMultiSeedFinder::~HistogrammingMultiSeedFinder()
 = default;

 std::vector< std::vector<const Trk::Track *> > HistogrammingMultiSeedFinder::seeds(const std::vector<const Trk::Track*>& tracks )const
 {
  const EventContext& ctx = Gaudi::Hive::currentContext();

  //step 1: preselection
  std::vector<const Trk::Track*> preselectedTracks(0);
  std::vector<const Trk::Track*>::const_iterator tr = tracks.begin();
  std::vector<const Trk::Track*>::const_iterator tre = tracks.end(); 
  
  //beamposition
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };

  Trk::RecVertex beamrecposition(beamSpotHandle->beamVtx());
  ATH_MSG_DEBUG("Beam spot position is: "<< beamrecposition.position());
  for(;tr!=tre;++tr) if(m_trkFilter->decision(**tr,&beamrecposition)) preselectedTracks.push_back(*tr);
  Trk::Vertex* beamposition=&beamrecposition;

  Trk::Vertex myVertex;
  if (m_ignoreBeamSpot)
  {
    myVertex = Trk::Vertex(m_vtxSeedFinder->findSeed(tracks));
    ATH_MSG_DEBUG(" vtx seed x: " << myVertex.position().x() <<
		  " vtx seed y: " << myVertex.position().y() <<
		  " vtx seed z: " << myVertex.position().z());
    beamposition = &myVertex;
  }

  //step 2: histogramming tracks
  //output container
  std::vector< std::vector<const Trk::Track *> > result(0);
  if(!preselectedTracks.empty())
  { 
   std::map<unsigned int, std::vector<const Trk::Track *> > histo;  
  
   //getting the bin size
   double bin_size  = 2. * m_histoRange/ m_nBins;
  
   std::vector<const Trk::Track*>::const_iterator p_tr = preselectedTracks.begin();
   std::vector<const Trk::Track*>::const_iterator p_tre = preselectedTracks.end();
   
   
   //creating a perigee surfce to extrapolate tracks
   Trk::PerigeeSurface perigeeSurface(beamposition->position());   
  
   for(;p_tr != p_tre; ++p_tr)
   {
     const Trk::TrackParameters* lexPerigee =
       m_extrapolator->extrapolateTrack(ctx, **p_tr, perigeeSurface, Trk::anyDirection, true, Trk::pion).release();

     double currentTrackZ0 = lexPerigee->parameters()[Trk::z0];
     delete lexPerigee;

     unsigned int bin_number =
       int(floor((currentTrackZ0 + m_histoRange) / bin_size)) + 1;

     // now checking whether this bin entry already exists and adding track, if
     // not, creating one.
     std::map<unsigned int, std::vector<const Trk::Track*>>::iterator map_pos =
       histo.find(bin_number);
     if (map_pos != histo.end()) {
      // this bin already exists, adding entry
      map_pos->second.push_back(*p_tr);
     }else{
      //this bin is not their yet, adding bin
      std::vector<const Trk::Track *> tmp_vec(0);
      tmp_vec.push_back(*p_tr);
      histo.insert( std::map<unsigned int, std::vector<const Trk::Track *> >::value_type(bin_number, tmp_vec));
    }//end of checking bin existence     
   }//end of loop over all sorted tracks

   //------------------------- Debug output -------------------------------------------------------------------
   //debug output: checking the bin contents of the histogram
   /*  std::cout<<"**********Checking the histogram ************"<<std::endl;
       for(std::map<unsigned int, std::vector<const Trk::Track *> >::iterator i = histo.begin(); i != histo.end(); ++i)
       {
       std::cout<<"Currennt bin N "<< i->first<<std::endl;
       std::cout<<"Containes entries: "<< i->second.size()<<std::endl;
       }//end of debug histogram check
   */
   //------------------------- End of debug output -------------------------------------------------------------------
   
   //step 3: merging clusters
   //bins closer to each other than several empty bins become
   //parts of the same cluster

   std::vector<std::vector<const Trk::Track *> > preClusters(0);
   std::vector<const Trk::Track *> tmp_cluster(0);
   unsigned int previous_bin = histo.begin()->first;
   for(auto & i : histo){
    unsigned int current_bin = i.first;
    if((current_bin - previous_bin)>m_sepNBins ){
     //forming a new cluster
     preClusters.push_back(tmp_cluster);
     tmp_cluster.clear();
    }

    // in any case filling tracks into this bin.
    for(const auto *j : i.second) tmp_cluster.push_back(j);
    previous_bin = current_bin;
   }//end of loop over the map entries
   preClusters.push_back(tmp_cluster);

   //step 4 iterative cleaning of formed clusters
   for(const auto & preCluster : preClusters){
    if(preCluster.size()>m_nRemaining){
     std::vector<const Trk::Track *> tracks_to_clean = preCluster;
     bool clean_again = false;
     do{
      std::pair<std::vector<const Trk::Track *>, std::vector<const Trk::Track *> > clusterAndOutl =
	m_cleaningTool->clusterAndOutliers(tracks_to_clean, beamposition);

      //if core size is miningfull, storing it
      std::vector<const Trk::Track *> core_cluster = clusterAndOutl.first;
      std::vector<const Trk::Track *> core_outl = clusterAndOutl.second;
      
      //--------------Debug output-----------------------------------------------------
      //      std::cout<<"Cleaning iteration "<<clean_count<<std::endl;
      //      std::cout<<"Reduced cluster size: "<<core_cluster.size()<<std::endl;
      //      std::cout<<"Outliers size:        "<<core_outl.size()<<std::endl;
      //      ++clean_count;
      //-------------------End of debug output -----------------------------------------

      if(core_cluster.empty()){
       ATH_MSG_DEBUG("Core cluster has 0 size, remaining tracks are discarded.");
       clean_again = false;
      }else{
       //storing clusters with >1 track (optional)
       if(core_cluster.size()>1) result.push_back(core_cluster);
       //checking the outliers, whether more cleaning is to be done
       if(core_outl.size()>m_nRemaining){
        clean_again = true;
        tracks_to_clean.clear();
        tracks_to_clean = core_outl;
       }else if(core_outl.size()>1){
        clean_again = false;
        ATH_MSG_DEBUG("There were remaining outliers of size: "<< core_outl.size());
        ATH_MSG_DEBUG("Not evident, whether these tracks form a cluster. Rejected...");
       }else clean_again = false;//end of outlier size check 
      }//end of core cluster 0 check
     }while(clean_again);//end of loop

    }else if(preCluster.size()==2){
     //case of two track cluster. accepting without cleaning
     result.push_back(preCluster);
    }//end of cluster size check
   }//end of loop over all the preclusters
  }//end of  preselection not zero check

  return result; 
 }//end of seed finding method
 
  std::vector< std::vector<const Trk::TrackParameters *> > HistogrammingMultiSeedFinder::seeds(const std::vector<const xAOD::TrackParticle*>& tracks )const
  {
   const EventContext& ctx = Gaudi::Hive::currentContext();

   //step 1: preselection
   std::vector<const xAOD::TrackParticle*> preselectedTracks(0);

   //selecting with respect to the beam spot
   xAOD::Vertex beamposition;
   SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey,ctx };
   beamposition.setPosition(beamSpotHandle->beamVtx().position());
   beamposition.setCovariancePosition(beamSpotHandle->beamVtx().covariancePosition());

   for (const auto *track : tracks) {
     if (m_trkFilter->decision(*track,&beamposition)) preselectedTracks.push_back(track);
   }

   std::vector<const Trk::TrackParameters*> perigeeList;
   std::vector<const xAOD::TrackParticle*>::const_iterator trackBegin=tracks.begin();
   std::vector<const xAOD::TrackParticle*>::const_iterator trackEnd=tracks.end();
   for (std::vector<const xAOD::TrackParticle*>::const_iterator trackIter=trackBegin;trackIter!=trackEnd;++trackIter){
    perigeeList.push_back(&((*trackIter)->perigeeParameters()));
   }

   Trk::RecVertex myVertex (m_vtxSeedFinder->findSeed(perigeeList));

   if (m_ignoreBeamSpot){
    ATH_MSG_DEBUG(" vtx seed x: " << myVertex.position().x() <<
		  " vtx seed y: " << myVertex.position().y() <<
		  " vtx seed z: " << myVertex.position().z());
    beamposition.setPosition(myVertex.position());
    beamposition.setCovariancePosition(myVertex.covariancePosition());
   }
  
   //step 2: sorting in z0
   //output container
   std::vector< std::vector<const Trk::TrackParameters *> > result(0);
   if(!preselectedTracks.empty()){
    std::map<unsigned int, std::vector<const xAOD::TrackParticle *> > histo;
	
    //getting the bin size
    double bin_size  = 2.* m_histoRange/ m_nBins;

    ATH_MSG_DEBUG("Beam spot position is: "<< beamposition.position());

    //getting a perigee surface to be  used
    Trk::PerigeeSurface perigeeSurface(beamposition.position());
	
    std::vector<const xAOD::TrackParticle*>::const_iterator p_tr = preselectedTracks.begin();
    std::vector<const xAOD::TrackParticle*>::const_iterator p_tre = preselectedTracks.end();
    for(;p_tr != p_tre; ++p_tr){
     const Trk::TrackParameters* lexPerigee =
       m_extrapolator->extrapolate(ctx, (*p_tr)->perigeeParameters(),
				   perigeeSurface, Trk::anyDirection, true, Trk::pion)
       .release();
     double currentTrackZ0 = lexPerigee->parameters()[Trk::z0];
     delete lexPerigee;

     unsigned int bin_number =
       int(floor((currentTrackZ0 + m_histoRange) / bin_size)) + 1;

     // now checking whether this bin entry already exists and adding
     // track, if not, creating one.
     std::map<unsigned int,
	      std::vector<const xAOD::TrackParticle*>>::iterator map_pos =
       histo.find(bin_number);
     if (map_pos != histo.end()) {
      // this bin already exists, adding entry
       map_pos->second.push_back(*p_tr);
     }else{
      //this bin is not their yet, adding bin
      std::vector<const xAOD::TrackParticle *> tmp_vec(0);
      tmp_vec.push_back(*p_tr);
      histo.insert( std::map<unsigned int, std::vector<const xAOD::TrackParticle *> >::value_type(bin_number, tmp_vec));
     }//end of checking bin existence
    }//end of loop over all sorted track particles
	
    //step 3: merging clusters
    //bins closer to each other than several empty bins become
    //parts of the same cluster
	
    std::vector<std::vector<const xAOD::TrackParticle *> > preClusters(0);
    std::vector<const xAOD::TrackParticle *> tmp_cluster(0);
    unsigned int previous_bin = histo.begin()->first;
    for(auto & i : histo){
     unsigned int current_bin = i.first;
     if((current_bin - previous_bin)>m_sepNBins){
      //forming a new cluster
      preClusters.push_back(tmp_cluster);
      tmp_cluster.clear();
     }
     // in any case filling tracks into this bin.
     for(const auto *j : i.second) tmp_cluster.push_back(j);
     previous_bin = current_bin;
    }//end of loop over the map entries
    preClusters.push_back(tmp_cluster);

    for(const auto & preCluster : preClusters){

     //------------------------------Debug code -------------------------------------------------------
     /*    std::vector<const Trk::Track *>::const_iterator cb = i->begin();
	   std::vector<const Trk::Track *>::const_iterator ce = i->end();
	   std::cout<<"*********Starting next cluster of size "<<i->size()<<std::endl;
	   for(;cb!=ce;++cb)
	   {
	   std::cout<<"Track Z0 in cluster: "<<(*cb)->perigeeParameters()->parameters()[Trk::z0]<<std::endl;
	   }//end of loop over the cluster entries
     */
     //-------------------------------end of debug code-------------------------------------------------
   
     if(preCluster.size()>m_nRemaining){
      //iterative cleaning until outlying tracks remain
      std::vector<const xAOD::TrackParticle *> tracks_to_clean = preCluster;
      bool clean_again = false;
      do{
       std::pair<std::vector<const Trk::TrackParameters *>,
		 std::vector<const xAOD::TrackParticle *> > clusterAndOutl = m_cleaningTool->clusterAndOutliers(tracks_to_clean, &beamposition);
      
       //if core size is miningfull, storing it
       std::vector<const Trk::TrackParameters *> core_cluster = clusterAndOutl.first;
       std::vector<const xAOD::TrackParticle *> core_outl = clusterAndOutl.second;

       //--------------Debug output-----------------------------------------------------
       //      std::cout<<"Cleaning iteration "<<clean_count<<std::endl;
       //      std::cout<<"Reduced cluster size: "<<core_cluster.size()<<std::endl;
       //      std::cout<<"Outliers size:        "<<core_outl.size()<<std::endl;
       //      ++clean_count;
       //-------------------End of debug output -----------------------------------------

       if(core_cluster.empty()){
        ATH_MSG_DEBUG("Core cluster has 0 size, remaining tracks are discarded.");
        clean_again = false;
       }else{
        //storing clusters with >1 track (optional)
        if(core_cluster.size()>1) result.push_back(core_cluster);	

        //checking the outliers, whether more cleaning is to be done
        if(core_outl.size()>m_nRemaining){
         clean_again = true;
         tracks_to_clean.clear();
         tracks_to_clean = core_outl;
        }else if(core_outl.size()>1){
         clean_again = false;
         ATH_MSG_DEBUG("There were remaining outliers of size: "<< core_outl.size());
         ATH_MSG_DEBUG("Not evident, whether these tracks form a cluster. Rejected...");
	}else clean_again = false;//end of outlier size check
       }//end of core cluster 0 check   

      }while(clean_again);//end of loop

     }else if(preCluster.size()==2){
      //case of two track cluster. accepting without cleaning
      std::vector<const Trk::TrackParameters *> twotrack;
      twotrack.push_back(&(preCluster[0]->perigeeParameters()));
      twotrack.push_back(&(preCluster[1]->perigeeParameters()));
      result.push_back(twotrack);
     }//end of cluster size check
    }//end of loop over all the clusters
   }//end of preselection size check
   return result;
  }

}//end of namespace definitions
