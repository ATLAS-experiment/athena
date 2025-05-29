/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "InDetSecVtxTruthMatchTool/InDetSecVtxTruthMatchTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h" // <-- Add this

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

using namespace InDetSecVtxTruthMatchUtils;

InDetSecVtxTruthMatchTool::InDetSecVtxTruthMatchTool( const std::string & name ) : asg::AsgTool(name) {}

StatusCode InDetSecVtxTruthMatchTool::initialize() {
  ATH_MSG_INFO("Initializing InDetSecVtxTruthMatchTool");

  // Retrieve the TrackTruthOriginTool
  ATH_CHECK(m_trackTruthOriginTool.retrieve());

  return StatusCode::SUCCESS;
}

namespace {
//Helper methods for this file only

//In the vector of match info, find the element corresponding to link and return its index; create a new one if necessary
size_t indexOfMatchInfo( std::vector<VertexTruthMatchInfo> & matches, const ElementLink<xAOD::TruthVertexContainer> & link ) {
  for ( size_t i = 0; i < matches.size(); ++i ) {
    if ( link.key() == std::get<0>(matches[i]).key() && link.index() == std::get<0>(matches[i]).index() )
      return i;
  }
  // This is the first time we've seen this truth vertex, so make a new entry
  matches.emplace_back( link, 0., 0. );
  return matches.size() - 1;
}

}

StatusCode InDetSecVtxTruthMatchTool::matchVertices( std::vector<const xAOD::Vertex*> recoVerticesToMatch,
                                                        std::vector<const xAOD::TruthVertex*> truthVerticesToMatch,
                                                        const xAOD::TrackParticleContainer* trackParticles) {

  ATH_MSG_DEBUG("Start vertex matching");

  //setup decorators for truth matching info
  static const xAOD::Vertex::Decorator<std::vector<VertexTruthMatchInfo> > matchInfoDecor("truthVertexMatchingInfos");
  static const xAOD::Vertex::Decorator<int> recoMatchTypeDecor("vertexMatchType");
  static const xAOD::Vertex::Decorator<std::vector<ElementLink<xAOD::VertexContainer> > > splitPartnerDecor("splitPartners");
  //optional for SM origin matching
  static const xAOD::Vertex::Decorator<int> smOriginDecor("vertexMatchOriginType");

  const xAOD::Vertex::Decorator<float> fakeScoreDecor("fakeScore");
  const xAOD::Vertex::Decorator<float> otherScoreDecor("otherScore");

  //setup accessors
  // can switch to built in method in xAOD::Vertex once don't have to deal with changing names anymore
  xAOD::Vertex::ConstAccessor<xAOD::Vertex::TrackParticleLinks_t> trkAcc("trackParticleLinks");
  xAOD::Vertex::ConstAccessor<std::vector<float> > weightAcc("trackWeights");
  xAOD::TrackParticle::ConstAccessor<ElementLink<xAOD::TruthParticleContainer> > trk_truthPartAcc("truthParticleLink");
  xAOD::TrackParticle::ConstAccessor<float> trk_truthProbAcc("truthMatchProbability");

  ATH_MSG_DEBUG("Starting Loop on Vertices");

  //=============================================================================
  //First loop over vertices: get tracks, then TruthParticles, and store relative
  //weights from each TruthVertex
  //=============================================================================
  for ( const xAOD::Vertex* vtx : recoVerticesToMatch ) {

    //create the vector we will add as matching info decoration later
    std::vector<VertexTruthMatchInfo> matchinfo;

    const xAOD::Vertex::TrackParticleLinks_t& trackParticles = trkAcc( *vtx );
    size_t ntracks = trackParticles.size();
    const std::vector<float> & trkWeights = weightAcc( *vtx );

    xAOD::Vertex::TrackParticleLinks_t trkMuSATrkParts; // For MuSA mode, we will populate this with MuSA track particles

    // Create a local variable for track particles to use in the rest of the function
    const xAOD::Vertex::TrackParticleLinks_t& trkParts = m_doMuSA ? trkMuSATrkParts : trackParticles;
    
    if ( m_doMuSA ) {
      // MuSA also creates new "MuSA Track" collection which does not have truth particle links
      // instead, we need to get the associated MS TrackParticle from the "MuSATrk_MSTPLink" decorations on the MuSA Track
      // and then get the truth particle link from there
      const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer>> acc_MSTPLink("MuSATrk_MSTPLink");
      // populate the MuSA track particle vector
      trkMuSATrkParts.reserve(ntracks);
      for (const auto& trkLink : trackParticles) {
        if (!trkLink.isValid()) continue;
        const xAOD::TrackParticle& trackParticle = **trkLink;
        if (acc_MSTPLink.isAvailable(trackParticle)) {
          const ElementLink<xAOD::TrackParticleContainer>& trkMuSATrkLink = acc_MSTPLink(trackParticle);
          if (trkMuSATrkLink.isValid()) {
            trkMuSATrkParts.push_back(trkMuSATrkLink);
          } else {
            ATH_MSG_WARNING("MS track particle link is not valid");
          }
        } else {
          ATH_MSG_WARNING("MuSATrk_MSTPLink decoration is not available on track particle");
        }
      }
    }
    
    //if don't have track particles
    if ((!trkAcc.isAvailable(*vtx) || !weightAcc.isAvailable(*vtx)) && !m_doMuSA) {
      ATH_MSG_WARNING("trackParticles or trackWeights not available, vertex is missing info");
      continue;
    }
    if ( trkWeights.size() != ntracks ) {
      ATH_MSG_WARNING("Vertex without same number of tracks and trackWeights, vertex is missing info");
      continue;
    }

    ATH_MSG_DEBUG("Matching new vertex at (" << vtx->x() << ", " << vtx->y() << ", " << vtx->z() << ")" << " with " << ntracks << " tracks, at index: " << vtx->index());

    float totalWeight = 0.;
    float totalPt = 0; 
    float otherPt = 0;
    float fakePt = 0;

    // Add for SM origin tracking
    int combinedSMOrigin = 0;
    
    //loop over the tracks in the vertex
    for ( size_t t = 0; t < ntracks; ++t ) {

      ATH_MSG_DEBUG("Checking track number " << t);

      // First check if the original track particle link is valid
      if (!trackParticles[t].isValid()) {
        ATH_MSG_DEBUG("Original track " << t << " is bad!");
        continue;
      }

      // Then check if we have a valid track after potential MuSA processing
      if (t >= trkParts.size() || !trkParts[t].isValid()) {
        ATH_MSG_DEBUG("Track " << t << " is invalid or out of bounds in trkParts!");
        continue;
      }
      const xAOD::TrackParticle trk = **trkParts[t];

      // Add to total weight
      if (m_doMuSA) {
        totalWeight += 1.0; // Add dummy weight of 1 for each track with MuSA
      } else {
        totalWeight += trkWeights[t];
      }

      // Safely get the track pt
      float trkPt = 0;
      try {
        trkPt = trk.pt();
        totalPt += trkPt;
      } catch (const std::exception& e) {
        ATH_MSG_WARNING("Exception when accessing track pt: " << e.what());
        continue;
      }

      // get the linked truth particle
      if (!trk_truthPartAcc.isAvailable(trk)) {
        ATH_MSG_DEBUG("The truth particle link decoration isn't available.");
        continue;
      }
      const ElementLink<xAOD::TruthParticleContainer> & truthPartLink = trk_truthPartAcc(trk);
      float prob = 0.0;
      if (m_doMuSA) {
        // For MuSA mode, assign a dummy probability of 1.0
        prob = 1.0;
        ATH_MSG_DEBUG("MuSA mode: using dummy truth prob of 1.0");
      } else {
        // For regular mode, use the decoration
        prob = trk_truthProbAcc(trk);
        ATH_MSG_DEBUG("Truth prob: " << prob);
      }

      // check the truth particle origin
      if (truthPartLink.isValid() && prob > m_trkMatchProb) {
        const xAOD::TruthParticle & truthPart = **truthPartLink;

        const int ancestorVertexUniqueID = checkProduction(truthPart, truthVerticesToMatch);
        // optional SM origin classification
        if (m_doSMOrigin) {
          const int smOrigin = checkSMProduction(truthPart);
          combinedSMOrigin |= smOrigin;
          
          // Check if this track is from LLP signal
          if (ancestorVertexUniqueID != HepMC::INVALID_VERTEX_ID) {
            combinedSMOrigin |= (0x1 << InDetSecVtxTruthMatchUtils::Signal);
          }
        }

        //check if the truth particle is "good"
        if (ancestorVertexUniqueID != HepMC::INVALID_VERTEX_ID) {
          //track in vertex is linked to LLP descendant
          //create link to truth vertex and add to matchInfo
          auto it = std::find_if(truthVerticesToMatch.begin(), truthVerticesToMatch.end(),
             [&](const auto& ele){ return HepMC::uniqueID(ele) == ancestorVertexUniqueID;} );

          if (it == truthVerticesToMatch.end()) {
            ATH_MSG_WARNING("Truth vertex with unique ID " << ancestorVertexUniqueID << " not found!");
          } else {
            ElementLink<xAOD::TruthVertexContainer> elLink;
            elLink.setElement(*it); 
            elLink.setStorableObject(*dynamic_cast<const xAOD::TruthVertexContainer*>((*it)->container()));
            size_t matchIdx = indexOfMatchInfo(matchinfo, elLink);

            if (m_doMuSA) {
              std::get<1>(matchinfo[matchIdx]) += 1.0; // Add dummy weight of 1 for MuSA
            } else {
              std::get<1>(matchinfo[matchIdx]) += trkWeights[t];
            }
            std::get<2>(matchinfo[matchIdx]) += trk.pt();
          }
        } else {
          //truth particle failed cuts
          ATH_MSG_DEBUG("Truth particle not from LLP decay.");
          otherPt += trk.pt();
        }
      } else {
        //not valid or low matching probability
        ATH_MSG_DEBUG("Invalid or low prob truth link!");
        fakePt += trk.pt();
        // Mark as fake for SM origin tracking
        if (m_doSMOrigin) {
          combinedSMOrigin |= (0x1 << InDetSecVtxTruthMatchUtils::FakeOrigin);
        }
      }
    }//end loop over tracks in vertex

    // normalize by total weight and pT
    std::for_each( matchinfo.begin(), matchinfo.end(), [&](VertexTruthMatchInfo& link)
    {
      std::get<1>(link) /= totalWeight;
      std::get<2>(link) /= totalPt;
    });

    float fakeScore = fakePt/totalPt;
    float otherScore = otherPt/totalPt;

    matchInfoDecor ( *vtx ) = matchinfo;
    fakeScoreDecor ( *vtx ) = fakeScore;
    otherScoreDecor( *vtx ) = otherScore;

    // Decorate with SM origin if enabled
    if (m_doSMOrigin) {
      smOriginDecor( *vtx ) = combinedSMOrigin;
    }
  }

  //After first loop, all vertices have been decorated with their vector of match info (link to TruthVertex paired with weight)
  //now we want to use that information from the whole collection to assign types
  //keep track of whether a type is assigned
  //useful since looking for splits involves a double loop, and then setting types ahead in the collection
  std::vector<bool> assignedType( recoVerticesToMatch.size(), false );
  static const xAOD::TruthVertex::Decorator<bool> isMatched("matchedToRecoVertex");
  static const xAOD::TruthVertex::Decorator<bool> isSplit("vertexSplit");

  for ( size_t i = 0; i < recoVerticesToMatch.size(); ++i ) {

    int recoVertexMatchType = 0;

    if ( assignedType[i] ) {
      ATH_MSG_DEBUG("Vertex already defined as split.");
      continue; // make sure we don't reclassify vertices already found in the split loop below
    }

    std::vector<VertexTruthMatchInfo> & info = matchInfoDecor( *recoVerticesToMatch[i] );
    float fakeScore  = fakeScoreDecor( *recoVerticesToMatch[i] );

    if(fakeScore > m_vxMatchWeight) {
      ATH_MSG_DEBUG("Vertex is fake.");
      recoVertexMatchType = recoVertexMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Fake);
    } else if (info.size() == 1) {
      if(std::get<2>(info[0]) > m_vxMatchWeight ) { // one truth matched vertex, sufficient weight
        ATH_MSG_DEBUG("One true decay vertices matched with sufficient weight. Vertex is matched.");
        recoVertexMatchType = recoVertexMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Matched);
        isMatched(**std::get<0>(info[0])) = true;
      }
      else {
        ATH_MSG_DEBUG("One true decay vertices matched with insufficient weight. Vertex is other.");
        recoVertexMatchType = recoVertexMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Other);
      }
    } else if (info.size() >= 2 ) {                        // more than one true deacy vertices matched
      ATH_MSG_DEBUG("Multiple true decay vertices matched. Vertex is merged.");
      recoVertexMatchType = recoVertexMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Merged);
      std::for_each( info.begin(), info.end(), [](VertexTruthMatchInfo& link)
      {
        isMatched(**std::get<0>(link)) = true;
      });
    } else {                                // zero truth matched vertices, but not fake 
      ATH_MSG_DEBUG("Vertex is neither fake nor LLP. Marking as OTHER.");
      recoVertexMatchType = recoVertexMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Other);
    }

    recoMatchTypeDecor(*recoVerticesToMatch[i]) = recoVertexMatchType;

    //check for splitting
    if ( InDetSecVtxTruthMatchUtils::isMatched(recoMatchTypeDecor(*recoVerticesToMatch[i])) || 
         InDetSecVtxTruthMatchUtils::isMerged(recoMatchTypeDecor(*recoVerticesToMatch[i])) ) {
      std::vector<size_t> foundSplits;
      for ( size_t j = i + 1; j < recoVerticesToMatch.size(); ++j ) {
        std::vector<VertexTruthMatchInfo> & info2 = matchInfoDecor( *recoVerticesToMatch[j] );
        //check second vertex is not dummy or fake, and that it has same elementlink as first vertex
        //equality test is in code but doesnt seem to work for ElementLinks that I have?
        //so i am just checking that the contianer key hash and the index are the same
        if (recoMatchTypeDecor( *recoVerticesToMatch[j] ) & (0x1 << InDetSecVtxTruthMatchUtils::Fake)) continue;
        if (!info2.empty() && std::get<0>(info2[0]).isValid() && std::get<0>(info[0]).key() == std::get<0>(info2[0]).key() && std::get<0>(info[0]).index() == std::get<0>(info2[0]).index() ) {
          //add split links; first between first one found and newest one
          ElementLink<xAOD::VertexContainer> splitLink_ij;
          splitLink_ij.setElement( recoVerticesToMatch[j] ); 
          splitLink_ij.setStorableObject( *dynamic_cast<const xAOD::VertexContainer*>(recoVerticesToMatch[j]->container()));
          splitPartnerDecor( *recoVerticesToMatch[i] ).emplace_back(splitLink_ij);

          ElementLink<xAOD::VertexContainer> splitLink_ji;
          splitLink_ji.setElement( recoVerticesToMatch[i] ); 
          splitLink_ji.setStorableObject( *dynamic_cast<const xAOD::VertexContainer*>(recoVerticesToMatch[i]->container()));
          splitPartnerDecor( *recoVerticesToMatch[j] ).emplace_back(splitLink_ji);

          //then between any others we found along the way
          for ( auto k : foundSplits ) { //k is a size_t in the vector of splits
            ElementLink<xAOD::VertexContainer> splitLink_kj;
            splitLink_kj.setElement( recoVerticesToMatch[j] ); 
            splitLink_kj.setStorableObject( *dynamic_cast<const xAOD::VertexContainer*>(recoVerticesToMatch[j]->container()));
            splitPartnerDecor( *recoVerticesToMatch[k] ).emplace_back(splitLink_kj);

            ElementLink<xAOD::VertexContainer> splitLink_jk;
            splitLink_jk.setElement( recoVerticesToMatch[k] );
            splitLink_jk.setStorableObject( *dynamic_cast<const xAOD::VertexContainer*>(recoVerticesToMatch[k]->container()));
            splitPartnerDecor( *recoVerticesToMatch[j] ).emplace_back(splitLink_jk);
          }
          //then keep track that we found this one
          foundSplits.push_back(j);
          recoMatchTypeDecor( *recoVerticesToMatch[i] ) = recoMatchTypeDecor( *recoVerticesToMatch[i] ) | (0x1 << InDetSecVtxTruthMatchUtils::Split);
          recoMatchTypeDecor( *recoVerticesToMatch[j] ) = recoMatchTypeDecor( *recoVerticesToMatch[j] ) | (0x1 << InDetSecVtxTruthMatchUtils::Split);
          isSplit(**std::get<0>(info[0])) = true;
          assignedType[j] = true;
        } //if the two vertices match to same TruthVertex
      }//inner loop over vertices
    } //if matched or merged

  } //outer loop

  // now label truth vertices

  ATH_MSG_DEBUG("Labeling truth vertices");

  static const xAOD::TruthVertex::Decorator<int> truthMatchTypeDecor("truthVertexMatchType");

  for(const xAOD::TruthVertex* truthVtx : truthVerticesToMatch) {

    std::vector<const xAOD::TruthParticle*> reconstructibleParticles;
    int counter = 0;
    countReconstructibleDescendentParticles( *truthVtx, reconstructibleParticles, counter );

    // hacky solution for keeping track of particles in the vertex
    std::vector<int> particleInfo = {0,0,0};
    std::vector<int> vertexInfo = {0,0,0};

    for(size_t n = 0; n < reconstructibleParticles.size(); n++){
      ATH_MSG_DEBUG("Checking daughter no. " << n);
      const xAOD::TruthParticle* outPart = reconstructibleParticles.at(n);
      
      if (trackParticles){
        particleInfo = checkParticle(*outPart, trackParticles);
      
        for(size_t h = 0; h < particleInfo.size(); h++){
          vertexInfo.at(h) += particleInfo.at(h);
        }
      }
    }
      
    int truthMatchType = 0;
    if( vertexInfo.at(0) > 1 && 
        ((m_doMuSA && truthVtx->perp() < 8000 && std::abs(truthVtx->z()) < 10000) ||
        (!m_doMuSA && truthVtx->perp() < 320 && std::abs(truthVtx->z()) < 1500))){
      ATH_MSG_DEBUG("Vertex is reconstructable and in " << (m_doMuSA ? "Muon Spectrometer" : "Inner Det") << " region");
      truthMatchType = truthMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Reconstructable);
    }
    if( InDetSecVtxTruthMatchUtils::isReconstructable(truthMatchType) and vertexInfo.at(1) > 1){
      ATH_MSG_DEBUG("Vertex has at least two tracks");
      truthMatchType = truthMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Accepted);
    }
    if(InDetSecVtxTruthMatchUtils::isAccepted(truthMatchType) and vertexInfo.at(2) > 1){
      ATH_MSG_DEBUG("Vertex is has at least two tracks passing track selection: " << vertexInfo.at(2));
      truthMatchType = truthMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Seeded);
    }
    if(InDetSecVtxTruthMatchUtils::isSeeded(truthMatchType) and isMatched(*truthVtx)){
      ATH_MSG_DEBUG("Vertex is matched to a reconstructed secVtx");
      truthMatchType = truthMatchType | (0x1 << InDetSecVtxTruthMatchUtils::Reconstructed);
    }
    if(InDetSecVtxTruthMatchUtils::isSeeded(truthMatchType) and isSplit(*truthVtx)){
      ATH_MSG_DEBUG("Vertex is matched to multiple secVtx");
      truthMatchType = truthMatchType | (0x1 << InDetSecVtxTruthMatchUtils::ReconstructedSplit);
    }
    truthMatchTypeDecor(*truthVtx) = truthMatchType;
  }
  ATH_MSG_DEBUG("Done labeling truth vertices");

  return StatusCode::SUCCESS;

}

std::vector<int> InDetSecVtxTruthMatchTool::checkParticle(const xAOD::TruthParticle &truthPart, const xAOD::TrackParticleContainer* trkCont) const {

  xAOD::TrackParticle::ConstAccessor<char>  trackPass(m_selectedTrackFlag);
  xAOD::TrackParticle::ConstAccessor<ElementLink<xAOD::TruthParticleContainer> > trk_truthPartAcc("truthParticleLink");
  xAOD::TrackParticle::ConstAccessor<float> trk_truthProbAcc("truthMatchProbability");

  if(truthPart.pt() < m_trkPtCut){
    ATH_MSG_DEBUG("Insufficient pt to reconstruct the particle");
    return {0,0,0};
  }
  else{

    for(const xAOD::TrackParticle* trkPart : *trkCont){
      // Handle differently for MuSA vs standard mode
      if (m_doMuSA) {
        if (!trk_truthPartAcc.isAvailable(*trkPart)) {
          ATH_MSG_DEBUG("Truth link not available on MS track");
          continue;
        }
        const ElementLink<xAOD::TruthParticleContainer>& truthLink = trk_truthPartAcc(*trkPart);
        if (!truthLink.isValid()) {
          ATH_MSG_DEBUG("Truth link on MS track not valid");
          continue;
        }
        const xAOD::TruthParticle& linkedTruth = **truthLink;
        if (HepMC::is_same_particle(linkedTruth, truthPart)) {
          // We found a match between truth particle and MS track!
          // no selected decoration so need to implement manually -- MSTP must be |eta| < 2.5
          // ideally we would want to check if the MSTP is also an SA muon but this is more complicated given we need the muon container
          if (std::abs(trkPart->eta()) < 2.5) {
            ATH_MSG_DEBUG("Particle has a track that passes track selection.");
            return {1,1,1};
          } else {
            ATH_MSG_DEBUG("Particle has a track, but did not pass track selection.");
            return {1,1,0};
          }
        }
      }
      
      // Standard mode - check truth matching
      if (!trk_truthPartAcc.isAvailable(*trkPart)) {
        ATH_MSG_DEBUG("truthParticleLink not available on track");
        continue;
      }
      
      const ElementLink<xAOD::TruthParticleContainer> & truthPartLink = trk_truthPartAcc(*trkPart);
      
      // Check if truth match probability is available
      float matchProb = 0.0;
      if (trk_truthProbAcc.isAvailable(*trkPart)) {
        matchProb = trk_truthProbAcc(*trkPart);
      } else {
        ATH_MSG_DEBUG("truthMatchProbability not available on track");
        continue;
      }

      if(truthPartLink.isValid() && matchProb > m_trkMatchProb) {
        const xAOD::TruthParticle& tmpPart = **truthPartLink;
        if( HepMC::is_same_particle(tmpPart,truthPart) ) {
          if(trackPass.isAvailable( *trkPart )) {
            if(trackPass( *trkPart )) {
              ATH_MSG_DEBUG("Particle has a track that passes track selection.");
              return {1,1,1};
            } else {
              ATH_MSG_DEBUG("Particle has a track, but did not pass track selection.");
              return {1,1,0};
            }
          } else {
            ATH_MSG_DEBUG("Track selection decoration not available, calling the track selected");
            return {1,1,1};
          }
        }
      }
    }
    ATH_MSG_DEBUG("Particle has enough pt.");
    return {1,0,0};
    
  }
  return {0,0,0};
}

// check if truth particle originated from decay of particle in the pdgIdList
int InDetSecVtxTruthMatchTool::checkProduction( const xAOD::TruthParticle & truthPart, std::vector<const xAOD::TruthVertex*> truthVerticesToMatch ) const {

  if (truthPart.nParents() == 0){
    ATH_MSG_DEBUG("Particle has no parents (end of loop)");
    return HepMC::INVALID_VERTEX_ID;
  }
  else{
    const xAOD::TruthParticle * parent = truthPart.parent(0);
    if(not parent) {
      ATH_MSG_DEBUG("Particle parent is null");
      return HepMC::INVALID_VERTEX_ID;
    }
    ATH_MSG_DEBUG("Parent ID: " << parent->pdgId());

    const xAOD::TruthVertex* parentVertex = parent->decayVtx();
    if(std::find(truthVerticesToMatch.begin(), truthVerticesToMatch.end(), parentVertex) != truthVerticesToMatch.end()) {
      ATH_MSG_DEBUG("Found LLP decay.");
      return HepMC::barcode(parentVertex); // FIXME barcode-based
    }
    // recurse on parent
    return checkProduction(*parent, truthVerticesToMatch);
  }
  return HepMC::INVALID_VERTEX_ID;
}

// secvtx origin has extra categories compared to original track origin, so need to map them to our enum
int mapTrkOriginToSecVtxOrigin(int trkOriginBits) {
  int myBits = 0;
  if (trkOriginBits & (1 << InDet::TrkOrigin::BHadronDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::BHadronDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::DHadronDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::DHadronDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::TauDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::TauDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::GammaConversion))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::GammaConversion);
  if (trkOriginBits & (1 << InDet::TrkOrigin::StrangeMesonDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::StrangeMesonDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::KshortDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::KshortDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::StrangeBaryonDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::StrangeBaryonDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::LambdaDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::LambdaDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::OtherDecay))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::OtherDecay);
  if (trkOriginBits & (1 << InDet::TrkOrigin::HadronicInteraction))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::HadronicInteraction);
  if (trkOriginBits & (1 << InDet::TrkOrigin::OtherSecondary))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::OtherSecondary);
  if (trkOriginBits & (1 << InDet::TrkOrigin::Fragmentation))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::Fragmentation);
  if (trkOriginBits & (1 << InDet::TrkOrigin::OtherOrigin))
    myBits |= (1 << InDetSecVtxTruthMatchUtils::OtherOrigin);
  return myBits;
}

int InDetSecVtxTruthMatchTool::checkSMProduction(const xAOD::TruthParticle & truthPart) const {
  if (m_trackTruthOriginTool.isSet()) {
    int trkOriginBits = m_trackTruthOriginTool->getTruthOrigin(&truthPart);
    return mapTrkOriginToSecVtxOrigin(trkOriginBits);
  }
  ATH_MSG_WARNING("TrackTruthOriginTool not set, returning 0 for origin");
  return 0;
}

void InDetSecVtxTruthMatchTool::countReconstructibleDescendentParticles(const xAOD::TruthVertex& signalTruthVertex,
                     std::vector<const xAOD::TruthParticle*>& set, int counter) const {

  counter++;

  for (size_t itrk = 0; itrk < signalTruthVertex.nOutgoingParticles(); itrk++) {
    const auto* particle = signalTruthVertex.outgoingParticle(itrk);
    if (!particle) continue;
  
    // Recursively add descendents
    if (particle->hasDecayVtx()) {
    
      TVector3 decayPos(particle->decayVtx()->x(), particle->decayVtx()->y(), particle->decayVtx()->z());
      TVector3 prodPos(particle->prodVtx()->x(), particle->prodVtx()->y(), particle->prodVtx()->z());
    
      // Inner detector criteria
      auto isInsideID = [](TVector3& v) { return (v.Perp() < 300. && std::abs(v.z()) < 1500.); };
      auto isOutsideID = [](TVector3& v) { return (v.Perp() > 563. || std::abs(v.z()) > 2720.); };
    
      // Muon Spectrometer criteria
      auto isOutsideID_MuSA = [](TVector3& v) { return (v.Perp() > 563. || std::abs(v.z()) > 2720.); };
      auto isInsideMS_MuSA = [](TVector3& v) { return (v.Perp() < 8000. && std::abs(v.z()) < 10000.); };
    
      const auto distance = (decayPos - prodPos).Mag();

      if (counter > 100) {
        ATH_MSG_WARNING("Vetoing particle that may be added recursively infinitely (potential loop in generator record");
        break;
      }
    
      // consider track reconstructible if it travels at least 10mm
      if (distance < 10.0) {
        countReconstructibleDescendentParticles(*particle->decayVtx(), set, counter);
      } else if (m_doMuSA) {
      // MuSA: particle originates outside ID and ends in MS
        if (isOutsideID_MuSA(prodPos) && isInsideMS_MuSA(decayPos) && (particle->isCharged() || particle->isMuon())) {
          set.push_back(particle);
        }
      } else {
        // Regular tracking: particle originates inside ID and ends outside ID
        if (isInsideID(prodPos) && isOutsideID(decayPos) && particle->isCharged()) {
          set.push_back(particle);
        } else if (particle->isElectron() || particle->isMuon()) {
          set.push_back(particle);
        }
      }
    } else {
      if (!(particle->isCharged())) continue;
      // For particles without decay vertex, include them if they're charged
      set.push_back(particle);
    }
  }
}

bool InDetSecVtxTruthMatchTool::isFrom(const xAOD::TruthParticle& truth, int flav) const {
  if (m_trackTruthOriginTool.isSet()) {
    return m_trackTruthOriginTool->isFrom(&truth, flav);
  }
  ATH_MSG_WARNING("TrackTruthOriginTool not set, returning false for isFrom");
  return false;
}


