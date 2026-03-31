/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloLArRetriever.h"

#include "AthenaKernel/Units.h"

#include "EventContainers/SelectAllObject.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArRawChannel.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "Identifier/HWIdentifier.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * Initialise the Tool
   */

  StatusCode CaloLArRetriever::initialize() {

    ATH_MSG_DEBUG( "Initialising Tool"  );
    ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );

    ATH_CHECK( m_sgKey.initialize() );
    ATH_CHECK( m_cablingKey.initialize() );
    ATH_CHECK( m_adc2mevKey.initialize(m_doLArCellDetails) );

    return StatusCode::SUCCESS;	
  }
   
  /**
   * LAr data retrieval from default collection
   */
  StatusCode CaloLArRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieve()"  );

    SG::ReadHandle<CaloCellContainer> cellContainer(m_sgKey);
    if (!cellContainer.isValid()){
	    ATH_MSG_WARNING( "Could not retrieve Calorimeter Cells "  );
    }
    else{
      if(m_lar){
        DataMap data = getLArData(&(*cellContainer));
        ATH_CHECK( FormatTool->AddToEvent(dataTypeName(), m_sgKey.key(), &data) );
        ATH_MSG_DEBUG( "LAr retrieved"  );
      }
    }

    //LAr cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve LAr cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap CaloLArRetriever::getLArData(const CaloCellContainer* cellContainer) {
    
    ATH_MSG_DEBUG( "getLArData()"  );
    const EventContext& ctx = Gaudi::Hive::currentContext();

    DataMap dataMap;

    DataVect phi; phi.reserve(cellContainer->size());
    DataVect eta; eta.reserve(cellContainer->size());
    DataVect energy; energy.reserve(cellContainer->size());
    DataVect idVec; idVec.reserve(cellContainer->size());
    DataVect channel; channel.reserve(cellContainer->size());
    DataVect feedThrough; feedThrough.reserve(cellContainer->size());
    DataVect slot; slot.reserve(cellContainer->size());

    DataVect cellTimeVec; cellTimeVec.reserve(cellContainer->size());
    DataVect cellGain; cellGain.reserve(cellContainer->size());
    DataVect cellPedestal; cellPedestal.reserve(cellContainer->size());
    DataVect adc2Mev; adc2Mev.reserve(cellContainer->size());
    DataVect BadCell; BadCell.reserve(cellContainer->size());

    char rndStr[30]; // for rounding (3 digit precision)

    CaloCellContainer::const_iterator it1 = cellContainer->beginConstCalo(CaloCell_ID::LAREM);
    CaloCellContainer::const_iterator it2 = cellContainer->endConstCalo(CaloCell_ID::LAREM);

    
    const ILArPedestal* larPedestal = nullptr;
    if(m_doLArCellDetails){
      if( detStore()->retrieve(larPedestal).isFailure() ){
        ATH_MSG_ERROR( "in getLArData(), Could not retrieve LAr Pedestal"  );
      }
    }
      
    const LArOnlineID* onlineId = nullptr;
    if ( detStore()->retrieve(onlineId, "LArOnlineID").isFailure()) {
      ATH_MSG_ERROR( "in getLArData(),Could not get LArOnlineID!"  );
    }

    const LArADC2MeV* adc2mev = nullptr;
    if (m_doLArCellDetails) {
      SG::ReadCondHandle<LArADC2MeV> adc2mevH (m_adc2mevKey, ctx);
      adc2mev = *adc2mevH;
    }

      double energyGeV,cellTime;
      double energyAllLArBarrel = 0.;

      ATH_MSG_DEBUG( "Start iterator loop over cells"  );
      
      SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
      const LArOnOffIdMapping* cabling{*cablingHdl};
      if(!cabling) {
         ATH_MSG_ERROR( "Do not have cabling mapping from key " << m_cablingKey.key() );
         return dataMap;
      }
     

      for(;it1!=it2;++it1){
	if ((*it1)->energy() < m_cellThreshold) continue; // skip to next cell if threshold not passed

	if((*it1)->badcell()){ BadCell.push_back(1); }
        else{ BadCell.push_back(-1); }

	  if ((((*it1)->provenance()&0xFF)!=0xA5)&&m_cellConditionCut) continue; // check full conditions for LAr
	  Identifier cellid = (*it1)->ID(); 

          HWIdentifier LArhwid = cabling->createSignalChannelIDFromHash((*it1)->caloDDE()->calo_hash());
	  
	  //ignore LAr cells that are to be masked
	  if (m_doMaskLArChannelsM5){
	    bool maskChannel = false;
	    for (size_t i = 0; i < m_LArChannelsToIgnoreM5.size(); i++){
              if (cellid == m_LArChannelsToIgnoreM5[i]){
		maskChannel = true; 
		break;  // exit loop over bad channels
	      }	      
	    }
	    if (maskChannel) continue;  // continue loop over all channels
	  }
	  energyGeV = (*it1)->energy()*(1./GeV);
	  if (energyGeV == 0) energyGeV = 0.001; // 1 MeV due to LegoCut > 0.0 (couldn't be >= 0.0) 
	  energy.emplace_back( gcvt( energyGeV, m_cellEnergyPrec, rndStr) );
          energyAllLArBarrel += energyGeV;

          idVec.emplace_back((Identifier::value_type)(*it1)->ID().get_compact() );
          phi.emplace_back((*it1)->phi());
          eta.emplace_back((*it1)->eta());
          channel.emplace_back(onlineId->channel(LArhwid)); 
          feedThrough.emplace_back(onlineId->feedthrough(LArhwid)); 
       	  slot.emplace_back(onlineId->slot(LArhwid)); 

          if ( m_doLArCellDetails){
            cellTime = (*it1)->time();
            cellTimeVec.emplace_back( gcvt( cellTime, m_cellTimePrec, rndStr)  );
            cellGain.emplace_back( (*it1)->gain()  ); 
              
            int largain = (*it1)->gain();
            float pedestal=larPedestal->pedestal(LArhwid,largain);
            float pedvalue=0;
            if (pedestal >= (1.0+LArElecCalib::ERRORCODE)) pedvalue = pedestal;
            else pedvalue = 0;
            cellPedestal.emplace_back(pedvalue);
            LArVectorProxy polynom_adc2mev = adc2mev->ADC2MEV(cellid,largain);
            if (polynom_adc2mev.size()==0){ adc2Mev.emplace_back(-1); }
            else{ adc2Mev.emplace_back(polynom_adc2mev[1]); }
          }
      }

    ATH_MSG_DEBUG( " Total energy in LAr barrel in GeV : " <<  energyAllLArBarrel  );

    // write values into dataMap
    const auto nEntries = phi.size() ;
    dataMap["phi"] = std::move(phi);
    dataMap["eta"] = std::move(eta);
    dataMap["energy"] = std::move(energy);
    dataMap["id"] = std::move(idVec);
    dataMap["channel"] = std::move(channel);
    dataMap["feedThrough"] = std::move(feedThrough);
    dataMap["slot"] = std::move(slot);

    //Bad Cells
    if (m_doBadLAr==true) {
      dataMap["BadCell"] = std::move(BadCell);
    }

   // adc counts
    if ( m_doLArCellDetails){
       dataMap["cellTime"] = std::move(cellTimeVec);
       dataMap["cellGain"] = std::move(cellGain);
       dataMap["cellPedestal"] = std::move(cellPedestal);
       dataMap["adc2Mev"] = std::move(adc2Mev);
    }
    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " , collection: " << dataTypeName()
                   << " retrieved with " << nEntries << " entries" );

    //All collections retrieved okay
    return dataMap;

  } // getLArData

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
