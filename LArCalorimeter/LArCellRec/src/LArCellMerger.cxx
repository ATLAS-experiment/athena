/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     LArCellMerger
PACKAGE:  offline/Calorimeter/LArCellRec

AUTHORS: G.Unal
CREATION:  9 July 2009

PURPOSE:

********************************************************************/

#include "LArCellMerger.h"

#include "LArRawEvent/LArRawChannelContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArIdentifier/LArOnlineID.h"


/////////////////////////////////////////////////////////////////////
// INITIALIZE:
// The initialize method will create all the required algorithm objects
/////////////////////////////////////////////////////////////////////

StatusCode LArCellMerger::initialize() {

  ATH_CHECK(detStore()->retrieve(m_calo_id,"CaloCell_ID"));
  ATH_CHECK( detStore()->retrieve(m_onlineID, "LArOnlineID") );
  ATH_CHECK( m_cablingKey.initialize() );

  ATH_CHECK(m_rawChannelContainerName.initialize());

  return StatusCode::SUCCESS;
}

StatusCode LArCellMerger::finalize(){
   return StatusCode::SUCCESS;
}

StatusCode LArCellMerger::process (CaloCellContainer* theCont,
                                   const EventContext& ctx) const
{
  ATH_MSG_DEBUG("in  LArCellMerger::process");
  
  SG::ReadHandle<LArRawChannelContainer> rawColl(m_rawChannelContainerName, ctx);
  if (!rawColl.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve LArRawChannelContainer with key " << 
		  rawColl.name());
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> larCablingHdl(m_cablingKey, ctx);
  const LArOnOffIdMapping* cabling=*larCablingHdl;

  unsigned nReplaced=0;

  // loop over raw channel container
  //   as this new container is supposed to contain only few cells, we do a simple loop and the basics onlineId to offlineId conversion
  //   this could be a little slow if by mistake this container contains all cells (not what this tool is supposed to be used for)
  for ( const LArRawChannel& theRawChannel : *rawColl ) {
      const HWIdentifier hwid=theRawChannel.channelID();
      const IdentifierHash onlHash=m_onlineID->channel_Hash(hwid);
      if( cabling->isOnlineConnectedFromHash(onlHash)) {
        const IdentifierHash theCellHashID= cabling->oflHashFromOnlHash(onlHash);
        const Identifier id=m_calo_id->cell_id(theCellHashID); 
          int index = theCont->findIndex(theCellHashID);
          if (index<0) {
	    ATH_MSG_WARNING( " cell " << hwid.get_compact() << " " << id.get_compact() << " is not in the container "  );
	    continue;
          }
          CaloCell* aCell = theCont->at(index);

          if (aCell) {
	    ATH_MSG_DEBUG(" replace energies in cell hwid= " << hwid.get_identifier32().get_compact() 
			  << " offlineid = " << id.get_identifier32().get_compact()
			  << " old energy " << aCell->e() << " new energy " << theRawChannel.energy());
	    nReplaced++;
	    aCell->setEnergy((float)(theRawChannel.energy()));
	    aCell->setTime((float)(theRawChannel.time())*0.001);    // convert from ps int in raw channel to ns float in calocell
	    aCell->setQuality(theRawChannel.quality());
	    aCell->setProvenance(theRawChannel.provenance());
          }

      }// isConnected
  }// loop over raw channel container

  if (nReplaced*5>theCont->size()) {
    ATH_MSG_WARNING("Replaced more than 20% of channels reco'ed online by channels reco'ed offline");
  }
  return StatusCode::SUCCESS;
}
