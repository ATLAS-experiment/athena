/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODPhotonRetriever.h"
#include "xAODEgamma/PhotonContainer.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODPhotonRetriever::xAODPhotonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * For each photon collection retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODPhotonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::PhotonContainer> cont(key);
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
  const DataMap xAODPhotonRetriever::getData(const xAOD::PhotonContainer* phCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(phCont->size());
    DataVect phi; phi.reserve(phCont->size());
    DataVect eta; eta.reserve(phCont->size());
    DataVect mass; mass.reserve(phCont->size());
    DataVect energy; energy.reserve(phCont->size());

    DataVect isEMString; isEMString.reserve(phCont->size());
    DataVect author; author.reserve(phCont->size());
    DataVect label; label.reserve(phCont->size());

    xAOD::PhotonContainer::const_iterator phItr  = phCont->begin();
    xAOD::PhotonContainer::const_iterator phItrE = phCont->end();

    int counter = 0;

    for (; phItr != phItrE; ++phItr) {
      ATH_MSG_DEBUG("  Photon #" << counter++ << " : eta = "  << (*phItr)->eta() << ", phi = "
		    << (*phItr)->phi());

      std::string photonAuthor = "";
      std::string photonIsEMString = "none";
      std::string photonLabel = "";

      phi.emplace_back(DataType((*phItr)->phi()));
      eta.emplace_back(DataType((*phItr)->eta()));
      pt.emplace_back(DataType((*phItr)->pt()/GeV));

      bool passesTight(false);
      bool passesMedium(false);
      bool passesLoose(false);
      const bool tightSelectionExists = (*phItr)->passSelection(passesTight, "Tight");
      ATH_MSG_VERBOSE("tight exists " << tightSelectionExists
		      << " and passes? " << passesTight);
      const bool mediumSelectionExists = (*phItr)->passSelection(passesMedium, "Medium");
      ATH_MSG_VERBOSE("medium exists " << mediumSelectionExists
		      << " and passes? " << passesMedium);
      const bool looseSelectionExists = (*phItr)->passSelection(passesLoose, "Loose");
      ATH_MSG_VERBOSE("loose exists " << looseSelectionExists
		      << " and passes? " << passesLoose);

      photonAuthor = "author"+DataType( (*phItr)->author() ).toString(); // for odd ones eg FWD
      photonLabel = photonAuthor;
      if (( (*phItr)->author()) == 0){ photonAuthor = "unknown"; photonLabel += "_unknown"; }
      if (( (*phItr)->author()) == 8){ photonAuthor = "forward"; photonLabel += "_forward"; }
      if (( (*phItr)->author()) == 2){ photonAuthor = "softe"; photonLabel += "_softe"; }
      if (( (*phItr)->author()) == 1){ photonAuthor = "egamma"; photonLabel += "_egamma"; }

      if ( passesLoose ){
	photonLabel += "_Loose";
	photonIsEMString = "Loose"; // assume that hierarchy is obeyed !
      }
      if ( passesMedium ){
	photonLabel += "_Medium";
	photonIsEMString = "Medium"; // assume that hierarchy is obeyed !
      }
      if ( passesTight ){
	photonLabel += "_Tight";
	photonIsEMString = "Tight"; // assume that hierarchy is obeyed !
      }
      author.emplace_back( DataType( photonAuthor ) );
      label.emplace_back( DataType( photonLabel ) );
      isEMString.emplace_back( DataType( photonIsEMString ) );

      mass.emplace_back(DataType((*phItr)->m()/GeV));
      energy.emplace_back( DataType((*phItr)->e()/GeV ) );
    } // end PhotonIterator

    // four-vectors
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["pt"] = pt;
    DataMap["energy"] = energy;
    DataMap["mass"] = mass;
    DataMap["isEMString"] = isEMString;
    DataMap["label"] = label;
    DataMap["author"] = author;

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << phi.size() << " entries");
    return DataMap;

  }


  const std::vector<std::string> xAODPhotonRetriever::getKeys() {

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
      evtStore()->keys<xAOD::PhotonContainer>(allKeys);
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
