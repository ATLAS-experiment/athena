/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloFCalRetriever.h"

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
#include "CaloIdentifier/CaloCell_ID.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * Initialise the Tool
   */

  StatusCode CaloFCalRetriever::initialize() {

    ATH_MSG_DEBUG( "Initialising Tool"  );
    ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );

    ATH_CHECK( m_sgKey.initialize() );
    ATH_CHECK( m_cablingKey.initialize() );
    ATH_CHECK( m_adc2mevKey.initialize(m_doFCalCellDetails) );

    return StatusCode::SUCCESS;	
  }
   
  /**
   * FCal data retrieval from default collection
   */
  StatusCode CaloFCalRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieve()"  );

    SG::ReadHandle<CaloCellContainer> cellContainer(m_sgKey);
    if (!cellContainer.isValid()){
	    ATH_MSG_WARNING( "Could not retrieve Calorimeter Cells "  );
    }
    else{
      if(m_fcal){
        DataMap data = getFCalData(&(*cellContainer));
        ATH_CHECK( FormatTool->AddToEvent(dataTypeName(), m_sgKey.key(), &data) );
        ATH_MSG_DEBUG( "FCal retrieved"  );
      }
    }

    //FCal cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve FCal cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap CaloFCalRetriever::getFCalData(const CaloCellContainer* cellContainer) {
    
    ATH_MSG_DEBUG( "getFCalData()"  );
    const EventContext& ctx = Gaudi::Hive::currentContext();

    DataMap DataMap;

    DataVect x; x.reserve(cellContainer->size());
    DataVect y; y.reserve(cellContainer->size());
    DataVect z; z.reserve(cellContainer->size());
    DataVect dx; dx.reserve(cellContainer->size());
    DataVect dy; dy.reserve(cellContainer->size());
    DataVect dz; dz.reserve(cellContainer->size());

    DataVect energy; energy.reserve(cellContainer->size());
    DataVect idVec; idVec.reserve(cellContainer->size());
    DataVect channel; channel.reserve(cellContainer->size());
    DataVect feedThrough; feedThrough.reserve(cellContainer->size());
    DataVect slot; slot.reserve(cellContainer->size());
    DataVect sub; sub.reserve(cellContainer->size());

    DataVect cellTimeVec; cellTimeVec.reserve(cellContainer->size());
    DataVect cellGain; cellGain.reserve(cellContainer->size());
    DataVect cellPedestal; cellPedestal.reserve(cellContainer->size());
    DataVect adc2Mev; adc2Mev.reserve(cellContainer->size());
    DataVect BadCell; BadCell.reserve(cellContainer->size());

    char rndStr[30]; // for rounding (3 digit precision)

    CaloCellContainer::const_iterator it1 = cellContainer->beginConstCalo(CaloCell_ID::LARFCAL);
    CaloCellContainer::const_iterator it2 = cellContainer->endConstCalo(CaloCell_ID::LARFCAL);

    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
    const LArOnOffIdMapping* cabling{*cablingHdl};

    if(!cabling) {
       ATH_MSG_WARNING( "Do not have cabling mapping from key " << m_cablingKey.key() );
       return DataMap;
    }

    const ILArPedestal* larPedestal = nullptr;
    if(m_doFCalCellDetails){
	if( detStore()->retrieve(larPedestal).isFailure() ){
	  ATH_MSG_ERROR( "in getFCalData(), Could not retrieve LAr Pedestal"  );
	}
      }
      
    const LArOnlineID* onlineId;
    if ( detStore()->retrieve(onlineId, "LArOnlineID").isFailure()) {
      ATH_MSG_ERROR( "in getFCalData(),Could not get LArOnlineID!"  );
     }
    
    const LArADC2MeV* adc2mev = nullptr;
    if (m_doFCalCellDetails) {
      SG::ReadCondHandle<LArADC2MeV> adc2mevH (m_adc2mevKey, ctx);
      adc2mev = *adc2mevH;
    }

      double energyGeV, xmm, ymm, zmm, dxmm, dymm, dzmm, cellTime;
      double energyAllLArFcal = 0.;

      for(;it1!=it2;++it1){

      if((*it1)->badcell()) BadCell.push_back(1);
      else if((*it1)->energy()>= m_cellThreshold) BadCell.push_back(0);
      else BadCell.push_back(-1);

	  if ((((*it1)->provenance()&0xFF)!=0xA5)&&m_cellConditionCut) continue; // check full conditions for FCal
	  Identifier cellid = (*it1)->ID(); 

          HWIdentifier LArhwid = cabling->createSignalChannelIDFromHash((*it1)->caloDDE()->calo_hash());
	  
	  //ignore FCal cells that are to be masked
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
	  energy.emplace_back( gcvt( energyGeV, m_cellEnergyPrec, rndStr) );
          energyAllLArFcal += energyGeV;
          idVec.emplace_back((Identifier::value_type)(*it1)->ID().get_compact() );
        
	  xmm = (*it1)->x()*0.1;
	  ymm = (*it1)->y()*0.1;
	  zmm = (*it1)->z()*0.1;
	  x.emplace_back( gcvt( xmm, 4, rndStr)  );
	  y.emplace_back( gcvt( ymm, 4, rndStr)  );
	  z.emplace_back( gcvt( zmm, 4, rndStr)  );
	  
          channel.emplace_back(onlineId->channel(LArhwid)); 
          feedThrough.emplace_back(onlineId->feedthrough(LArhwid)); 
       	  slot.emplace_back(onlineId->slot(LArhwid)); 

	  if ( m_doFCalCellDetails){
	    cellTime = (*it1)->time();
	    cellTimeVec.emplace_back( gcvt( cellTime, m_cellTimePrec, rndStr)  );
	    cellGain.emplace_back( (*it1)->gain()  ); 
      	
	    int fcalgain = (*it1)->gain();
	    float pedestal=larPedestal->pedestal(LArhwid,fcalgain);
	    float pedvalue=0;
	    if (pedestal >= (1.0+LArElecCalib::ERRORCODE)) pedvalue = pedestal;
	    else pedvalue = 0;
	    cellPedestal.emplace_back(pedvalue);
	         
            LArVectorProxy polynom_adc2mev = adc2mev->ADC2MEV(cellid,fcalgain);
            if (polynom_adc2mev.size()==0){ adc2Mev.emplace_back(-1); }
            else{ adc2Mev.emplace_back(polynom_adc2mev[1]); }
	  }

	  const CaloDetDescrElement* elt = (*it1)->caloDDE();

	  dxmm = elt->dx()*0.1;
	  dymm = elt->dy()*0.1;
	  dzmm = elt->dz()*0.1;
	  dx.emplace_back( gcvt( dxmm, 4, rndStr)  );
	  dy.emplace_back( gcvt( dymm, 4, rndStr)  );
	  dz.emplace_back( gcvt( dzmm, 4, rndStr)  );
	    
	  if(m_calocell_id->pos_neg(cellid)==2)
	    sub.emplace_back(1);
	  else
	    sub.emplace_back(0);
      }

    ATH_MSG_DEBUG( " Total energy in FCAL (LAr) in GeV : " <<  energyAllLArFcal  );

    // write values into DataMap
    const auto nEntries = x.size();
    DataMap["x"] = std::move(x);
    DataMap["y"] = std::move(y);
    DataMap["z"] = std::move(z);
    DataMap["dx"] = std::move(dx);
    DataMap["dy"] = std::move(dy);
    DataMap["dz"] = std::move(dz);
    DataMap["energy"] = std::move(energy);
    DataMap["id"] = std::move(idVec);
    DataMap["channel"] = std::move(channel);
    DataMap["feedThrough"] = std::move(feedThrough);
    DataMap["slot"] = std::move(slot);
    //Bad Cells
    if (m_doBadFCal) {
      DataMap["BadCell"] = std::move(BadCell);
    }    DataMap["sub"] = std::move(sub);

    // adc counts
    if ( m_doFCalCellDetails){
       DataMap["cellTime"] = std::move(cellTimeVec);
       DataMap["cellGain"] = std::move(cellGain);
       DataMap["cellPedestal"] = std::move(cellPedestal);
       DataMap["adc2Mev"] = std::move(adc2Mev);
    }
    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << nEntries << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // getFCalData

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
