/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODMuonRetriever.h"
#include "xAODMuon/MuonContainer.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODMuonRetriever::xAODMuonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * For each muon collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODMuonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::MuonContainer> cont(key);
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
  const DataMap xAODMuonRetriever::getData(const xAOD::MuonContainer* muCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(muCont->size());
    DataVect phi; phi.reserve(muCont->size());
    DataVect eta; eta.reserve(muCont->size());
    DataVect mass; mass.reserve(muCont->size());
    DataVect energy; energy.reserve(muCont->size());
    DataVect chi2; chi2.reserve(muCont->size());
    DataVect pdgId; pdgId.reserve(muCont->size());

    xAOD::MuonContainer::const_iterator muItr  = muCont->begin();
    xAOD::MuonContainer::const_iterator muItrE = muCont->end();

    int counter = 0;

    for (; muItr != muItrE; ++muItr) {

      ATH_MSG_DEBUG("  Muon #" << counter++ << " : eta = "  << (*muItr)->eta()
		    << ", phi = "  << (*muItr)->phi() << ", pt = " <<  (*muItr)->pt()
		    << ", pdgId = " << -13.*(*muItr)->primaryTrackParticle()->charge());

      phi.emplace_back(DataType((*muItr)->phi()));
      eta.emplace_back(DataType((*muItr)->eta()));
      pt.emplace_back(DataType((*muItr)->pt()/GeV));

      mass.emplace_back(DataType((*muItr)->m()/GeV));
      energy.emplace_back( DataType((*muItr)->e()/GeV ) );
      chi2.emplace_back( 1.0 ); //placeholder
      pdgId.emplace_back(DataType( -13.*(*muItr)->primaryTrackParticle()->charge() )); // pdgId not available anymore in xAOD
    } // end MuonIterator

    // four-vectors
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["pt"] = pt;
    DataMap["energy"] = energy;
    DataMap["mass"] = mass;
    DataMap["chi2"] = chi2;
    DataMap["pdgId"] = pdgId;

    ATH_MSG_DEBUG(" retrieved with " << phi.size() << " entries");
    return DataMap;
  }


  const std::vector<std::string> xAODMuonRetriever::getKeys() {

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
      evtStore()->keys<xAOD::MuonContainer>(allKeys);
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
