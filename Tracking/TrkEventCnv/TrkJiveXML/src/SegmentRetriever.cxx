/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkJiveXML/SegmentRetriever.h"

#include "TrkSegment/Segment.h"
#include "TrkSegment/TrackSegment.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

#include "JiveXML/DataType.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  SegmentRetriever::SegmentRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){}


  StatusCode SegmentRetriever::initialize(){
    ATH_CHECK(m_keys.initialize());
    return StatusCode::SUCCESS;
  }

  
  /**
   * For each segement collections retrieve all data
   * - loop over segments in all collections
   * - for each segment get basic coordinates
   * - for each segment try to obtain hits and add as multiple collection
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode SegmentRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG("Retrieving " << dataTypeName()); 

    for( const auto& key : m_keys ) {
      SG::ReadHandle<Trk::SegmentCollection > cont(key);
      if (cont.isValid()) {

	//Get number of segments in this collection
	Trk::SegmentCollection::size_type NSegs=cont.cptr()->size();

	//Define the data vectors we want to fill and create space
	DataVect x; x.reserve(NSegs);
	DataVect y; y.reserve(NSegs);
	DataVect z; z.reserve(NSegs);
	DataVect phi; phi.reserve(NSegs);
	DataVect theta; theta.reserve(NSegs);
	DataVect numHits; numHits.reserve(NSegs);
	DataVect hits;
      
	//Loop over the segments
	Trk::SegmentCollection::const_iterator SegmentItr;
	for (SegmentItr=cont.cptr()->begin(); SegmentItr!=cont.cptr()->end(); ++SegmentItr) {
        
	  //Retrieve primitive variables
	  x.emplace_back((*SegmentItr)->globalPosition().x()/10.);
	  y.emplace_back((*SegmentItr)->globalPosition().y()/10.);
	  z.emplace_back((*SegmentItr)->globalPosition().z()/10.);
	  phi.emplace_back((*SegmentItr)->localParameters()[Trk::phi]);
	  theta.emplace_back((*SegmentItr)->localParameters()[Trk::theta]);

	  //Count number of valid (non-null) RIO_OnTracks
	  int NRoTs = 0;
	  //Reserve space for expected number
	  hits.reserve(hits.size()+(*SegmentItr)->containedMeasurements().size());

	  //Loop over segment measurments
	  std::vector< const Trk::MeasurementBase * >::const_iterator measItr, measEnd;
	  measItr=(*SegmentItr)->containedMeasurements().begin();
	  measEnd=(*SegmentItr)->containedMeasurements().end();

	  //Now loop over measurements
	  for (; measItr!=measEnd; ++measItr) {
          
	    //dynamic_cast to RIO_OnTrack - will return NULL if not a RIO_OnTrack object
	    const Trk::RIO_OnTrack *RoT = dynamic_cast<const Trk::RIO_OnTrack*>(*measItr);  

	    //Ignore failed dynamic_casts
	    if (!RoT) continue ;

	    //Add the hit
	    hits.emplace_back(RoT->identify().get_compact() );
	    //count as valid
	    NRoTs++;
	  }

	  //Store number of hits for this segement
	  numHits.emplace_back(NRoTs);
	}

	//Add data to our map
	DataMap DataMap;
	DataMap["x"] = x;
	DataMap["y"] = y;
	DataMap["z"] = z;
	DataMap["phi"] = phi;
	DataMap["theta"] = theta;
	DataMap["numHits"] = numHits;

	//Hits are stored as multiple with average size given in XML header
	if (NSegs > 0) {
	  std::string multiple = "hits multiple=\"" + DataType( hits.size()*1./numHits.size()).toString() + "\"";
	  DataMap[multiple] = hits;
	}
    
	//forward data to formating tool
	if ( FormatTool->AddToEvent(dataTypeName(), key.key(), &DataMap).isFailure())
	  return StatusCode::RECOVERABLE;
      
	ATH_MSG_DEBUG(dataTypeName() << " collection " << key << " retrieved with " << NSegs << " entries");
	
      }
      else{
	ATH_MSG_WARNING("Collection " << key << " not found in SG ");
      }
    }
  
    //All collections retrieved - done
    return StatusCode::SUCCESS;

  } // retrieve
} //namespace
