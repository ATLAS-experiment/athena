/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/AODJetRetriever.h"

#include "JetEvent/JetCollection.h"

#include "CLHEP/Units/SystemOfUnits.h"


namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  AODJetRetriever::AODJetRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("Jet"){

    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    
    declareProperty("FavouriteJetCollection" ,m_sgKeyFavourite = "AntiKt4TopoEMJets" ,
        "Collection to be first in output, shown in Atlantis without switching");
    declareProperty("OtherJetCollections" ,m_otherKeys,
        "Other collections to be retrieved. If list left empty, all available retrieved");
    declareProperty("DoWriteHLT", m_doWriteHLT = false,"Ignore HLTAutokey object by default."); // ignore HLTAutoKey objects
    declareProperty("WriteJetQuality", m_writeJetQuality = false,"Don't write extended jet quality details by default."); 
  }
   
  /**
   * For each jet collections retrieve basic parameters.
   * 'Favourite' jet collection first, then 'Other' jet collections.
   */
  StatusCode AODJetRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "in retrieveAll()" << endmsg;
    
    SG::ConstIterator<JetCollection> iterator, end;
    const JetCollection* jets;

    //obtain the default collection first
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve " << dataTypeName() << " (" << m_sgKeyFavourite << ")" << endmsg;
    StatusCode sc = evtStore()->retrieve(jets, m_sgKeyFavourite);
    if (sc.isFailure() ) {
      if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << m_sgKeyFavourite << " not found in SG " << endmsg; 
    }else{
      DataMap data = getData(jets);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKeyFavourite+"_AOD", &data).isFailure()){
	if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << m_sgKeyFavourite << " not found in SG " << endmsg;
      }else{
         if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << dataTypeName() << " (" << m_sgKeyFavourite << ") AODJet retrieved" << endmsg;
      }
    }

    if ( m_otherKeys.empty() ) {
      //obtain all other collections from StoreGate
      if (( evtStore()->retrieve(iterator, end)).isFailure()){
         if (msgLvl(MSG::WARNING)) msg(MSG::WARNING)  << "Unable to retrieve iterator for Jet collection" << endmsg;
//        return false;
      }
      
      for (; iterator!=end; ++iterator) {

        std::string::size_type position = iterator.key().find("HLTAutoKey",0);
        if ( m_doWriteHLT ){ position = 99; } // override SG key find

//      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << " AODJet: HLTAutoKey in " << iterator.key() << " at position " 
//	    << position << endmsg;
        if ( position != 0 ){  // SG key doesn't contain HLTAutoKey         
	  if (iterator.key()!=m_sgKeyFavourite) {
             if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" << endmsg;
             DataMap data = getData(&(*iterator));
             if ( FormatTool->AddToEvent(dataTypeName(), iterator.key()+"_AOD", &data).isFailure()){
	       if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << iterator.key() << " not found in SG " << endmsg;
	    }else{
	      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << " (" << iterator.key() << ") AODJet retrieved" << endmsg;
	    }
          }
	}
      }
    }else {
      //obtain all collections with the given keys
      std::vector<std::string>::const_iterator keyIter;
      for ( keyIter=m_otherKeys.begin(); keyIter!=m_otherKeys.end(); ++keyIter ){
        if ( !evtStore()->contains<JetCollection>( (*keyIter) ) ){ continue; } // skip if not in SG
	StatusCode sc = evtStore()->retrieve( jets, (*keyIter) );
	if (!sc.isFailure()) {
          if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve selected " << dataTypeName() << " (" << (*keyIter) << ")" << endmsg;
          DataMap data = getData(jets);
          if ( FormatTool->AddToEvent(dataTypeName(), (*keyIter)+"_AOD", &data).isFailure()){
	    if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << (*keyIter) << " not found in SG " << endmsg;
	  }else{
	     if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << " (" << (*keyIter) << ") retrieved" << endmsg;
	  }
	}
      }
    }
    //All collections retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Rretrieve basic parameters, mainly four-vectors.
   * AOD Jets have no cells (trying to access them without
   * back-navigation causes Athena crash).
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap AODJetRetriever::getData(const JetCollection* jets) {
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "retrieve()" << endmsg;

    DataMap DataMap;

    DataVect phi; phi.reserve(jets->size());
    DataVect eta; eta.reserve(jets->size());
    DataVect et; et.reserve(jets->size());
    DataVect pt; pt.reserve(jets->size());
/* need to be added to AtlantisJava/event.dtd !
    DataVect flavourTagWeight; flavourTagWeight.reserve(jets->size());
    DataVect charge; charge.reserve(jets->size());
*/
    DataVect energy; energy.reserve(jets->size());
    DataVect mass; mass.reserve(jets->size());
    DataVect px; px.reserve(jets->size());
    DataVect py; py.reserve(jets->size());
    DataVect pz; pz.reserve(jets->size());
    DataVect idVec; idVec.reserve(jets->size());

    DataVect bTagName; bTagName.reserve(jets->size());
    DataVect bTagValue; bTagValue.reserve(jets->size());

    // jet quality variables:
    DataVect quality; quality.reserve(jets->size());
    DataVect qualityLAr; qualityLAr.reserve(jets->size());
    DataVect qualityTile; qualityTile.reserve(jets->size());
    DataVect time; time.reserve(jets->size());
    DataVect timeClusters; timeClusters.reserve(jets->size());
    DataVect n90cells; n90cells.reserve(jets->size());
    DataVect n90const; n90const.reserve(jets->size());
    DataVect hecf; hecf.reserve(jets->size());
    DataVect emfrac; emfrac.reserve(jets->size());
    DataVect tileGap3f; tileGap3f.reserve(jets->size());
    DataVect fcorCell; fcorCell.reserve(jets->size());
    DataVect fcorDotx; fcorDotx.reserve(jets->size());
    DataVect fcorJet; fcorJet.reserve(jets->size());
    DataVect fcorJetForCell; fcorJetForCell.reserve(jets->size());
    DataVect nbadcells; nbadcells.reserve(jets->size());
    DataVect fracSamplingMax; fracSamplingMax.reserve(jets->size());
    DataVect sMax; sMax.reserve(jets->size());
    DataVect OutOfTimeEfrac; OutOfTimeEfrac.reserve(jets->size());
    DataVect isGood; isGood.reserve(jets->size());
    DataVect isBad; isBad.reserve(jets->size());
    DataVect isUgly; isUgly.reserve(jets->size()); 
    DataVect jvf; jvf.reserve(jets->size());

    int id = 0;

    JetCollection::const_iterator itr = jets->begin();
    for (; itr != jets->end(); ++itr) {
      phi.push_back(DataType((*itr)->phi()));
      eta.push_back(DataType((*itr)->eta()));
      et.push_back(DataType((*itr)->et()/CLHEP::GeV));
      pt.push_back(DataType((*itr)->pt()/CLHEP::GeV));
      idVec.push_back( DataType( ++id ));

//from: http://alxr.usatlas.bnl.gov/lxr/source/atlas/PhysicsAnalysis/AnalysisCommon/AnalysisExamples/src/JetTagAna.cxx
//
      // bjet tagger values
      bTagName.push_back( DataType( "JetFitterCOMBNN" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("JetFitterCOMBNN") ));
      bTagName.push_back( DataType( "JetFitterTagNN" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("JetFitterTagNN") ));
      bTagName.push_back( DataType( "IP3D+SV1" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight() ));
      bTagName.push_back( DataType( "IP2D" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("IP2D") ));
      bTagName.push_back( DataType( "IP3D" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("IP3D") ));
      bTagName.push_back( DataType( "SV1" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("SV1") ));
      bTagName.push_back( DataType( "SV2" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("SV2") ));
      bTagName.push_back( DataType( "MV1" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("MV1") ));
      bTagName.push_back( DataType( "MV2" ));
      bTagValue.push_back( DataType( (*itr)->getFlavourTagWeight("MV2") ));

      // basic jet quality 
      quality.push_back(DataType((*itr)->getMoment("LArQuality")));

      isGood.push_back(DataType( 1 ));
      isBad.push_back(DataType( 0 ));
      isUgly.push_back(DataType( 0 ));
      emfrac.push_back(DataType( 0.5 ));


      jvf.push_back( DataType((*itr)->getMoment("JVF") ));

      energy.push_back( DataType((*itr)->e()/CLHEP::GeV ) );
      mass.push_back(DataType((*itr)->m()/CLHEP::GeV));
      px.push_back( DataType((*itr)->px()/CLHEP::GeV ) );
      py.push_back( DataType((*itr)->py()/CLHEP::GeV ) );
      pz.push_back( DataType((*itr)->pz()/CLHEP::GeV ) );
    }

    // Start with mandatory entries
    const auto n = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["et"] = std::move(et);
    DataMap["pt"] = std::move(pt);
    DataMap["id"] = std::move(idVec);

    DataMap["bTagName multiple=\"9\""] = std::move(bTagName); // assigned by hand !
    DataMap["bTagValue multiple=\"9\""] = std::move(bTagValue);
	
    // basic jet quality
    DataMap["quality"] = std::move(quality);
    DataMap["isGood"] = std::move(isGood);
    DataMap["isBad"] = std::move(isBad);
    DataMap["isUgly"] = std::move(isUgly);
    DataMap["emfrac"] = std::move(emfrac);

    DataMap["jvf"] = std::move(jvf);

    if (m_writeJetQuality){ // extended jet quality
      DataMap["qualityLAr"] = std::move(qualityLAr);
      DataMap["qualityTile"] = std::move(qualityTile);
      DataMap["time"] = std::move(time);
      DataMap["timeClusters"] = std::move(timeClusters);
      DataMap["n90cells"] = std::move(n90cells);
      DataMap["n90const"] = std::move(n90const);
      DataMap["hecf"] = std::move(hecf);
      DataMap["tileGap3f"] = std::move(tileGap3f);
      DataMap["fcorCell"] = std::move(fcorCell);
      DataMap["fcorDotx"] = std::move(fcorDotx);
      DataMap["fcorJet"] = std::move(fcorJet);
      DataMap["fcorJetForCell"] = std::move(fcorJetForCell);
      DataMap["nbadcells"] = std::move(nbadcells);
      DataMap["fracSamplingMax"] = std::move(fracSamplingMax);
      DataMap["sMax"] = std::move(sMax);
      DataMap["OutOfTimeEfrac"] = std::move(OutOfTimeEfrac);
    } // writeJetQuality
 
    // further details
    // four-vectors

    DataMap["mass"] = std::move(mass);
    DataMap["px"] = std::move(px);
    DataMap["py"] = std::move(py);
    DataMap["pz"] = std::move(pz);
    DataMap["energy"] = std::move(energy);

    if (msgLvl(MSG::DEBUG)) {
      msg(MSG::DEBUG) << dataTypeName() << " (AOD, no cells), collection: " << dataTypeName();
      msg(MSG::DEBUG) << " retrieved with " << n << " entries"<< endmsg;
    }

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
