/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ConcurrencyFlags.h"

#include "TrigT1NSWSimTools/StripTdsOfflineTool.h"

namespace NSWL1 {

    class StripHits {
    public:
      Identifier      t_id;
      StripOfflineData* t_strip;
      int             t_cache_index;
      StripHits(Identifier id, StripOfflineData* p, int c)
        : t_id (id),
          t_strip (p),
          t_cache_index (c)
      {  }
    };

    using STRIP_MAP=std::map < Identifier,std::vector<StripHits> >;
    using STRIP_MAP_IT=std::map < Identifier,std::vector<StripHits> >::iterator;
    using STRIP_MAP_ITEM=std::pair< Identifier,std::vector<StripHits> >;

    StripTdsOfflineTool::StripTdsOfflineTool( const std::string& type, const std::string& name, const IInterface* parent) :
      AthAlgTool(type,name,parent)
    {
      declareInterface<NSWL1::IStripTdsTool>(this);
    }

  StatusCode StripTdsOfflineTool::initialize() {
    ATH_MSG_DEBUG( "initializing " << name() );

    ATH_MSG_DEBUG( name() << " configuration:");

    ATH_CHECK(m_sTgcDigitContainer.initialize());
    ATH_CHECK(m_sTgcSdoContainer.initialize(m_isMC));

    ATH_CHECK(m_detManagerKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode StripTdsOfflineTool::gather_strip_data(std::vector<std::unique_ptr<StripData>>& strips, const std::vector<std::unique_ptr<PadTrigger>>& padTriggers) const {
      ATH_MSG_DEBUG( "gather_strip_data: start gathering all strip htis");

      std::vector<std::unique_ptr<StripData>> strip_cache;
      ATH_CHECK(fill_strip_cache(padTriggers, strip_cache));

      // delivering the required collection
      for (unsigned int i=0; i< strip_cache.size(); i++) {
        // Check if a stip should be read according to pad triggers
        strips.push_back(std::move(strip_cache.at(i)));
      }
      strip_cache.clear();
      ATH_MSG_DEBUG( "Delivered n. " << strips.size() << " STRIP hits." );
      return StatusCode::SUCCESS;
  }


    StatusCode StripTdsOfflineTool::fill_strip_cache( const std::vector<std::unique_ptr<PadTrigger>>& padTriggers, std::vector<std::unique_ptr<StripData>> &strip_cache) const {
      SG::ReadCondHandle<MuonGM::MuonDetectorManager> detManager{m_detManagerKey, Gaudi::Hive::currentContext()};
      ATH_MSG_DEBUG( "fill_strip_cache: start filling the cache for STRIP hits" );

      if(m_isMC){
	SG::ReadHandle<MuonSimDataCollection> sdo_container(m_sTgcSdoContainer);
	if(!sdo_container.isValid()){
	  ATH_MSG_WARNING("could not retrieve the sTGC SDO container: it will not be possible to associate the MC truth");
	}
      }

      SG::ReadHandle<sTgcDigitContainer> digit_container(m_sTgcDigitContainer);
      if(!digit_container.isValid()){
        ATH_MSG_ERROR("could not retrieve the sTGC Digit container: cannot return the STRIP hits");
      }

      sTgcDigitContainer::const_iterator it   = digit_container->begin();
      sTgcDigitContainer::const_iterator it_e = digit_container->end();

      ATH_MSG_DEBUG( "retrieved sTGC Digit Container with " << digit_container->digit_size() << " collection" );
      int strip_hit_number = 0;
      for(; it!=it_e; ++it) {
        const sTgcDigitCollection* coll = *it;

        ATH_MSG_DEBUG( "processing collection with size " << coll->size() );

        for (unsigned int item=0; item<coll->size(); item++) {
            const sTgcDigit* digit = coll->at(item);
            Identifier Id = digit->identify();
            const MuonGM::sTgcReadoutElement* rdoEl = detManager->getsTgcReadoutElement(Id);
            int channel_type   = m_idHelperSvc->stgcIdHelper().channelType(Id);
            // process only Strip data
            if (channel_type!=1) continue;

            Amg::Vector2D strip_lpos;
            Amg::Vector3D strip_gpos;
            rdoEl->stripPosition(Id,strip_lpos);
            const auto& stripSurface=rdoEl->surface(Id);
            stripSurface.localToGlobal(strip_lpos, strip_gpos, strip_gpos);

            std::string stName = m_idHelperSvc->stgcIdHelper().stationNameString(m_idHelperSvc->stgcIdHelper().stationName(Id));
            int stationEta     = m_idHelperSvc->stgcIdHelper().stationEta(Id);
            int stationPhi     = m_idHelperSvc->stgcIdHelper().stationPhi(Id);
            int wedge          = m_idHelperSvc->stgcIdHelper().multilayer(Id);
            int layer          = m_idHelperSvc->stgcIdHelper().gasGap(Id);
            int channel        = m_idHelperSvc->stgcIdHelper().channel(Id);
            int bctag          = digit->bcTag();

            strip_hit_number++;
            int strip_eta        = 0;
            int strip_phi        = 0;

            ATH_MSG_DEBUG("sTGC Strip hit "    << strip_hit_number << ":  Station Name [" << stName << "]"
                          << "  Station Eta [" << stationEta       << "]"
                          << "  Station Phi [" << stationPhi       << "]"
                          << "  Wedge ["       << wedge            << "]"
                          << "  Layer ["       << layer            << "]"
                          << "  Type ["        << channel_type     << "]"
                          << "  ChNr ["        << channel          << "]"
                          << "  Strip Eta ["   << strip_eta        << "]"
                          << "  Strip Phi ["   << strip_phi        << "]"
                          << "  Strip bcTAg [" << bctag            << "]");

            bool isSmall = m_idHelperSvc->stgcIdHelper().isSmall(Id);
            int trigger_sector = (isSmall)? stationPhi*2 : stationPhi*2-1;//
            int cache_index    = (stationEta>0)? trigger_sector + 16 : trigger_sector;
            ATH_MSG_DEBUG("sTGC Strip hit " << strip_hit_number << ":  Trigger Sector [" << trigger_sector << "]" << "  Cache Index ["  << cache_index << "]" );

            // process STRIP hit time: apply the time delay, set the BC tag for the hit according to the trigger capture window
            auto strip=std::make_unique<StripOfflineData>(Id,&m_idHelperSvc->stgcIdHelper(),digit);
            strip->set_locX(strip_lpos.x());
            strip->set_locY(strip_lpos.y());
            int sideid= (stationEta>0) ? 1 : 0;
            int sectortype= (isSmall==1) ? 0 : 1;
            int sectorid=stationPhi;
            int moduleid=std::abs(stationEta);
            int wedgeid=wedge;
            int layerid=layer;
            strip->setSideId(sideid);
            strip->setSectorType(sectortype);
            strip->setSectorId(sectorid);
            strip->setModuleId(moduleid);
            strip->setWedgeId(wedgeid);
            strip->setLayerId(layerid);
            strip->set_globX(strip_gpos.x());
            strip->set_globY(strip_gpos.y());
            strip->set_globZ(strip_gpos.z());
            strip->set_locZ(0);

            bool read_strip=readStrip(strip.get(),padTriggers);
            if (read_strip && (strip->bandId() ==-1 || strip->phiId()==-1 ) ) {
                ATH_MSG_FATAL("StripTdsOfflineTool:NO MATCH ALL " << "\n"
                              << "wedge:" << strip->wedge() << "\n"
                              << "layer:"<< strip->layer() << "\n"
                              << "loc_x:"<< strip->locX()<< "\n");
            }

            //set coordinates above ! readStrip needs that variables !
            strip->set_readStrip(read_strip);
            strip_cache.push_back(std::move(strip));
        }//collections
      }//items
      ATH_MSG_DEBUG( "fill_strip_cache: end of processing" );
      return StatusCode::SUCCESS;
  }


  bool StripTdsOfflineTool::readStrip(StripData* strip,const std::vector<std::unique_ptr<PadTrigger>>& padTriggers) const {

    if (strip->bandId() !=-1) {
      ATH_MSG_DEBUG("StripTdsOfflineTool:ReadStrip: BandId already set\n" << "moduleID:" << strip->moduleId() +1 << "\n"
                    << "sectiorID:" << strip->sectorId() + 1 << "\n" << "layer:" << strip->wedge() << "\n");
    }
    if (strip->phiId() !=-1) {
      ATH_MSG_DEBUG("StripTdsOfflineTool:ReadStrip: PhiId already set\n" << "moduleID:"<< strip->moduleId() +1 << "\n"
                    << "sectiorID:" << strip->sectorId() + 1 << "\n" << "layer:" << strip->wedge() << "\n");
    }
    for(const std::unique_ptr<PadTrigger>& trig :padTriggers){
      //std::shared_ptr<PadData> padIn=trig->firstPadInner();
      for(const std::shared_ptr<PadData>& pad : trig->m_pads){
        if (strip->sideId()!=pad->sideId() ||
            strip->isSmall()==pad->sectorType() || //strip returns 1 pad returns 0
            strip->sectorId()!=pad->sectorId() ||  
            std::abs(strip->etaCenter() )> trig->etaMax() || //use abs / sideC
            std::abs(strip->etaCenter() ) < trig->etaMin() ||
            strip->layer()!=pad->gasGapId()
           ) continue;
          else {
            strip->setBandId(trig->bandId());
            strip->setPhiId(trig->phiId());
            return true;
          }
      }//pad loop
    }//padtrigger loop
    return false;
  }
}
