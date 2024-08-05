/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BadLArRetriever.h"

#include "AthenaKernel/Units.h"

#include "EventContainers/SelectAllObject.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawEvent/LArRawChannel.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "Identifier/HWIdentifier.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  BadLArRetriever::BadLArRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_calocell_id(nullptr)
  {
    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareInterface<IDataRetriever>(this);
    declareProperty("LArlCellThreshold", m_cellThreshold = 50.);
    declareProperty("RetrieveLAr" , m_lar = true);
    declareProperty("DoBadLAr",     m_doBadLAr = false);
    declareProperty("CellConditionCut", m_cellConditionCut = false);

    declareProperty("CellEnergyPrec", m_cellEnergyPrec = 3);
  }

  /**
   * Initialise the ToolSvc
   */

  StatusCode BadLArRetriever::initialize() {

    ATH_MSG_DEBUG( "Initialising Tool"  );
    ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );

    ATH_CHECK(m_sgKey.initialize());
    ATH_CHECK( m_cablingKey.initialize() );

    return StatusCode::SUCCESS;	
  }
  
  /**
   * LAr data retrieval from default collection
   */
  StatusCode BadLArRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieve()"  );

    SG::ReadHandle<CaloCellContainer> cellContainer(m_sgKey);
    if (!cellContainer.isValid()){
	    ATH_MSG_WARNING( "Could not retrieve Calorimeter Cells "  );
    }
    else{
      if(m_lar){
        DataMap data = getBadLArData(&(*cellContainer));
        ATH_CHECK( FormatTool->AddToEvent(dataTypeName(), m_sgKey.key(), &data) );
        ATH_MSG_DEBUG( "Bad LAr retrieved"  );
      }
    }
    
    //LAr cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve LAr bad cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap BadLArRetriever::getBadLArData(const CaloCellContainer* cellContainer) {
    
    ATH_MSG_DEBUG( "getBadLArData()"  );

    DataMap DataMap;
    const auto nCells = cellContainer->size();
    DataVect phi; phi.reserve(nCells);
    DataVect eta; eta.reserve(nCells);
    DataVect energy; energy.reserve(nCells);
    DataVect idVec; idVec.reserve(nCells);
    DataVect channel; channel.reserve(nCells);
    DataVect feedThrough; feedThrough.reserve(nCells);
    DataVect slot; slot.reserve(nCells);


    char rndStr[30]; // for rounding (3 digit precision)
    CaloCellContainer::const_iterator it1 = cellContainer->beginConstCalo(CaloCell_ID::LAREM);
    CaloCellContainer::const_iterator it2 = cellContainer->endConstCalo(CaloCell_ID::LAREM);
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
    const LArOnOffIdMapping* cabling{*cablingHdl};
    const LArOnlineID* onlineId = nullptr;
    if ( detStore()->retrieve(onlineId, "LArOnlineID").isFailure()) {
      ATH_MSG_ERROR( "in getBadLArData(),Could not get LArOnlineID!"  );
    }

    if (m_doBadLAr && cabling) {
      double energyGeV;
      ATH_MSG_DEBUG( "Start iterator loop over cells"  );
      for(;it1!=it2;++it1){
        if( !(*it1)->badcell() ) continue;        
        if ((((*it1)->provenance()&0xFF)!=0xA5)&&m_cellConditionCut) continue; // check full conditions for LAr        
        HWIdentifier LArhwid = cabling->createSignalChannelIDFromHash((*it1)->caloDDE()->calo_hash());
        energyGeV = (*it1)->energy()*(1./GeV);
        if (energyGeV == 0) energyGeV = 0.001; // 1 MeV due to LegoCut > 0.0 (couldn't be >= 0.0) 

        energy.emplace_back( gcvt( energyGeV, m_cellEnergyPrec, rndStr) );
        idVec.emplace_back((Identifier::value_type)(*it1)->ID().get_compact() );
        phi.emplace_back((*it1)->phi());
        eta.emplace_back((*it1)->eta());
        channel.emplace_back(onlineId->channel(LArhwid)); 
        feedThrough.emplace_back(onlineId->feedthrough(LArhwid)); 
        slot.emplace_back(onlineId->slot(LArhwid)); 
      } // end cell iterator
    } // doBadLAr
    // write values into DataMap
    const auto nEntries = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["energy"] = std::move(energy);
    DataMap["id"] = std::move(idVec);
    DataMap["channel"] = std::move(channel);
    DataMap["feedThrough"] = std::move(feedThrough);
    DataMap["slot"] = std::move(slot);
    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " , collection: " << dataTypeName()
                   << " retrieved with " << nEntries << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // getBadLArData

  //--------------------------------------------------------------------------
  
} // JiveXML namespace