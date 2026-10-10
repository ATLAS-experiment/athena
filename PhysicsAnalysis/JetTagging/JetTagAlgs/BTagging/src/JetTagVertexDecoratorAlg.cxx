/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BTagging/JetTagVertexDecoratorAlg.h"

#include "SecVertexSignificance.h"

#include "VxSecVertex/VxSecVKalVertexInfo.h"
#include "xAODBase/IParticle.h"

#include <cmath>

namespace Analysis {

  /// @brief The JetTagVertexDecoratorAlg algorithm decorates jets with additional vertex-based features.
  /// 
  /// This algorithm retrieves the jet and vertex collections from the event store
  /// and loops over the available secondary vertex information produced by different vertex
  /// finders (e.g. SV1, SV1Flip). For each jet, the algorithm computes
  /// relevant quantities (e.g. vertex mass, energy fraction, distances) and writes them as decorations
  /// on the jet container.

  JetTagVertexDecoratorAlg::JetTagVertexDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator): 
    AthReentrantAlgorithm(name, pSvcLocator) {
    declareProperty("SecVtxFinderxAODBaseNameList", m_secVertexFinderBaseNameList);
  }


  StatusCode JetTagVertexDecoratorAlg::initialize()
  {
    ATH_CHECK( m_JetCollectionName.initialize() );
    ATH_CHECK( m_VertexCollectionName.initialize() );
    ATH_CHECK( m_TrackCollectionName.initialize() );
    ATH_CHECK( m_jetSVLinkName.initialize() );
    ATH_CHECK( m_jetSVFlipLinkName.initialize(!m_jetSVFlipLinkName.empty()) );
    ATH_CHECK( m_VxSecVertexInfoNames.initialize() );
      
    // For each base name (e.g. "SV1", "SV1Flip")
    
    m_decorKeys.clear();
    for (size_t i = 0; i < m_secVertexFinderBaseNameList.size(); i++) {
      std::string basename  = m_secVertexFinderBaseNameList[i];

      std::string prefix = m_JetCollectionName.key() + "." + basename;
      if (basename == "SV1" || basename == "SV1Flip") {
        m_decorKeys.emplace(basename, std::make_unique<SV1DecorHandles>(this, prefix));
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

    SG::ReadHandle<xAOD::TrackParticleContainer> h_TrackCollectionName (m_TrackCollectionName, ctx);
    if (!h_TrackCollectionName.isValid()) {
      ATH_MSG_ERROR( " cannot retrieve track container with key " << m_TrackCollectionName.key()  );
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
            distnrm = signedDecayLengthSignificance(*primaryVertex, vecVertices, jetDir);
            distnrmCorr = maxSignedDecayLengthSignificance(*primaryVertex, vecVertices, jetDir);
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
              tpel.toContainedElement(*h_TrackCollectionName, tp);
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

      ++nameiter;

    } 

    return StatusCode::SUCCESS;
    
  }
} 
