/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODCaloClusterRetriever.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODCaloClusterRetriever::xAODCaloClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {}

  /**
   * For each cluster collection retrieve basic parameters.
   * 'Favourite' cluster collection first, then 'Other' collections.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODCaloClusterRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::CaloClusterContainer> cont(key);
      if (cont.isValid()) {
	DataMap data = getData(&(*cont));
	if (FormatTool->AddToEvent(dataTypeName(), key + "_xAOD", &data).isFailure()) {
	  ATH_MSG_WARNING("Failed to retrieve Collection " << key);
	} else {
	  ATH_MSG_DEBUG(" (" << key << ") retrieved");
	}
      } else {
	ATH_MSG_WARNING("Collection " << key << " not found in SG");
      }
    }
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors.
   * AOD Clusters have no cells (trying to access them without
   * back-navigation causes Athena crash).
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap xAODCaloClusterRetriever::getData(const xAOD::CaloClusterContainer* ccc) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect phi; phi.reserve(ccc->size());
    DataVect eta; eta.reserve(ccc->size());
    DataVect et; et.reserve(ccc->size());
    DataVect cells; cells.reserve(ccc->size());
    DataVect numCells; numCells.reserve(ccc->size());
    DataVect idVec; idVec.reserve(ccc->size());
    DataVect emfracVec; emfracVec.reserve(ccc->size());
    DataVect labelVec; labelVec.reserve(ccc->size());

    std::string label="";
    int id = 0;
    int s = 0;
    float eInSample = 0.;
    float eInSampleFull = 0.;
    float emfrac = 0.;
    float rawemfrac = 0.;

    // cells n/a in AOD, but keep this for compatibility
    // with 'full' clusters in AtlantisJava
    std::string tagCells;
    tagCells = "cells multiple=\"1.0\"";

    xAOD::CaloClusterContainer::const_iterator itr = ccc->begin();
    for (; itr != ccc->end(); ++itr) {

      // sum over samplings to get EMfraction:
      //// works from AOD ! Info from Sven Menke 5Aug10
      // full sum:
      for (s=0;s<CaloSampling::Unknown; s++){
	eInSampleFull += (*itr)->eSample(CaloSampling::CaloSample(s));
      }
      // Now only EMB1-3, EME1-3 and FCAL1:
      eInSample += (*itr)->eSample(CaloSampling::EMB1);
      eInSample += (*itr)->eSample(CaloSampling::EMB2);
      eInSample += (*itr)->eSample(CaloSampling::EMB3);
      eInSample += (*itr)->eSample(CaloSampling::EME1);
      eInSample += (*itr)->eSample(CaloSampling::EME2);
      eInSample += (*itr)->eSample(CaloSampling::EME3);
      eInSample += (*itr)->eSample(CaloSampling::FCAL1);

      emfrac  = eInSample/eInSampleFull;
      rawemfrac = emfrac;
      // sanity cut: emfrac should be within [0,1]
      if ( emfrac > 1.0 ) emfrac = 1.;
      if ( emfrac < 0.0 ) emfrac = 0.;
      emfracVec.emplace_back(  DataType(emfrac).toString() );

      if ( DataType( eInSample ).toString() != "0." ){
	label = "AllMeV_SumEMSampl=" + DataType( eInSample ).toString() +
	  "_SumAllSampl=" + DataType( eInSampleFull ).toString() +
	  "_calcEMFrac=" + DataType( rawemfrac ).toString()+
	  "_outEMFrac=" + DataType( emfrac ).toString();
      }else{ label = "n_a"; }
      eInSample = 0.;
      eInSampleFull = 0.;

      labelVec.emplace_back( label );
      ATH_MSG_VERBOSE("label is " << label);

      // now the standard variables
      // getBasicEnergy n/a in xAOD !

      phi.emplace_back(DataType((*itr)->phi()));
      eta.emplace_back(DataType((*itr)->eta()));
      et.emplace_back(DataType((*itr)->et()/GeV));
      numCells.emplace_back(DataType( "0" ));
      cells.emplace_back(DataType( "0" ));
      idVec.emplace_back(DataType( ++id ));

      ATH_MSG_VERBOSE( dataTypeName() << " cluster #" << id
		       << " ,e=" <<  (*itr)->e()/GeV  << ", et="
		       << (*itr)->et()/GeV << ", eta=" << (*itr)->eta()
		       << ", phi=" << (*itr)->phi());
    }
    // Start with mandatory entries
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["et"] = et;
    DataMap[tagCells] = cells;
    DataMap["numCells"] = numCells;
    DataMap["id"] = idVec;
    DataMap["emfrac"] = emfracVec; // not in Atlantis yet ! Could be used in legoplot
    DataMap["label"] = labelVec; // not in Atlantis yet !

    ATH_MSG_DEBUG(dataTypeName() << " (AOD, no cells), collection: " << dataTypeName()
		  << " retrieved with " << phi.size() << " entries");
    return DataMap;
  }


  const std::vector<std::string> xAODCaloClusterRetriever::getKeys() {

    ATH_MSG_DEBUG("in getKeys()");

    std::vector<std::string> keys = {};

    // Remove m_priorityKey from m_otherKeys if it exists, we don't want to write it twice
    auto it = std::find(m_otherKeys.begin(), m_otherKeys.end(), m_priorityKey);
    if(it != m_otherKeys.end()){
      m_otherKeys.erase(it);
    }

    // Add m_priorityKey as the first element if it is not ""
    if(m_priorityKey!=""){
      keys.push_back(m_priorityKey);
    }

    if(!m_otherKeys.empty()){
      keys.insert(keys.end(), m_otherKeys.begin(), m_otherKeys.end());
    }

    // If all collections are requested, obtain all available keys from StoreGate
    std::vector<std::string> allKeys;
    if(m_doWriteAllCollections){
      evtStore()->keys<xAOD::CaloClusterContainer>(allKeys);
      // Add keys that are not the priority key and do not add containers with "HLT" in their name if requested
      for(const std::string& key : allKeys){
	// Don't include key if it's already in keys
	auto it2 = std::find(keys.begin(), keys.end(), key);
	if(it2 != keys.end())continue;
	if(key.find("HLT") == std::string::npos || m_doWriteHLT){
	  keys.emplace_back(key);
	}
      }
    }
    return keys;
  }
} // JiveXML namespace
