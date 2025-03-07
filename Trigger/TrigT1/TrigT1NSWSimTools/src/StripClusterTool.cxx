/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ConcurrencyFlags.h"

#include "TrigT1NSWSimTools/StripClusterTool.h"

namespace NSWL1 {

  StripClusterTool::StripClusterTool( const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type,name,parent),
    m_detManager(nullptr)
  {
    declareInterface<NSWL1::IStripClusterTool>(this);
  }

  StatusCode StripClusterTool::initialize() {
    ATH_MSG_DEBUG( "initializing " << name() );

    ATH_CHECK(m_sTgcSdoContainerKey.initialize(m_isMC));

    // retrieve the MuonDetectormanager
    ATH_CHECK(detStore()->retrieve( m_detManager ));
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode StripClusterTool::fill_strip_validation_id(const EventContext& ctx,
							std::vector<std::unique_ptr<StripClusterData>>& clusters,
                                                        std::vector<std::shared_ptr<std::vector<std::unique_ptr<StripData> >>  > &cluster_cache) const {

    ATH_MSG_DEBUG("Cluster cache received " << cluster_cache.size());

    bool first_strip=true;
    for(const auto &this_cl : cluster_cache){
      float x_pos=0;
      float y_pos=0;
      float z_pos=0;

      float x_lpos=0;
      float y_lpos=0;
      float z_lpos=0;

      int charge=0;
      int n_strip=0;

      first_strip=true;
      float locx=-999999;
      float locy=-999999;
      if (this_cl->empty()){
        ATH_MSG_WARNING("Zero size cluster!!");
        continue;
      }

      const MuonSimDataCollection* sdo_container = nullptr;
      if(m_isMC) {
        SG::ReadHandle<MuonSimDataCollection> readMuonSimDataCollection( m_sTgcSdoContainerKey, ctx );
        if( !readMuonSimDataCollection.isValid() ){
          ATH_MSG_WARNING("could not retrieve the sTGC SDO container: it will not be possible to associate the MC truth");
          return StatusCode::FAILURE;
        }
        sdo_container = readMuonSimDataCollection.cptr();
      }

      for(const auto &strip_cl : *this_cl){
        n_strip++;
        ATH_MSG_DEBUG("Start strip" << n_strip);
        // Save truth deposits associated with cluster should be the same on for the whole strip, so take the first one need to work on this logic
        if(m_isMC && first_strip) {
          first_strip=false;
          Identifier Id = strip_cl->Identity();
          const MuonGM::sTgcReadoutElement* rdoEl = m_detManager->getsTgcReadoutElement(Id);
          auto it = sdo_container->find(Id);
          if(it == sdo_container->end()) continue;
          const MuonSimData strip_sdo = it->second;
          std::vector<MuonSimData::Deposit> deposits;
          strip_sdo.deposits(deposits);
          //retrieve the info of the first associated hit, i.e. the fastest in time
          if (deposits.size()!=1) ATH_MSG_WARNING("Multiple cluster hits for strip!");
          if (deposits.empty()){
            ATH_MSG_WARNING("Empty hit here");
            continue;
          }

          int    truth_barcode   = deposits[0].first.barcode();
          double truth_localPosX = deposits[0].second.firstEntry();
          double truth_localPosY = deposits[0].second.secondEntry();
          Amg::Vector3D hit_gpos(0.,0.,0.);
          Amg::Vector2D lpos(truth_localPosX,truth_localPosY);
          rdoEl->surface(Id).localToGlobal(lpos, hit_gpos,hit_gpos);
          double truth_globalPosX = hit_gpos.x();
          double truth_globalPosY = hit_gpos.y();
          double truth_globalPosZ = hit_gpos.z();
          float  truth_energy     = strip_sdo.word();

          if(std::abs(locx-lpos.x())>.001 || std::abs(locy - lpos.y())>.001){
            ATH_MSG_DEBUG("OLD locx " << locx << " new locx " << lpos.x() << " b " << int(locx!=lpos.x()));
            ATH_MSG_DEBUG("OLD locy " << locy << " new locy " << lpos.y() << " b " << int(locy!=lpos.y()));
            ATH_MSG_DEBUG("Cluster hit, truth barcode = " << truth_barcode);
            ATH_MSG_DEBUG("Cluster hit, truth globalPosX = " << truth_globalPosX
                          << ", truth globalPosY = " << truth_globalPosY
                          << ", truth globalPosZ = " << truth_globalPosZ
                          << ", truth enegy deposit = " << truth_energy);
            ATH_MSG_DEBUG("Cluster hit, truth localPosX = " << lpos.x()
                          << ", truth localPosY = " << lpos.y()
                          << ", truth enegy deposit = " << truth_energy);
          }
        }

        float s_charge=strip_cl->strip_charge_6bit();
        charge+=s_charge;
        x_pos+=strip_cl->globX()*s_charge;
        y_pos+=strip_cl->globY()*s_charge;
        z_pos+=strip_cl->globZ()*s_charge;

        x_lpos+=(strip_cl->locX())*s_charge;
        y_lpos+=(strip_cl->locY())*s_charge;
        z_lpos+=(strip_cl->locZ())*s_charge;


        ATH_MSG_DEBUG("Cluster ------------------------------------------" );
        ATH_MSG_DEBUG("Cluster strip charge: " << s_charge);
        ATH_MSG_DEBUG("Cluster strip loc X: " << strip_cl->locX());
        ATH_MSG_DEBUG("Cluster strip loc Y: " << strip_cl->locY());
        ATH_MSG_DEBUG("Cluster strip glob X: " << strip_cl->globX());
        ATH_MSG_DEBUG("Cluster strip glob Y: " << strip_cl->globY());
        ATH_MSG_DEBUG("Cluster strip glob Z: " << strip_cl->globZ());
        ATH_MSG_DEBUG("Cluster strip locx dist: " << locx-strip_cl->locX());
        ATH_MSG_DEBUG("Cluster strip charge o dist: " << s_charge/(locx-strip_cl->locX()));
        ATH_MSG_DEBUG("Channel " << strip_cl->channelId());

      }//end of this_cl loop

      if (charge != 0){
        if ( std::abs(x_pos/charge)<200. && std::abs(y_pos/charge)<200.){
          ATH_MSG_WARNING("Cluster ------------------------------------------" );
          ATH_MSG_WARNING("Cluster strip charge: " << charge );
          ATH_MSG_WARNING("Cluster strip glob X: " << x_pos << x_pos/charge);
          ATH_MSG_WARNING("Cluster strip glob Y: " << y_pos << y_pos/charge);
          ATH_MSG_WARNING("Cluster strip glob Z: " << z_pos << z_pos/charge);
        }
        x_pos=x_pos/charge;
        y_pos=y_pos/charge;
        z_pos=z_pos/charge;
        x_lpos=x_lpos/charge;
        y_lpos=y_lpos/charge;
        z_lpos=z_lpos/charge;
      }
      ATH_MSG_DEBUG("Cluster dump with X:" << x_pos << " Y: " << y_pos << " Z: " << z_pos << " cluster charge: " << charge);
      ATH_MSG_DEBUG("Cluster dump with lX:" << x_lpos << " lY: " << y_lpos << " lZ: " << z_lpos << " cluster charge: " << charge);

      auto stripClOfflData=std::make_unique<StripClusterOfflineData>(
               this_cl->at(0)->bandId(),
               this_cl->at(0)->trig_BCID(),
               this_cl->at(0)->sideId(),
               this_cl->at(0)->phiId(),
               this_cl->at(0)->isSmall(),
               this_cl->at(0)->moduleId(),
               this_cl->at(0)->sectorId(),
               this_cl->at(0)->wedge(),
               this_cl->at(0)->layer(),
               n_strip,
               charge,
               x_pos,
               y_pos,
               z_pos);
      clusters.push_back(std::move(stripClOfflData));
    }
    return StatusCode::SUCCESS;
  }


  StatusCode StripClusterTool::cluster_strip_data(const EventContext& ctx, std::vector<std::unique_ptr<StripData>>& strips, std::vector< std::unique_ptr<StripClusterData> >& clusters) const {

      if(strips.empty()){
        ATH_MSG_WARNING("Received 0 strip hits... Skip event");
        return StatusCode::SUCCESS;
      }
      
      std::map<uint32_t, std::vector<std::unique_ptr<StripData>> > stripMap;
      // Filter by layer:
      for (auto& st : strips) {
        // sideId [0, 1]
        auto sideId = st->sideId();
        // sectorType: small==0, large==1
        auto sectorType = st->sectorType();
        // sectorId [1,8]
        auto sectorId = st->sectorId();
        // etaId [1,3]
        auto etaId = st->moduleId();
        // wedgeId [1,2]
        auto wedgeId = st->wedge();
        // layer [1,4]
        auto layerId = st->layer();
        
        // add 1 as the first placeholder, hence cache_hash always has 7 digits
        std::string id_str = std::to_string(1)+std::to_string(sideId)+std::to_string(sectorType)+std::to_string(sectorId)+std::to_string(etaId)+std::to_string(wedgeId)+std::to_string(layerId);
        const uint32_t cache_hash = atoi(id_str.c_str()); 

        auto it = stripMap.find(cache_hash);
        if (it != stripMap.end()){
          it->second.push_back(std::move(st));
        }
        else{
          stripMap[cache_hash].push_back(std::move(st));
        }

      }
      
      std::vector< std::shared_ptr<std::vector<std::unique_ptr<StripData> >>  > cluster_cache;
      for (auto &item : stripMap) {
        std::vector<std::unique_ptr<StripData>>& stripList = item.second;
      
        //S.I sort strip w.r.t channelId in ascending order
        std::sort(stripList.begin(), stripList.end(), [](const auto& s1,const auto& s2) { return s1->channelId()<s2->channelId(); });

        auto hit=stripList.begin();
        ATH_MSG_DEBUG("Cluster Hits :" << (*hit)->channelId() << " " << m_idHelperSvc->stgcIdHelper().gasGap( (*hit)->Identity())
                      << "   " << (*hit)->moduleId() << "   " << (*hit)->sectorId() << "   " << (*hit)->wedge() << "  " << (*hit)->sideId() );
        int first_ch=(*hit)->channelId();//channel id of the first strip
        int prev_ch=-1;

        auto cr_cluster=std::make_shared< std::vector<std::unique_ptr<StripData>> >();
        
        for(auto& this_hit : stripList){
          if(!(this_hit)->readStrip() ) continue;
          if( ((this_hit)->bandId()==-1 || this_hit->phiId()==-1) ){
            ATH_MSG_WARNING("Read Strip without BandId :" << (this_hit)->channelId() << " " << m_idHelperSvc->stgcIdHelper().gasGap( (this_hit)->Identity())
                            << "   " << (this_hit)->moduleId() << "   " << (this_hit)->sectorId() << "   " << (this_hit)->wedge() << "  " << (this_hit)->sideId() );
            continue;
          }

          if (prev_ch==-1){//for the first time...
            prev_ch = first_ch;
            cr_cluster->push_back(std::move(this_hit));
            continue;
          }
                 
          int this_ch=(this_hit)->channelId();
          if ( (this_ch < prev_ch)) {
            ATH_MSG_ERROR("Hits Ordered incorrectly!!!" );
            return StatusCode::FAILURE;
          }

          if (this_ch == prev_ch || this_ch == prev_ch+1) cr_cluster->push_back(std::move(this_hit)); // form cluster with adjacent +-1 strips 
          else {
            cluster_cache.push_back(std::move(cr_cluster));                         //put the current cluster into the clusters buffer
            cr_cluster=std::make_shared<std::vector<std::unique_ptr<StripData>>>(); //create a new empty cluster and assign this hit as the first hit
            cr_cluster->push_back(std::move(this_hit));
          }

          prev_ch=this_ch;
        }

        if(!cr_cluster->empty()) cluster_cache.push_back(std::move(cr_cluster));    //don't forget the last cluster in the loop
      }

      ATH_MSG_DEBUG("Found :" << cluster_cache.size() << " clusters");
      ATH_CHECK(fill_strip_validation_id(ctx, clusters, cluster_cache));
      cluster_cache.clear();

      return StatusCode::SUCCESS;
  }
}
