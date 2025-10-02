/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BTagging/JetTagVertexDecoratorAlg.h"
#include <cmath>

#include "VxJetVertex/RecVertexPositions.h"

//general interface for secondary vertex finders
#include "VxSecVertex/VxSecVKalVertexInfo.h"
#include "VxSecVertex/VxJetFitterVertexInfo.h"
#include "VxJetVertex/TwoTrackVerticesInJet.h"

#include "VxJetVertex/VxJetCandidate.h"
#include "VxJetVertex/VxVertexOnJetAxis.h"
#include "VxJetVertex/VxClusteringTable.h"
#include "VxJetVertex/PairOfVxVertexOnJetAxis.h"

#include "VxVertex/VxTrackAtVertex.h"

#include "TrkLinks/LinkToXAODTrackParticle.h"

#include "xAODBTagging/BTagVertexAuxContainer.h"
#include "xAODTracking/VertexAuxContainer.h"

#include "xAODBase/IParticle.h"


namespace Analysis {

  /// @brief The JetTagVertexDecoratorAlg algorithm decorates jets with additional vertex-based features.
  /// 
  /// This algorithm retrieves the jet and vertex collections from the event store
  /// and loops over the available secondary vertex information produced by different vertex
  /// finders (e.g. SV1, SV1Flip, JetFitter, JetFitterFlip). For each jet, the algorithm computes
  /// relevant quantities (e.g. vertex mass, energy fraction, distances) and writes them as decorations
  /// on the jet container.

  JetTagVertexDecoratorAlg::JetTagVertexDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator): 
    AthReentrantAlgorithm(name, pSvcLocator), 
    m_JFvarFactory("Analysis::JetFitterVariablesFactory",this){
    declareProperty("SecVtxFinderxAODBaseNameList", m_secVertexFinderBaseNameList);
    declareProperty("JetFitterVariableFactory", m_JFvarFactory);
    
  }


  StatusCode JetTagVertexDecoratorAlg::initialize()
  {
    ATH_CHECK( m_svTag.retrieve() );
    ATH_CHECK( m_JetCollectionName.initialize() );
    ATH_CHECK( m_VertexCollectionName.initialize() );
    ATH_CHECK( m_jetSVLinkName.initialize() );
    ATH_CHECK( m_jetSVFlipLinkName.initialize(!m_jetSVFlipLinkName.empty()) );
    ATH_CHECK( m_jetJFVtxLinkName.initialize(!m_jetJFVtxLinkName.empty()) );
    ATH_CHECK( m_jetJFFlipVtxLinkName.initialize(!m_jetJFFlipVtxLinkName.empty()));
    ATH_CHECK( m_VxSecVertexInfoNames.initialize() );
      
    // For each base name (e.g. "SV1", "SV1Flip", "JetFitter", "JetFitterFlip", etc.)
    
    m_decorKeys.clear();
    for (size_t i = 0; i < m_secVertexFinderBaseNameList.size(); i++) {
      std::string basename  = m_secVertexFinderBaseNameList[i];

      std::string prefix = m_JetCollectionName.key() + "." + basename;
      if (basename == "SV1" || basename == "SV1Flip") {
        m_decorKeys.emplace(basename, std::make_unique<SV1DecorHandles>(this, prefix));
      }
      else if (basename == "JetFitter" || basename == "JetFitterFlip") {
        m_decorKeys.emplace(basename, std::make_unique<JetFitterDecorHandles>(this, prefix));
      }
      else {
        ATH_MSG_WARNING("Unknown secondary vertex finder basename: " << basename);
        continue;
      }
    
      ATH_MSG_DEBUG("Constructing TaggerDecorHandles with prefix: " << prefix);
      if (!m_decorKeys.at(basename)) {
        ATH_MSG_ERROR("Construction of TaggerDecorHandles for " << basename << " failed: unique_ptr is null!");
        return StatusCode::FAILURE;
      }

      if (basename == "SV1" || basename == "SV1Flip") {
        auto sv1Handles = dynamic_cast<SV1DecorHandles*>(m_decorKeys.at(basename).get());
        ATH_CHECK(sv1Handles->massKey.initialize());
        ATH_CHECK(sv1Handles->efracKey.initialize());
        ATH_CHECK(sv1Handles->energyTrkInJetKey.initialize());
        ATH_CHECK(sv1Handles->dstToMatLayKey.initialize());
        ATH_CHECK(sv1Handles->n2trkKey.initialize());
        ATH_CHECK(sv1Handles->NGTinSvxKey.initialize());
        ATH_CHECK(sv1Handles->L3dKey.initialize());
        ATH_CHECK(sv1Handles->LxyKey.initialize());
        ATH_CHECK(sv1Handles->deltaRKey.initialize());
        ATH_CHECK(sv1Handles->isDefaultsKey.initialize());
        ATH_CHECK(sv1Handles->normdistKey.initialize());
        ATH_CHECK(sv1Handles->significance3dKey.initialize());
        ATH_CHECK(sv1Handles->correctSignificance3dKey.initialize());
        ATH_CHECK(sv1Handles->trackLinksKey.initialize());
        ATH_CHECK(sv1Handles->badTracksIPKey.initialize());
        ATH_CHECK(sv1Handles->verticesKey.initialize());
      }
      else if (basename == "JetFitter" || basename == "JetFitterFlip") {
        auto jfHandles = dynamic_cast<JetFitterDecorHandles*>(m_decorKeys.at(basename).get());
        ATH_CHECK(jfHandles->jfnVTXKey.initialize());
        ATH_CHECK(jfHandles->jfnTracksAtVtxKey.initialize());
        ATH_CHECK(jfHandles->jfnSingleTracksKey.initialize());
        ATH_CHECK(jfHandles->jfenergyFractionKey.initialize());
        ATH_CHECK(jfHandles->jfmassKey.initialize());
        ATH_CHECK(jfHandles->jfmassUncorrKey.initialize());
        ATH_CHECK(jfHandles->jfsignificance3dKey.initialize());
        ATH_CHECK(jfHandles->jfdeltaetaKey.initialize());
        ATH_CHECK(jfHandles->jfdeltaphiKey.initialize());
        ATH_CHECK(jfHandles->jfdeltaRKey.initialize());
        ATH_CHECK(jfHandles->jfchi2Key.initialize());
        ATH_CHECK(jfHandles->jfndofKey.initialize());
        ATH_CHECK(jfHandles->isDefaultsKey.initialize());
        ATH_CHECK(jfHandles->jfdRFlightDirKey.initialize());
        ATH_CHECK(jfHandles->jfN2TpairKey.initialize());
        ATH_CHECK(jfHandles->jfVerticesKey.initialize());
        ATH_CHECK(jfHandles->jfFittedPositionKey.initialize());
        ATH_CHECK(jfHandles->jfFittedCovKey.initialize());
        ATH_CHECK(jfHandles->jftracksAtPVchi2Key.initialize());
        ATH_CHECK(jfHandles->jftracksAtPVndfKey.initialize());
        ATH_CHECK(jfHandles->jftracksAtPVlinksKey.initialize());

      }
    }
    return StatusCode::SUCCESS;
  }


  StatusCode JetTagVertexDecoratorAlg::execute(const EventContext& ctx) const{

    // Retrieve the jet container from the event store.
    SG::ReadHandle<xAOD::JetContainer> h_JetCollectionName (m_JetCollectionName, ctx);
    if (!h_JetCollectionName.isValid()) {
      ATH_MSG_ERROR(" cannot retrieve jet container with key: " << h_JetCollectionName.key());
      return StatusCode::FAILURE;
    }

    const xAOD::JetContainer* jetContainer = h_JetCollectionName.cptr();

    // Retrieve the primary vertex container and select the primary vertex.
    const xAOD::Vertex* primaryVertex(nullptr);
    SG::ReadHandle<xAOD::VertexContainer> h_VertexCollectionName (m_VertexCollectionName, ctx);
    if (!h_VertexCollectionName.isValid()) {
      ATH_MSG_ERROR( " cannot retrieve primary vertex container with key " << m_VertexCollectionName.key()  );
      return StatusCode::FAILURE;
    }

    unsigned int nVertexes = h_VertexCollectionName->size();

    if (nVertexes == 0) {
      ATH_MSG_DEBUG("#JetTagVertexDecoratorAlg#  Vertex container is empty");
      return StatusCode::SUCCESS;
    }
    for (const auto *fz : *h_VertexCollectionName) {
      if (fz->vertexType() == xAOD::VxType::PriVtx) {
      primaryVertex = fz;
      break;
      }
    }

    if (! primaryVertex) {
      ATH_MSG_DEBUG("#JetTagVertexDecoratorAlg#  No vertex labeled as VxType::PriVtx!");
      xAOD::VertexContainer::const_iterator fz = h_VertexCollectionName->begin();
      primaryVertex = *fz;
      if (primaryVertex->nTrackParticles() == 0) {
        ATH_MSG_DEBUG("#JetTagVertexDecoratorAlg#  PV==BeamSpot: probably poor tagging");
      }
    }
    


    // Loop over the VxSecVertexInfo containers (each corresponding to a different secondary vertex finder).
    int nameiter = 0;
    for(const SG::ReadHandleKey<Trk::VxSecVertexInfoContainer>& infoCont : m_VxSecVertexInfoNames) {

      SG::ReadHandle<Trk::VxSecVertexInfoContainer> h_VxSecVertexInfoName(infoCont, ctx);
      if (!h_VxSecVertexInfoName.isValid()) {
        ATH_MSG_WARNING("Could not retrieve secondary vertex container " << h_VxSecVertexInfoName.key());
        return StatusCode::FAILURE;
      }

      if (h_VxSecVertexInfoName->size() != jetContainer->size()) {
        ATH_MSG_ERROR("Size mismatch: Jet container has "
                      << jetContainer->size() << " jets, but VxSecVertexInfo has "
                      << h_VxSecVertexInfoName->size());
        return StatusCode::FAILURE;
      }

      // Setup iterators in parallel: jets, BTagging objects, and VxSec infos
      auto infoSVIter = h_VxSecVertexInfoName->begin();

      std::string basename  = m_secVertexFinderBaseNameList[nameiter];

      if (basename == "SV1" || basename == "SV1Flip") {
        // Configure decoration handles for the SV1 (or SV1Flip) vertex finder.
        auto sv1Handles = dynamic_cast<SV1DecorHandles*>(m_decorKeys.at(basename).get());
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1Mass(sv1Handles->massKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1efracs(sv1Handles->efracKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1energyTrkInJet(sv1Handles->energyTrkInJetKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1dstToMatLay(sv1Handles->dstToMatLayKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, int>   writeSv1n2trk(sv1Handles->n2trkKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, int>   writeSv1NGTinSvx(sv1Handles->NGTinSvxKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1L3d(sv1Handles->L3dKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1Lxy(sv1Handles->LxyKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1deltaR(sv1Handles->deltaRKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, char>  writeSv1isDefaults(sv1Handles->isDefaultsKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1normdist(sv1Handles->normdistKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1significance3d(sv1Handles->significance3dKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeSv1correctsignificance3d(sv1Handles->correctSignificance3dKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>>> writeSv1TrackLinks(sv1Handles->trackLinksKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::VertexContainer>>> writeSv1VertexLinks(sv1Handles->verticesKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>>> writeSv1BadTracksIP(sv1Handles->badTracksIPKey, ctx);

        for (auto jetIter = jetContainer->begin(); jetIter != jetContainer->end(); ++jetIter, ++infoSVIter){

          const xAOD::Jet& jetToTag = **jetIter;
          const Trk::VxSecVertexInfo* myVertexInfo = *infoSVIter;

          Amg::Vector3D jetDir(jetToTag.p4().Px(),jetToTag.p4().Py(),jetToTag.p4().Pz());

          const auto& key = basename.find("Flip")!=std::string::npos ? m_jetSVFlipLinkName : m_jetSVLinkName; 
          SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink< xAOD::VertexContainer> > > h_jetSVLinkName (key, ctx);
          
          // Variable declarations with default values.
          float distnrm = NAN, drJPVSV = NAN, Lxy = NAN, L3d = NAN, sv1mass = NAN, distnrmCorr = NAN, energyfrc = NAN, energyTrk = NAN, dsttomatlayer = NAN;
          int n2trk = -1, npsec = 0;
          char isDefaults = 1;

          std::vector<ElementLink<xAOD::TrackParticleContainer> > TrkList;
          std::vector<const xAOD::Vertex*> vecVertices;
          const Trk::VxSecVKalVertexInfo* myVertexInfoVKal = dynamic_cast<const Trk::VxSecVKalVertexInfo*>(myVertexInfo);

          // If the vertex info is from the VKal based finder, loop over vertices.
          if (myVertexInfoVKal) {    
            std::vector<xAOD::Vertex*>::const_iterator verticesBegin = myVertexInfo->vertices().begin(); 
            std::vector<xAOD::Vertex*>::const_iterator verticesEnd   = myVertexInfo->vertices().end(); 
            for (std::vector<xAOD::Vertex*>::const_iterator verticesIter = verticesBegin; verticesIter!=verticesEnd;++verticesIter) { 
              vecVertices.push_back(*verticesIter);
              std::vector<ElementLink<xAOD::TrackParticleContainer> > theseTracks = (*verticesIter)->trackParticleLinks();
              npsec += theseTracks.size();
              for (auto & theseTrack : theseTracks){
                TrkList.push_back(theseTrack);
              }
            }

          }
      
          // If at least one vertex is found, compute vertex-based kinematics.
          if (!vecVertices.empty()) {
            isDefaults = 0;
            const xAOD::Vertex* myVert  = vecVertices[0];

            // from here: https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/JetTagging/JetTagTools/src/SVTag.cxx#L241
            const Amg::Vector3D PVposition = (*primaryVertex).position();
            const Amg::Vector3D position = myVert->position();
    
            Amg::Vector3D PvSvDir(  position.x() - PVposition.x(),
                                    position.y() - PVposition.y(),
                                    position.z() - PVposition.z() );

            double drJPVSV_1 = Amg::deltaR(jetDir,PvSvDir);
            double drJPVSV_2 = Amg::deltaR(-jetDir, PvSvDir); 
            drJPVSV = drJPVSV_1; 
            if ( basename == "SV1Flip" ) 
              drJPVSV = drJPVSV_2; 

            Lxy = std::hypot(PvSvDir(0,0), PvSvDir(1,0));
            L3d = std::hypot(PvSvDir(0,0), PvSvDir(1,0), PvSvDir(2,0));
            distnrm = m_svTag->get3DSignificance(*primaryVertex, vecVertices, jetDir);
            distnrmCorr = m_svTag->get3DSignificanceCorr(*primaryVertex, vecVertices, jetDir);
            sv1mass = myVertexInfoVKal->mass();
            energyfrc = myVertexInfoVKal->energyFraction();
            energyTrk =  myVertexInfoVKal->energyTrkInJet();
            dsttomatlayer = myVertexInfoVKal->dstToMatLay();
            n2trk = myVertexInfoVKal->n2trackvertices();

            ATH_MSG_VERBOSE("#JetTagVertexDecoratorAlg# SVX x = " << myVert->position().x() << " y = " << myVert->position().y() << " z = " << myVert->position().z() << " mass = " << sv1mass);

          } else {
            npsec = -1;
            ATH_MSG_VERBOSE("#JetTagVertexDecoratorAlg# No vertex. Cannot calculate normalized distance.");
          }
    


          // Prepare additional decoration collections for vertex and track links.
          std::vector<ElementLink< xAOD::VertexContainer>> SVertexLinks;
          std::vector<ElementLink<xAOD::TrackParticleContainer>> badtrackEL;

          
          
          if (myVertexInfoVKal){
            if (!h_jetSVLinkName.isAvailable()) {
              ATH_MSG_ERROR( "#JetTagVertexDecoratorAlg#: cannot retrieve vertex container EL decoration with key " << m_jetSVLinkName.key()  );
              return StatusCode::FAILURE;
            }
            SVertexLinks = h_jetSVLinkName(jetToTag);
          
            std::vector<const xAOD::IParticle*> btip =  myVertexInfoVKal->badTracksIP();
            std::vector<const xAOD::IParticle*>::iterator ipBegin = btip.begin();
            std::vector<const xAOD::IParticle*>::iterator ipEnd   = btip.end();
  
            for (std::vector<const xAOD::IParticle*>::iterator ipIter=ipBegin; ipIter!=ipEnd; ++ipIter) {
              const xAOD::TrackParticle* tp = dynamic_cast<const xAOD::TrackParticle*>(*ipIter);
              if (!tp) {
                ATH_MSG_WARNING("#JetTagVertexDecoratorAlg# bad track IParticle is not a TrackParticle");
                continue;
              }
              ElementLink<xAOD::TrackParticleContainer> tpel;
              badtrackEL.push_back(tpel);
            }
  

          }
          else{
            ATH_MSG_DEBUG("#JetTagVertexDecoratorAlg#: Can't cast to VxSecVKalVertexInfo for " << basename);
          }
      
          // Finally, decorate the jet with the computed SV1 features.
          writeSv1Mass(jetToTag) = sv1mass;
          writeSv1efracs(jetToTag) = energyfrc; 
          writeSv1energyTrkInJet(jetToTag) = energyTrk; 
          writeSv1dstToMatLay(jetToTag) = dsttomatlayer; 
          writeSv1n2trk(jetToTag) = n2trk;
          writeSv1NGTinSvx(jetToTag) = npsec;
          writeSv1L3d(jetToTag) = L3d;
          writeSv1Lxy(jetToTag) = Lxy;
          writeSv1deltaR(jetToTag) = drJPVSV;
          writeSv1significance3d(jetToTag) = distnrm;
          writeSv1correctsignificance3d(jetToTag) = distnrmCorr;
          writeSv1normdist(jetToTag) = distnrmCorr;
          writeSv1isDefaults(jetToTag) = isDefaults;
          writeSv1TrackLinks(jetToTag) = TrkList; 
          writeSv1VertexLinks(jetToTag) = SVertexLinks;
          writeSv1BadTracksIP(jetToTag) = badtrackEL;
          ATH_MSG_DEBUG("Decorated jet with pt : " << jetToTag.pt() << ", eta : " << jetToTag.eta() 
                        << " with features for alg : " << basename << ", mass: " << sv1mass 
                        << ", energy fraction: " << energyfrc);

        }
      }

      else if (basename == "JetFitter" || basename == "JetFitterFlip") {

        // Configure decoration handles for the JetFitter (or JetFitterFlip) vertex finder.
        auto jfHandles = dynamic_cast<JetFitterDecorHandles*>(m_decorKeys.at(basename).get());
        SG::WriteDecorHandle<xAOD::JetContainer, int> writeJfnVTX( jfHandles->jfnVTXKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, int> writeJfnTracksAtVtx(jfHandles->jfnTracksAtVtxKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, int> writeJfnSingleTracks(jfHandles->jfnSingleTracksKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfEnergyFraction(jfHandles->jfenergyFractionKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfMass(jfHandles->jfmassKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfMassUncorr(jfHandles->jfmassUncorrKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfSignificance3d(jfHandles->jfsignificance3dKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfDeltaEta(jfHandles->jfdeltaetaKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfDeltaPhi(jfHandles->jfdeltaphiKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfDeltaR(jfHandles->jfdeltaRKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfChi2(jfHandles->jfchi2Key, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, int> writeJfNDOF(jfHandles->jfndofKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, char> writeJfIsDefaults(jfHandles->isDefaultsKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, float> writeJfdRFlightDir(jfHandles->jfdRFlightDirKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, int> writeJfN2TrkPair(jfHandles->jfN2TpairKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::BTagVertexContainer>>> writeJfVertices(jfHandles->jfVerticesKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> writeJfFittedPosition(jfHandles->jfFittedPositionKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> writeJfFittedCov(jfHandles->jfFittedCovKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> writeJfTracksAtPVChi2(jfHandles->jftracksAtPVchi2Key, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<float>> writeJfTracksAtPVndf(jfHandles->jftracksAtPVndfKey, ctx);
        SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>>> writeJfTracksAtPVLinks(jfHandles->jftracksAtPVlinksKey, ctx);

        for (auto jetIter = jetContainer->begin(); jetIter != jetContainer->end(); ++jetIter, ++infoSVIter) {

          const xAOD::Jet& jetToTag = **jetIter;
          const Trk::VxSecVertexInfo* myVertexInfo = *infoSVIter;
          const Trk::VxJetFitterVertexInfo* myVertexInfoJetFitter = dynamic_cast<const Trk::VxJetFitterVertexInfo*>(myVertexInfo);

          // Retrieve the associated vertex links for JetFitter.
          const auto& key = basename.find("Flip")!=std::string::npos ? m_jetJFFlipVtxLinkName : m_jetJFVtxLinkName;
          SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink< xAOD::BTagVertexContainer> > > h_jetJFVtxLinkName (key, ctx);
          std::vector< ElementLink< xAOD::BTagVertexContainer > > JFVerticesLinks;      
          std::vector< const xAOD::Vertex*> vecTwoTrkVtx;
          std::vector<Trk::VxJetCandidate*> JFvertices;
          char isDefaults = 1;

          if (myVertexInfoJetFitter) {
            const Trk::TwoTrackVerticesInJet* TwoTrkVtxInJet = myVertexInfoJetFitter->getTwoTrackVerticesInJet();
            vecTwoTrkVtx = TwoTrkVtxInJet->getTwoTrackVertice();
            
            JFvertices = myVertexInfoJetFitter->verticesJF();

            if (!h_jetJFVtxLinkName.isAvailable()) {
              ATH_MSG_ERROR( " cannot retrieve vertex container EL decoration with key " << key.key()  );
              return StatusCode::FAILURE;
            }
            JFVerticesLinks = h_jetJFVtxLinkName ( jetToTag );
          }
          int N2TrkVtx = vecTwoTrkVtx.size();
          float deltaR = NAN;

          Trk::VxJetCandidate* vxjetcand = nullptr;
          std::vector<Trk::VxVertexOnJetAxis*> Vtxonjetaxes;

          if (!JFvertices.empty()) {
            vxjetcand = dynamic_cast< Trk::VxJetCandidate*>(JFvertices[0]);
            if (!vxjetcand) {
              ATH_MSG_WARNING("#JetTagVertexDecoratorAlg# bad VxCandidate is not a VxJetCandidate");
              return StatusCode::SUCCESS;
            }
            // Obtain the vertices on the jet axis.
            Vtxonjetaxes = vxjetcand->getVerticesOnJetAxis();
          }
      

          int nVtx = 0; 
          nVtx = Vtxonjetaxes.size();

          // Retrieve fitted position and covariance information.
          Amg::VectorX vtxPositions = Amg::VectorX::Zero(5);
          Amg::MatrixX vtxCovMatrix = Amg::MatrixX::Zero(5,5);
          if (nVtx > 0){
            const Trk::RecVertexPositions& recVtxposition = vxjetcand->getRecVertexPositions();
            vtxPositions = recVtxposition.position();
            vtxCovMatrix = recVtxposition.covariancePosition();
            ATH_MSG_DEBUG("#BTAGJF# size vtxPosition "<<vtxPositions.size());
          }
          std::vector<float> fittedPosition = std::vector<float>(nVtx+5,-1);
          std::vector<float> fittedCov = std::vector<float>(nVtx+5,-1); //only store the diagonal terms
          if (fittedPosition.size() < 5) std::abort(); // suppress cppcheck warnings
          if(vtxPositions.rows()>4 ) {
            fittedPosition[0] = vtxPositions[Trk::jet_xv]; //position x,y,z of PV
            fittedPosition[1] = vtxPositions[Trk::jet_yv];
            fittedPosition[2] = vtxPositions[Trk::jet_zv];
            fittedPosition[3] = nVtx > 0 ? vtxPositions[Trk::jet_phi] : NAN;  // direction of the jet axis
            fittedPosition[4] = nVtx > 0 ? vtxPositions[Trk::jet_theta] : NAN;

            fittedCov[0] = vtxCovMatrix(0,0);
            fittedCov[1] = vtxCovMatrix(1,1);
            fittedCov[2] = vtxCovMatrix(2,2);
            fittedCov[3] = vtxCovMatrix(3,3);
            fittedCov[4] = vtxCovMatrix(4,4);
          }

          for(int i=0; i<nVtx; ++i){
            fittedPosition[i+5] = vtxPositions[i+5]; //dist of vtxi on jet axis from PV
            fittedCov[i+5] = vtxCovMatrix(i+5,i+5);
          }

          // Retrieve track information at the primary vertex.
          std::vector<Trk::VxTrackAtVertex*> trackatPV;
          std::vector< float > tracksAtPVchi2;
          std::vector< float > tracksAtPVndf;
          std::vector< ElementLink< xAOD::TrackParticleContainer > > tracksAtPVlinks;
          if (vxjetcand) {
            trackatPV = vxjetcand->getPrimaryVertex()->getTracksAtVertex();
            std::vector<Trk::VxTrackAtVertex*>::const_iterator irBegin = trackatPV.begin();
            std::vector<Trk::VxTrackAtVertex*>::const_iterator irEnd   = trackatPV.end();
            for (std::vector<Trk::VxTrackAtVertex*>::const_iterator it=irBegin; it!=irEnd; ++it) {
              const Trk::FitQuality& trkquality = (*it)->trackQuality();
              double tmpchi2 = trkquality.chiSquared();
              int tmpndf = trkquality.numberDoF();
              tracksAtPVchi2.push_back(float(tmpchi2));
              tracksAtPVndf.push_back(float(tmpndf));
              //links
              Trk::ITrackLink* trklinks = (*it)->trackOrParticleLink();
              const Trk::LinkToXAODTrackParticle* trkLinkTPxAOD = dynamic_cast<const Trk::LinkToXAODTrackParticle *>(trklinks);
              if (!trkLinkTPxAOD) {
                ATH_MSG_WARNING("#JetTagVertexDecoratorAlg# bad ITrackLink is not a LinkToXAODTrackParticle");
                continue;
              }
              // Prepare an empty ElementLink for each track.
              ElementLink<xAOD::TrackParticleContainer> tpel;
              tracksAtPVlinks.push_back(tpel);
            }
          }
      
          // Compute JetFitter variables using the provided factory.
          Analysis::JetFitterVariables jfVars;
          ATH_CHECK(m_JFvarFactory->computeJetFitterVariables(jetToTag, myVertexInfoJetFitter, basename, jfVars));

          if (!JFvertices.empty()){
            if (JFvertices.size() > 0 && (jfVars.nVTX > 0 || jfVars.nSingleTracks > 0)) {
              isDefaults = 0; // at least vertex and track
              deltaR = std::sqrt(jfVars.deltaeta*jfVars.deltaeta + jfVars.deltaphi*jfVars.deltaphi);

            } else {
              jfVars.nVTX = -1;
              jfVars.nTracksAtVtx = -1;
              jfVars.nSingleTracks = -1;
              jfVars.energyFraction = NAN;
              jfVars.mass = NAN;
              jfVars.massUncorr = NAN;
              jfVars.significance3d = NAN;
              jfVars.deltaphi = NAN;
              jfVars.deltaeta = NAN;
              jfVars.chi2 = NAN;
              jfVars.ndof = -1;
              jfVars.dRFlightDir = NAN;
              N2TrkVtx = -1;
            }
          }

          // Decorate the jet with the computed JetFitter variables.
          writeJfnVTX(jetToTag) = jfVars.nVTX;
          writeJfnTracksAtVtx(jetToTag) = jfVars.nTracksAtVtx;
          writeJfnSingleTracks(jetToTag) = jfVars.nSingleTracks;
          writeJfEnergyFraction(jetToTag) = jfVars.energyFraction;
          writeJfMass(jetToTag) = jfVars.mass;
          writeJfMassUncorr(jetToTag) = jfVars.massUncorr;
          writeJfSignificance3d(jetToTag) = jfVars.significance3d;
          writeJfDeltaEta(jetToTag) = jfVars.deltaeta;
          writeJfDeltaPhi(jetToTag) = jfVars.deltaphi;
          writeJfDeltaR(jetToTag) = deltaR;
          writeJfChi2(jetToTag) = jfVars.chi2;
          writeJfNDOF(jetToTag) = jfVars.ndof;
          writeJfIsDefaults(jetToTag) = isDefaults;
          writeJfdRFlightDir(jetToTag) = jfVars.dRFlightDir;
          writeJfN2TrkPair(jetToTag) = N2TrkVtx;
          writeJfVertices(jetToTag) = JFVerticesLinks;
          writeJfFittedPosition(jetToTag) = fittedPosition;
          writeJfFittedCov(jetToTag) = fittedCov;
          writeJfTracksAtPVChi2(jetToTag) = tracksAtPVchi2;
          writeJfTracksAtPVndf(jetToTag) = tracksAtPVndf;
          writeJfTracksAtPVLinks(jetToTag) = tracksAtPVlinks;

          ATH_MSG_DEBUG("Decorated jet with pt : " << jetToTag.pt() << ", eta : " << jetToTag.eta() 
                        << " with features for alg : " << basename << ", nVtx: " << jfVars.nVTX 
                        << ", nSingleTracks: " << jfVars.nSingleTracks << ", energyFraction: " << jfVars.energyFraction 
                        << ", mass: " << jfVars.mass << ", massUncorr: " << jfVars.massUncorr 
                        << ", significance3d: " << jfVars.significance3d << ", deltaR: " << deltaR);
      
        }
      }
      ++nameiter;

    } 

    return StatusCode::SUCCESS;
    
  }
} 
