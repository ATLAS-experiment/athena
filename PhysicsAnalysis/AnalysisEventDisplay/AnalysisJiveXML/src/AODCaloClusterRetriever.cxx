/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/AODCaloClusterRetriever.h"

#include "CaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloClusterMoment.h"
#include "CaloEvent/CaloVariableType.h"
#include "CaloGeoHelpers/CaloSampling.h"

#include "CLHEP/Units/SystemOfUnits.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  AODCaloClusterRetriever::AODCaloClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("Cluster"){

    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    declareProperty("FavouriteClusterCollection" ,m_sgKeyFavourite= "egClusterCollection",
        "Collection to be first in output, shown in Atlantis without switching");
    declareProperty("OtherClusterCollections" ,m_otherKeys,
        "Other collections to be retrieved. If list left empty, all available retrieved");
    declareProperty("DoWriteHLT", m_doWriteHLT = false,"Ignore HLTAutokey object by default."); // ignore HLTAutoKey objects
  }
   
  /**
   * For each cluster collections retrieve basic parameters.
   * 'Favourite' cluster collection first, then 'Other' collections.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode AODCaloClusterRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieveAll()" );
    
    SG::ConstIterator<CaloClusterContainer> iterator, end;
    const CaloClusterContainer* ccc;

    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKeyFavourite << ")" );
    StatusCode sc = evtStore()->retrieve(ccc, m_sgKeyFavourite);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKeyFavourite << " not found in SG " ); 
    }else{
      DataMap data = getData(ccc, false);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKeyFavourite, &data).isFailure()){
	ATH_MSG_WARNING( "Collection " << m_sgKeyFavourite << " not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKeyFavourite << ") AODCaloCluster retrieved" );
      }
    }

    // uncalibrated topo clusters: calibFlag=true
    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve CaloCalTopoCluster (for non-calib)" );
    StatusCode sc3 = evtStore()->retrieve(ccc, "CaloCalTopoCluster");
    if (sc3.isFailure() ) {
      ATH_MSG_WARNING( "Collection CaloCalTopoCluster (for non-calib) not found in SG " ); 
    }else{
      DataMap data = getData(ccc, true); // calibFlag: If true, use getBasicEnergy() instead of et()
      if ( FormatTool->AddToEvent(dataTypeName(), "BasicEnergyCaloCalTopoCluster", &data).isFailure()){
	ATH_MSG_WARNING( "Collection CaloCalTopoCluster (for non-calib) not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " CaloCalTopoCluster (for non-calib) AODCaloCluster retrieved" );
      }
    }

    if ( m_otherKeys.empty() ) {
      //obtain all other collections from StoreGate
      if (( evtStore()->retrieve(iterator, end)).isFailure()){
         ATH_MSG_WARNING( "Unable to retrieve iterator for AODCaloCluster collection" );
      }
      
      for (; iterator!=end; ++iterator) {

        std::string::size_type position = iterator.key().find("HLTAutoKey",0);
        if ( m_doWriteHLT ){ position = 99; } // override SG key find


        if ( position != 0 ){  // SG key doesn't contain HLTAutoKey         
	  if (iterator.key()!=m_sgKeyFavourite) {
             ATH_MSG_DEBUG( "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" );
             DataMap data = getData(&(*iterator), false);
             if ( FormatTool->AddToEvent(dataTypeName(), iterator.key(), &data).isFailure()){
	       ATH_MSG_WARNING( "Collection " << iterator.key() << " not found in SG " );
	    }else{
	      ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") AODCaloCluster retrieved" );
	    }
          }
	}
      }
    }else {
      //obtain all collections with the given keys
      std::vector<std::string>::const_iterator keyIter;
      for ( keyIter=m_otherKeys.begin(); keyIter!=m_otherKeys.end(); ++keyIter ){
	StatusCode sc = evtStore()->retrieve( ccc, (*keyIter) );
	if (!sc.isFailure()) {
          ATH_MSG_DEBUG( "Trying to retrieve selected " << dataTypeName() << " (" << (*keyIter) << ")" );
          DataMap data = getData(ccc, false);
          if ( FormatTool->AddToEvent(dataTypeName(), (*keyIter), &data).isFailure()){
	    ATH_MSG_WARNING( "Collection " << (*keyIter) << " not found in SG " );
	  }else{
	     ATH_MSG_DEBUG( dataTypeName() << " (" << (*keyIter) << ") retrieved" );
	  }
	}
      }
    }
    //All collections retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors.
   * AOD Clusters have no cells (trying to access them without
   * back-navigation causes Athena crash).
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap AODCaloClusterRetriever::getData(const CaloClusterContainer* ccc, bool calibFlag) {
    
    ATH_MSG_DEBUG( "retrieve()" );

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

    CaloClusterContainer::const_iterator itr = ccc->begin();
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
      emfracVec.push_back( emfrac );

      label = "AllMeV_SumEMSampl=" + DataType( eInSample ).toString() +
  	"_SumAllSampl=" + DataType( eInSampleFull ).toString() +
  	"_basicEnergy=" + DataType( (*itr)->getBasicEnergy()).toString() +
  	"_calcEMFrac=" + DataType( rawemfrac ).toString()+
  	"_outEMFrac=" + DataType( emfrac ).toString();
      eInSample = 0.;
      eInSampleFull = 0.;

      labelVec.push_back( label );
      ATH_MSG_DEBUG(  "label is " << label );
      

// now the standard variables

      phi.emplace_back((*itr)->phi());
      eta.emplace_back((*itr)->eta());
      if (!calibFlag){ // default: just take et
        et.emplace_back((*itr)->et()/CLHEP::GeV);
      }else{ // non-calib energies: need to convert to et by hand
        et.emplace_back( (((*itr)->getBasicEnergy()/CLHEP::GeV)*((*itr)->sinTh())) );

      }
      numCells.emplace_back( "0" );
      cells.emplace_back( "0" );
      idVec.emplace_back( ++id );


    }
    // Start with mandatory entries
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["et"] = std::move(et);
    DataMap[tagCells] = std::move(cells);
    DataMap["numCells"] = std::move(numCells);
    DataMap["id"] = std::move(idVec);
    DataMap["emfrac"] = std::move(emfracVec); // not in Atlantis yet ! Could be used in legoplot
    DataMap["label"] = std::move(labelVec); // not in Atlantis yet ! 

    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " (AOD, no cells), collection: " << dataTypeName()
      << " retrieved with " << phi.size() << " entries");
    

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
