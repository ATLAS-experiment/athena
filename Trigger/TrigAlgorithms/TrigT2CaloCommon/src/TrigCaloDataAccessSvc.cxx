/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCaloDataAccessSvc.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "IRegionSelector/IRoiDescriptor.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandle.h"


StatusCode TrigCaloDataAccessSvc::initialize() {

  /// Temporary fix
  m_autoRetrieveTools = false;
  m_checkToolDeps = false;
          
  ATH_CHECK( m_larDecoder.retrieve() );
  ATH_CHECK( m_tileDecoder.retrieve() );
  ATH_CHECK( m_robDataProvider.retrieve() );
  ATH_CHECK( m_bcidAvgKey.initialize() );
  ATH_CHECK( m_onOffIdMappingKey.initialize() );
  ATH_CHECK( m_larRoIMapKey.initialize() );
  ATH_CHECK( m_febRodMappingKey.initialize() );
  ATH_CHECK( m_regionSelector_TTEM.retrieve() );
  ATH_CHECK( m_mcsymKey.initialize() );
  ATH_CHECK( m_bcContKey.initialize() );
  ATH_CHECK( m_caloMgrKey.initialize());
  ATH_CHECK( m_regionSelector_TTHEC.retrieve() );
  ATH_CHECK( m_regionSelector_FCALEM.retrieve() );
  ATH_CHECK( m_regionSelector_FCALHAD.retrieve() );
  ATH_CHECK( m_regionSelector_TILE.retrieve() );
  ATH_CHECK( m_deadOTXFromSCKey.initialize(SG::AllowEmpty) );
  m_correctDead = !m_deadOTXFromSCKey.empty();
  ATH_CHECK( m_tileHid2RESrcIDKey.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode TrigCaloDataAccessSvc::finalize() {

  m_vrodid32fullDet.clear();
  m_vrodid32fullDetHG.clear();
  for (HLTCaloEventCache& cache : m_hLTCaloSlot) {
    if (cache.larContainer) ATH_CHECK( cache.larContainer->finalize() );
    if (cache.tileContainer) ATH_CHECK( cache.tileContainer->finalize() );
    if (cache.d0cells) cache.d0cells->clear();
    if (cache.fullcont) {
      for (unsigned int i : m_insertedCells) delete cache.fullcont->at(i);
    }
  }

  return StatusCode::SUCCESS;
}

void TrigCaloDataAccessSvc::reset_LArCol ( LArCellCollection* coll ){
  for (LArCell* cell : *coll) {
    cell->setEnergyFast(0.0);
  }
}

void TrigCaloDataAccessSvc::reset_TileCol(TileCellCollection* col) {
  for (TileCell* cell: *col) {
    cell->setEnergy_nonvirt(0.0F, 0.0F, 0, CaloGain::INVALIDGAIN);
    cell->setTime_nonvirt(-100.0F);
    cell->setQuality_nonvirt(static_cast<unsigned char>(255), 0, 0);
    cell->setQuality_nonvirt(static_cast<unsigned char>(255), 0, 1);
  }
}

StatusCode TrigCaloDataAccessSvc::loadCollections ( const EventContext& context,
                                                    const IRoiDescriptor& roi,
                                                    const DETID detID,
                                                    const int sampling,
                                                    LArTT_Selector<LArCellCont>& loadedCells ) {

  std::vector<IdentifierHash> requestHashIDs;  

  ATH_MSG_DEBUG( "LArTT requested for event " << context << " and RoI " << roi );  
  ATH_CHECK( prepareLArCollections(context, roi, sampling, detID) );

  {
    // this has to be guarded because getTT called on the LArCollection bu other threads updates internal map
    std::scoped_lock lock{m_hLTCaloSlot.get( context )->mutex};
    switch ( detID ) {
    case TTEM: {m_regionSelector_TTEM->lookup(context)->HashIDList( sampling, roi, requestHashIDs ); break; }
    case TTHEC: {m_regionSelector_TTHEC->lookup(context)->HashIDList( sampling, roi, requestHashIDs ); break; }
    case FCALEM: {m_regionSelector_FCALEM->lookup(context)->HashIDList( sampling, roi, requestHashIDs ); break; }
    case FCALHAD: {m_regionSelector_FCALHAD->lookup(context)->HashIDList( sampling, roi, requestHashIDs ); break; }
    default: break;
    }
  }
  
  ATH_MSG_DEBUG( "requestHashIDs.size() in LoadColl = " << requestHashIDs.size()  << " hash checksum " 
                 << std::accumulate( requestHashIDs.begin(), requestHashIDs.end(), IdentifierHash( 0 ),
                                     []( IdentifierHash h1, IdentifierHash h2 ){  return h1+h2; } ) );
  if ( msgLvl( MSG::VERBOSE ) ) {    
    for( unsigned int i = 0 ; i < requestHashIDs.size() ; i++ )
      ATH_MSG_VERBOSE( "m_rIds[" << i << "]=" << requestHashIDs[i] );
  }
  SG::ReadCondHandle<LArRoIMap> roimap ( m_larRoIMapKey, context);
  loadedCells.setContainer( m_hLTCaloSlot.get( context )->larContainer.get() );
  loadedCells.setMap( *roimap );

  { 
    // this has to be guarded because getTT called on the LArCollection bu other threads updates internal map
    std::scoped_lock lock{m_hLTCaloSlot.get( context )->mutex};
    loadedCells.setRoIs( requestHashIDs );
  }
  return StatusCode::SUCCESS;
}

StatusCode TrigCaloDataAccessSvc::loadCollections ( const EventContext& context,
                                                    const IRoiDescriptor& roi,
                                                    std::vector<const TileCell*>& loadedCells ) {
  std::vector<IdentifierHash> requestHashIDs;

  ATH_MSG_DEBUG( "Tile requested for event " << context << " and RoI " << roi );
  ATH_CHECK( prepareTileCollections(context, roi) );

  {
    // this has to be guarded because getTT called on the LArCollection bu other threads updates internal map
    std::scoped_lock lock{m_hLTCaloSlot.get( context )->mutex};

    m_regionSelector_TILE->lookup(context)->HashIDList( roi, requestHashIDs );
  }
  ATH_MSG_DEBUG( "requestHashIDs.size() in LoadColl = " << requestHashIDs.size()  << " hash checksum "
                 << std::accumulate( requestHashIDs.begin(), requestHashIDs.end(), IdentifierHash( 0 ),
                                     []( IdentifierHash h1, IdentifierHash h2 ){  return h1+h2; } ) );
  if ( msgLvl( MSG::VERBOSE ) ) {
    for( unsigned int i = 0 ; i < requestHashIDs.size() ; i++ )
      ATH_MSG_VERBOSE( "requestHashIDs[" << i << "]=" << requestHashIDs[i] );
  }

  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );
  const TileCellCont* tilecell = cache->tileContainer.get();

  loadedCells.clear();
  for (size_t i = 0; i < requestHashIDs.size(); ++i){
    // Find the collection to dump
    const auto it = tilecell->find(requestHashIDs[i]);
    if (it == tilecell->end()) continue;

    TileCellCollection* col = *it;
    if ( col == nullptr ) continue;

    for (const TileCell* cell : *col) {
      loadedCells.push_back( cell );
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode TrigCaloDataAccessSvc::loadFullCollections ( const EventContext& context,
                                                        CaloConstCellContainer& cont ) {

  ATH_CHECK( prepareLArFullCollections( context ) );

  ATH_CHECK( prepareTileFullCollections( context ) );

  m_hLTCaloSlot.get(context)->lastFSEvent = context.evt();

  std::scoped_lock lock{m_getCollMutex};

  const CaloCellContainer* cont_to_copy = m_hLTCaloSlot.get(context)->fullcont.get() ;
  cont.clear();
  cont.reserve( cont_to_copy->size() );
  for( const CaloCell* c : *cont_to_copy ) {
    cont.push_back_fast( c );
  }
  cont.setIsOrdered(true);
  cont.setIsOrderedAndComplete(true);
      
  return StatusCode::SUCCESS;
}


StatusCode TrigCaloDataAccessSvc::prepareLArFullCollections( const EventContext& context) {

  std::call_once(m_lateInitFlag, &TrigCaloDataAccessSvc::lateInit, this, context);

  ATH_MSG_DEBUG( "Full Col " << " requested for event " << context );

  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );
  
  auto lockTime = Monitored::Timer ( "TIME_locking_LAr_FullDet" );
  std::scoped_lock lock{cache->mutex};

  lockTime.stop();

  // If the full event was already unpacked, don't need to unpack RoI
  if ( cache->lastFSEvent == context.evt() ) return StatusCode::SUCCESS;
  cache->larContainer->eventNumber( context.evt() ) ;

  if ( m_applyOffsetCorrection && cache->larContainer->lumiBCIDCheck( context ) ) {
	SG::ReadHandle<CaloBCIDAverage> avg (m_bcidAvgKey, context);
	SG::ReadCondHandle<LArOnOffIdMapping> onoff (m_onOffIdMappingKey, context);
	if ( avg.cptr() && onoff.cptr() ) {
      cache->larContainer->updateBCID( *avg, **onoff );
    }
  }

  const LArDeadOTXFromSC* deadHandle = nullptr;
  if ( m_correctDead ){
    SG::ReadHandle<LArDeadOTXFromSC> deadHdl(m_deadOTXFromSCKey, context);
    if ( !deadHdl.isValid() ){
      ATH_MSG_WARNING("Should not try to use LArDEADOTXFromSC" );
    } else {
      deadHandle = deadHdl.cptr();
    }
  }

  for(std::vector<uint32_t>& vrodid32fullDet : m_vrodid32fullDetHG) {
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> robFrags;
    m_robDataProvider->getROBData( context, vrodid32fullDet, robFrags );

    convertROBs( robFrags, cache->larContainer.get(), cache->larRodBlockStructure_per_slot,
                 cache->rodMinorVersion, cache->robBlockType, deadHandle );
      
    if ( vrodid32fullDet.size() != robFrags.size() ) {
      ATH_MSG_DEBUG( "Missing ROBs, requested " << vrodid32fullDet.size() << " obtained " << robFrags.size() );
      clearMissing( vrodid32fullDet, robFrags, cache->larContainer.get() );
    }
  } // end of for m_vrodid32fullDetHG.size()

  Monitored::Group( m_monTool, lockTime );
  return StatusCode::SUCCESS;
}

StatusCode TrigCaloDataAccessSvc::prepareTileFullCollections( const EventContext& context) {

  std::call_once(m_lateInitFlag, &TrigCaloDataAccessSvc::lateInit, this, context);

  ATH_MSG_DEBUG( "Full Col " << " requested for event " << context );

  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );

  auto lockTime = Monitored::Timer ( "TIME_locking_LAr_FullDet" );
  std::scoped_lock lock{cache->mutex};
  lockTime.stop();

  // If the full event was already unpacked, don't need to unpack RoI
  if ( cache->lastFSEvent == context.evt() ) return StatusCode::SUCCESS;
  if ( cache->tileContainer->eventNumber() != context.evt() ) {
    cache->d0cells->clear();
  }
  cache->tileContainer->eventNumber( context.evt() );

  convertROBs( context, m_rIdstile, cache->tileContainer.get(), cache->d0cells.get() );

  Monitored::Group( m_monTool, lockTime );
  return StatusCode::SUCCESS;
}

void TrigCaloDataAccessSvc::lateInit(const EventContext& context) { // non-const this thing

  ATH_MSG_DEBUG( "Performing late init" );

  // preparing full container list of ROBs - tile will be included soon
  std::vector<uint32_t> vrodid32lar;
  std::vector<uint32_t> vrodid32em;
  std::vector<uint32_t> vrodid32hec;
  std::vector<uint32_t> vrodid32hec0;
  std::vector<uint32_t> vrodid32hec1;
  std::vector<uint32_t> vrodid32hec2;
  std::vector<uint32_t> vrodid32hec3;
  std::vector<uint32_t> vrodid32fcalem;
  std::vector<uint32_t> vrodid32fcalhad;

  TrigRoiDescriptor tmproi(true);
  std::vector<uint32_t> vrodid32tile;
  std::vector<IdentifierHash> rIdstile;
  // TTEM
  m_regionSelector_TTEM->lookup(context)->ROBIDList(-1,tmproi,vrodid32em);
  // TTHEC
  m_regionSelector_TTHEC->lookup(context)->ROBIDList(0,tmproi,vrodid32hec0);
  m_regionSelector_TTHEC->lookup(context)->ROBIDList(1,tmproi,vrodid32hec1);
  m_regionSelector_TTHEC->lookup(context)->ROBIDList(2,tmproi,vrodid32hec2);
  m_regionSelector_TTHEC->lookup(context)->ROBIDList(3,tmproi,vrodid32hec3);
  // FCALHAD
  m_regionSelector_FCALEM->lookup(context)->ROBIDList(-1,tmproi,vrodid32fcalem);
  m_regionSelector_FCALHAD->lookup(context)->ROBIDList(-1,tmproi,vrodid32fcalhad);
  m_regionSelector_TILE->lookup(context)->ROBIDList(tmproi,vrodid32tile);
  m_regionSelector_TILE->lookup(context)->HashIDList(tmproi,rIdstile);

  m_vrodid32tile.resize( vrodid32tile.size() );
  m_rIdstile.resize(rIdstile.size() );
  // Tile RODs and ID coming from the Tile tables are not unique
  // iii and iij are local variables helping to clear non-unique IDs
  auto iii = std::unique_copy(vrodid32tile.begin(),vrodid32tile.end(),m_vrodid32tile.begin());
  auto iij = std::unique_copy(rIdstile.begin(),rIdstile.end(),m_rIdstile.begin());
  std::sort( m_vrodid32tile.begin(), iii );
  std::sort( m_rIdstile.begin(), iij );
  iii = std::unique_copy(m_vrodid32tile.begin(),iii,m_vrodid32tile.begin());
  iij = std::unique_copy(m_rIdstile.begin(),iij,m_rIdstile.begin());
  m_vrodid32tile.resize( std::distance(m_vrodid32tile.begin(), iii) );
  m_rIdstile.resize( std::distance(m_rIdstile.begin(), iij) );

  vrodid32lar.insert(vrodid32lar.end(),vrodid32em.begin(),vrodid32em.end());
  vrodid32hec.insert(vrodid32hec.end(),vrodid32hec0.begin(),vrodid32hec0.end());
  vrodid32lar.insert(vrodid32lar.end(),vrodid32hec.begin(),vrodid32hec.end());
  vrodid32lar.insert(vrodid32lar.end(),vrodid32fcalhad.begin(),vrodid32fcalhad.end());
  vrodid32lar.insert(vrodid32lar.end(),vrodid32fcalem.begin(),vrodid32fcalem.end());
  m_vrodid32fullDet.insert(m_vrodid32fullDet.end(), vrodid32lar.begin(), vrodid32lar.end() );
  

  SG::ReadCondHandle<LArMCSym> mcsym (m_mcsymKey, context);
  SG::ReadCondHandle<LArFebRodMapping> febrod(m_febRodMappingKey, context);
  SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, context };
  SG::ReadCondHandle<LArOnOffIdMapping> onoff ( m_onOffIdMappingKey, context);
  SG::ReadCondHandle<LArRoIMap> roimap ( m_larRoIMapKey, context);
  SG::ReadCondHandle<TileHid2RESrcID> tileHid2RESrcID ( m_tileHid2RESrcIDKey, context);

  const unsigned int nFebs=70;
  const unsigned int high_granu = (unsigned int)ceilf(m_vrodid32fullDet.size()/((float)nFebs) );
  unsigned int jj=0;
  unsigned int kk=0;
  m_vrodid32fullDetHG.resize(high_granu);
  for( unsigned int ii=0; ii<m_vrodid32fullDet.size();ii++){
	if ( kk >= nFebs ) {
      kk-=nFebs;
      jj++;
	}
	m_vrodid32fullDetHG[jj].push_back(m_vrodid32fullDet[ii]);
	kk++;
  }
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, context};
  const CaloDetDescrManager* theCaloDDM = *caloMgrHandle;
  const CaloCell_ID* theCaloCCIDM = theCaloDDM->getCaloCell_ID();
  const unsigned int hashMax = theCaloCCIDM->calo_cell_hash_max();

  // Prepare cache containers to be used for LAr unpacking.
  size_t slot = 0;
  for (HLTCaloEventCache& cache : m_hLTCaloSlot) {
    cache.larContainer = std::make_unique<LArCellCont>();
    cache.larRodBlockStructure_per_slot = nullptr;

    cache.larContainer->initialize(**roimap, **onoff, **mcsym, **febrod, **larBadChan, *theCaloDDM).
      orThrow("Cannot initialize LAr container", name());

    std::vector<CaloCell*> local_cell_copy;
    local_cell_copy.reserve(200000);
    const LArCellCont* larcell = cache.larContainer.get();
    const LArBadChannelCont& badchannel = **larBadChan;

    cache.lastFSEvent = 0xFFFFFFFF;
    auto cachefullcont = std::make_unique<CaloCellContainer>(SG::VIEW_ELEMENTS);
    cachefullcont->reserve(190000);

    for(unsigned int lcidx=0; lcidx < larcell->size(); lcidx++){
      LArCellCollection* lcc = larcell->at(lcidx);
      for(unsigned int lccidx=0; lccidx<lcc->size(); lccidx++){
        CaloCell* cell = lcc->at(lccidx);
        if ( cell && cell->caloDDE() ) {
          const LArBadChannel& bc = badchannel.offlineStatus(cell->ID());
          const bool bad = !bc.good() &&
            // only consider specific problems
            (bc.deadReadout() || bc.deadPhys() || bc.highNoiseHG() ||
             bc.highNoiseMG() || bc.highNoiseLG() || bc.problematicForUnknownReason());

          if ( !bad ) local_cell_copy.push_back( cell );
        }
      } // end of loop over cells
    } // end of loop over collection

    // This should stay here as this will be enabled when tile is ready to be decoded as well

    auto tilecell = std::make_unique<TileCellCont>();
    tilecell->setHashIdToROD( *tileHid2RESrcID );
    tilecell->initialize().orThrow("Cannot initialize Tile container", name());

    for (unsigned int i=0; i<4; i++) {
      m_tileDecoder->loadRw2Cell ( i, tilecell->Rw2CellMap(i) );
      m_tileDecoder->loadRw2Pmt  ( i, tilecell->Rw2PmtMap (i) );
    }
    m_tileDecoder->loadMBTS( tilecell->MBTS_map(), tilecell->MBTS_channel() );

    auto d0cellsp = std::make_unique<TileROD_Decoder::D0CellsHLT>();
    for (unsigned int lcidx=0; lcidx < tilecell->size(); lcidx++){
      TileCellCollection* lcc = tilecell->at(lcidx);
      for (unsigned int lccidx=0; lccidx<lcc->size(); lccidx++){
        CaloCell* cell = lcc->at(lccidx);
        if ( cell ) local_cell_copy.push_back( cell );
      } // end of loop over cells

	  TileRawChannelCollection::ID frag_id = ((*lcc).identify() & 0x0FFF);
	  const int ros = (frag_id >> 8);
	  if ( ros == 1 ) { //treatment for d0Cells in barrel
	    const int drawer = (frag_id & 0xFF);
	    TileCellCollection::iterator pCell = lcc->begin();
	    pCell+=2;
	    d0cellsp->m_cells[drawer] = *pCell;
	  }
    } // end of loop over collection
    cache.tileContainer = std::move(tilecell);

    // d0merge cells
    cache.d0cells = std::move(d0cellsp);

    // For the moment the container has to be completed by hand (again, because of tile)
    for(unsigned int i=0; i<hashMax; i++){
      cachefullcont->push_back_fast(nullptr);
    }

    for(unsigned int i=0; i<local_cell_copy.size(); i++){
      unsigned int j = local_cell_copy.at(i)->caloDDE()->calo_hash();
      if ( j < hashMax ) {
        cachefullcont->at(j) = local_cell_copy.at(i);
      }
    }
    for(unsigned int i=0;i<hashMax;i++)
      if ( cachefullcont->at(i) == nullptr ){
        Identifier id = theCaloCCIDM->cell_id(i);
        if ( id!=0 ){
          const CaloDetDescrElement* el = theCaloDDM->get_element(id);
		  if ( el->is_tile() ) {
            cachefullcont->at(i) = new TileCell(el,0,0,0,0,CaloGain::TILEHIGHHIGH);
		  } else {
            cachefullcont->at(i) = new LArCell(el,0,0,0,CaloGain::LARHIGHGAIN);
		  }

		  if (slot==0) m_insertedCells.push_back(i);
        }
      }

    cachefullcont->setHasCalo(CaloCell_ID::LAREM);
    cachefullcont->setHasCalo(CaloCell_ID::LARHEC);
    cachefullcont->setHasCalo(CaloCell_ID::LARFCAL);
    cachefullcont->setHasCalo(CaloCell_ID::TILE); //lying... But this needs to be checked later

    // make sure this "map" container has a good hashID
    cachefullcont->order();

    if ( hashMax != cachefullcont->size() ) {
      ATH_MSG_ERROR("Problem in the size of the full container");
    }

    cachefullcont->setIsOrdered(true);
    cachefullcont->setIsOrderedAndComplete(true);
    cache.fullcont = std::move(cachefullcont);

    local_cell_copy.clear();
    ++slot;
  }
}

void TrigCaloDataAccessSvc::convertROBs( const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& robFrags,
                                         LArCellCont* larcell, LArRodBlockStructure*& larRodBlockStructure_per_slot,
                                         uint16_t rodMinorVersion, uint32_t robBlockType, const LArDeadOTXFromSC* deadHandle ) {

  for ( auto rob: robFrags ) {
    uint32_t sourceID = rob->source_id();
    const auto it = larcell->find( sourceID );
    //TB The find also switches the state of the collection to "decoded" so repeated decoding is avoided

    if ( it != larcell->end() && ( *it )->size() != 0 ) { // Already decoded collection

      // TB if we have entered here it means the decoding did not occur yet ( or failed )
      // 
      LArCellCollection* coll = *it; 
      ATH_MSG_DEBUG( "ROB of ID " << sourceID << " to be decoded"   );

      std::scoped_lock decoderLock{m_lardecoderProtect};
      //TB next two lines seem danger, as they seem to rely on the decoder state
      const HWIdentifier& feb_id_local = larcell->findsec( sourceID );
      m_larDecoder->setsecfeb( feb_id_local );
      if ( ! m_larDecoder->check_valid( rob, msg() ) ){
      	ATH_MSG_WARNING( "Error reading bytestream"<<
                         "event: Bad ROB block ( eformat checks ) : 0x"
                         << std::hex << sourceID << std::dec );
      	// Data seems corrupted
      	reset_LArCol ( coll );

      } else {
        // Get Rod Data and size of fragment
        const uint32_t* roddata = 0;
        rob->rod_data( roddata );
        const size_t roddatasize = rob->rod_ndata();
        if ( roddatasize < 3 ) {
          ATH_MSG_WARNING( "Error reading bytestream"<<
                           "event: Empty ROD block ( less than 3 words ) : 0x"
                           << std::hex << sourceID << std::dec );
          // Data seems corrupted
          reset_LArCol ( coll );
        } else {
          //TB the converter has state
          m_larDecoder->fillCollectionHLT( *rob, roddata, roddatasize, *coll, larRodBlockStructure_per_slot, rodMinorVersion, robBlockType );
          if ( deadHandle ){
            if ( deadHandle->isThisOTXdead(feb_id_local) ) {
              const std::vector<float>& corr = deadHandle->correctionFromThisOTXdead(feb_id_local);
              // The sizes here are guaranteed by construction
              for(size_t i=0;i<128;i++) {
                (*coll)[i+128]->set(corr[i],0,0,0x1000,CaloGain::LARHIGHGAIN);
              }
            }
            HWIdentifier feb_id_local2(larRodBlockStructure_per_slot->getFEBID());
            if ( deadHandle->isThisOTXdead(feb_id_local2) ) {
              const std::vector<float>& corr = deadHandle->correctionFromThisOTXdead(feb_id_local2);
              // The sizes here are guaranteed by construction
              for(size_t i=0;i<128;i++) {
                (*coll)[i]->set(corr[i],0,0,0x1000,CaloGain::LARHIGHGAIN);
              }
            }
          }

          if ( m_applyOffsetCorrection ) larcell->applyBCIDCorrection( sourceID );
        }
	
      }
    } else {
      ATH_MSG_VERBOSE( "ROB of ID " <<  sourceID << " already decoded" );
    }
  }
  ATH_MSG_DEBUG( "finished decoding" );
}

void TrigCaloDataAccessSvc::convertROBs( const EventContext& context,
                                         const std::vector<IdentifierHash>& rIds,
                                         TileCellCont* tilecell,
                                         TileROD_Decoder::D0CellsHLT* d0cells) {

  TileCellCollection* mbts = tilecell->MBTS_collection();
  const TileHid2RESrcID* hid2re = tilecell->getHashIdToROD();

  std::vector<unsigned int> tile{0};
  // Tile likes rob by rob
  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> robFrags;
  for (size_t i = 0; i < rIds.size(); ++i){
    tile[0] = tilecell->find_rod(rIds[i]);
    m_robDataProvider->getROBData(context,tile,robFrags);
    // Find the collection to fill
    const auto it = tilecell->find(rIds[i]);
    if (it == tilecell->end()) continue;

    TileCellCollection* col = *it;
    if ( !robFrags.empty() && col != nullptr ) {
      const size_t roddatasize = robFrags[0]->rod_ndata();
      // insert data into vector (to be removed soon)
      if (roddatasize < 3) {
        ATH_MSG_WARNING( "Error reading bytestream"<<
                         "event: Empty ROD block (less than 3 words) : 0x"
                         << std::hex << tile[0] << std::dec );
        if ( !tilecell->cached(rIds[i])){
          // resets collection
          reset_TileCol(col);
        }
        robFrags.clear();
      } else  {
        std::scoped_lock decoderLock{m_tiledecoderProtect};
        if ( !tilecell->cached(rIds[i]) ) {
          m_tileDecoder->fillCollectionHLT(robFrags[0],*col,*d0cells,mbts,hid2re);
        }
        m_tileDecoder->mergeD0cellsHLT(*d0cells,*col);
        robFrags.clear();
      }
    } // end of if robFrags.size
  } // End of for through RobFrags

  ATH_MSG_DEBUG( "finished decoding" );
}

void TrigCaloDataAccessSvc::missingROBs( const std::vector<uint32_t>& request,
                                         const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& response,
                                         std::set<uint32_t>& missing ) const {

  std::set<uint32_t> receivedROBsSet;
  for ( auto rob: response ) {
    receivedROBsSet.insert( rob->source_id() );
  }

  std::set<uint32_t> requestedROBsSet( request.begin(), request.end() );
  std::set_difference( requestedROBsSet.begin(), requestedROBsSet.end(),
                       receivedROBsSet.begin(), receivedROBsSet.end(),
                       std::inserter( missing, missing.begin() ) );
}


void TrigCaloDataAccessSvc::clearMissing( const std::vector<uint32_t>& request,
                                          const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& response,
                                          LArCellCont* larcell ) {
  std::set<uint32_t> missing;
  missingROBs( request, response, missing );
  for ( uint32_t robID : missing ) {
    const auto it = larcell->find( robID );
    if ( it != larcell->end() && not (*it)->empty() ) { // Already decoded collection
      reset_LArCol ( *it );
    } 
  }  
}


StatusCode TrigCaloDataAccessSvc::prepareLArCollections( const EventContext& context,
                                                         const IRoiDescriptor& roi,
                                                         const int sampling,
                                                         DETID detector ) {

  std::call_once(m_lateInitFlag, &TrigCaloDataAccessSvc::lateInit, this, context);

  // If the full event was already unpacked, don't need to unpack RoI
  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );
  if ( cache->lastFSEvent == context.evt() ) return StatusCode::SUCCESS;

  std::vector<uint32_t> requestROBs;

  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> robFrags;
  switch ( detector ) {
  case TTEM: {m_regionSelector_TTEM->lookup(context)->ROBIDList( sampling, roi, requestROBs ); break; }
  case TTHEC: {m_regionSelector_TTHEC->lookup(context)->ROBIDList( sampling, roi, requestROBs ); break; }
  case FCALEM: {m_regionSelector_FCALEM->lookup(context)->ROBIDList( sampling, roi, requestROBs ); break; }
  case FCALHAD: {m_regionSelector_FCALHAD->lookup(context)->ROBIDList( sampling, roi, requestROBs ); break; }
  default: break;
  }

  m_robDataProvider->getROBData( context, requestROBs, robFrags );
  if ( robFrags.empty() && !requestROBs.empty() ) {
    return StatusCode::SUCCESS;
  }

  auto lockTime = Monitored::Timer ( "TIME_locking_LAr_RoI" );
  std::scoped_lock lock{cache->mutex};
  lockTime.stop();

  // TB, what would happen from now inside the collection unapcking
  // if this event number is different than the one for each collection the unpacking will happen, 
  // if it is the same the unpacking will not be repeated
  // same in prepareLArFullCollections
  cache->larContainer->eventNumber( context.evt() );
  if ( m_applyOffsetCorrection && cache->larContainer->lumiBCIDCheck( context ) ) {
	SG::ReadHandle<CaloBCIDAverage> avg (m_bcidAvgKey, context);
    SG::ReadCondHandle<LArOnOffIdMapping> onoff ( m_onOffIdMappingKey, context);
	if ( avg.cptr() && onoff.cptr() ) cache->larContainer->updateBCID( *avg, **onoff );
  }

  const LArDeadOTXFromSC* deadHandle = nullptr;
  if ( m_correctDead ){
    SG::ReadHandle<LArDeadOTXFromSC> deadHdl(m_deadOTXFromSCKey, context);
    if ( !deadHdl.isValid() ){
      ATH_MSG_WARNING("Should not try to use LArDEADOTXFromSC" );
    } else {
      deadHandle = deadHdl.cptr();
    }
  }
  
  convertROBs( robFrags, cache->larContainer.get(), cache->larRodBlockStructure_per_slot,
               cache->rodMinorVersion, cache->robBlockType, deadHandle );

  if ( requestROBs.size() != robFrags.size() ) {
    ATH_MSG_DEBUG( "Missing ROBs, requested " << requestROBs.size() << " obtained " << robFrags.size() );
    clearMissing( requestROBs, robFrags, cache->larContainer.get() );
  }
  auto roiROBs = Monitored::Scalar( "roiROBs_LAr", robFrags.size() );
  auto roiEta = Monitored::Scalar( "roiEta_LAr", roi.eta() );
  auto roiPhi = Monitored::Scalar( "roiPhi_LAr", roi.phi() );

  Monitored::Group( m_monTool, lockTime, roiEta, roiPhi, roiROBs );
  return StatusCode::SUCCESS;
}

StatusCode TrigCaloDataAccessSvc::prepareTileCollections( const EventContext& context,
                                                          const IRoiDescriptor& roi) {

  std::call_once(m_lateInitFlag, &TrigCaloDataAccessSvc::lateInit, this, context);

  // If the full event was already unpacked, don't need to unpack RoI
  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );
  if ( cache->lastFSEvent == context.evt() ) return StatusCode::SUCCESS;

  std::vector<uint32_t> requestROBs;
  std::vector<IdentifierHash> rIds;
  m_regionSelector_TILE->lookup(context)->ROBIDList( 0, roi, requestROBs ); 
  m_regionSelector_TILE->lookup(context)->HashIDList(roi, rIds);

  std::scoped_lock lock{cache->mutex};
  if ( cache->tileContainer->eventNumber() != context.evt() ) {
    cache->d0cells->clear();
  }
  cache->tileContainer->eventNumber( context.evt() );
 
  convertROBs( context, rIds, cache->tileContainer.get(), cache->d0cells.get() );

  return StatusCode::SUCCESS;
}

StatusCode TrigCaloDataAccessSvc::prepareMBTSCollections( const EventContext& context) {

  std::call_once(m_lateInitFlag, &TrigCaloDataAccessSvc::lateInit, this, context);

  // If the full event was already unpacked, don't need to unpack RoI
  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );
  if ( cache->lastFSEvent == context.evt() ) return StatusCode::SUCCESS;

  std::scoped_lock lock{cache->mutex};
  if ( cache->tileContainer->eventNumber() != context.evt() ) {
    cache->d0cells->clear();
  }
  cache->tileContainer->eventNumber( context.evt() );
 
  const std::vector<unsigned int>* ids = cache->tileContainer->MBTS_IDs();
  std::vector<IdentifierHash> tileIds;
  tileIds.reserve(ids->size());
  for (size_t i=0; i<ids->size(); i++) {
    tileIds.push_back( (*ids)[i] );
  }
  convertROBs( context, tileIds, cache->tileContainer.get(), cache->d0cells.get() );

  return StatusCode::SUCCESS;
}



StatusCode TrigCaloDataAccessSvc::loadMBTS ( const EventContext& context,
                                             std::vector<const TileCell*>& loadedCells ) {

  ATH_MSG_DEBUG( "MBTS requested for event " << context );
  ATH_CHECK( prepareMBTSCollections(context) );

  HLTCaloEventCache* cache = m_hLTCaloSlot.get( context );
  {
    std::scoped_lock lock{cache->mutex};
    TileCellCollection* mbts = cache->tileContainer->MBTS_collection();
    loadedCells.reserve(mbts->size());
    for (size_t i=0; i<mbts->size(); ++i) {
      loadedCells.push_back(mbts->at(i));
    }
  }
  return StatusCode::SUCCESS;

}
