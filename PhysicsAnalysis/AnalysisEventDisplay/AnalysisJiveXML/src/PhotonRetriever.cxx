/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/PhotonRetriever.h"

#include "egammaEvent/PhotonContainer.h"
#include "egammaEvent/EMShower.h"

#include "CLHEP/Units/SystemOfUnits.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  PhotonRetriever::PhotonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("Photon"){

    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareProperty("StoreGateKey", m_sgKey = "PhotonAODCollection",
        "Collection to be first in output, shown in Atlantis without switching");
  }
   
  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode PhotonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "in retrieveAll()" << endmsg;
    
    SG::ConstIterator<PhotonContainer> iterator, end;
    const PhotonContainer* photons;
    
    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKey << ")" );
    StatusCode sc = evtStore()->retrieve(photons, m_sgKey);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " ); 
    }else{
      DataMap data = getData(photons);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKey, &data).isFailure()){
	      ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKey << ") Photon retrieved" );
      }
    }

    //obtain all other collections from StoreGate
    if (( evtStore()->retrieve(iterator, end)).isFailure()){
       if (msgLvl(MSG::WARNING)) msg(MSG::WARNING)  << "Unable to retrieve iterator for Jet collection" << endmsg;
//        return StatusCode::WARNING;
    }
      
    for (; iterator!=end; ++iterator) {
       if (iterator.key()!=m_sgKey) {
          ATH_MSG_DEBUG( "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" );
            DataMap data = getData(&(*iterator));
            if ( FormatTool->AddToEvent(dataTypeName(), iterator.key(), &data).isFailure()){
	            ATH_MSG_WARNING( "Collection " << iterator.key() << " not found in SG " );
	          }else{
	            ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") Photon retrieved" );
            }
	  }
    }	  
    //All collections retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors, for each collection.
   * Also association with clusters (ElementLink).
   */
  const DataMap PhotonRetriever::getData(const PhotonContainer* photcont) {
    
    ATH_MSG_DEBUG( "retrieve()" );

    DataMap DataMap;

    DataVect phi; phi.reserve(photcont->size());
    DataVect eta; eta.reserve(photcont->size());
    DataVect pt; pt.reserve(photcont->size());
    DataVect mass; mass.reserve(photcont->size());
    DataVect energy; energy.reserve(photcont->size());
    DataVect px; px.reserve(photcont->size());
    DataVect py; py.reserve(photcont->size());
    DataVect pz; pz.reserve(photcont->size());

    DataVect isEM; isEM.reserve(photcont->size());
    DataVect f1Vec; f1Vec.reserve(photcont->size());
    DataVect etConeVec; etConeVec.reserve(photcont->size());
    DataVect fracs1Vec; fracs1Vec.reserve(photcont->size());
    DataVect et37Vec; et37Vec.reserve(photcont->size());

    DataVect author; author.reserve(photcont->size());
    DataVect label; label.reserve(photcont->size());
    DataVect isEMString; isEMString.reserve(photcont->size());
    	
    // for associations:
    DataVect clusterKeyVec; clusterKeyVec.reserve(photcont->size());
    DataVect clusterIndexVec; clusterIndexVec.reserve(photcont->size());

    PhotonContainer::const_iterator photonItr  = photcont->begin();
    PhotonContainer::const_iterator photonItrE = photcont->end();

    int MCdataType = 1;
    std::string clusterKey = "none"; // Storegate key of container 
    int clusterIndex = -1; // index number inside the container 

    std::string photonAuthor = "";
    std::string photonIsEMString = "none";
    std::string photonLabel = "";

    // reference for authors: Reconstruction/egamma/egammaEvent/egammaParamDefs.h
    //     https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhotonIdentification
    for (; photonItr != photonItrE; ++photonItr) {
      photonIsEMString = "none";
      photonAuthor = "author"+DataType( (*photonItr)->author() ).toString(); // for odd ones eg FWD
      photonLabel = photonAuthor;
      if (( (*photonItr)->author()) == 0x0){ photonAuthor = "unknown"; photonLabel += "_unknown"; }
      if (( (*photonItr)->author()) == 0x8){ photonAuthor = "forward"; photonLabel += "_forward"; }
      if (( (*photonItr)->author()) == 0x10){ photonAuthor = "rconv"; photonLabel += "_recoveredconversion"; }
      if (( (*photonItr)->author()) == 0x4){ photonAuthor = "photon"; photonLabel += "_photon"; }
      if ( (*photonItr)->isem(egammaPIDObs::PhotonLoose)==0){
	      photonLabel += "_Loose";
	      photonIsEMString = "Loose"; // assume that hierarchy is obeyed !
      }  
      if ( (*photonItr)->isem(egammaPIDObs::PhotonTight)==0){
	      photonLabel += "_Tight";
	      photonIsEMString = "Tight"; // assume that hierarchy is obeyed !
      }  
      if ( (*photonItr)->isem(egammaPIDObs::PhotonLooseAR)==0){
	      photonLabel += "_LooseAR";
      }  
      if ( (*photonItr)->isem(egammaPIDObs::PhotonTightAR)==0){
	      photonLabel += "_TightAR";
      }  
      if ( (*photonItr)->isem(egammaPIDObs::PhotonTightARIso)==0){
	       photonLabel += "_TightARIso";
      }  
      if ( (*photonItr)->isem(egammaPIDObs::PhotonTightIso)==0){
	      photonLabel += "_TightIso";
      }  

      phi.emplace_back((*photonItr)->phi());
      eta.emplace_back((*photonItr)->eta());
      pt.emplace_back((*photonItr)->pt()/CLHEP::GeV);
      mass.emplace_back((*photonItr)->m()/CLHEP::GeV);
      energy.emplace_back((*photonItr)->e()/CLHEP::GeV  );
      px.emplace_back((*photonItr)->px()/CLHEP::GeV  );
      py.emplace_back((*photonItr)->py()/CLHEP::GeV  );
      pz.emplace_back((*photonItr)->pz()/CLHEP::GeV  );

      MCdataType = (*photonItr)->dataType();  
      if (MCdataType != 3){ // full simulation
          isEM.emplace_back((**photonItr).isem() );
// do associations:
          const ElementLink<CaloClusterContainer> clusterLink = (*photonItr)->clusterElementLink();
          if (clusterLink.isValid()) {
            clusterKey = clusterLink.dataID(); // Storegate key of container 
            clusterIndex = clusterLink.index(); // index number inside the container 
            clusterKeyVec.emplace_back( clusterKey );
	          clusterIndexVec.emplace_back( clusterIndex );
          } else { // no clusterLink
	          clusterKeyVec.emplace_back( "none" );
	          clusterIndexVec.emplace_back( -1 );
          }
	  /// get shower variables. Booked in AtlantisJava/event.dtd:
          // emWeight|et37|etCone|etHad1|f1|fracs1|pionWeight
	  const EMShower* emShower = (*photonItr)->detail<EMShower>("egDetailAOD");
	  if (emShower) {
      f1Vec.emplace_back( emShower->parameter(egammaParameters::f1) ); 
      etConeVec.emplace_back( emShower->parameter(egammaParameters::etcone20));
      fracs1Vec.emplace_back( emShower->parameter(egammaParameters::fracs1));
      et37Vec.emplace_back(  emShower->parameter(egammaParameters::e237));
    }else{ //placeholders if no shower available
      f1Vec.emplace_back( -1.); 
      etConeVec.emplace_back( -1. );
      fracs1Vec.emplace_back( -1. );
      et37Vec.emplace_back( -1.);
    }
        } else {  // fast simulation: just placeholders
          isEM.emplace_back( 0  );
          clusterKeyVec.emplace_back( "none" );
          clusterIndexVec.emplace_back( -1 );
          f1Vec.push_back( -1.); 
          etConeVec.emplace_back( -1. );
          fracs1Vec.emplace_back( -1. );
          et37Vec.emplace_back( -1. );
          photonLabel += "_fastSim";
          photonIsEMString = "fastSim";
	  }
        author.emplace_back(photonAuthor  );
        label.emplace_back( photonLabel  );
        isEMString.emplace_back( photonIsEMString  );
    }
    // four-vectors
    const auto nEntries = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["pt"] = std::move(pt);
    DataMap["energy"] = std::move(energy);
    DataMap["mass"] = std::move(mass);
    DataMap["px"] = std::move(px);
    DataMap["py"] = std::move(py);
    DataMap["pz"] = std::move(pz);

    // further details and associations
    DataMap["isEM"] = std::move(isEM);
    DataMap["clusterKey"] = std::move(clusterKeyVec);
    DataMap["clusterIndex"] = std::move(clusterIndexVec);
    // shower details
    DataMap["f1"] = std::move(f1Vec);
    DataMap["etCone"] = std::move(etConeVec);
    DataMap["fracs1"] = std::move(fracs1Vec);
    DataMap["et37"] = std::move(et37Vec);

    DataMap["author"] = std::move(author);
    DataMap["isEMString"] = std::move(isEMString);
    DataMap["label"] = std::move(label);

    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << nEntries << " entries");
    

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
