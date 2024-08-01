/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/CompositeParticleRetriever.h"

#include "CompositeParticleEvent/CompositeParticle.h"
#include "CompositeParticleEvent/CompositeParticleContainer.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include <cmath> //std::abs

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  CompositeParticleRetriever::CompositeParticleRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("CompositeParticle"){
    //Only declare the interface
    declareInterface<IDataRetriever>(this);
    declareProperty("StoreGateKey", m_sgKey = "AllObjects", 
        "Collection to be first in output, shown in Atlantis without switching");
  }
   
  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode CompositeParticleRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    ATH_MSG_DEBUG("in retrieveAll()" );
    SG::ConstIterator<CompositeParticleContainer> iterator, end;
    const CompositeParticleContainer* compPart{};
    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKey << ")");
    StatusCode sc = evtStore()->retrieve(compPart, m_sgKey);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " ); 
    }else{
      DataMap data = getData(compPart);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKey, &data).isFailure()){
	      ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKey << ") CompositeParticle retrieved" );
      }
    }
    //obtain all other collections from StoreGate
    if (( evtStore()->retrieve(iterator, end)).isFailure()){
       ATH_MSG_WARNING( "Unable to retrieve iterator for Jet collection" );
    }
    for (; iterator!=end; ++iterator) {
      if (iterator.key()!=m_sgKey) {
        ATH_MSG_DEBUG( "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" );
        DataMap data = getData(&(*iterator));
        if ( FormatTool->AddToEvent(dataTypeName(), iterator.key(), &data).isFailure()){
	        ATH_MSG_WARNING( "Collection " << iterator.key() << " not found in SG " );
	      }else{
	         ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") CompositeParticle retrieved");
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
  const DataMap CompositeParticleRetriever::getData(const CompositeParticleContainer* cpcont) {
    ATH_MSG_DEBUG( "retrieve()");
    DataMap DataMap;
    const auto nParticles = cpcont->size();
    DataVect phi; phi.reserve(nParticles);
    DataVect eta; eta.reserve(nParticles);
    DataVect et; et.reserve(nParticles);
    DataVect mass; mass.reserve(nParticles);
    DataVect energy; energy.reserve(nParticles);
    DataVect px; px.reserve(nParticles);
    DataVect py; py.reserve(nParticles);
    DataVect pz; pz.reserve(nParticles);
    DataVect pdgId; pdgId.reserve(nParticles);
    DataVect typeEV; typeEV.reserve(nParticles);
    DataVect charge; charge.reserve(nParticles);
    DataVect dataType; dataType.reserve(nParticles);
    DataVect label; label.reserve(nParticles);

    CompositeParticleContainer::const_iterator compPartItr  = cpcont->begin();
    CompositeParticleContainer::const_iterator compPartItrE = cpcont->end();

    std::string typeLabel = "n_a"; // same as in TruthParticleRetriever
    int pdgId2 = 0;
    for (; compPartItr != compPartItrE; ++compPartItr) {
      const auto & pCompPart = *compPartItr;
      phi.emplace_back(pCompPart->phi());
      eta.emplace_back(pCompPart->eta());
      et.emplace_back(pCompPart->et()/CLHEP::GeV);
      mass.emplace_back(pCompPart->m()/CLHEP::GeV);
      energy.emplace_back( pCompPart->e()/CLHEP::GeV  );
      px.emplace_back( pCompPart->px()/CLHEP::GeV  );
      py.emplace_back( pCompPart->py()/CLHEP::GeV  );
      pz.emplace_back( pCompPart->pz()/CLHEP::GeV  );

      charge.emplace_back(  pCompPart->charge() );
      dataType.emplace_back(  pCompPart->dataType() );
      pdgId2 = pCompPart->pdgId();
      pdgId.emplace_back(pdgId2);
      const auto absId = std::abs(pdgId2);
      switch (absId){
        case 11:
          typeLabel = "EV_Electron";
          break;
        case 12:
          typeLabel = "EV_NeutrinoElectron";
          break;
        case 13:
          typeLabel = "EV_Muon";
          break;
        case 14:
          typeLabel = "EV_NeutrinoMuon";
          break;
        case 15:
          typeLabel = "EV_Tau";
          break;
        case 16:
          typeLabel = "EV_NeutrinoTau";
          break;
        case  6:
          typeLabel = "EV_Top";
          break;
        case 5:
          typeLabel = "EV_Bottom";
          break;
        case 22:
          typeLabel = "EV_Photon";
          break;
        case 23:
          typeLabel = "EV_Z0";
          break;
        case 24:
          typeLabel = ( pdgId2 == 24) ? "EV_Wplus":  "EV_Wminus";
          break;
        default:
          typeLabel = "n_a";
          break;
      }
      
      typeEV.emplace_back(typeLabel);
      label.emplace_back( "none" );
    }
    // four-vectors
    const auto nEntries = phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["et"] = std::move(et);
    DataMap["energy"] = std::move(energy);
    DataMap["mass"] = std::move(mass);
    DataMap["px"] = std::move(px);
    DataMap["py"] = std::move(py);
    DataMap["pz"] = std::move(pz);
    DataMap["pdgId"] = std::move(pdgId);
    DataMap["typeEV"] = std::move(typeEV);
    DataMap["charge"] = std::move(charge);
    DataMap["dataType"] = std::move(dataType);
    DataMap["label"] = std::move(label);

    ATH_MSG_DEBUG( " retrieved with " << nEntries << " entries");
    

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
