/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BadTileRetriever.h"

#include "AthenaKernel/Units.h"

#include "EventContainers/SelectAllObject.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "TileEvent/TileCell.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileCellContainer.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileTBID.h"
#include "TileConditions/TileInfo.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  BadTileRetriever::BadTileRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_calocell_id(nullptr)
  {
    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    declareProperty("CellThreshold", m_cellThreshold = 50.);
    declareProperty("RetrieveTILE" , m_tile = true);
    declareProperty("DoBadTile",     m_doBadTile = false);
  
    declareProperty("CellEnergyPrec", m_cellEnergyPrec = 3);
  }

  /**
   * Initialise the ToolSvc
   */

  StatusCode BadTileRetriever::initialize() {
    ATH_MSG_DEBUG( "Initialising Tool"  );
    ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );
    ATH_CHECK(m_sgKey.initialize());
    return StatusCode::SUCCESS;	
  }
  
  /**
   * Tile data retrieval from chosen collection
   */
  StatusCode BadTileRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    ATH_MSG_DEBUG( "in retrieve()"  );
    SG::ReadHandle<CaloCellContainer> cellContainer(m_sgKey);
    if (!cellContainer.isValid()){
	    ATH_MSG_WARNING( "Could not retrieve Calorimeter Cells "  );
    } else {
      if(m_tile){
        DataMap data = getBadTileData(&(*cellContainer));
        ATH_CHECK( FormatTool->AddToEvent(dataTypeName(), m_sgKey.key(), &data) );
        ATH_MSG_DEBUG( "Bad Tile retrieved"  );
      }
    }
    //Tile cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve Tile bad cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap BadTileRetriever::getBadTileData(const CaloCellContainer* cellContainer) {
    ATH_MSG_DEBUG( "getBadTileData()"  );
    char rndStr[30];
    DataMap DataMap;
    DataVect phi; phi.reserve(cellContainer->size());
    DataVect eta; eta.reserve(cellContainer->size());
    DataVect idVec; idVec.reserve(cellContainer->size());
    DataVect energyVec; energyVec.reserve(cellContainer->size());
    m_sub.clear();
    //Loop Over CaloCellContainer to retrieve TileCell information
    CaloCellContainer::const_iterator it1 = cellContainer->beginConstCalo(CaloCell_ID::TILE);
    CaloCellContainer::const_iterator it2 = cellContainer->endConstCalo(CaloCell_ID::TILE);
    if (m_doBadTile==true) {
      double energyGeV;
      //int cellInd;
      ATH_MSG_DEBUG( "Start iterator loop over cells"  );
      for(;it1!=it2;++it1){
        if( !(*it1)->badcell() ) continue;
        Identifier cellid = (*it1)->ID();   
        calcTILELayerSub(cellid);
        energyGeV = (*it1)->energy()*(1./GeV);
        energyVec.emplace_back( gcvt( energyGeV, m_cellEnergyPrec, rndStr) );
        idVec.emplace_back( (Identifier::value_type)(*it1)->ID().get_compact() );
        phi.emplace_back((*it1)->phi());
        eta.emplace_back((*it1)->eta());
      } // end cell iterator
    } // doBadTile
    
    // write values into DataMap
    const auto nEntries = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["sub"] = std::move(m_sub);
    DataMap["id"] = std::move(idVec);
    DataMap["energy"] = std::move(energyVec);
    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << nEntries << " entries" );
    //All collections retrieved okay
    return DataMap;
  } // getTileData

  //-----------------------------------------------------------------------------------------------------
    
  void BadTileRetriever::calcTILELayerSub(Identifier& cellid)
  {
    if(m_calocell_id->is_tile_barrel(cellid))
      {
	if(m_calocell_id->is_tile_negative(cellid))
	  m_sub.emplace_back(2);
	else
	  m_sub.emplace_back(3);
      }
    else if(m_calocell_id->is_tile_extbarrel(cellid))
      {
	if(m_calocell_id->is_tile_negative(cellid))
	  m_sub.emplace_back(0);
	else
	  m_sub.emplace_back(5);
      }
    //else in ITC or scint
    else
      {
	if(m_calocell_id->is_tile_negative(cellid))
	  m_sub.emplace_back(1);
	else
	  m_sub.emplace_back(4);
      }
  }

  //--------------------------------------------------------------------------
  
} // JiveXML namespace