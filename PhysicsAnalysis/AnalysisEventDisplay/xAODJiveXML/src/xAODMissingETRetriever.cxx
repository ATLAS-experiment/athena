/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODMissingETRetriever.h"

#include "AthenaKernel/Units.h"
using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODMissingETRetriever::xAODMissingETRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * For each MET collection retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODMissingETRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG( "in retrieve()" );

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::MissingETContainer> cont(key);
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
   * Retrieve basic parameters, mainly four-vectors, for each collection.
   * Also association with clusters and tracks (ElementLink).
   */
  const DataMap xAODMissingETRetriever::getData(const xAOD::MissingETContainer* metCont) {

    ATH_MSG_DEBUG( "in getData()" );

    DataMap DataMap;

    DataVect etx; etx.reserve(metCont->size());
    DataVect ety; ety.reserve(metCont->size());
    DataVect et; et.reserve(metCont->size());

    float mpx = 0.;
    float mpy = 0.;
    float sumet = 0.;

    xAOD::MissingETContainer::const_iterator metItr  = metCont->begin();
    xAOD::MissingETContainer::const_iterator metItrE = metCont->end();

    // current understanding is that we only need the final value
    // out of the ~9 values within each MET container ('final')

    for (; metItr != metItrE; ++metItr) {
      sumet = (*metItr)->sumet()/GeV;
      mpx = (*metItr)->mpx()/GeV;
      mpy = (*metItr)->mpy()/GeV;

      ATH_MSG_DEBUG( "  Components: MissingET [GeV] mpx= "  << mpx
		     << ", mpy= " << mpy
		     << ", sumet= " << sumet );

    } // end MissingETIterator

    ATH_MSG_DEBUG( "  FINAL: MissingET [GeV] mpx= "  << mpx
		   << ", mpy= " << mpy << ", sumet= " << sumet );

    etx.emplace_back(DataType( mpx ));
    ety.emplace_back(DataType( mpy ));
    et.emplace_back(DataType( sumet ));

    // four-vectors
    DataMap["et"] = et;
    DataMap["etx"] = etx;
    DataMap["ety"] = ety;

    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << et.size() << " entries" );

    //All collections retrieved okay
    return DataMap;
  }
  const std::vector<std::string> xAODMissingETRetriever::getKeys() {

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
      evtStore()->keys<xAOD::MissingETContainer>(allKeys);
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
