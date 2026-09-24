/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthVertexDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include <cstdlib>
#include <unordered_set>

namespace ParticleJetTools{

inline const xAOD::TruthParticle* get_initial_particle(
  const xAOD::TruthParticle* part
) {
  const xAOD::TruthParticle* parent = part;
  for ( int i = 0; i < 1000; i++ ) {
    const xAOD::TruthParticle* next_parent = nullptr;

    if(parent->nParents() > 1){
      float cur_pt = -1.0;
      for (size_t j = 0; j < parent->nParents(); j++) {
        const xAOD::TruthParticle* p = parent->parent(j);
        if ( !p ) continue;
        if ( p->pt() > cur_pt ) {
          cur_pt = p->pt();
          next_parent = p;
        }
      }
    }else{
      next_parent = parent->parent(0);
    }
    if ( !next_parent ) break;
    if ( !next_parent->hasDecayVtx() || !next_parent->decayVtx() ) break;
    parent = next_parent;
  }
  return parent;
}

std::vector<const xAOD::TruthParticle*> get_all_initial_particles(
    const std::vector<const xAOD::TruthParticle*>& truth_particles
) {
  std::unordered_set<const xAOD::TruthParticle*> pv_parts;

  for ( const auto& part : truth_particles ) {
    auto initial_part = get_initial_particle(part);
    if ( initial_part ) {
      pv_parts.insert(initial_part);
    }
  }

  std::vector<const xAOD::TruthParticle*> initial_particles;
  initial_particles.reserve(pv_parts.size());
  for ( const auto& part : pv_parts ) {
    initial_particles.push_back(part);
  }
  return initial_particles;

}

/**
 * @brief Get the matched primary vertex for a given truth vertex.
 * The TruthPrimaryVertices container does NOT contain particle links,
 * so we match by position to find the corresponding vertex in TruthVertices.
 */
std::vector<const xAOD::TruthVertex*> get_matched_primary_vertices(
    const xAOD::TruthVertex* pv, const std::vector<const xAOD::TruthVertex*>& truth_vertices,
    float selection_distance = 0.001,
    SG::ConstAccessor<int> acc_uid = SG::ConstAccessor<int>("barcode")
) {
  std::vector<const xAOD::TruthVertex*> matched_vertices;

  float closest_distance = 1e9;
  const xAOD::TruthVertex* closest_vertex = nullptr;
  for( const auto& vert : truth_vertices ) {
    if ( !vert ) continue;
    float r = FatVertex::vertex_distance(pv, vert);
    if(r < selection_distance){
      matched_vertices.push_back(vert);
      if (r < closest_distance) {
        closest_distance = r;
        closest_vertex = vert;
      }else if ((r == closest_distance) && closest_vertex) {
        // If we have two vertices at the same distance, we'll take the one with the lowest
        // barcode as it *should* be the case that this is the actual PV.
        if ( std::abs(acc_uid(*vert)) < std::abs(acc_uid(*closest_vertex)) ) {
          closest_vertex = vert;
        }
      }
    }
  }
  // Set it such that the first vertex is the closest vertex
  if ( !matched_vertices.empty() && closest_vertex ) {
    // remove the closest vertex from the matched vertices
    matched_vertices.erase(
        std::remove(matched_vertices.begin(), matched_vertices.end(), closest_vertex),
        matched_vertices.end()
    );
    // Insert into the front of the vector
    matched_vertices.insert(matched_vertices.begin(), closest_vertex);

  }
  return matched_vertices;

}

TruthVertexDecoratorAlg::TruthVertexDecoratorAlg(const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}

StatusCode TruthVertexDecoratorAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "... ");

  // Initialize Container keys
  ATH_MSG_DEBUG("Initializing containers:");
  ATH_MSG_DEBUG("    ** " << m_TruthContainerKey);
  ATH_MSG_DEBUG("    ** " << m_TruthPVsKey);
  ATH_MSG_DEBUG("    ** " << m_TruthVertexContainerKey);

  ATH_CHECK(m_TruthContainerKey.initialize());
  ATH_CHECK(m_TruthPVsKey.initialize());
  ATH_CHECK(m_TruthVertexContainerKey.initialize());
  ATH_CHECK(m_TrackContainerKey.initialize());

  m_vertexValid = m_TruthContainerKey.key() + "." + m_vertexValid.key();
  m_vertexDecayType = m_TruthContainerKey.key() + "." + m_vertexDecayType.key();
  m_vertexSimpleDecayType = m_TruthContainerKey.key() + "." + m_vertexSimpleDecayType.key();
  m_vertexDecayID = m_TruthContainerKey.key() + "." + m_vertexDecayID.key();
  m_vertexDecayParentID = m_TruthContainerKey.key() + "." + m_vertexDecayParentID.key();
  m_vertexDecayNCharged = m_TruthContainerKey.key() + "." + m_vertexDecayNCharged.key();
  m_vertexDecayNNeutral = m_TruthContainerKey.key() + "." + m_vertexDecayNNeutral.key();
  m_vertexDecayAllNCharged = m_TruthContainerKey.key() + "." + m_vertexDecayAllNCharged.key();
  m_vertexDecayAllNNeutral = m_TruthContainerKey.key() + "." + m_vertexDecayAllNNeutral.key();
  m_vertexDecayPVDistance = m_TruthContainerKey.key() + "." + m_vertexDecayPVDistance.key();
  m_vertexDecayDistance = m_TruthContainerKey.key() + "." + m_vertexDecayDistance.key();
  m_vertexIsVertex = m_TruthContainerKey.key() + "." + m_vertexIsVertex.key();

  ATH_CHECK(m_vertexValid.initialize());
  ATH_CHECK(m_vertexDecayType.initialize());
  ATH_CHECK(m_vertexSimpleDecayType.initialize());
  ATH_CHECK(m_vertexDecayID.initialize());
  ATH_CHECK(m_vertexDecayParentID.initialize());
  ATH_CHECK(m_vertexDecayNCharged.initialize());
  ATH_CHECK(m_vertexDecayNNeutral.initialize());
  ATH_CHECK(m_vertexDecayAllNCharged.initialize());
  ATH_CHECK(m_vertexDecayAllNNeutral.initialize());
  ATH_CHECK(m_vertexDecayPVDistance.initialize());
  ATH_CHECK(m_vertexDecayDistance.initialize());
  ATH_CHECK(m_vertexIsVertex.initialize());

  m_trackTPDecayVertexID = m_TrackContainerKey.key() + '.' + m_trackTPDecayVertexID.key();
  m_trackTPDecayVertexType = m_TrackContainerKey.key() + '.' + m_trackTPDecayVertexType.key();
  m_trackTPDecaySimpleVertexType = m_TrackContainerKey.key() + '.' + m_trackTPDecaySimpleVertexType.key();
  m_trackPDGID = m_TrackContainerKey.key() + '.' + m_trackPDGID.key();
  m_trackParentPDGID = m_TrackContainerKey.key() + '.' + m_trackParentPDGID.key();
  ATH_CHECK(m_trackTPDecayVertexID.initialize());
  ATH_CHECK(m_trackTPDecayVertexType.initialize());
  ATH_CHECK(m_trackTPDecaySimpleVertexType.initialize());
  ATH_CHECK(m_trackPDGID.initialize());
  ATH_CHECK(m_trackParentPDGID.initialize());

  ATH_CHECK(m_trackTruthOriginTool.retrieve());

  m_acc_truth_particle_vertex_id = "TruthParticles." + m_acc_truth_particle_vertex_id.key();
  ATH_CHECK(m_acc_truth_particle_vertex_id.initialize());

  if(m_use_barcode) m_acc_uid = SG::ConstAccessor<int>("barcode");
  m_truthMatchProbabilityAcc = SG::AuxElement::ConstAccessor<float>(m_truthMatchProbabilityAuxName);
  return StatusCode::SUCCESS;
}

StatusCode TruthVertexDecoratorAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << "... ");

  // read collections
  SG::ReadHandle<xAOD::TruthParticleContainer> truth_particles(m_TruthContainerKey, ctx);
  ATH_CHECK(truth_particles.isValid());
  ATH_MSG_DEBUG("Retrieved " << truth_particles->size() << " truth_particles...");

  // Get the truth PV
  SG::ReadHandle<xAOD::TruthVertexContainer> truth_PVs(m_TruthPVsKey, ctx);
  ATH_CHECK(truth_PVs.isValid());
  if ( truth_PVs->size() != 1 ) {
    ATH_MSG_ERROR("Truth PVs != 1");
    return StatusCode::FAILURE;
  }
  const xAOD::TruthVertex* truth_PV = truth_PVs->at(0);

  // Get all truth vertices, and then get the 'matched' primary vertex
  SG::ReadHandle<xAOD::TruthVertexContainer> truth_vertices(m_TruthVertexContainerKey, ctx);
  ATH_CHECK(truth_vertices.isValid());
  std::vector<const xAOD::TruthVertex*> all_truth_vertices;
  for ( auto vert : *truth_vertices ) { all_truth_vertices.push_back(vert); }
  auto pvs = get_matched_primary_vertices(truth_PV, all_truth_vertices, 0.001, m_acc_uid);
  if (pvs.empty()) {
    ATH_MSG_ERROR("No matched primary vertex found!");
    return StatusCode::FAILURE;
  }
  const xAOD::TruthVertex* pv = pvs[0];
  // Get all truth particles, and generate a map for speedier lookup
  std::vector<const xAOD::TruthParticle*> sorted_truth_particles;
  for ( const auto tp : *truth_particles ) { sorted_truth_particles.push_back(tp); }
  std::sort(
      sorted_truth_particles.begin(), sorted_truth_particles.end(),
      [](const xAOD::IParticle* p1, const xAOD::IParticle* p2) {
        return p1->pt() < p2->pt();
    }
  );

  // We search all particles to get a vector of all initial particles. We need to do this
  // as just looking at the PV isn't sufficient, due to ISR etc
  auto initial_particles = get_all_initial_particles(sorted_truth_particles);


  // Generate a vector of all fat veritces, starting at the PV. We include all initial particles
  // as part of this fat-pv
  std::vector<FatVertex::FatVertex> fat_vertices = FatVertex::generateFatVertices(
    pv, m_truthVertexMergeDistance,
    initial_particles
  );

  // Setup parent/child links for all vertices
  for(auto& fat_vertex : fat_vertices){
    fat_vertex.doParentChildLinks(fat_vertices);
  }

  // Select based on a pT threshold
  std::vector<FatVertex::FatVertex> hpt_fat_vertices;
  for ( auto& fat_vertex : fat_vertices ) {
    if ( fat_vertex.getPT() > 0 || fat_vertex.is_pv ) hpt_fat_vertices.push_back(fat_vertex);
  }



  // Define the write handles for decorating the truth particles with their vertex information
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexValid(m_vertexValid, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexType(m_vertexDecayType, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexSimpleType(m_vertexSimpleDecayType, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexDecayID(m_vertexDecayID, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexDecayParentID(m_vertexDecayParentID, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexDecayNCharged(m_vertexDecayNCharged, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexDecayNNeutral(m_vertexDecayNNeutral, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexDecayAllNCharged(m_vertexDecayAllNCharged, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexDecayAllNNeutral(m_vertexDecayAllNNeutral, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float> vertexDecayPVDistance(m_vertexDecayPVDistance, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float> vertexDecayDistance(m_vertexDecayDistance, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> vertexIsVertex(m_vertexIsVertex, ctx);

  // Anything which doesn't have a decay vertex is a stable particle
  std::vector<const xAOD::TruthParticle*> stable_neutrals;
  std::vector<const xAOD::TruthParticle*> stable_charged;
  std::unordered_map<int, int> barcode_to_other_id;

  // Default: unclassified (-1). PV particles get type 0 through the
  // actual PV vertex traversal below. Only fakes (-3) and pileup (-2)
  // use other negative values.
  for ( const auto& truth_particle : sorted_truth_particles ) {
    vertexValid(*truth_particle) = 0;
    vertexType(*truth_particle) = -1;
    vertexSimpleType(*truth_particle) = -1;
    vertexDecayID(*truth_particle) = -1;
    vertexDecayParentID(*truth_particle) = -1;
    vertexDecayPVDistance(*truth_particle) = NAN;
    vertexDecayDistance(*truth_particle) = NAN;
    vertexDecayNCharged(*truth_particle) = -1;
    vertexDecayNNeutral(*truth_particle) = -1;
    vertexDecayAllNCharged(*truth_particle) = -1;
    vertexDecayAllNNeutral(*truth_particle) = -1;
    vertexIsVertex(*truth_particle) = 0;
  }
  // Iterate all the vertices, defining each based on the counter
  std::unordered_map<int, int> barcode_to_vertex_id;
  int counter = 0;
  for ( auto fat_vertex : hpt_fat_vertices ) {
    for ( auto part : fat_vertex.internal ) {
        barcode_to_other_id[m_acc_uid(*part)] = counter;
        vertexDecayID(*part) = counter;
    }
    for ( auto part : fat_vertex.inparts ) {

        FatVertex::VertexType vt = fat_vertex.getType(m_acc_uid);
        // This is the PV, as we are writing the 'input' particles of the PV, this doesn't really
        // make sense, e.g. this could be argued as one/both protons, but more likely its a gluon
        // and so not a particle we want to write.
        if(vt.detailedType == FatVertex::DetailedVertexType::PrimaryVertex){
          continue;
        }

        vertexValid(*part) = 1;
        vertexType(*part) = int(vt.detailedType);
        vertexSimpleType(*part) = int(vt.getSimpleType());
        vertexDecayID(*part) = counter;
        // The number of neutral and charged decay products at truth level -> excludes neutrinos
        vertexDecayNCharged(*part) = fat_vertex.getNumChargedDecays(m_truthParticleMinimumPt);
        vertexDecayNNeutral(*part) = fat_vertex.getNumNeutralDecays(m_truthParticleMinimumPt);
        vertexDecayAllNCharged(*part) = fat_vertex.getNumChargedDecays();
        vertexDecayAllNNeutral(*part) = fat_vertex.getNumNeutralDecays();
        // Any inpart of a created fat vertex is a vertex
        vertexIsVertex(*part) = 1;
        barcode_to_other_id[m_acc_uid(*part)] = counter;
    }

    for ( auto part : fat_vertex.outparts ) {
      if (!part){
        ATH_MSG_ERROR("Found a null particle in the out parts of a fat vertex, skipping it - this should never happen");
        continue;
      }

      vertexDecayParentID(*part) = counter;
      barcode_to_vertex_id[m_acc_uid(*part)] = counter;

      // Outparts don't inherit their parent's vertex type — they're
      // truth particles, not vertices. Their parent is recorded in
      // vertexDecayParentID. If an outpart decays, it becomes an inpart
      // of its own vertex and gets its type there.

      // If this particle has a decay vertex, then we label its PV-Lxy
      if ( part->decayVtx() && FatVertex::has_valid_child(part)) {
        float pvDist = FatVertex::vertex_distance(part->decayVtx(), pv);
        vertexDecayPVDistance(*part) = pvDist;
        // Production→decay flight distance
        if ( part->hasProdVtx() && part->prodVtx() ) {
          vertexDecayDistance(*part) = FatVertex::vertex_distance(part->prodVtx(), part->decayVtx());
        }

        // If it doesn't, we add it to a a vector based on its charge for later use
      }else{
        // If we always keep charged/neutrals, then we do so here
        if (part->isCharged()){
          stable_charged.push_back(part);
          if(m_alwaysKeepStableCharged){
            vertexValid(*part) = 1;
          }
        }
        else{
          stable_neutrals.push_back(part);
          if(m_alwaysKeepStableNeutrals){
            vertexValid(*part) = 1;
          }
        }
      }
    }

    counter++;
  }

  // Get all tracks so we can decorate them
  SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_TrackContainerKey, ctx);
  ATH_CHECK(tracks.isValid());
  std::vector<const xAOD::TrackParticle*> tracks_vector(tracks->begin(), tracks->end());

  SG::ReadDecorHandle<xAOD::TruthParticleContainer, int> acc_truth_id(m_acc_truth_particle_vertex_id, ctx);

  using TrackIntWriter = SG::WriteDecorHandle<xAOD::TrackParticleContainer, int>;
  TrackIntWriter trackTPDecayVertexID(m_trackTPDecayVertexID, ctx);
  TrackIntWriter trackTPDecayVertexType(m_trackTPDecayVertexType, ctx);
  TrackIntWriter trackTPDecaySimpleVertexType(m_trackTPDecaySimpleVertexType, ctx);
  TrackIntWriter trackPDGID(m_trackPDGID, ctx);
  TrackIntWriter trackParentPDGID(m_trackParentPDGID, ctx);

  for ( auto track : tracks_vector ) {
    // Default: unclassified. PV tracks get type 0 via barcode lookup.
    // Fakes = -3, pileup = -2, unclassified = -1.
    trackTPDecayVertexID(*track) = -1;
    trackTPDecayVertexType(*track) = -1;
    trackTPDecaySimpleVertexType(*track) = -1;
    trackPDGID(*track) = 0;
    trackParentPDGID(*track) = 0;
    const auto truth = m_trackTruthOriginTool->getTruth(track);
    // No truth match → pileup
    if ( !truth ){
        trackTPDecayVertexType(*track) = -2;
        trackTPDecaySimpleVertexType(*track) = -2;
        continue;
    }
    float truthProb = m_truthMatchProbabilityAcc(*track);

    if(truthProb < m_truthMatchProbabilityCut){
        // Fake track
        trackTPDecayVertexType(*track) = -3;
        trackTPDecaySimpleVertexType(*track) = -3;
        continue;
    }
    // If we aren't *always* keeping stable charged TP, but we're keeping them whenever
    // there's no associated track, then here we remove anything matched from out
    // stable charged particle vector, so we know to only write those that remain
    if(!m_alwaysKeepStableCharged && m_alwaysKeepTracklessStableCharged){
      stable_charged.erase(
        std::remove(stable_charged.begin(), stable_charged.end(), truth),
        stable_charged.end()
      );
    }
    trackPDGID(*track) = truth->pdgId();

    if ( truth->hasProdVtx() && truth->prodVtx() ) {
      auto parent = truth->prodVtx()->incomingParticle(0);
      if ( parent ) {
        trackParentPDGID(*track) = parent->pdgId();
      }
    }

    // Get vertex ID from barcode
    if ( barcode_to_vertex_id.find(m_acc_uid(*truth)) != barcode_to_vertex_id.end() ) {
      int vertex_id = barcode_to_vertex_id[m_acc_uid(*truth)];
      trackTPDecayVertexID(*track) = vertex_id;
      FatVertex::VertexType vt = hpt_fat_vertices[vertex_id].getType(m_acc_uid);
      trackTPDecayVertexType(*track) = int(vt.detailedType);
      trackTPDecaySimpleVertexType(*track) = int(vt.getSimpleType());
    } else {
      // Lets go for a different default value for debug purposes
      if(barcode_to_other_id.find(m_acc_uid(*truth)) != barcode_to_other_id.end()){
        ATH_MSG_DEBUG("Found a track which is truth matched to an internal particle, not an out part ");
        int vertex_id = barcode_to_other_id[m_acc_uid(*truth)];
        trackTPDecayVertexID(*track) = vertex_id;
        FatVertex::VertexType vt = hpt_fat_vertices[vertex_id].getType(m_acc_uid);
        trackTPDecayVertexType(*track) = int(vt.detailedType);
        trackTPDecaySimpleVertexType(*track) = int(vt.getSimpleType());
      }else{
        // Truth-matched track not found in any fat vertex — default is PV (already set above).
        // This can happen for tracks from ISR/fragmentation or from truth particles whose
        // decay chain wasn't fully traversed.
      }

    }
  }
  // Now we set all charged stable TPs without an associated track valid so we can save them
  if(!m_alwaysKeepStableCharged && m_alwaysKeepTracklessStableCharged){
    for(auto stable_charged_tp : stable_charged){
      vertexValid(*stable_charged_tp) = 1;
    }

  }

  // TODO match ID hits to TP if they exist
  // TODO match flows to TP if possible (this is a bit more difficult)
  return StatusCode::SUCCESS;
}

} // End of ParticleJetTools namespace
