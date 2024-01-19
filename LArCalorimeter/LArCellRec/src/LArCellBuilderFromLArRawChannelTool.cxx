/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/


// implementation of LArCellBuilderFromLArRawChannelTool 
//                             H.Ma              Mar 2001

// Updated Jun 10, 2001.    H. Ma
// made a tool June 2, 2004 D. Rousseau
// Major overhaul, Feb 2008, W.Lampl
// Migrate to athenaMT, March 2018, W.Lampl


#include "LArCellBuilderFromLArRawChannelTool.h"
#include "LArRecEvent/LArCell.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloDetDescr/CaloDetDescriptor.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <bitset>


StatusCode LArCellBuilderFromLArRawChannelTool::initialize() {

  ATH_CHECK(m_rawChannelsKey.initialize());

  ATH_CHECK( detStore()->retrieve (m_caloCID, "CaloCell_ID") );

  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( detStore()->retrieve(m_onlineID, "LArOnlineID") );

  if (!m_missingFebKey.key().empty()) {
    ATH_CHECK( m_missingFebKey.initialize() );
  }
  else {
    if (m_addDeadOTX) {
      ATH_MSG_ERROR( "Configuration problem: 'addDeadOTX' set, but no missing FEB container given."  );
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(m_caloMgrKey.initialize());

  //Compute total number of cells
  m_nTotalCells=0;

  IdentifierHash caloCellMin, caloCellMax ;
  m_caloCID->calo_cell_hash_range(CaloCell_ID::LAREM,caloCellMin,caloCellMax);
  m_nTotalCells+=caloCellMax-caloCellMin;
  m_caloCID->calo_cell_hash_range(CaloCell_ID::LARHEC,caloCellMin,caloCellMax);
  m_nTotalCells+=caloCellMax-caloCellMin;
  m_caloCID->calo_cell_hash_range(CaloCell_ID::LARFCAL,caloCellMin,caloCellMax);
  m_nTotalCells+=caloCellMax-caloCellMin;
  
  if (m_initialDataPoolSize<0)  m_initialDataPoolSize=m_nTotalCells;
  ATH_MSG_DEBUG("Initial size of DataPool<LArCell>: " << m_initialDataPoolSize);
  
  ATH_MSG_DEBUG("Initialisating finished");
  return StatusCode::SUCCESS;
}



// ========================================================================================== //
StatusCode
LArCellBuilderFromLArRawChannelTool::process (CaloCellContainer* theCellContainer,
                                              const EventContext& ctx) const
{
  if (theCellContainer->ownPolicy() == SG::OWN_ELEMENTS) {
    ATH_MSG_ERROR( "Called with a CaloCellContainer with wrong ownership policy! Need a VIEW container!"  );
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<LArRawChannelContainer> rawColl(m_rawChannelsKey, ctx);
  if(!rawColl.isValid()) { 
    ATH_MSG_ERROR( " Can not retrieve LArRawChannelContainer: "
                   << m_rawChannelsKey.key()  );
    return StatusCode::FAILURE;      
   }  

   const size_t nRawChannels=rawColl->size();
   if (nRawChannels==0) {
     ATH_MSG_WARNING( "Got empty LArRawChannel container. Do nothing"  );
     return StatusCode::SUCCESS;
   }
   else
     ATH_MSG_DEBUG("Got " << nRawChannels << " LArRawChannels");


  unsigned nCellsAdded=0;
  std::bitset<CaloCell_ID::NSUBCALO> includedSubcalos;
  // resize calo cell container to correct size
  if (!theCellContainer->empty()) {
    ATH_MSG_WARNING( "Container should be empty! Clear now.");
    theCellContainer->clear();
  }

  DataPool<LArCell> pool (ctx,m_initialDataPoolSize);

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey, ctx);
  const LArOnOffIdMapping* cabling=*cablingHdl;

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey,ctx};
  const CaloDetDescrManager* caloDDM = *caloMgrHandle;

  theCellContainer->resize(m_nTotalCells);
 
  for (const LArRawChannel& rawChan : *rawColl) {
    const HWIdentifier hwid=rawChan.channelID();
    const IdentifierHash onlHash=m_onlineID->channel_Hash(hwid);
    if( cabling->isOnlineConnectedFromHash(onlHash)) {
      const IdentifierHash oflHash= cabling->oflHashFromOnlHash(onlHash);
      const Identifier id=m_caloCID->cell_id(oflHash); 
      const CaloDetDescrElement * theDDE=caloDDM->get_element(oflHash);
        
      LArCell *pCell   = pool.nextElementPtr();

      *pCell = LArCell (theDDE,
			id,
			rawChan.energy(),
			rawChan.time()*Gaudi::Units::picosecond/Gaudi::Units::nanosecond,    // convert time from ps (LArRawChannel) to ns
			rawChan.quality(),
			rawChan.provenance(),
			rawChan.gain());

      if ((*theCellContainer)[oflHash]) {
        ATH_MSG_WARNING("Channel added twice! Data corruption? hash="
                        << oflHash << " online ID=0x" << std::hex
                        << hwid.get_identifier32().get_compact() << std::dec
                        << "  " << m_onlineID->channel_name(hwid));
      } else {
        (*theCellContainer)[oflHash] = pCell;
        ++nCellsAdded;
        includedSubcalos.set(m_caloCID->sub_calo(oflHash));
      }
    }//end if connected
  }//end loop over LArRawChannelContainer

  //Now add in dummy cells (if requested)
    unsigned nMissingButPresent=0;

  if (m_addDeadOTX) {
    SG::ReadCondHandle<LArBadFebCont> missingFebHdl(m_missingFebKey, ctx);
    const LArBadFebCont::BadChanVec& allMissingFebs=(*missingFebHdl)->fullCont();

    for (const LArBadFebCont::BadChanEntry& it : allMissingFebs) {
      const LArBadFeb& febstatus=it.second;
      if (febstatus.deadReadout()  ||  febstatus.deadAll() || febstatus.deactivatedInOKS() ) {
	const HWIdentifier febId(it.first);
	//Loop over channels belonging to this FEB
	const int chansPerFeb=m_onlineID->channelInSlotMax(febId);
	for (int ch=0; ch<chansPerFeb; ++ch) {
	  const HWIdentifier hwid = m_onlineID->channel_Id(febId, ch);
	  if ( cabling->isOnlineConnected(hwid)) {
	    const Identifier id=cabling->cnvToIdentifier(hwid);
	    const IdentifierHash hashid= m_caloCID->calo_cell_hash(id);
	    const CaloDetDescrElement * theDDE=caloDDM->get_element(hashid);
	    LArCell *pCell   = pool.nextElementPtr();
	    *pCell = LArCell (theDDE,
			      id,
			      0.0, //energy
			      0.0, //time
			      0.0, //quality;
			      0x0A00,   // 0x0800 (dead) + 0x0200  (to indicate missing readout)
			      CaloGain::LARHIGHGAIN);
	  
	
	    if ((*theCellContainer)[hashid]) {
	      ++nMissingButPresent;
	      ATH_MSG_DEBUG("The supposedly missing channel with online ID=0x" << std::hex 
			    << hwid.get_identifier32().get_compact() << std::dec 
			    << "  " << m_onlineID->channel_name(hwid) 
			    << " is actually present in the LArRawChannel container");
	    }
	    else {
	      (*theCellContainer)[hashid]=pCell;
	      ++nCellsAdded;
	      includedSubcalos.set(m_caloCID->sub_calo(hashid));
	    }
	  }//end if connected
	}//end loop over channels of one missing FEB
      }//end if is dead/missing FEB
    }//end loop over bad/missing FEBs
  }//end if  m_addDeadOTX

  //Set the 'hasCalo' variable of CaloCellContainer
  for (int iCalo=0;iCalo<CaloCell_ID::NSUBCALO;++iCalo) {
    if (includedSubcalos.test(iCalo))
      theCellContainer->setHasCalo(static_cast<CaloCell_ID::SUBCALO>(iCalo));
  }

  if (nMissingButPresent) 
    ATH_MSG_WARNING( "A total of " << nMissingButPresent 
                     << " supposedly missing channels where present in the LArRawChannelContainer"  );

  if (nCellsAdded!=m_nTotalCells) {
    ATH_MSG_DEBUG("Filled only  " << nCellsAdded << " out of " << m_nTotalCells << " cells. Removing holes");
    auto end1=std::remove(theCellContainer->begin(),theCellContainer->end(),nullptr);
    theCellContainer->erase(end1,theCellContainer->end());
    ATH_MSG_DEBUG("Shrunk the cell container to " << theCellContainer->size() << " (" << m_nTotalCells-nCellsAdded << " cells missing)");
  }//end if nCellsAdded!=m_nTotalCells
  else
    ATH_MSG_DEBUG("All " << nCellsAdded << " cells filled (no holes)");

  return StatusCode::SUCCESS;
}
