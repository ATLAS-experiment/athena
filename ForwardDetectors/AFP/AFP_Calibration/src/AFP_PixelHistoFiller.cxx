/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AFP_Calibration/AFP_PixelHistoFiller.h"

//local object includes
#include "xAODForward/AFPSiHit.h"
#include "xAODForward/AFPTrack.h"
#include "xAODForward/AFPSiHitsClusterContainer.h"
#include "xAODForward/AFPSiHitsCluster.h"
#include "AFP_Geometry/AFP_constants.h"

//STL
#include <memory> //for unique_ptr

//ROOT
#include "TFile.h"
#include "TH1I.h"

AFP_PixelHistoFiller::AFP_PixelHistoFiller(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{
}


StatusCode AFP_PixelHistoFiller::initialize()
{
    ATH_CHECK( m_eventInfoKey.initialize() );
    ATH_CHECK( m_afpHitContainerKey.initialize() );
    ATH_CHECK( m_afpTrackContainerKey.initialize() );
    
    m_nPixelsX=AFP_CONSTANTS::SiT_Pixel_amount_x;
    m_nPixelsY=AFP_CONSTANTS::SiT_Pixel_amount_y;
    
    for(int st=0;st<m_nStations;++st)
    {
        for(int la=0;la<m_nLayers;++la)
        {
            m_pixelHits[st][la].clear();
            m_pixelCluster[st][la].clear();


            TProfile lb_dx(Form("lb_dx_station_%d_layer_%d", st,la), Form("lb_dx, station %d, layer %d", st,la), 100, 0, 1000);
            m_lb_xDistSiTrackCluster[st][la] = lb_dx;
            
            TProfile lb_dy(Form("lb_dy_station_%d_layer_%d", st,la), Form("lb_dy, station %d, layer %d", st,la), 100, 0, 1000);
            m_lb_yDistSiTrackCluster[st][la] = lb_dy;
            
            TProfile2D lb_yc_dy(Form("lb_yCluster_dy_station_%d_layer_%d", st,la), Form("lb_yCluster_dy, station %d, layer %d", st,la), 100, 0, 1000, 40, -20., 20.);
            m_lb_yCluster_yDistSiTrackCluster[st][la] = lb_yc_dy;
            
            TProfile2D lb_xc_dx(Form("lb_xCluster_dx_station_%d_layer_%d", st,la), Form("lb_xCluster_dx, station %d, layer %d", st,la), 100, 0, 1000, 40, -20., 20.);
            m_lb_xCluster_xDistSiTrackCluster[st][la] = lb_xc_dx;
            
            TProfile2D lb_xC_dy(Form("lb_xCluster_dy_station_%d_layer_%d", st, la), Form("lb_xCluster_dy, station %d, layer %d", st, la), 100, 0, 1000, 40, -20, 20);
            m_lb_xCluster_yDistSiTrackCluster[st][la] = lb_xC_dy;
            
            TProfile2D lb_yC_dx(Form("lb_yCluster_dx_station_%d_layer_%d", st, la), Form("lb_yCluster_dx, station %d, layer %d", st, la), 100, 0, 1000, 40, -20, 20);
            m_lb_yCluster_xDistSiTrackCluster[st][la] = lb_yC_dx;
            
            TProfile2D lb_zC_dx(Form("lb_zCluster_dx_station_%d_layer_%d", st, la), Form("lb_zCluster_dx, station %d, layer %d", st, la), 100, 0, 1000, 100, 9*la - 3, 9*(la+1));
            m_lb_zCluster_xDistSiTrackCluster[st][la] = lb_zC_dx;
            
            TProfile2D lb_zC_dy(Form("lb_zCluster_dy_station_%d_layer_%d", st, la), Form("lb_zCluster_dy, station %d, layer %d", st, la), 100, 0, 1000, 100, 9*la - 3, 9*(la+1));
            m_lb_zCluster_yDistSiTrackCluster[st][la] = lb_zC_dy;
            
            TProfile2D lb_sx_dx(Form("lb_xSlopeTrack_dx_station_%d_layer_%d",st,la), Form("xSlopeTrack_dx, station %d, layer %d", st,la), 100, 0, 1000, 100, -1., 1.);
            m_lb_sxTrack_xDistSiTrackCluster[st][la] = lb_sx_dx;
            
            TProfile2D lb_sy_dy(Form("lb_ySlopeTrack_dy_station_%d_layer_%d", st,la), Form("lb_ySlopeTrack_dy, station %d, layer %d", st,la), 100, 0, 1000, 100, -1., 1.);
            m_lb_syTrack_yDistSiTrackCluster[st][la] = lb_sy_dy;
            
            TProfile2D lb_sy_dx(Form("lb_ySlopeTrack_dx_station_%d_layer_%d", st,la), Form("lb_ySlopeTrack_dx, station %d, layer %d", st,la), 100, 0, 1000, 100, -1., 1.);
            m_lb_syTrack_xDistSiTrackCluster[st][la] = lb_sy_dx;
            
            TProfile2D lb_sx_dy(Form("lb_xSlopeTrack_dy_station_%d_layer_%d", st,la), Form("lb_xSlopeTrack_dy, station %d, layer %d", st,la), 100, 0, 1000, 100, -1., 1.);
            m_lb_sxTrack_yDistSiTrackCluster[st][la] = lb_sx_dy; 
            
        }
    }

    return StatusCode::SUCCESS;
}


StatusCode AFP_PixelHistoFiller::execute(const EventContext& ctx)
{
    
    // get event info
    SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
    if (!eventInfo.isValid())
    {
        ATH_MSG_WARNING("cannot get eventInfo");
        return StatusCode::SUCCESS;
    }
    else ATH_MSG_DEBUG("successfully got eventInfo");
    
    // make sure all the histograms are defined
    int current_lb=eventInfo->lumiBlock();
        
    if(static_cast<int>(m_pixelHits[0][0].size())-1<current_lb/m_LBRangeLength)
    {
        for(int st=0;st<m_nStations;++st)
        {
            for(int la=0;la<m_nLayers;++la)
            {
                for(int add=static_cast<int>(m_pixelHits[st][la].size());add<=current_lb/m_LBRangeLength;++add)
                {
                    TH2F p_hist(Form("pixel_hits_lb_%d_%d_station_%d_layer_%d", add*m_LBRangeLength,(add+1)*m_LBRangeLength-1,st,la), 
                              Form("pixel hits, lb %d-%d, station %d, layer %d", add*m_LBRangeLength,(add+1)*m_LBRangeLength-1,st,la),
                              m_nPixelsX,0.5,m_nPixelsX+0.5, m_nPixelsY,0.5,m_nPixelsY+1);
                    
                    m_pixelHits[st][la].push_back(p_hist);
                    
                    TH2F c_hist(Form("pixel_clusters_lb_%d_%d_station_%d_layer_%d", add*m_LBRangeLength,(add+1)*m_LBRangeLength-1,st,la),
                              Form("pixel_clusters, lb %d-%d, station %d, layer %d", add*m_LBRangeLength,(add+1)*m_LBRangeLength-1,st,la),
                              400,-20, 20, 400, -20, 20);
                    
                    m_pixelCluster[st][la].push_back(c_hist);
                    
                    
                }
                
            }
        }
    }
    
    int lb_index=current_lb/m_LBRangeLength;
    
    // Get containers:
    
    SG::ReadHandle<xAOD::AFPSiHitContainer> afpHitContainer (m_afpHitContainerKey, ctx);
    if (!afpHitContainer.isValid())
    {
        ATH_MSG_WARNING("cannot get AFPSiHitContainer");
        return StatusCode::SUCCESS;
    }
    else ATH_MSG_DEBUG("successfully got AFPSiHitContainer");
    
    SG::ReadHandle<xAOD::AFPTrackContainer> afpTrackContainer (m_afpTrackContainerKey, ctx);
    if(!afpTrackContainer.isValid()){
        ATH_MSG_WARNING("cannot get AFPTrackContainer");
        return StatusCode::SUCCESS;
    }else{
        ATH_MSG_DEBUG("successfully got AFPTrackContainer");
    }
    
    
// Fill histograms:
// Pixel hit histograms:
    
      for (const xAOD::AFPSiHit* hit : *afpHitContainer)
      {
        int st=hit->stationID();
        int la=hit->pixelLayerID();
        
        if(st<0 || m_nStations<=st)
        {
            ATH_MSG_INFO("stationID = " <<st<<", but expected values are from 0 to "<<m_nStations-1 );
            continue;
        }
        if(la<0 || m_nLayers<=la)
        {
            ATH_MSG_INFO("pixelLayerID = " <<la<<", but expected values are from 0 to "<<m_nLayers-1 );
            continue;
        }
        
        m_pixelHits[st][la].at(lb_index).Fill(hit->pixelRowIDChip(),hit->pixelColIDChip());
    }
    
// Histograms for local alignment:
    
    // determining numbers for good track selection
    
    int nTrks[m_nStations]={0};
    int nClusterHits[m_nStations]={0};
    int nClusterHistPerPlane[m_nStations][m_nLayers]={{0}};
    
    for(const xAOD::AFPTrack* track: *afpTrackContainer){
        nTrks[track->stationID()]++;
        for(const auto& cluster : track->clusters()){
            nClusterHits[(*cluster)->stationID()]++;
            nClusterHistPerPlane[(*cluster)->stationID()][(*cluster)->pixelLayerID()]++;
        }
    }
    
    // looping over tracks
        
    for(const xAOD::AFPTrack* track: *afpTrackContainer){
      
      if(nTrks[track->stationID()] != 1) continue;    //exactly 1 track per station
        
      for (const auto& cluster : track->clusters())
        {
          int st=(*cluster)->stationID();
          int la=(*cluster)->pixelLayerID();
        
       
          
          if(st<0 || m_nStations<=st)
          {
              ATH_MSG_INFO("stationID = " <<st<<", but expected values are from 0 to "<<m_nStations-1 );
              continue;
          }
        
          if(la<0 || m_nLayers<=la)
          {
              ATH_MSG_INFO("pixelLayerID = " <<la<<", but expected values are from 0 to "<<m_nLayers-1 );
              continue;
          }
        
          if(nClusterHits[st]<3 || nClusterHits[st]>4) continue; // 3 or 4 cluster hits per track
          if(nClusterHistPerPlane[st][la]>1) continue;  // 1 cluster hit per plane
        
          m_pixelCluster[st][la].at(lb_index).Fill((*cluster)->xLocal(),(*cluster)->yLocal());
        
          double dx = 1e3*(track->xLocal() + (*cluster)->zLocal()*track->xSlope() - (*cluster)->xLocal());
          double dy = 1e3*(track->yLocal() + (*cluster)->zLocal()*track->ySlope() - (*cluster)->yLocal());
        
          m_lb_yCluster_yDistSiTrackCluster[st][la].Fill(current_lb, (*cluster)->yLocal(), dy);
          m_lb_xCluster_xDistSiTrackCluster[st][la].Fill(current_lb, (*cluster)->xLocal(), dx);
          m_lb_sxTrack_xDistSiTrackCluster[st][la].Fill(current_lb, track->xSlope(), dx); 
          m_lb_syTrack_yDistSiTrackCluster[st][la].Fill(current_lb, track->ySlope(), dy); 
          m_lb_syTrack_xDistSiTrackCluster[st][la].Fill(current_lb, track->ySlope(), dx); 
          m_lb_sxTrack_yDistSiTrackCluster[st][la].Fill(current_lb, track->xSlope(), dy);
        
          m_lb_xDistSiTrackCluster[st][la].Fill(current_lb, dx);
          m_lb_yDistSiTrackCluster[st][la].Fill(current_lb, dy);
        
          m_lb_xCluster_yDistSiTrackCluster[st][la].Fill(current_lb, (*cluster)->xLocal(), dy);
          m_lb_yCluster_xDistSiTrackCluster[st][la].Fill(current_lb, (*cluster)->yLocal(), dx);
          m_lb_zCluster_xDistSiTrackCluster[st][la].Fill(current_lb, (*cluster)->zLocal(), dx);
          m_lb_zCluster_yDistSiTrackCluster[st][la].Fill(current_lb, (*cluster)->zLocal(), dy);
          
        }
    
    }
    
    return StatusCode::SUCCESS;
}


StatusCode AFP_PixelHistoFiller::finalize()
{    
    std::unique_ptr<TFile> output_file(new TFile("AFP_PixelHistoFiller.root","recreate"));
    
    TH1I lb("LBRangeLength","LBRangeLength",2,0,2);
    lb.Fill(0.5);
    lb.Fill(1.5,m_LBRangeLength);
    lb.Write();
    
    for(int st=0;st<m_nStations;++st)
    {
        for(int la=0;la<m_nLayers;++la)
        {
            m_lb_xDistSiTrackCluster[st][la].Write();
            m_lb_yDistSiTrackCluster[st][la].Write();
            m_lb_xCluster_yDistSiTrackCluster[st][la].Write();
            m_lb_yCluster_xDistSiTrackCluster[st][la].Write();
            m_lb_zCluster_xDistSiTrackCluster[st][la].Write();
            m_lb_zCluster_yDistSiTrackCluster[st][la].Write();
            m_lb_yCluster_yDistSiTrackCluster[st][la].Write();
            m_lb_xCluster_xDistSiTrackCluster[st][la].Write();
            m_lb_sxTrack_xDistSiTrackCluster[st][la].Write();
            m_lb_syTrack_yDistSiTrackCluster[st][la].Write();
            m_lb_syTrack_xDistSiTrackCluster[st][la].Write();
            m_lb_sxTrack_yDistSiTrackCluster[st][la].Write();
            
            for(TH2F& p_hist: m_pixelHits[st][la])
            {
                p_hist.Write();
            }
            for(TH2F& c_hist: m_pixelCluster[st][la]){
                c_hist.Write();
            }
            
        }
    }
    
    output_file->Close();
    
    return StatusCode::SUCCESS;
}

