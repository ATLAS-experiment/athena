/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AnalysisJiveXML/ElectronRetriever.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "egammaEvent/ElectronContainer.h"

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
  ElectronRetriever::ElectronRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_typeName("Electron"){

    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareProperty("StoreGateKey", m_sgKey= "ElectronAODCollection", 
        "Collection to be first in output, shown in Atlantis without switching");
  }
   
  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode ElectronRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieveAll()" );
    
    SG::ConstIterator<ElectronContainer> iterator, end;
    const ElectronContainer* electrons;
    
    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKey << ")" );
    StatusCode sc = evtStore()->retrieve(electrons, m_sgKey);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " ); 
    }else{
      DataMap data = getData(electrons);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKey, &data).isFailure()){
	ATH_MSG_WARNING( "Collection " << m_sgKey << " not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKey << ") Electron retrieved" );
      }
    }

    //obtain all other collections from StoreGate
    if (( evtStore()->retrieve(iterator, end)).isFailure()){
       ATH_MSG_WARNING( "Unable to retrieve iterator for Jet collection" );
    }
      
    for (; iterator!=end; ++iterator) {
       if (iterator.key()!=m_sgKey) {
          if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" << endmsg;
            DataMap data = getData(&(*iterator));
            if ( FormatTool->AddToEvent(dataTypeName(), iterator.key(), &data).isFailure()){
	       ATH_MSG_WARNING("Collection " << iterator.key() << " not found in SG ");
	    }else{
	      ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") Electron retrieved" );
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
  const DataMap ElectronRetriever::getData(const ElectronContainer* elCont) {
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "in getData()" << endmsg;

    DataMap DataMap;
    const auto nElectrons = elCont->size();
    DataVect pt; pt.reserve(nElectrons);
    DataVect phi; phi.reserve(nElectrons);
    DataVect eta; eta.reserve(nElectrons);
    DataVect mass; mass.reserve(nElectrons);
    DataVect energy; energy.reserve(nElectrons);
    DataVect px; px.reserve(nElectrons);
    DataVect py; py.reserve(nElectrons);
    DataVect pz; pz.reserve(nElectrons);

    DataVect eOverp; eOverp.reserve(nElectrons);
    DataVect isEM; isEM.reserve(nElectrons);
    DataVect isEMString; isEMString.reserve(nElectrons);
    DataVect hasTrack; hasTrack.reserve(nElectrons);
    DataVect author; author.reserve(nElectrons);
    DataVect label; label.reserve(nElectrons);
    
    DataVect pdgId; pdgId.reserve(nElectrons);
    DataVect dataType; dataType.reserve(nElectrons);
    DataVect hitsBLayer; hitsBLayer.reserve(nElectrons);
    DataVect hitsPixel; hitsPixel.reserve(nElectrons);
    DataVect hitsSCT; hitsSCT.reserve(nElectrons);
    DataVect hitsTRT; hitsTRT.reserve(nElectrons);

    // for associations:
    DataVect clusterKeyVec; clusterKeyVec.reserve(nElectrons);
    DataVect clusterIndexVec; clusterIndexVec.reserve(nElectrons);
    DataVect trackKeyVec; trackKeyVec.reserve(nElectrons);
    DataVect trackIndexVec; trackIndexVec.reserve(nElectrons);

    ElectronContainer::const_iterator elItr  = elCont->begin();
    ElectronContainer::const_iterator elItrE = elCont->end();

    int MCdataType = 1;

// isEM from
//   https://uimon.cern.ch/twiki/bin/view/Atlas/RemoveOverlap

    double ep = 0., p=0., e=0.; // for eOverP calculation from track
    //int counter = 0;
    std::string electronAuthor = "";
    std::string electronIsEMString = "none";
    std::string electronLabel = "";
        
    for (; elItr != elItrE; ++elItr) {
      electronIsEMString = "none";
      phi.emplace_back((*elItr)->phi());
      eta.emplace_back((*elItr)->eta());
      pt.emplace_back((*elItr)->pt()/CLHEP::GeV);

      mass.emplace_back((*elItr)->m()/CLHEP::GeV);
      energy.emplace_back( (*elItr)->e()/CLHEP::GeV  );
      px.emplace_back( (*elItr)->px()/CLHEP::GeV  );
      py.emplace_back( (*elItr)->py()/CLHEP::GeV  );
      pz.emplace_back( (*elItr)->pz()/CLHEP::GeV  );

      pdgId.emplace_back( (*elItr)->pdgId()  );

      electronAuthor = "author"+DataType( (*elItr)->author() ).toString(); // for odd ones eg FWD
      electronLabel = electronAuthor;
      if (( (*elItr)->author()) == 0){ electronAuthor = "unknown"; electronLabel += "_unknown"; }
      if (( (*elItr)->author()) == 8){ electronAuthor = "forward"; electronLabel += "_forward"; }
      if (( (*elItr)->author()) == 2){ electronAuthor = "softe"; electronLabel += "_softe"; }
      if (( (*elItr)->author()) == 1){ electronAuthor = "egamma"; electronLabel += "_egamma"; }

      // Tight but without the use of isolation criteria:   
      // code from https://twiki.cern.ch/twiki/bin/view/Atlas/ElectronReconstruction#isEM_flag
      // New PP Ids: PhysicsAnalysis/D3PDMaker/TrigEgammaD3PDMaker/python/EFElectronD3PDObject.py

      if ( (*elItr)->isem(egammaPIDObs::ElectronLoose)==0){ 
            electronLabel += "_Loose"; 
            electronIsEMString = "Loose"; // assume that hierarchy is obeyed !
      } 
      if ( (*elItr)->isem(egammaPIDObs::ElectronMedium)==0){ 
            electronLabel += "_Medium"; 
            electronIsEMString = "Medium"; // assume that hierarchy is obeyed !
      }   
      if ( (*elItr)->isem(egammaPIDObs::ElectronTight)==0){ 
            electronLabel += "_Tight"; 
            electronIsEMString = "Tight"; // assume that hierarchy is obeyed !
      }     
      if ( (*elItr)->isem(egammaPIDObs::ElectronTightPP)==0){ 
            electronLabel += "_TightPP"; 
            electronIsEMString = "TightPP"; // last entry overwrites all others ! 
      }     
      if ( (*elItr)->isem(egammaPIDObs::ElectronMediumPP)==0){ 
            electronLabel += "_MediumPP"; 
//            electronIsEMString = "MediumNoIso"; // would need AtlantisJava changes
      }     

      if ( (*elItr)->isem(egammaPIDObs::ElectronMediumNoIso)==0){ 
            electronLabel += "_MediumNoIso"; 
//            electronIsEMString = "MediumNoIso"; // would need AtlantisJava changes
      }     
      if ( (*elItr)->isem(egammaPIDObs::ElectronTightTRTNoIso)==0){ 
            electronLabel += "_TightTRTNoIso"; 
//            electronIsEMString = "TightTRTNoIso"; // would need AtlantisJava changes
      }     
      // Tight but without the use of isolation criteria:   
      if ( (*elItr)->isem(egammaPIDObs::ElectronTightNoIsolation)==0){ 
            electronLabel += "_TightNoIsolation"; 
//            electronIsEMString = "_TightNoIsolation"; // would need AtlantisJava changes
      }     
      MCdataType = (*elItr)->dataType();
      dataType.emplace_back(   MCdataType  );

// check: full simulation input file (1) or fast (0) 
// code from:
// PhysicsAnalysis/AnalysisCommon/AnalysisExamples/src/MiscellaneousExamples.cxx

      if (MCdataType != 3){ // full simulation

          isEM.emplace_back( (**elItr).isem()  );

	  const Trk::TrackSummary *summary;
          bool elecTrack = (*elItr)->trackParticle();
            if ( elecTrack ){

// eOperp calculation from: Reconstruction/egamma/egammaRec/egammaAODBuilder.cxx, 
// advised by Frederic Derue 16Mar09
// Old version with eOverP from EMTrackMatch is unreliable.

	      ep = 0.;
              p = (*elItr)->trackParticle()->p();
              e = (*elItr)->e();
              ep = p>0. ? e/p : 0.;
              eOverp.emplace_back(  ep );

              summary = (*elItr)->trackParticle()->trackSummary();
              hasTrack.emplace_back(  1  );
	      hitsBLayer.emplace_back(  summary->get(Trk::numberOfInnermostPixelLayerHits) );
              hitsPixel.emplace_back(   summary->get(Trk::numberOfPixelHits) );
              hitsSCT.emplace_back(  summary->get(Trk::numberOfSCTHits) );
              hitsTRT.emplace_back(  summary->get(Trk::numberOfTRTHits) );
            } else {
  	      eOverp.emplace_back( DataType( "0." ));
              hasTrack.emplace_back(  0  );
	      hitsBLayer.emplace_back(  -1 );
              hitsPixel.emplace_back(  -1 );
              hitsSCT.emplace_back(  -1 );
              hitsTRT.emplace_back(  -1 );
            }

// code from:
// PhysicsAnalysis/AnalysisCommon/AnalysisExamples/src/MiscellaneousExamples.cxx

// parameters for associations: Storegate key and Index

       const ElementLink<CaloClusterContainer> clusterLink = (*elItr)->clusterElementLink();
       if (clusterLink.isValid()) {
         std::string clusterKey = clusterLink.dataID(); // Storegate key of 
         int clusterIndex = clusterLink.index(); // index into the contianer


	  clusterKeyVec.emplace_back( clusterKey );
	  clusterIndexVec.emplace_back( clusterIndex );
       } else { // no clusterLink
	  clusterKeyVec.emplace_back( "none" );
	  clusterIndexVec.emplace_back( -1 );
       }
       
       const ElementLink<Rec::TrackParticleContainer> trackLink = (*elItr)->trackParticleElementLink();
       if (trackLink.isValid()) {
         std::string trackKey = trackLink.dataID(); // Storegate key of 
         int trackIndex = trackLink.index(); // index into the contianer


	  trackKeyVec.emplace_back( trackKey );
	  trackIndexVec.emplace_back( trackIndex);
        } else { // no trackLink
	  trackKeyVec.emplace_back( "none" );
	  trackIndexVec.emplace_back( -1 );
        }

       //counter++;

// end of associations data    

     } else {  // fast simulation: placeholders
          hasTrack.emplace_back(  1  );
          eOverp.emplace_back(  1.  );
          isEM.emplace_back(  0  );
	  electronIsEMString += "fastSim";
	  electronLabel += "fastSim" ;
          hitsBLayer.emplace_back(  0  );
          hitsPixel.emplace_back(  0  );
          hitsSCT.emplace_back(  0  );
          hitsTRT.emplace_back(  0  );

	  clusterKeyVec.emplace_back( "none" );
	  clusterIndexVec.emplace_back( -1 );
	  trackKeyVec.emplace_back( "none" );
	  trackIndexVec.emplace_back( -1 );
      } // end datatype case

      author.emplace_back(  electronAuthor  );
      label.emplace_back(  electronLabel  );
      isEMString.emplace_back(  electronIsEMString  );
    } // end ElectronIterator 

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

    // special Electron parameters
    DataMap["eOverp"] = std::move(eOverp);
    DataMap["isEM"] = std::move(isEM);
    DataMap["isEMString"] = std::move(isEMString);
    DataMap["label"] = std::move(label);
    DataMap["hasTrack"] = std::move(hasTrack);
    DataMap["author"] = std::move(author);
    DataMap["pdgId"] = std::move(pdgId);
    DataMap["dataType"] = std::move(dataType);
    DataMap["hitsBLayer"] = std::move(hitsBLayer);
    DataMap["hitsPixel"] = std::move(hitsPixel);
    DataMap["hitsSCT"] = std::move(hitsSCT);
    DataMap["hitsTRT"] = std::move(hitsTRT);

    // associations
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
