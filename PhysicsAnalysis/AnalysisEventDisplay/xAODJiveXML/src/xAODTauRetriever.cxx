/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODTauRetriever.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauxAODHelpers.h"

#include "AthenaKernel/Units.h"
using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  xAODTauRetriever::xAODTauRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * For each Tau collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODTauRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::TauJetContainer> cont(key);
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
  const DataMap xAODTauRetriever::getData(const xAOD::TauJetContainer* tauCont) {

    ATH_MSG_DEBUG("in getData()");

    DataMap DataMap;

    DataVect pt; pt.reserve(tauCont->size());
    DataVect phi; phi.reserve(tauCont->size());
    DataVect eta; eta.reserve(tauCont->size());
    DataVect charge; charge.reserve(tauCont->size());

    DataVect numTracks; numTracks.reserve(tauCont->size());
    DataVect isolFrac; isolFrac.reserve(tauCont->size());
    DataVect logLhRatio; logLhRatio.reserve(tauCont->size());
    DataVect label; label.reserve(tauCont->size());
    DataVect tracks; tracks.reserve(tauCont->size());
    DataVect sgKey; sgKey.reserve(tauCont->size());
    DataVect trackLinkCount; trackLinkCount.reserve(tauCont->size());
    DataVect isTauString; isTauString.reserve(tauCont->size());

    DataVect mass; mass.reserve(tauCont->size());
    DataVect energy; energy.reserve(tauCont->size());

    xAOD::TauJetContainer::const_iterator tauItr  = tauCont->begin();
    xAOD::TauJetContainer::const_iterator tauItrE = tauCont->end();

    int counter = 0;

    for (; tauItr != tauItrE; ++tauItr) {
      ATH_MSG_DEBUG("  Tau #" << counter++ << " : eta = "  << (*tauItr)->eta() << ", phi = "
		    << (*tauItr)->phi());

      phi.emplace_back(DataType((*tauItr)->phi()));
      eta.emplace_back(DataType((*tauItr)->eta()));
      pt.emplace_back(DataType((*tauItr)->pt()/GeV));

      isolFrac.emplace_back(DataType( 1. ));
      logLhRatio.emplace_back(DataType( 1. ));
      label.emplace_back(DataType( "xAOD_tauJet_withoutQuality" ));
      charge.emplace_back(DataType( (*tauItr)->charge() ));
      isTauString.emplace_back(DataType( "xAOD_tauJet_withoutQuality" ));

      mass.emplace_back(DataType((*tauItr)->m()/GeV));
      energy.emplace_back( DataType((*tauItr)->e()/GeV ) );

      // track-vertex association code in xAOD from Nick Styles, Apr14:
      // InnerDetector/InDetRecAlgs/InDetPriVxFinder/InDetVxLinksToTrackParticles

      int trkCnt = 0;
#ifndef XAODTAU_VERSIONS_TAUTRACK_V1_H
      const std::vector< ElementLink< xAOD::TrackParticleContainer > > tpLinks =  (*tauItr)->trackLinks();
#else
      const std::vector< ElementLink< xAOD::TrackParticleContainer > > tpLinks =  xAOD::TauHelpers::trackParticleLinks(*tauItr);
#endif

      //iterating over the links
      unsigned int tp_size = tpLinks.size();
      numTracks.emplace_back(DataType( tp_size )); // same as:  (*tauItr)->nTracks()
      trackLinkCount.emplace_back(DataType( tp_size ));
      if(tp_size){ // links exist
	for(unsigned int tp = 0; tp<tp_size; ++tp)
	  {
	    ElementLink< xAOD::TrackParticleContainer >  tpl = tpLinks.at(tp);

	    //checking a container name consitency
	    //       if(tpl.key() == m_tracksName) // doesn't work. tpl.key is a number ?

	    ATH_MSG_DEBUG("  tau #" << counter << " track association index: " << tpl.index()
			  << ", collection : "  << tpl.key()
			  << ", Tracks : " << tp  << " out of " << tp_size << ", own count: " << trkCnt++);
	    tracks.emplace_back(DataType( tpl.index() ));
	    sgKey.emplace_back( m_tracksName.value() );
	  }
      } //links exist

    } // end TauIterator

    // four-vectors
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["pt"] = pt;
    DataMap["mass"] = mass;
    DataMap["energy"] = energy;

    DataMap["numTracks"] = numTracks;
    DataMap["isolFrac"] = isolFrac;
    DataMap["logLhRatio"] = logLhRatio;
    DataMap["label"] = label;
    DataMap["charge"] = charge;
    DataMap["trackLinkCount"] = trackLinkCount;
    DataMap["isTauString"] = isTauString;

    //This is needed once we know numTracks and associations:
    //If there had been any tracks, add a tag
    if ((numTracks.size()) != 0){
      //Calculate average number of tracks per vertex
      double NTracksPerVertex = tracks.size()*1./numTracks.size();
      std::string tag = "trackIndex multiple=\"" +DataType(NTracksPerVertex).toString()+"\"";
      DataMap[tag] = tracks;
      tag = "trackKey multiple=\"" +DataType(NTracksPerVertex).toString()+"\"";
      DataMap[tag] = sgKey;
    }

    //    DataMap["energy"] = energy;
    //    DataMap["mass"] = mass;

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << phi.size() << " entries");

    return DataMap;

  }


  const std::vector<std::string> xAODTauRetriever::getKeys() {

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
      evtStore()->keys<xAOD::TauJetContainer>(allKeys);
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
