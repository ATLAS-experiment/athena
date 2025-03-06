/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODElectronRetriever.h"
#include "xAODEgamma/ElectronContainer.h"
#include "AthenaKernel/Units.h"

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODElectronRetriever::xAODElectronRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {}

  /**
   * For each electron collection retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODElectronRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("in retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::ElectronContainer> cont(key);
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
  const DataMap xAODElectronRetriever::getData(const xAOD::ElectronContainer* elCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(elCont->size());
    DataVect phi; phi.reserve(elCont->size());
    DataVect eta; eta.reserve(elCont->size());
    DataVect mass; mass.reserve(elCont->size());
    DataVect energy; energy.reserve(elCont->size());
    DataVect pdgId; energy.reserve(elCont->size());

    DataVect isEMString; isEMString.reserve(elCont->size());
    DataVect author; author.reserve(elCont->size());
    DataVect label; label.reserve(elCont->size());

    xAOD::ElectronContainer::const_iterator elItr  = elCont->begin();
    xAOD::ElectronContainer::const_iterator elItrE = elCont->end();

    int counter = 0;

    for (; elItr != elItrE; ++elItr) {

      std::string electronAuthor = "";
      std::string electronIsEMString = "none";
      std::string electronLabel = "";
      phi.emplace_back(DataType((*elItr)->phi()));
      eta.emplace_back(DataType((*elItr)->eta()));
      pt.emplace_back(DataType((*elItr)->pt()/GeV));
      mass.emplace_back(DataType((*elItr)->m()/GeV));
      energy.emplace_back( DataType((*elItr)->e()/GeV ));

      if ((*elItr)->trackParticle()){ // ForwardElectrons have no track !
	pdgId.emplace_back(DataType( -11.*(*elItr)->trackParticle()->charge() )); // pdgId not available anymore in xAOD
      }else{
	pdgId.emplace_back(DataType( 0. ) );
      }

      ATH_MSG_DEBUG("  Electron #" << counter++ << " : eta = "  << (*elItr)->eta() << ", phi = "
		    << (*elItr)->phi()
		    // << ", ntrk = " << (*elItr)->getNumberOfTrackParticles()
		    << ", author = " << (*elItr)->author()
		    // these don't work for ForwardElectrons !
		    // << ", isEM/Tight: " << (*elItr)->passSelection(passesTight, "Tight")
		    // << ", charge = " << (*elItr)->trackParticle()->charge()
		    // << ", pdgId = " << -11.*(*elItr)->trackParticle()->charge()
		    );


      bool passesTight(false);
      bool passesMedium(false);
      bool passesLoose(false);
      const bool tightSelectionExists = (*elItr)->passSelection(passesTight, "Tight");
      ATH_MSG_VERBOSE("tight exists " << tightSelectionExists
		      << " and passes? " << passesTight);
      const bool mediumSelectionExists = (*elItr)->passSelection(passesMedium, "Medium");
      ATH_MSG_VERBOSE("medium exists " << mediumSelectionExists
		      << " and passes? " << passesMedium);
      const bool looseSelectionExists = (*elItr)->passSelection(passesLoose, "Loose");
      ATH_MSG_VERBOSE("loose exists " << looseSelectionExists
		      << " and passes? " << passesLoose);

      electronAuthor = "author"+DataType( (*elItr)->author() ).toString(); // for odd ones eg FWD
      electronLabel = electronAuthor;
      if (( (*elItr)->author()) == 0){ electronAuthor = "unknown"; electronLabel += "_unknown"; }
      if (( (*elItr)->author()) == 8){ electronAuthor = "forward"; electronLabel += "_forward"; }
      if (( (*elItr)->author()) == 2){ electronAuthor = "softe"; electronLabel += "_softe"; }
      if (( (*elItr)->author()) == 1){ electronAuthor = "egamma"; electronLabel += "_egamma"; }

      if ( passesLoose ){
	electronLabel += "_Loose";
	electronIsEMString = "Loose"; // assume that hierarchy is obeyed !
      }
      if ( passesMedium ){
	electronLabel += "_Medium";
	electronIsEMString = "Medium"; // assume that hierarchy is obeyed !
      }
      if ( passesTight ){
	electronLabel += "_Tight";
	electronIsEMString = "Tight"; // assume that hierarchy is obeyed !
      }
      author.emplace_back( DataType( electronAuthor ) );
      label.emplace_back( DataType( electronLabel ) );
      isEMString.emplace_back( DataType( electronIsEMString ) );

    } // end ElectronIterator

    // four-vectors
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["pt"] = pt;
    DataMap["energy"] = energy;
    DataMap["mass"] = mass;
    DataMap["pdgId"] = pdgId;
    DataMap["isEMString"] = isEMString;
    DataMap["label"] = label;
    DataMap["author"] = author;

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << phi.size() << " entries");

    return DataMap;
  }


  const std::vector<std::string> xAODElectronRetriever::getKeys() {

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
      evtStore()->keys<xAOD::ElectronContainer>(allKeys);

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
