/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODVertexRetriever.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

#include "AthenaKernel/Units.h"
using Athena::Units::cm;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   *
   */
  xAODVertexRetriever::xAODVertexRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}

  /**
   * For each Vertex collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODVertexRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG("In retrieve()");

    std::vector<std::string> keys = getKeys();

    if(keys.empty()){
      ATH_MSG_WARNING("No StoreGate keys found");
      return StatusCode::SUCCESS;
    }

    // Loop through the keys and retrieve the corresponding data
    for (const std::string& key : keys) {
      SG::ReadHandle<xAOD::VertexContainer> cont(key);
      if (cont.isValid()) {
	DataMap data = getData(&(*cont),key);
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
  const DataMap xAODVertexRetriever::getData(const xAOD::VertexContainer* cont, const std::string &key) {

    ATH_MSG_DEBUG("in getData()");

    float chi2val = 0.;

    DataMap DataMap;

    DataVect x;
    DataVect y;
    DataVect z;
    DataVect chi2;
    DataVect vertexType;
    DataVect primVxCand;
    DataVect covMatrix;
    DataVect numTracks;
    DataVect tracks;
    DataVect sgkey;

    //Get size of current container
    xAOD::VertexContainer::size_type NVtx = cont->size();

    ATH_MSG_DEBUG("Reading vertex container " << key
		  << " with " << NVtx << " entries");

    x.reserve(x.size()+NVtx);
    y.reserve(y.size()+NVtx);
    z.reserve(z.size()+NVtx);
    chi2.reserve(chi2.size()+NVtx);
    vertexType.reserve(vertexType.size()+NVtx);
    primVxCand.reserve(primVxCand.size()+NVtx);
    covMatrix.reserve(covMatrix.size()+NVtx);
    numTracks.reserve(numTracks.size()+NVtx);
    tracks.reserve(tracks.size()+NVtx);
    sgkey.reserve(sgkey.size()+NVtx);

    int counter = 0;

    //Loop over vertices
    xAOD::VertexContainer::const_iterator VertexItr = cont->begin();
    for ( ; VertexItr != cont->end(); ++VertexItr) {

      ATH_MSG_DEBUG("  Vertex #" << counter++ << " : x = "  << (*VertexItr)->x()/cm << ", y = "
		    << (*VertexItr)->y()/cm << ", z[GeV] = "  << (*VertexItr)->z()/cm
		    << ", vertexType = "  << (*VertexItr)->vertexType()
		    << ", chiSquared = "  << (*VertexItr)->chiSquared()
		    << ", numberDoF = "  << (*VertexItr)->numberDoF());

      x.emplace_back(DataType((*VertexItr)->x()/cm));
      y.emplace_back(DataType((*VertexItr)->y()/cm));
      z.emplace_back(DataType((*VertexItr)->z()/cm));

      if ( key == m_secondaryVertexKey){
	vertexType.emplace_back( 2 );
      }else{
	vertexType.emplace_back( DataType((*VertexItr)->vertexType()));
      }

      if ((*VertexItr)->vertexType() == 1 ){
	primVxCand.emplace_back( 1 );
      }else{
	primVxCand.emplace_back( 0 );
      }
      sgkey.emplace_back (m_tracksName.value());

      //// placeholder ! From old V0CandidateRetriever.
      covMatrix.emplace_back(DataType("2 -.1 .5 -.01 0.002 .01"));

      //degrees of freedom might be zero - beware
      if ( (*VertexItr)->numberDoF()  != 0 ){
	chi2val = (*VertexItr)->chiSquared()/(*VertexItr)->numberDoF() ;
      }else{
	chi2val = -1.;
      }
      chi2.emplace_back(DataType( chi2val ));

      // track-vertex association code in xAOD from Nick Styles, Apr14:
      // InnerDetector/InDetRecAlgs/InDetPriVxFinder/InDetVxLinksToTrackParticles

      int trkCnt = 0;
      const std::vector< ElementLink< xAOD::TrackParticleContainer > > tpLinks =  (*VertexItr)->trackParticleLinks();

      //iterating over the links
      unsigned int tp_size = tpLinks.size();
      numTracks.emplace_back(DataType( tp_size ));
      if(tp_size){ // links exist
	for(unsigned int tp = 0; tp<tp_size; ++tp)
	  {
	    ElementLink< xAOD::TrackParticleContainer >  tpl = tpLinks.at(tp);

	    ATH_MSG_DEBUG("  Vertex #" << counter << " track association index: " << tpl.index()
			  << ", collection : "  << tpl.key()
			  << ", Tracks : " << tp  << " out of " << tp_size << ", own count: " << trkCnt++);

	    if ( tpl.index() < 1000 ){ // sanity check, this can be huge number
	      tracks.emplace_back(DataType( tpl.index() ));
	    }else{
	      tracks.emplace_back(DataType( 0 ));
	    }
	  } //links exist
      }//end of track particle collection size check

    } // end VertexIterator

      // four-vectors
    DataMap["x"] = x;
    DataMap["y"] = y;
    DataMap["z"] = z;
    DataMap["chi2"] = chi2;
    DataMap["vertexType"] = vertexType;
    DataMap["primVxCand"] = primVxCand;
    DataMap["covMatrix multiple=\"6\""] = covMatrix;
    DataMap["numTracks"] = numTracks;
    DataMap["sgkey"] = sgkey;

    //This is needed once we know numTracks and associations:
    //If there had been any tracks, add a tag
    if ((numTracks.size()) != 0){
      //Calculate average number of tracks per vertex
      double NTracksPerVertex = tracks.size()*1./numTracks.size();
      std::string tag = "tracks multiple=\"" +DataType(NTracksPerVertex).toString()+"\"";
      DataMap[tag] = tracks;
    }

    ATH_MSG_DEBUG(dataTypeName() << " retrieved with " << x.size() << " entries");

    return DataMap;

  }


  const std::vector<std::string> xAODVertexRetriever::getKeys() {
    ATH_MSG_DEBUG("in getKeys()");
    
    std::vector<std::string> keys = {};
    
    // Remove m_primaryVertexKey and m_secondaryVertexKey from m_otherKeys if they  exist, we don't want to write them twice
    auto it = std::find(m_otherKeys.begin(), m_otherKeys.end(), m_primaryVertexKey);
    if(it != m_otherKeys.end()){
      m_otherKeys.erase(it);
    }
    auto it2 = std::find(m_otherKeys.begin(), m_otherKeys.end(), m_secondaryVertexKey);
    if(it2 != m_otherKeys.end()){
      m_otherKeys.erase(it2);
    }

    // Add m_primaryVertexKey as the first element and m_secondaryVertexKey as the second if they are not ""
    if(m_primaryVertexKey!=""){
      keys.push_back(m_primaryVertexKey);
    }
    if(m_secondaryVertexKey!=""){
      keys.push_back(m_secondaryVertexKey);
    }

    if(!m_otherKeys.empty()){
      keys.insert(keys.end(), m_otherKeys.begin(), m_otherKeys.end());
    }

    // If all collections are requested, obtain all available keys from StoreGate
    std::vector<std::string> allKeys;

    if(m_doWriteAllCollections){
      evtStore()->keys<xAOD::VertexContainer>(allKeys);

      // Add keys that are not the priority key and do not add containers with "HLT" in their name if requested, , and do not contain V0 in their name, if requested
      for(const std::string& key : allKeys){
	// Don't include key if it's already in keys
	auto it2 = std::find(keys.begin(), keys.end(), key);
	if(it2 != keys.end())continue;
	if((key.find("HLT") == std::string::npos || m_doWriteHLT) && (m_doWriteV0 || key.find("V0") == std::string::npos)){
	  keys.emplace_back(key);
	}
      }
    }
    return keys;
  }

} // JiveXML namespace
