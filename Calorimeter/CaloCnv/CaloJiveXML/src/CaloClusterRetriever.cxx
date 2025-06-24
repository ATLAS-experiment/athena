/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterRetriever.h"

#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  CaloClusterRetriever::CaloClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_sgKeyFavourite ("egammaClusters")
  {
    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    declareProperty("FavouriteClusterCollection" ,m_sgKeyFavourite,
        "Collection to be first in output, shown in Atlantis without switching");
    declareProperty("OtherClusterCollections" ,m_otherKeys,
        "Other collections to be retrieved. If list left empty, all available retrieved");
    declareProperty("DoWriteHLT", m_doWriteHLT = false,"Ignore HLTAutokey object by default."); // ignore HLTAutoKey objects
  }

  StatusCode CaloClusterRetriever::initialize()
  {

    ATH_MSG_DEBUG("Initialising Tool");

    ATH_CHECK(m_sgKeyFavourite.initialize());

    return StatusCode::SUCCESS;
  }

  /**
   * For each cluster collections retrieve basic parameters.
   * 'Favourite' cluster collection first, then 'Other' collections.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode CaloClusterRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieveAll()"  );
    
    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKeyFavourite.key() << ")"  );

    SG::ReadHandle<xAOD::CaloClusterContainer> ccc_primary(m_sgKeyFavourite);
    if (ccc_primary.isValid()) {
      DataMap data = getData(&(*ccc_primary));
      if (FormatTool->AddToEvent(dataTypeName(), m_sgKeyFavourite.key() + "_ESD", &data).isFailure()) {
        ATH_MSG_WARNING("Failed to retrieve favourite Collection " << m_sgKeyFavourite.key() );
      }
      else {
        ATH_MSG_DEBUG(dataTypeName() << " (" << m_sgKeyFavourite.key() << ") CaloCluster retrieved");
      }
    }
    else{
      ATH_MSG_WARNING("Favourite Collection " << m_sgKeyFavourite.key() << " not found in SG ");
    }

    if ( m_otherKeys.empty() ) {
      //obtain all other collections from StoreGate
      std::vector<std::string> allkeys;
      evtStore()->keys(static_cast<CLID>( ClassID_traits<xAOD::CaloClusterContainer>::ID() ), allkeys);
      for (auto key : allkeys) {
        if (key!=m_sgKeyFavourite.key()) {
          ATH_MSG_DEBUG( "Trying to retrieve all " << dataTypeName() << " (" << key << ")"  );
          SG::ReadHandle<xAOD::CaloClusterContainer> containerRH(key);
          if (!containerRH.isValid()) {
            ATH_MSG_DEBUG( "Unable to retrieve CaloCluster collection " << key );
          } else {
            std::string::size_type position = key.find("HLTAutoKey",0);
            if ( m_doWriteHLT ){ position = 99; } // override SG key find
            if ( position != 0 ){  // SG key doesn't contain HLTAutoKey
              DataMap data = getData(&*containerRH);
              if ( FormatTool->AddToEvent(dataTypeName(), containerRH.key()+"_ESD", &data).isFailure()){
                 ATH_MSG_WARNING( "Collection " << containerRH.key() << " not found in SG "  );
              }else{
                 ATH_MSG_DEBUG( dataTypeName() << " (" << containerRH.key() << ") CaloCluster retrieved"  );
              }
            }
          }
        }
      }
    }else {
      //obtain all collections with keys provided by user: m_otherKeys
      for (auto key : m_otherKeys) {
        if (key!=m_sgKeyFavourite.key()) {
          ATH_MSG_DEBUG( "Trying to retrieve selected " << dataTypeName() << " (" << key << ")"  );
          SG::ReadHandle<xAOD::CaloClusterContainer> containerRH(key);
          if (!containerRH.isValid()) {
            ATH_MSG_DEBUG( "Unable to retrieve CaloCluster collection " << key );
          } else {
            DataMap data = getData(&*containerRH);
            if ( FormatTool->AddToEvent(dataTypeName(), containerRH.key()+"_ESD", &data).isFailure()){
               ATH_MSG_WARNING( "Collection " << containerRH.key() << " not found in SG "  );
            }else{
               ATH_MSG_DEBUG( dataTypeName() << " (" << containerRH.key() << ") retrieved"  );
            }
          }
        }
      }
    }
    //All collections retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors.
   *  Clusters have no cells (trying to access them without
   * back-navigation causes Athena crash).
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap CaloClusterRetriever::getData(const xAOD::CaloClusterContainer* ccc) {
    
    ATH_MSG_DEBUG( "getData()"  );

    DataMap DataMap;

    DataVect phi; phi.reserve(ccc->size());
    DataVect eta; eta.reserve(ccc->size());
    DataVect et; et.reserve(ccc->size());
    DataVect idVec; idVec.reserve(ccc->size());
    DataVect numCellsVec; numCellsVec.reserve(ccc->size());
    DataVect cells; cells.reserve( ccc->size() );

    int noClu = ccc->size();
    int noCells = 0;
    
    int id = 0;
    for (const auto cluster : *ccc) {
      phi.push_back(DataType(cluster->phi()));
      eta.push_back(DataType(cluster->eta()));
      et.push_back(DataType(cluster->et()*(1./GeV)));
      idVec.push_back(DataType( ++id ));

      int numCells = cluster->size();
      numCellsVec.push_back(DataType( numCells ));
      noCells += numCells;

      for (const auto cell : *cluster) {
        cells.push_back(cell->ID().get_compact());
      }
    }

    std::string tagCells;
    if(noClu){
      tagCells = "cells multiple=\"" +DataType(noCells/(noClu*1.0)).toString()+"\"";
    }else{
      tagCells = "cells multiple=\"1.0\"";
    }

    // Start with mandatory entries
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["et"] = et;
    DataMap[tagCells] = cells;
    DataMap["numCells"] = numCellsVec;
    DataMap["id"] = idVec;

    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " , collection: " << dataTypeName()
                   << " retrieved with " << phi.size() << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
