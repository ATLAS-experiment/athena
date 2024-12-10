/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */

#include "xAODJiveXML/xAODJetRetriever.h"

#include "xAODJet/JetContainer.h" 
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODJet/JetAttributes.h"
#include "xAODPFlow/PFO.h"
#include "xAODPFlow/FlowElement.h"

#include "AthenaKernel/Units.h"
using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   *
   * code reference for xAOD:     jpt6Feb14
   *  https://svnweb.cern.ch/trac/atlasgroups/browser/PAT/AODUpgrade/xAODReaderAlgs
   *
   * This is afirst 'skeleton' try for many xAOD retrievers to be done:
   *  xAOD::Jet, xAOD::Vertex, xAOD::Photon, xAOD::CaloCluster, xAOD::Jet
   *  xAOD::TrackParticle, xAOD::TauJet, xAOD::Muon
   *
   **/
  xAODJetRetriever::xAODJetRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent){

      //Only declare the interface
      declareInterface<IDataRetriever>(this);

    }

  StatusCode xAODJetRetriever::initialize() {
    if (m_bTaggerNames.size()!=m_CDIPaths.size()){
      ATH_MSG_WARNING("Number of btaggers and CDI files do not match. Will not retrieve b-tagging information.");
      return StatusCode::SUCCESS;
    } else {
      m_nTaggers = m_bTaggerNames.size();
    }
    for (unsigned int i=0; i<m_nTaggers; i++){
      std::string taggerName = m_bTaggerNames[i];
      asg::AnaToolHandle<IBTaggingSelectionTool> btagSelTool;
      btagSelTool.setTypeAndName("BTaggingSelectionTool/btagSelTool_"+taggerName);
      ATH_CHECK(btagSelTool.setProperty("TaggerName", taggerName));
      ATH_CHECK(btagSelTool.setProperty("JetAuthor", "AntiKt4EMPFlowJets")); // only AntiKt4EMPFlowJets is supported
      ATH_CHECK(btagSelTool.setProperty("OperatingPoint", "FixedCutBEff_70")); // the working point doesn't matter because we don't cut on the tagger discriminant, but it must exist in CDI.
      ATH_CHECK(btagSelTool.setProperty("FlvTagCutDefinitionsFileName", m_CDIPaths[i]));
      ATH_CHECK(btagSelTool.setProperty( "MinPt", 0.0));
      ATH_CHECK(btagSelTool.initialize());
      m_btagSelTools.emplace(taggerName, btagSelTool);
    }

    return StatusCode::SUCCESS;
  }

  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODJetRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG( "in retrieveAll()" );

    SG::ConstIterator<xAOD::JetContainer> iterator, end;
    const xAOD::JetContainer* Jets;

    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKeyFavourite << ")" );
    StatusCode sc = evtStore()->retrieve(Jets, m_sgKeyFavourite);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKeyFavourite << " not found in SG " );
    }else{
      DataMap data = getData(Jets, m_sgKeyFavourite);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKeyFavourite+"_xAOD", &data).isFailure()){
        ATH_MSG_WARNING( "Collection " << m_sgKeyFavourite << " not found in SG " );
      }else{
        ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKeyFavourite << ") Jet retrieved" );
      }
    }

    if ( m_otherKeys.empty() ) {
      //obtain all other collections from StoreGate
      if (( evtStore()->retrieve(iterator, end)).isFailure()){
        ATH_MSG_WARNING( "Unable to retrieve iterator for Jet collection" );
        return StatusCode::SUCCESS;
      }

      for (; iterator!=end; ++iterator) {

        std::string::size_type position = iterator.key().find("HLT",0);
        if ( m_doWriteHLT ){ position = 99; } // override SG key find
        if ( position != 0 ){  // SG key doesn't contain HLTAutoKey
          if (iterator.key()!=m_sgKeyFavourite) {
            ATH_MSG_DEBUG( "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" );
            DataMap data = getData(&(*iterator), iterator.key());
            if ( FormatTool->AddToEvent(dataTypeName(), iterator.key()+"_xAOD", &data).isFailure()){
              ATH_MSG_WARNING( "Collection " << iterator.key() << " not found in SG " );
            }else{
              ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") AODJet retrieved" );
            }
          }
        }
      }
    }else {
      //obtain all collections with the given keys
      for (auto jetkey : m_otherKeys) {
        if (jetkey==m_sgKeyFavourite) { continue; } // skip if already retrieved
        if ( !evtStore()->contains<xAOD::JetContainer>(jetkey)){ continue; } // skip if not in SG
        StatusCode sc = evtStore()->retrieve( Jets, jetkey );
        if (!sc.isFailure()) {
          ATH_MSG_DEBUG( "Trying to retrieve selected " << dataTypeName() << " (" << jetkey << ")" );
          DataMap data = getData(Jets, jetkey);
          if ( FormatTool->AddToEvent(dataTypeName(), jetkey+"_xAOD", &data).isFailure()){
            ATH_MSG_WARNING( "Collection " << jetkey << " not found in SG " );
          }else{
            ATH_MSG_DEBUG( dataTypeName() << " (" << jetkey << ") retrieved" );
          }
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
  const DataMap xAODJetRetriever::getData(const xAOD::JetContainer* jetCont, const std::string& jetkey) {

    ATH_MSG_DEBUG( "in getData()" );

    DataMap DataMap;

    DataVect et; et.reserve(jetCont->size());
    DataVect phi; phi.reserve(jetCont->size());
    DataVect eta; eta.reserve(jetCont->size());
    DataVect mass; mass.reserve(jetCont->size());
    DataVect energy; energy.reserve(jetCont->size());
    DataVect bTagName; bTagName.reserve(jetCont->size());
    DataVect bTagValue; bTagValue.reserve(jetCont->size());
    DataVect charge; energy.reserve(jetCont->size());
    DataVect idVec; idVec.reserve(jetCont->size());
    DataVect px; px.reserve(jetCont->size());
    DataVect py; py.reserve(jetCont->size());
    DataVect pz; pz.reserve(jetCont->size());
    DataVect jvf; jvf.reserve(jetCont->size());
    DataVect jvt; jvt.reserve(jetCont->size());
    DataVect emfrac; emfrac.reserve(jetCont->size());

    DataVect trackKey; trackKey.reserve(jetCont->size());
    DataVect trackContKey; trackContKey.reserve(jetCont->size());
    DataVect trackLinkCount; trackLinkCount.reserve(jetCont->size());
    DataVect clusterID; clusterID.reserve(jetCont->size());

    DataVect cellID; cellID.reserve(jetCont->size());
    DataVect numCells; numCells.reserve(jetCont->size());

    int id = 0;

    int counter = 0;
    for (const auto jet : *jetCont) {
      ATH_MSG_DEBUG( "  Jet #" << counter++ << " : eta = "  << jet->eta() << ", phi = " << jet->phi() << ", pt = " << jet->pt() );

      /* retrieve associated tracks and calo clusters */

      size_t numConstit = jet->numConstituents();
      std::vector<std::string> tempCellID;
      size_t trackcounter = 0;
      if (numConstit > 0) {

        xAOD::Type::ObjectType ctype = jet->rawConstituent(0)->type();

        // PFlow and Flow jets from athena/Reconstruction/Jet/JetMomentTools/Root/JetTrackMomentsTool.cxx
        if (ctype == xAOD::Type::ParticleFlow) {
          // This jet is either a PFlow jet (constituent type: xAOD::FlowElement::PFlow) or UFO jets
          for (size_t i = 0; i < numConstit; i++) {
            const xAOD::PFO *constit =
                dynamic_cast<const xAOD::PFO *>(jet->rawConstituent(i));
            if (constit->isCharged()) {
              const xAOD::TrackParticle *thisTrack = constit->track(0); // by construction xAOD::PFO can only have one track, in eflowRec usage
              trackKey.push_back(DataType(thisTrack->index()));
              trackContKey.push_back(m_tracksName.value());
              trackcounter++;
            } // We have a charged PFO
          } // Loop on jet constituents
        } else if (ctype == xAOD::Type::FlowElement) {
          // This jet is made from xAOD::FlowElement, so we calculate the pflow moments if they're PFOs
          size_t numConstit = jet->numConstituents();
          for (size_t i = 0; i < numConstit; i++) {
            const xAOD::FlowElement *constit = dynamic_cast<const xAOD::FlowElement *>(jet->rawConstituent(i));
            // UFO jet constituents have signalType xAOD::FlowElement::Charged or xAOD::FlowElement::Neutral
            // PFlow jet constituents have signalType xAOD::FlowElement::ChargedPFlow or xAOD::FlowElement::NeutralPFlow
            if (constit != nullptr && ((constit->signalType() & xAOD::FlowElement::PFlow) || constit->signalType() == xAOD::FlowElement::Charged)) {
              if (constit->isCharged()) {
                const xAOD::TrackParticle *thisTrack = dynamic_cast<const xAOD::TrackParticle *>( constit->chargedObject( 0)); // PFO should have only 1 track
                if (thisTrack != nullptr) {
                  trackKey.push_back(DataType(thisTrack->index()));
                  trackContKey.push_back(m_tracksName.value());
                  trackcounter++;
                }
                else
                  ATH_MSG_WARNING( "Charged PFO had no associated TrackParticle");
              } // We have a charged PFO
            }   // The FlowElement is a PFO
          }     // Loop on jet constituents
        } else if (ctype == xAOD::Type::CaloCluster) {
          // get associated cluster
          for (size_t j = 0; j < numConstit; ++j) {
            const xAOD::CaloCluster *cluster = dynamic_cast<const xAOD::CaloCluster *>(jet->rawConstituent(j));
            clusterID.push_back(DataType(cluster->index()));
            for (const auto cc : *(cluster->getCellLinks())) {
              if (std::find(tempCellID.begin(), tempCellID.end(), std::to_string( cc->caloDDE()->identify().get_compact())) != tempCellID.end()) {
                continue;
              } else {
                cellID.push_back( DataType(cc->caloDDE()->identify().get_compact()));
                tempCellID.push_back( std::to_string(cc->caloDDE()->identify().get_compact()));
              }
            }
            ATH_MSG_VERBOSE("  Associated cluster: eta = " << cluster->eta() << ", phi = " << cluster->phi());
          }

          // get ghost associated tracks
          std::vector<const xAOD::TrackParticle *> ghosttracks = jet->getAssociatedObjects<xAOD::TrackParticle>( xAOD::JetAttribute::GhostTrack);
          if (ghosttracks.empty()) {
            ATH_MSG_VERBOSE("  Associated track: ERROR");
          } else {
            for (size_t i = 0; i < ghosttracks.size(); i++) {

              // can access the base track class, should be able to get tracker hits ?
              // const Trk::Track* baseTrack = dynamic_cast< const Trk::Track* >( ghosttracks[i]->track());

              trackKey.push_back(DataType(ghosttracks[i]->index()));
              trackContKey.push_back(m_tracksName.value());

              ATH_MSG_VERBOSE("  Associated track: d0 = " << ghosttracks[i]->d0() << ", pt = " << ghosttracks[i]->pt());
            }
            trackcounter = ghosttracks.size();
          }
        } else if (ctype == xAOD::Type::TrackParticle) {
          for (size_t j = 0; j < numConstit; ++j) {
            const xAOD::TrackParticle *track =
                dynamic_cast<const xAOD::TrackParticle *>(jet->rawConstituent(j));
            if (!track) {
              ATH_MSG_VERBOSE("  Associated track: ERROR");
            } else {
              trackKey.push_back(DataType(track->index()));
              trackContKey.push_back(m_tracksName.value());
              trackcounter++;
              ATH_MSG_VERBOSE("  Associated track: d0 = " << track->d0() << ", pt = " << track->pt());
            }
          }
        }
          }
      trackLinkCount.push_back(DataType(trackcounter));
      numCells.push_back(DataType(tempCellID.size()));


      phi.push_back(DataType(jet->phi()));
      eta.push_back(DataType(jet->eta()));
      et.push_back(DataType(jet->pt()/GeV)); // hack ! no et in xAOD_Jet_v1 currently
      idVec.push_back( DataType( ++id ));

      mass.push_back(DataType(jet->m()/GeV));
      energy.push_back( DataType(jet->e()/GeV ) );

      px.push_back(DataType(jet->px()/GeV));
      py.push_back(DataType(jet->py()/GeV));
      pz.push_back(DataType(jet->pz()/GeV));

      // bjet tagger values
      if (jetkey!="AntiKt4EMPFlowJets" || (m_nTaggers==0)){
        bTagName.push_back(DataType("None"));
        bTagValue.push_back(DataType(0.));
      }else{
        double btagValue;
        for (auto taggerName : m_bTaggerNames) {
          CP::CorrectionCode code = m_btagSelTools[taggerName]->getTaggerWeight(*jet, btagValue);
          if (code != CP::CorrectionCode::Ok) {
            ATH_MSG_DEBUG("Failed to get btagging weight for tagger " << taggerName);
            btagValue = 0;
          }
          bTagName.push_back(DataType(taggerName));
          bTagValue.push_back(DataType(btagValue));
        }
      }

      float chargeread;
      if (!jet->getAttribute<float>(xAOD::JetAttribute::Charge, chargeread)) {
        ATH_MSG_DEBUG("Jet charge unavailable!");
        charge.push_back( DataType( 0. ));
      }else{
        charge.push_back( DataType( chargeread ));
      }

      // updated for data15
      // from: Reconstruction/MET/METReconstruction/Root/METJetFilterTool.cxx
      std::vector<float> jvfread;
      if(!jet->getAttribute<std::vector<float> >(xAOD::JetAttribute::JVF,jvfread)) {
        ATH_MSG_DEBUG("Jet JVF unavailable!");
        jvf.push_back( DataType( 1. ));
      }else{
        jvf.push_back( DataType(  jvfread[0] ));
      }

      float jvtread;
      if(!jet->getAttribute<float>(xAOD::JetAttribute::Jvt,jvtread)) {
        ATH_MSG_DEBUG("Jet JVT unavailable!");
        jvt.push_back(DataType(0.));
      } else {
        jvt.push_back(DataType(jvtread));
      }

      float emfracread = 0;
      if(!jet->getAttribute(xAOD::JetAttribute::EMFrac,emfracread)) {
         ATH_MSG_DEBUG("Jet EMFrac unavailable!");
         emfrac.push_back( DataType( 0. ));
      }else{
         emfrac.push_back( DataType( emfracread ));
      }

    } // end loop

    // four-vectors
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["et"] = et;
    DataMap["energy"] = energy;
    DataMap["mass"] = mass;
    std::string str_nTaggers = m_nTaggers>0 ? std::to_string(m_nTaggers) : "1"; // default to 1 if no btaggers so that atlantis can process the jets properly
    DataMap["bTagName multiple=\""+str_nTaggers+"\""] = bTagName; // assigned by hand !
    DataMap["bTagValue multiple=\""+str_nTaggers+"\""] = bTagValue;
    DataMap["charge"] = charge;
    DataMap["id"] = idVec;
    DataMap["px"] = px;
    DataMap["py"] = py;
    DataMap["pz"] = pz;
    DataMap["jvf"] = jvf;
    DataMap["jvt"] = jvt;
    DataMap["emfrac"] = emfrac;

    if ((trackKey.size()) != 0){
      double NTracksPerVertex = trackKey.size()*1./jetCont->size();
      std::string tag = "trackIndex multiple=\"" +DataType(NTracksPerVertex).toString()+"\"";
      DataMap[tag] = trackKey;
      tag = "trackKey multiple=\"" +DataType(NTracksPerVertex).toString()+"\"";
      DataMap[tag] = trackContKey;
    }

    if ((clusterID.size())!=0){
      std::string tag = "clusterIndex multiple=\"" + DataType(clusterID.size()).toString()+"\"";
      double NCellsPerJet = cellID.size()*1./jetCont->size();
      tag = "cells multiple=\"" +DataType(NCellsPerJet).toString()+"\"";
      DataMap[tag]=cellID;
    }

    DataMap["trackLinkCount"] = trackLinkCount;
    DataMap["numCells"] = numCells;

    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << phi.size() << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------

} // JiveXML namespace
