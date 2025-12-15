/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#include "TauVertexFinder.h"

TauVertexFinder::TauVertexFinder(const std::string& name ) :
  TauRecToolBase(name) {}

TauVertexFinder::~TauVertexFinder() {}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauVertexFinder::initialize() {
  ATH_CHECK( m_vertexInputContainer.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_trackPartInputContainer.initialize(SG::AllowEmpty) );
 
  if( m_useTJVA) {
     ATH_MSG_INFO("using TJVA to determine tau vertex");	  
     ATH_CHECK( m_TrackSelectionToolForTJVA.retrieve() );
     ATH_CHECK( m_trkVertexAssocTool.retrieve() );
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauVertexFinder::executeVertexFinder(xAOD::TauJet& pTau,
                                                const xAOD::VertexContainer* vertexContainer) const 
{
  const xAOD::VertexContainer * vxContainer = nullptr;

  if (!m_vertexInputContainer.empty()) {
    SG::ReadHandle<xAOD::VertexContainer> vertexInHandle( m_vertexInputContainer );
    if (!vertexInHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << vertexInHandle.key());
      return StatusCode::FAILURE;
    }
    vxContainer = vertexInHandle.cptr();
  }
  else {
    if (vertexContainer != nullptr) {
      vxContainer = vertexContainer;
    }
    else {
      ATH_MSG_WARNING ("No Vertex Container in trigger");
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_VERBOSE("size of VxPrimaryContainer is: "  << vxContainer->size() );
  if (vxContainer->empty()) return StatusCode::SUCCESS;

  // find default PrimaryVertex (needed if TJVA is switched off or fails)
  const xAOD::Vertex* primaryVertex = nullptr;
  if (inTrigger()) { // trigger: find default PrimaryVertex (highest sum pt^2)
    primaryVertex = (*vxContainer)[0];
  }
  else { // offline: the first and only primary vertex candidate is picked
    for (const auto vertex : *vxContainer) {
      if (vertex->vertexType() ==  xAOD::VxType::PriVtx) {
        primaryVertex = vertex;
        break;
      }
    }
  }

  // associate vertex to tau
  if (primaryVertex) pTau.setVertex(vxContainer, primaryVertex);

  //stop here if TJVA is disabled
  if (!m_useTJVA) return StatusCode::SUCCESS;

  // additional protection in case TJVA is run online - not supported for Run3
  if ( m_useTJVA && inTrigger()){
     ATH_MSG_ERROR("TJVA not available for online");
     return StatusCode::FAILURE;
  }	  

  // try to find new PV with TJVA
  ATH_MSG_DEBUG("TJVA enabled -> try to find new PV for the tau candidate");

  float maxJVF = -100.;
  ElementLink<xAOD::VertexContainer> newPrimaryVertexLink = getPV_TJVA(pTau, *vxContainer, maxJVF );
  if (newPrimaryVertexLink.isValid()) {
    // set new primary vertex
    // will overwrite default one which was set above
    pTau.setVertexLink(newPrimaryVertexLink);
    // save highest JVF value
    pTau.setDetail(xAOD::TauJetParameters::TauJetVtxFraction,static_cast<float>(maxJVF));
    ATH_MSG_DEBUG("TJVA vertex found and set");
  }
  else {
    ATH_MSG_WARNING("couldn't find new PV for TJVA");
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
ElementLink<xAOD::VertexContainer>
TauVertexFinder::getPV_TJVA(const xAOD::TauJet& pTau,
                            const xAOD::VertexContainer& vertices,
                            float& maxJVF) const
{
  const xAOD::Jet* pJetSeed = pTau.jet();
  std::vector<const xAOD::TrackParticle*> tracksForTJVA;

  // the implementation follows closely the example given in modifyJet(...) in https://svnweb.cern.ch/trac/atlasoff/browser/Reconstruction/Jet/JetMomentTools/trunk/Root/JetVertexFractionTool.cxx#15

  std::vector<const xAOD::TrackParticle*> assocTracks;
  if (! pJetSeed->getAssociatedObjects(m_assocTracksName, assocTracks)) {
    ATH_MSG_ERROR("Could not retrieve the AssociatedObjects named \""<< m_assocTracksName <<"\" from jet");
    return ElementLink<xAOD::VertexContainer>();
  }
 
  // Store tracks that meet TJVA track selection criteria and are between deltaR of 0.2 with the jet seed
  // To be included in the TJVA calculation
  // Maybe not as efficient as deleting unwanted tracks from assocTrack but quicker and safer for now.
  float sumTrackAll = 0.0;
  for ( auto xTrack : assocTracks ) {
    if ( (xTrack->p4().DeltaR(pJetSeed->p4())<m_dDeltaRMax) && m_TrackSelectionToolForTJVA->accept(*xTrack) ) {
      if (!inEleRM()) { 
          tracksForTJVA.push_back(xTrack); 
      } 
      else {
        static const SG::ConstAccessor<ElementLink<xAOD::TrackParticleContainer>> acc_originalTrack("ERMOriginalTrack");
        if (!acc_originalTrack.isAvailable(*xTrack)) {
          ATH_MSG_WARNING("Original ERM track link is not available, skipping track");
          continue;
        }
        auto original_id_track_link = acc_originalTrack(*xTrack);
        if (!original_id_track_link.isValid()) {
          ATH_MSG_WARNING("Original ERM track link is not valid, skipping track");
          continue;
        }
        tracksForTJVA.push_back(*original_id_track_link);
      }
      sumTrackAll += xTrack->pt();
    }
  }

  if (this->msgLevel() <= MSG::DEBUG){
    ATH_MSG_DEBUG("tracksForTJVA # = " << tracksForTJVA.size() << ", sum pT = " << sumTrackAll);

    for (uint i = 0; i < tracksForTJVA.size(); i++){
      ATH_MSG_DEBUG("tracksForTJVA[" << i << "].pt = " << tracksForTJVA[i]->pt());
    }
  }

  xAOD::TrackVertexAssociationMap trktovxmap;

  // Get track vertex map
  std::vector<const xAOD::Vertex*> vertVec;
  for (const xAOD::Vertex* vert : vertices) {
    vertVec.push_back(vert);
  }

  if (this->msgLevel() <= MSG::DEBUG){
    ATH_MSG_DEBUG("Vertex # = " << vertVec.size());
    for (uint i = 0; i < vertVec.size(); i++){
      ATH_MSG_DEBUG("vertVec[" << i << "].z = " << vertVec[i]->z());
    }
  }

  // Tool returns map between vertex and tracks associated to that vertex (based on selection criteria set in config)
  trktovxmap = m_trkVertexAssocTool->getMatchMap(tracksForTJVA, vertVec);
 
  if (this->msgLevel() <= MSG::DEBUG){
    for (const auto& [vtx, trks] : trktovxmap){
      std::stringstream ss;
      for (auto ass_trk : trks){
        for (uint i=0; i < tracksForTJVA.size(); i++){
          if (ass_trk->p4() == tracksForTJVA[i]->p4()) {
            ss << i << ", ";
            break;
          }
        }
      }
      ATH_MSG_DEBUG("Vtx[" << vtx->index() << "] associated with trks [" << ss.str() << "]");
    }
  }

  // Get the highest JVF vertex and store maxJVF for later use
  // Note: the official JetMomentTools/JetVertexFractionTool doesn't provide any possibility to access the JVF value, but just the vertex.
  maxJVF=-100.;
  // Store sum(deltaz(track-vertex)) and jet vertex fraction scores
  std::vector<float> sumDz;
  std::vector<float> v_jvf;
  size_t maxIndex = 0;
  for (const xAOD::Vertex* vert : vertices) {
    float jvf = 0.0;
    std::vector<const xAOD::TrackParticle*> tracks = trktovxmap[vert];
    // get jet vertex fraction and sumdeltaZ scores
    std::pair<float, float> spair = getVertexScores(tracks, vert->z());
    jvf = (sumTrackAll!=0. ? spair.first/sumTrackAll : 0.);
    v_jvf.push_back(jvf);
    sumDz.push_back(spair.second);
    
    if (jvf > maxJVF) {
      maxJVF = jvf;
      maxIndex = vert->index();
    }
  }

  ATH_MSG_DEBUG("First TJVA");
  ATH_MSG_DEBUG("TJVA vtx found at z: " << vertices.at(maxIndex)->z() << " i_vtx = " << maxIndex << "jvf = " << maxJVF);
  ATH_MSG_DEBUG("highest pt vtx found at z (i=0): " << vertices.at(0)->z());

  float min_sumDz = 99999999.;
  for (const xAOD::Vertex* vert : vertices) {
    ATH_MSG_DEBUG("i_vtx=" << vert->index() << ", z=" << vert->z() << ", JVF=" << v_jvf[vert->index()] << ", sumDz=" << sumDz[vert->index()]);
    if ( v_jvf[vert->index()] == maxJVF ){
      // in case of 0 tracks, first vertex will have sumDz=0, and be selected
      if (sumDz[vert->index()] < min_sumDz){
        min_sumDz = sumDz[vert->index()];
        maxIndex = vert->index();
      }
    }
  }


  ATH_MSG_DEBUG("Final TJVA");
  ATH_MSG_DEBUG("TJVA vtx found at z: " << vertices.at(maxIndex)->z() << " i_vtx = " << maxIndex);

  return ElementLink<xAOD::VertexContainer> (vertices, maxIndex);
}

// get sum of pT from tracks associated to vertex (tracks from track to vertex map) (pair first)
// sum over tracks associated to vertex of deltaZ(track-vertex) (pair second)
std::pair<float,float> TauVertexFinder::getVertexScores(const std::vector<const xAOD::TrackParticle*>& tracks, float vx_z) const{

  float sumTrackPV = 0.;
  float sumDeltaZ = 0.;
  for (auto trk : tracks){
    sumTrackPV += trk->pt();
    sumDeltaZ += std::abs(trk->z0() - vx_z + trk->vz());
  }

  return std::make_pair(sumTrackPV, sumDeltaZ);
}
#endif
