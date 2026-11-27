/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "TrigT2CaloCommon/LArCellCont.h"

#include "AthenaKernel/errorcheck.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloEvent/CaloBCIDAverage.h"
#include "CaloUtils/CaloCellCorrection.h"
#include "LArBadChannelTool/LArBadFebMasker.h"
#include "LArRecConditions/LArRoIMap.h"
#include "LArRecUtils/MakeLArCellFromRaw.h"

#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/EventContext.h"

#include <limits>

// For debugging only
//#define TRIGLARCELLDEBUG
#ifdef TRIGLARCELLDEBUG
#include <iostream>
#define LOG_DEBUG(msg) do { std::cout << "LArCellCont DEBUG " << msg << std::endl; } while(0)
#else
#define LOG_DEBUG(msg) do {} while (0)
#endif


StatusCode
LArCellCont::initialize( const LArRoIMap& roiMap,
                         const LArOnOffIdMapping& onOffMap,
                         const LArMCSym& mcsym, 
                         const LArFebRodMapping& febrod, 
                         const LArBadChannelCont& badchannel, 
                         const CaloDetDescrManager& man) {

  LOG_DEBUG("in initialize");

  ServiceHandle<IToolSvc> toolSvc("ToolSvc", "LArCellCont");
  CHECK_WITH_CONTEXT( toolSvc.retrieve(), "LArCellCont");

  ServiceHandle<StoreGateSvc> detStore("DetectorStore", "LArCellCont");
  CHECK_WITH_CONTEXT( detStore.retrieve(), "LArCellCont");

  const LArOnlineID* onlineId = nullptr;
  CHECK_WITH_CONTEXT( detStore->retrieve(onlineId,"LArOnlineID"), "LArCellCont" );

  CHECK_WITH_CONTEXT( m_conv.initialize(febrod), "LArCellCont" );
  m_hash.initialize(0, febrod.getLArRoModIDvec() );

  const bool toolAvailable = toolSvc->retrieveTool("LArBadFebMasker", m_badFebMasker).isSuccess();
  if(!toolAvailable) //not a critical error. LArCellCont can proceed as usual, without masking.
    REPORT_MESSAGE_WITH_CONTEXT(MSG::INFO, "LArCellCont") <<
      "Failed to retrieve LArBadFebMasker - no masking will be done." << endmsg;
  std::vector<uint32_t> RobsFromMissingFeb;
  RobsFromMissingFeb.reserve(10);

  size_t count = 0;
  for( const HWIdentifier& id : onlineId->channel_range() ){
	HWIdentifier hwid = mcsym.ZPhiSymOnl(id);
    if ( m_indexset.try_emplace (hwid, count).second ) {
      ++count;
    }
  } // end of loop over online IDs
  const size_t indexsetmax = m_indexset.size();
  // the extra indexsetmax is used for invalid cells
  m_corrBCID.resize(indexsetmax+1,0.0);

  const HWIdentifier larrodid(0);
  this->reserve(m_hash.max());
  m_second.resize(m_hash.max(),HWIdentifier(0));
  m_eventNumber.resize(m_hash.max(),std::numeric_limits<unsigned int>::max());
  for(size_t i=0;i<m_hash.max();i++){
    LOG_DEBUG(i << " " << std::hex << m_hash.identifier(i) << std::dec);

    // A collection per ROD/ROB/HashId
    LArCellCollection* mycoll = new LArCellCollection(larrodid);
    mycoll->reserve(256);
    for(int ch=0;ch<256;ch++) mycoll->push_back(new LArCell());
    this->push_back(mycoll);
  } // end of for id

  uint32_t febHashMax = onlineId->febHashMax();
  m_hashSym.clear();
  m_hashSym.resize(m_hash.max());
  for(uint32_t i=0;i<m_hash.max();i++) m_hashSym[i].reserve(256);
  // vectors to organize quantities
  std::vector<const CaloDetDescrElement*> ddes;
  std::vector<LArRoIMap::TT_ID> ttids;
  std::vector<bool> cell_ok;
  std::vector<bool> pushone;
  ddes.resize(128,nullptr);
  ttids.resize(128);
  cell_ok.resize(128);
  pushone.resize(m_hash.max(),false);

  for (size_t iFeb=0; iFeb<febHashMax; ++iFeb) {
    const HWIdentifier& febid=onlineId->feb_Id(IdentifierHash(iFeb));
    if( (toolAvailable && (m_badFebMasker->febMissing(febid)) ) || !toolAvailable ){
      RobsFromMissingFeb.push_back( m_conv.getRobID( m_conv.getRodID( febrod, febid ) ) );
    }
    if( (toolAvailable && !(m_badFebMasker->febMissing(febid)) ) || !toolAvailable ){
      // get RodID associated with the collection
      const HWIdentifier& rodId = febrod.getReadoutModuleID(febid);
      const unsigned int rodId32 = m_conv.getRodIDFromROM(rodId);
      cell_ok.assign(128,false);
      // index in the collection vector
      const size_t idx = m_hash(rodId32);
	
      LOG_DEBUG(idx << " " << std::hex << m_hash.identifier(idx) << std::dec);

      // get all channels for a FEB
      if ( pushone[idx] ) { // This is the second FEB
		m_second[idx] = febid;
      }
      std::vector<int>& hashTab = m_hashSym[idx];
      for(int ch=0; ch<128; ch++){
        HWIdentifier onlId = onlineId->channel_Id(febid,ch);
        HWIdentifier hwsym = mcsym.ZPhiSymOnl(onlId);
        ddes[ch] = man.get_element(onOffMap.oflHashFromOnlHash(onlineId->channel_Hash(onlId)));
        const CaloDetDescrElement* dde = ddes[ch];
        if ( dde ) { // if it is a good cell
          LArRoIMap::TT_ID ttId = roiMap.trigTowerID(onlId);
          ttids[ch]=ttId;
          LArBadChannel bc = badchannel.offlineStatus(dde->identify());

          const bool bad = !bc.good() &&
            // cell has some specific problems
            ( bc.deadReadout() ||
              bc.deadPhys() ||
              bc.highNoiseHG() ||
              bc.highNoiseMG() ||
              bc.highNoiseLG() ||
              bc.problematicForUnknownReason() );

          if ( !bad ) cell_ok[ch]=true; // cell masked if not know to be good
          auto itr = m_indexset.find( hwsym );
          if ( itr != m_indexset.end() ){
            const int index = itr->second;
            hashTab.push_back( index );
          } else hashTab.push_back(indexsetmax);
        } else {// add a dummy cell
          ddes[ch]=nullptr;
          cell_ok[ch]=false;
          ttids[ch]=0xffffffff;
          hashTab.push_back( indexsetmax);
        } // end of if bad cell
      } // end of for ch loop
      LArCell* larcell;
      int add=0;
      LArCellCollection* collection = (*this)[idx];
      if (pushone[idx]) add=128;
      for(int ch=0;ch<128;ch++){
        larcell = collection->at(ch+add);
	larcell ->setCaloDDE(ddes[ch]);
        larcell->setGain(CaloGain::LARHIGHGAIN);
      }
      std::map<LArRoIMap::TT_ID,std::vector<LArCell* > > collMap;
      for(int ch=0;ch<128;ch++){
        if ( !cell_ok[ch] ) continue;
        larcell = collection->at(ch+add);
        if ( (ddes[ch]!=nullptr) && larcell) {
          collMap[ttids[ch]].push_back(larcell);
        }
      }
#ifdef TRIGLARCELLDEBUG
      bool first=true;
      LArCell* p1=nullptr;
      larcell=nullptr;
      int max_dist=0;
      for(int ch=0;ch<128;ch++){
        larcell = collection->at(ch+add);
        if ( larcell ) { // if it is a good cell
          if(first) {
              p1=larcell;
              first=false;
          } else {
              int dist=larcell-p1;
              if (dist < 0 ) dist=-dist;
              if ( dist > max_dist ) max_dist=dist;
          }
	} //end of if larcell
      }
      LOG_DEBUG("For FEB : 0x" << std::hex << febid.get_identifier32().get_compact() << " max_dist : " << std::dec << max_dist );
#endif
      pushone[idx]=true;

      for (const auto& [ttid, cells] : collMap) {
		// Ones needs to dump the mapped vector to an allocated vector
		auto vec = new DataVector<LArCell>(SG::VIEW_ELEMENTS);
		vec->reserve(cells.size());
		for(LArCell* c : cells) {
          vec->push_back(c);
        }
		collection->setTT(ttid,vec->begin(),vec->end());
		m_vecs.push_back(vec);
      }

    } // End of check for missing FEB
  } // end of FEB for

  m_MissingROBs.clear();
  for(size_t i = 0 ; i < RobsFromMissingFeb.size() ; i++)
    for(size_t j = i+1 ; j < RobsFromMissingFeb.size() ; j++)
      if ( RobsFromMissingFeb[i] == RobsFromMissingFeb[j] )
        m_MissingROBs.push_back(RobsFromMissingFeb[i]);

  return StatusCode::SUCCESS;
}

StatusCode LArCellCont::finalize() {
  for(size_t i=0; i<m_hash.max(); i++){
    LArCellCollection* coll = (*this)[i];
	for(size_t j=0; j<coll->size(); j++) {
      delete (*coll)[j];
    }
    delete coll;
  }
  // Destroy also the vector of vectors to TT maps.
  for(size_t i=0; i<m_vecs.size(); i++) delete m_vecs[i];

  m_hashSym.clear();
  return StatusCode::SUCCESS;
}

// This WILL NOT trigger BSCNV. This assumes BSCNV was done before
const std::vector<LArCellCollection*>::const_iterator
LArCellCont::find(const HWIdentifier& rodid) const{
  const size_t idx = m_hash(m_conv.getRodIDFromROM (rodid));
  return (this->begin()+idx);
}

// This WILL NOT trigger BSCNV. This assumes BSCNV was done before
const std::vector<LArCellCollection*>::const_iterator
LArCellCont::find(unsigned int rodid) {
  const size_t idx = m_hash(rodid);
  if ( m_eventNumber[idx] != m_event ) { // Decoding a new event
    m_eventNumber[idx] = m_event;
    return (this->begin()+idx);
    // Keep track of last decoded number
  } else { // Event already decoded. Return Null
    return this->end();
  }
}

void LArCellCont::applyBCIDCorrection(unsigned int rodid) {
  const size_t idx = m_hash(rodid);
  LArCellCollection* col = *(this->begin()+idx);
  std::vector<int>& hashTab = m_hashSym[idx];
  for(size_t i=0; i<col->size(); ++i){
    LArCell* cell = (*col)[i];
    const float cellenergy = cell->energy();
    const float cor = m_corrBCID[ hashTab[i] ];
    cell->setEnergyFast( cellenergy  - cor );
    LOG_DEBUG("LArCellId= " << cell->ID() << " Ecorr= " << cor << " MeV");
  }
  return;
}

HWIdentifier
LArCellCont::findsec(unsigned int rodid) const{
  const size_t idx = m_hash(rodid);
  return m_second[idx];
}

void LArCellCont::lumiBlock_BCID(const unsigned int lumi_block, const unsigned int BCID){
  if ( m_bcid != BCID  ) {

    LOG_DEBUG("Update : CURRENT lumi_block, BCID = " << lumi_block << ", " << BCID <<
              " <--> PREVIOUS lumi_block = " << m_lumi_block << ", " << m_bcid);

    m_lumi_block = lumi_block;
    m_bcid = BCID;
    m_BCIDcache = false;
  }    
}

void LArCellCont::updateBCID( const CaloBCIDAverage& avg, const LArOnOffIdMapping& onoff ) {

  for (const auto& [hwid, idx] : m_indexset) {
    if ( idx < m_corrBCID.size() ){
      Identifier id = onoff.cnvToIdentifier(hwid);
      float corr = avg.average(id);
      m_corrBCID[idx] = corr;
    }
  }
}

bool LArCellCont::lumiBCIDCheck( const EventContext& context ) {
  const unsigned int bcid = context.eventID().bunch_crossing_id();
  if ( bcid == m_bcid ) return false;

  const EventIDBase::event_number_t evN = context.eventID().event_number();
  if ( evN == m_bcidEvt ) return false;

  m_bcid = bcid;
  m_bcidEvt = evN;
  return true;
}
