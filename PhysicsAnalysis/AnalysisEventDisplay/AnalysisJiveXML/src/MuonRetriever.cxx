/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/MuonRetriever.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "muonEvent/MuonContainer.h"

// for associations:
#include "Particle/TrackParticleContainer.h"
#include "CaloEvent/CaloClusterContainer.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  MuonRetriever::MuonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("Muon"){

    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareProperty("StoreGateKey", m_sgKey= "Muons", 
        "Collection to be first in output, shown in Atlantis without switching");
  }
   
  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode MuonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieveAll()" );
    
    SG::ConstIterator<Analysis::MuonContainer> iterator, end;
    const Analysis::MuonContainer* muCont;
    
    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKey << ")" );
    StatusCode sc = evtStore()->retrieve(muCont, m_sgKey);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " ); 
    }else{
      DataMap data = getData(muCont);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKey, &data).isFailure()){
	ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKey << ") Muon retrieved" );
      }
    }

    //obtain all other collections from StoreGate
    if (( evtStore()->retrieve(iterator, end)).isFailure()){
       ATH_MSG_WARNING( "Unable to retrieve iterator for Jet collection" );
    }
      
    for (; iterator!=end; ++iterator) {
      // hack to remove CaloMuons, jpt 31Mar10 
      if ((iterator.key() != m_sgKey) && (iterator.key() != "CaloMuonCollection") && 
	  (iterator.key() != "CaloESDMuonCollection" ))  {
      //// was:
      //       if (iterator.key()!=m_sgKey) {
          if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" << endmsg;
            DataMap data = getData(&(*iterator));
            if ( FormatTool->AddToEvent(dataTypeName(), iterator.key(), &data).isFailure()){
	       ATH_MSG_WARNING( "Collection " << iterator.key() << " not found in SG " );
	    }else{
	      ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") Muon retrieved" );
            }
	  }
    }	  
    //All collections retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors, for each collection.
   * Also association with clusters and tracks (ElementLink).
   */
  const DataMap MuonRetriever::getData(const Analysis::MuonContainer* muoncont) {
    
    ATH_MSG_DEBUG( "in getData()" );

    DataMap DataMap;

    DataVect phi; phi.reserve(muoncont->size());
    DataVect eta; eta.reserve(muoncont->size());
    DataVect pt; pt.reserve(muoncont->size());
    DataVect mass; mass.reserve(muoncont->size());
    DataVect energy; energy.reserve(muoncont->size());
    DataVect px; px.reserve(muoncont->size());
    DataVect py; py.reserve(muoncont->size());
    DataVect pz; pz.reserve(muoncont->size());

    DataVect dataType; dataType.reserve(muoncont->size());
    DataVect chi2; chi2.reserve(muoncont->size());
    DataVect pdgId; pdgId.reserve(muoncont->size());
    DataVect etConeIsol; etConeIsol.reserve(muoncont->size());
    DataVect author; author.reserve(muoncont->size());

    // for associations:
    DataVect clusterKeyVec; clusterKeyVec.reserve(muoncont->size());
    DataVect clusterIndexVec; clusterIndexVec.reserve(muoncont->size());
    DataVect trackKeyVec; trackKeyVec.reserve(muoncont->size());
    DataVect trackIndexVec; trackIndexVec.reserve(muoncont->size());

    Analysis::MuonContainer::const_iterator muonItr  = muoncont->begin();
    Analysis::MuonContainer::const_iterator muonItrE = muoncont->end();

    int MCdataType = 1;
    std::string clusterKey = "none"; // Storegate key of container 
    int clusterIndex = -1; // index number inside the container 
    std::string trackKey = "none"; // Storegate key of container 
    int trackIndex = -1; // index number inside the container 

    for (; muonItr != muonItrE; ++muonItr) {
      phi.emplace_back((*muonItr)->phi());
      eta.emplace_back((*muonItr)->eta());
      pt.emplace_back((*muonItr)->pt()/CLHEP::GeV);
      mass.emplace_back((*muonItr)->m()/CLHEP::GeV);
      energy.emplace_back( (*muonItr)->e()/CLHEP::GeV  );
      px.emplace_back( (*muonItr)->px()/CLHEP::GeV  );
      py.emplace_back( (*muonItr)->py()/CLHEP::GeV  );
      pz.emplace_back( (*muonItr)->pz()/CLHEP::GeV  );

      pdgId.emplace_back( (*muonItr)->pdgId()  );

      std::string muonAuthor = "none";
      if (( (*muonItr)->author()) == 0){ muonAuthor = "unknown"; } 
      if (( (*muonItr)->author()) == 1){ muonAuthor = "highpt"; } 
      if (( (*muonItr)->author()) == 2){ muonAuthor = "lowpt"; } 
      author.emplace_back(  muonAuthor  );
 
      MCdataType = (*muonItr)->dataType();
      dataType.emplace_back(   MCdataType  );

// check: full simulation input file (1) or fast (0) 
// code from:
// PhysicsAnalysis/AnalysisCommon/AnalysisExamples/src/MiscellaneousExamples.cxx
      if (MCdataType != 3){ // full simulation
          chi2.emplace_back( (*muonItr)->matchChi2OverDoF()  );
          etConeIsol.emplace_back( 
              ((*muonItr)->parameter(MuonParameters::etcone20))/CLHEP::GeV  );

// print some more variables, taken from: 
//   PhysicsAnalysis/EventViewBuilder/EventViewUserData/EVUDMuonAll

     ATH_MSG_DEBUG( 
       " Muon: matchChi2OverDoF: " << (*muonItr)->matchChi2OverDoF() << 
       ", matchChi2: " << (*muonItr)->matchChi2() << 
       ", fitChi2: " << (*muonItr)->fitChi2() << 
       ", isCombined: " << (int)(*muonItr)->isCombinedMuon() );
       
// parameters for associations:
// stricly speaking, these should be ElementLinkVectors (see TauJet association)
//   const ElementLinkVector<Rec::TrackParticleContainer> trackLink = (*muonItr)->trackLinkVector();
// necessary to change ?

          const ElementLink<Rec::TrackParticleContainer> trackLink = (*muonItr)->inDetTrackLink();
          if (trackLink.isValid()) {
             trackKey = trackLink.dataID(); // Storegate key of 
             trackIndex = trackLink.index(); // index into the contianer
	     trackKeyVec.emplace_back( trackKey );
	     trackIndexVec.emplace_back( trackIndex );
          } else {
             trackKeyVec.emplace_back(  "none"  );
             trackIndexVec.emplace_back(  -1  );
          }
          const ElementLink<CaloClusterContainer> clusterLink = (*muonItr)->clusterLink();
          if (clusterLink.isValid()) {
            clusterKey = clusterLink.dataID(); // Storegate key of container 
            clusterIndex = clusterLink.index(); // index number inside the container 
            clusterKeyVec.emplace_back( clusterKey );
	    clusterIndexVec.emplace_back( clusterIndex );
          } else { // no clusterLink
	    clusterKeyVec.emplace_back( "none" );
	    clusterIndexVec.emplace_back( -1 );
          }

      } else {  // fast simulation
          chi2.emplace_back(  0  );
          etConeIsol.emplace_back(  0  );
          trackKeyVec.emplace_back( "none" );
          trackIndexVec.emplace_back( -1 );
          clusterKeyVec.emplace_back( "none");
          clusterIndexVec.emplace_back( -1 );
      }
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

    // special muon parameters
    DataMap["chi2"] = std::move(chi2);
    DataMap["etConeIsol"] = std::move(etConeIsol);
    DataMap["author"] = std::move(author);
    DataMap["pdgId"] = std::move(pdgId);
    DataMap["dataType"] = std::move(dataType);
    // further details and associations
    DataMap["clusterKey"] = std::move(clusterKeyVec);
    DataMap["clusterIndex"] = std::move(clusterIndexVec);
    DataMap["trackKey"] = std::move(trackKeyVec);
    DataMap["trackIndex"] = std::move(trackIndexVec);

    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << nEntries << " entries");

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
