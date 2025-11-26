/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/TrigEMClusterRetriever.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TrigCaloEvent/TrigEMClusterContainer.h"
#include <string>

namespace JiveXML {

  //--------------------------------------------------------------------------

  TrigEMClusterRetriever::TrigEMClusterRetriever(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent),
    m_typeName("Cluster")
  {

    declareInterface<IDataRetriever>(this);

    declareProperty("StoreGateKey", m_sgKey = "HLT_TrigT2CaloEgamma", 
            "Storegate key for TrigEMClusters");
  }

  //--------------------------------------------------------------------------

  StatusCode TrigEMClusterRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    const TrigEMClusterContainer* tec;
    
    if ( evtStore()->retrieve(tec,m_sgKey).isFailure() ) {
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) <<  "No TrigEMCluster found in SG at "
          << m_sgKey << endmsg;
      return StatusCode::SUCCESS;
    } 
    int noClu = tec->size();

    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << " Retrieving TrigEMCluster with size " << noClu << endmsg;

    DataVect et; et.reserve(noClu);
    DataVect phi; phi.reserve(noClu);
    DataVect eta; eta.reserve(noClu);
    DataVect numCells; numCells.reserve(noClu);
    DataVect cells; cells.reserve(noClu);
    DataVect idVec; idVec.reserve(noClu);

    // cells n/a in AODs. This is necessary for compatibility
    // with 'full' clusters in AtlantisJava: placeholders 
    int id = 0;
    int nCells = 0;
    std::string tagCells;
    tagCells = "cells multiple=\"1.0\"";

    TrigEMClusterContainer::const_iterator cluster = tec->begin();
    for (; cluster != tec->end(); ++cluster){
      et.push_back(DataType((*cluster)->energy() / CLHEP::GeV));
      eta.push_back(DataType((*cluster)->eta()));
      phi.push_back(DataType((*cluster)->phi()));
      idVec.push_back(DataType(++id));
      cells.push_back(DataType( "0" ));  // placeholder
      numCells.push_back(DataType(nCells)); // placeholder
    }

    DataMap myDataMap;
    const auto n = phi.size();
    myDataMap["et"] = std::move(et);
    myDataMap["phi"] = std::move(phi);
    myDataMap["eta"] = std::move(eta);
    myDataMap["id"] = std::move(idVec);
    myDataMap[tagCells] = std::move(cells);
    myDataMap["numCells"] = std::move(numCells);

    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << ": "<< n << endmsg;

    //forward data to formating tool
    return FormatTool->AddToEvent(dataTypeName(), m_sgKey, &myDataMap);
  }
}
