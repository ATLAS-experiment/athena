/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ParticleJetTools/FatVertex.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include <stdexcept>

namespace ParticleJetTools {
    namespace FatVertex {

        TLorentzVector sum_4vec(const std::vector<const xAOD::TruthParticle*>& parts) {
          TLorentzVector sum;
          for ( auto part : parts ) {
            if ( !part ) continue;
            sum += part->p4();
          }
          return sum;
        }

        int num_valid_children(const xAOD::TruthParticle* part) {
            if ( part->nChildren() == 0 ) { return 0; }
            if ( !part->hasDecayVtx() || !part->decayVtx() ) { return 0; }
            int num_valid = 0;
            for ( auto child : part->decayVtx()->particles_out() ) {
              if ( child ) num_valid++;
            }
            return num_valid;
        }

        bool has_valid_child(const xAOD::TruthParticle* part) {
            if ( part->nChildren() == 0 ) { return false; }
            if ( !part->hasDecayVtx() || !part->decayVtx() ) { return false; }
            for ( auto child : part->decayVtx()->particles_out() ) {
              if ( child ) return true;
            }
            return false;
        }

        std::vector<const xAOD::TruthParticle*> get_valid_children_by_pt(const xAOD::TruthParticle* part) {
            std::vector<const xAOD::TruthParticle*> children;
            if ( !part->hasDecayVtx() || !part->decayVtx() ) return children;
            for ( auto child : part->decayVtx()->particles_out() ) {
              if ( child ) children.push_back(child);
            }
            std::sort(children.begin(), children.end(),
                [](const xAOD::TruthParticle* a, const xAOD::TruthParticle* b) { return a->pt() > b->pt(); });
            return children;
        }

        float vertex_distance(const xAOD::TruthVertex* v1, const xAOD::TruthVertex* v2) {
            if ( !v1 || !v2 ) return NAN;
            return std::sqrt(
                std::pow(v1->x() - v2->x(), 2) + std::pow(v1->y() - v2->y(), 2) +
                std::pow(v1->z() - v2->z(), 2)
            );
        }

        float vertex_distance_xy(const xAOD::TruthVertex* v1, const xAOD::TruthVertex* v2){
            if ( !v1 || !v2 ) return NAN;
            return std::sqrt(
                std::pow(v1->x() - v2->x(), 2) + std::pow(v1->y() - v2->y(), 2)
            );
        }

        /// @brief Generates a fat vertex by recursively searching children of the provided
        /// truth vertex and adding them to either the in parts, internal parts, or out parts.
        /// @param vertex The truth vertex to search truth particles from
        /// @param fat_inparts The vector of incoming particles to the fat vertex
        /// @param fat_internal The vector of internal particles to the fat vertex
        /// @param fat_outparts The vector of outgoing particles to the fat vertex
        /// @param internal Whether the current vertex is internal to the process
        /// @param truthVertexMergeDistance The distance in mm to merge two vertices
        /// @param additional input particles which come from the PV
        void generateFatVertex(
            const xAOD::TruthVertex* vertex,
            std::vector<const xAOD::TruthParticle*>& fat_inparts,
            std::vector<const xAOD::TruthParticle*>& fat_internal,
            std::vector<const xAOD::TruthParticle*>& fat_outparts,
            bool internal,
            const float truthVertexMergeDistance,
            const std::vector<const xAOD::TruthParticle*>& additional_in_parts
        ) {
          if ( !vertex ) return;
          std::vector<const xAOD::TruthParticle*> out_particles = vertex->particles_out();
          std::vector<const xAOD::TruthParticle*> in_particles = vertex->particles_in();

          for(auto part : additional_in_parts){
            if(part){
              if (std::find(in_particles.begin(), in_particles.end(), part) == in_particles.end()){
                in_particles.push_back(part);
              }

              if(part->hasDecayVtx() && part->decayVtx()){
              for(auto child : part->decayVtx()->particles_out()){
                if(child && std::find(out_particles.begin(), out_particles.end(), child) == out_particles.end()){
                  out_particles.push_back(child);
                }
              }
            }
            }
          }
          for ( auto part : in_particles ) {
              if ( !part ) continue;
              // Check that the current particle being checked is not already in the fat vertex
              if ( std::find(fat_inparts.begin(), fat_inparts.end(), part) == fat_inparts.end()
                && std::find(fat_internal.begin(), fat_internal.end(), part) == fat_internal.end()
                && std::find(fat_outparts.begin(), fat_outparts.end(), part) == fat_outparts.end() ) {
              // If 'internal' is false, we treat any 'internal' parts of a process as inputs
              if ( internal ) fat_internal.push_back(part);
              else fat_inparts.push_back(part);
              }
          }
          for ( auto part : out_particles ) {
            if ( !part ) continue;
            if (
              std::find(fat_inparts.begin(), fat_inparts.end(), part) != fat_inparts.end() ||
              std::find(fat_internal.begin(), fat_internal.end(), part) != fat_internal.end() ||
              std::find(fat_outparts.begin(), fat_outparts.end(), part) != fat_outparts.end()
            ) continue;
            // Simulation particles (status != 1,2) without a usable decay vertex:
            // add as outparts (they're stable children of the current vertex).
            // Sim particles WITH a decay vertex fall through to normal processing
            // (merge distance check, single-child skip, outpart logic).
            if (!(part->status() == 1 || part->status() == 2)){
              if (!part->hasDecayVtx() || !part->decayVtx() || !has_valid_child(part)) {
                fat_outparts.push_back(part);
                continue;
              }
            }

            // We have a particle which says it has a child, but its invalid -> end state particle,
            // ->we add it to the outputs
            if ( !has_valid_child(part) || !(part->hasDecayVtx() && part->decayVtx()) ) {
              fat_outparts.push_back(part);
              continue;
            }

            if(
            !fat_inparts.empty() &&
            part->hasDecayVtx() &&  part->decayVtx() &&
            fat_inparts[0]->hasDecayVtx() && fat_inparts[0]->decayVtx() &&
            vertex_distance(part->decayVtx(), fat_inparts[0]->decayVtx()) < truthVertexMergeDistance
            ){
              generateFatVertex(
                  part->decayVtx(), fat_inparts, fat_internal, fat_outparts, true, truthVertexMergeDistance
                );
                continue;
            }

            // Check valid children sorted by pT
            std::vector<const xAOD::TruthParticle*> valid_children = get_valid_children_by_pt(part);

            // Single valid child with same pdgId = simple scattering (e- -> e-, pi -> pi)
            // Not a reconstructable vertex — absorb as internal and continue
            if ( valid_children.size() == 1 &&
                 valid_children[0]->pdgId() == part->pdgId() ) {
                generateFatVertex(
                    part->decayVtx(), fat_inparts, fat_internal, fat_outparts, true,
                    truthVertexMergeDistance
                );
                continue;
            }

            // Multiple children but highest-pT has same pdgId = brem/scattering + radiation
            // (e- -> e- + gamma). The leading child is the same particle continuing —
            // absorb as internal so the vertex is attributed to where it actually interacts
            if ( valid_children.size() >= 2 &&
                 valid_children[0]->pdgId() == part->pdgId() ) {
                generateFatVertex(
                    part->decayVtx(), fat_inparts, fat_internal, fat_outparts, true,
                    truthVertexMergeDistance
                );
                continue;
            }

            // Strange hadron oscillation (K0 -> K0bar etc)
            if ( valid_children.size() == 1 &&
                 part->isStrangeHadron() && valid_children[0]->isStrangeHadron() ) {
                generateFatVertex(
                    part->decayVtx(), fat_inparts, fat_internal, fat_outparts, true,
                    truthVertexMergeDistance
                );
                continue;
            }
            fat_outparts.push_back(part);
          }
        }

        std::vector<FatVertex> generateFatVertices(
            const xAOD::TruthVertex* pv,
            const float truthVertexMergeDistance,
            const std::vector<const xAOD::TruthParticle*>& truth_particles
        ) {
          std::vector<FatVertex> fat_vertices;
          std::vector<const xAOD::TruthParticle*> searched;
          std::vector<const xAOD::TruthParticle*> to_search;

          to_search.push_back(pv->incomingParticle(0));

          bool is_pv = true;
          while ( !to_search.empty() ) {
              const xAOD::TruthParticle* part = to_search.back();

              to_search.pop_back();
              searched.push_back(part);
              if (!part || !part->hasDecayVtx() ) continue;
              // If the particles first child is invalid, then all are, and we can skip it...
              if(num_valid_children(part) == 0){
                continue;
              }
              std::vector<const xAOD::TruthParticle*> inparts, internal, outparts;
              generateFatVertex(
                  part->decayVtx(), inparts, internal, outparts,
                  false, truthVertexMergeDistance,
                  is_pv ? truth_particles : std::vector<const xAOD::TruthParticle*>(0, nullptr)
              );
              // A vertex requires multiple outgoing particles. Accept if either:
              // - multiple outparts survived thinning, OR
              // - the inpart's nChildren >= 2 (truth record says real multi-body
              //   decay/interaction, even if some children were thinned)
              bool is_real_vertex = outparts.size() > 1;
              if (!is_real_vertex && !inparts.empty() && inparts[0] && inparts[0]->nChildren() >= 2) {
                is_real_vertex = true;
              }
              if ( is_real_vertex ){
                  fat_vertices.push_back(FatVertex(inparts, internal, outparts, is_pv));
                  is_pv = false;
              }

              // Once a fat-vertex is completed, we ensure we don't search any of its particles
              // again
              for ( auto inp : inparts ) {
                if ( std::find(searched.begin(), searched.end(), inp) == searched.end() ) {
                  searched.push_back(inp);
                }
                if(std::find(to_search.begin(), to_search.end(), inp) != to_search.end()){
                  to_search.erase(
                    std::remove(to_search.begin(), to_search.end(), inp),
                    to_search.end());
                }
              }
              for ( auto intern : internal ) {
                if ( std::find(searched.begin(), searched.end(), intern) == searched.end() ) {
                  searched.push_back(intern);
                }
                if(std::find(to_search.begin(), to_search.end(), intern) != to_search.end()){
                  to_search.erase(
                    std::remove(to_search.begin(), to_search.end(), intern),
                    to_search.end());
                }
              }
              for ( auto outp : outparts ) {
                  if ( std::find(searched.begin(), searched.end(), outp) == searched.end()
                    && std::find(to_search.begin(), to_search.end(), outp) == to_search.end() ) {
                    to_search.push_back(outp);
                  }
              }
          }

          // Iterate all fat vertices, and remove any with 0 out particles, as these are actually
          // just stable particles
          fat_vertices.erase(
            std::remove_if(
                fat_vertices.begin(),
                fat_vertices.end(),
                [](const FatVertex& fv) {
                    return fv.outparts.empty();
                }
            ),
            fat_vertices.end()
        );

          return fat_vertices;
        }


        float FatVertex::getPT() const {
          if ( inparts.size() == 1 ) { return inparts[0]->pt(); }
          return sum_4vec(inparts).Pt();
        }

        int FatVertex::getNumChargedDecays(float minPt) const {
          int num_charged = 0;
          for ( auto part : outparts ) {
            if ( part->isCharged() && part->pt() >= minPt ) num_charged++;
          }
          return num_charged;
        }

        int FatVertex::getNumNeutralDecays(float minPt, bool include_neutrinos) const {
          int num_neutral = 0;
          for ( auto part : outparts ) {
            if ( part->isNeutral() && part->pt() >= minPt && (include_neutrinos || !part->isNeutrino()) )
              num_neutral++;
          }
          return num_neutral;
        }

        VertexType FatVertex::getType(
            const SG::ConstAccessor<int>& uid_accessor
        ) const {
          if ( is_pv ) return DetailedVertexType::PrimaryVertex;
          if ( inparts.size() != 1 ) return DetailedVertexType::OtherNInparts;
          if ( outparts.empty() ) return DetailedVertexType::OtherNoOutparts;

          // Get first valid child
          const xAOD::TruthParticle* input_child = nullptr;
          for (auto child : inparts[0]->decayVtx()->particles_out()) {
            if (child) { input_child = child; break; }
          }
          if ( !input_child ) return DetailedVertexType::Other;

          bool has_parent = parent != nullptr;
          DetailedVertexType parent_type = has_parent
              ? parent->getType(uid_accessor).detailedType
              : DetailedVertexType::Other;

          // Heavy flavor: B and C hadron decays
          if ( inparts[0]->isBottomHadron() ) return DetailedVertexType::BHadronDecay;
          if ( inparts[0]->isCharmHadron() ) {
            if (has_parent && parent_type == DetailedVertexType::BHadronDecay)
              return DetailedVertexType::CHadronDecayFromBHadron;
            return DetailedVertexType::CHadronDecay;
          }

          // Tau decay (can fall through if it's tau bremsstrahlung that doesn't match)
          if ( inparts[0]->isTau() ) {
            if (auto t = classifyTauVertex(parent_type, has_parent)) return *t;
          }

          // Compute shared context for remaining classifiers
          size_t truth_out_parts = inparts[0]->nChildren();
          size_t truth_valid_out_parts = num_valid_children(inparts[0]);
          std::vector<const xAOD::TruthParticle*> truth_children;
          for ( auto child : inparts[0]->decayVtx()->particles_out() ) {
            if ( child ) truth_children.push_back(child);
          }
          TLorentzVector in4 = inparts[0]->p4();
          TLorentzVector out4 = sum_4vec(inparts[0]->decayVtx()->particles_out());
          float deltaE = std::abs(in4.E() - out4.E());

          // Energy-based material interaction (all valid children, large energy loss)
          if ( truth_out_parts == truth_valid_out_parts && truth_valid_out_parts > 1 ) {
            if ( deltaE > 100.0 ) return DetailedVertexType::MaterialInteraction;
          }

          // Mass-based material interaction: if the sum of children rest masses
          // exceeds the parent rest mass, mass was created from kinetic energy
          // via nuclear interaction with detector material. In a genuine decay,
          // parent >= sum(m_children) always holds by conservation.
          if ( truth_valid_out_parts > 1 ) {
            double children_mass_sum = 0.0;
            for (const auto& child : truth_children) {
              children_mass_sum += child->p4().M();
            }
            if ( children_mass_sum > in4.M() + 1.0 ) {  // 1 MeV tolerance
              return DetailedVertexType::MaterialInteraction;
            }
          }

          // Strange hadron
          if ( inparts[0]->isStrangeHadron() ) {
            if (auto t = classifyStrangeVertex(parent_type, has_parent)) return *t;
          }

          // Photon (conversion, photoelectric, compton)
          if ( inparts[0]->isPhoton() ) {
            if (auto t = classifyPhotonVertex(truth_out_parts, truth_valid_out_parts, truth_children)) return *t;
          }

          // Charged lepton (bremsstrahlung, e+e- annihilation, material interaction)
          if ( inparts[0]->isChLepton() ) {
            if (auto t = classifyLeptonVertex(uid_accessor, truth_out_parts, truth_valid_out_parts, truth_children)) return *t;
          }

          // Pion decay: pi0 -> gamma gamma is always a decay.
          // Charged pi -> mu + nu: check for muon+neutrino signature.
          // Anything else from a charged pion (hadronic products, nuclear
          // fragments) is a material interaction.
          if ( std::abs(inparts[0]->pdgId()) == 111 ) {
            return DetailedVertexType::PionDecay;
          }
          if ( std::abs(inparts[0]->pdgId()) == 211 ) {
            bool has_muon = false, has_neutrino = false;
            for (auto c : truth_children) {
              if (std::abs(c->pdgId()) == 13) has_muon = true;
              if (std::abs(c->pdgId()) == 14) has_neutrino = true;
            }
            if (has_muon && has_neutrino) return DetailedVertexType::PionDecay;
            return DetailedVertexType::MaterialInteraction;
          }

          // Fallback: simulation particle with missing children
          if ( truth_out_parts > truth_valid_out_parts && truth_out_parts > 1 &&
               inparts[0]->child(0) && HepMC::is_simulation_particle(inparts[0]->child(0)) ) {
            return DetailedVertexType::MaterialInteraction;
          }

          // Single output particle fallbacks
          if ( truth_out_parts == 1 && truth_valid_out_parts == 1 ) {
            if ( deltaE > 10 ) return DetailedVertexType::MaybeMaterialInteraction;
            return DetailedVertexType::OtherSingleOutpart;
          }

          return DetailedVertexType::Other;
        }

        std::optional<DetailedVertexType> FatVertex::classifyTauVertex(
            DetailedVertexType parent_type, bool has_parent
        ) const {
          int num_child_taus = 0;
          int num_child_photons = 0;
          for ( auto child : outparts ) {
            if ( child->isTau() ) num_child_taus++;
            else if ( child->isPhoton() ) num_child_photons++;
          }
          if ( num_child_taus == 0 ) {
            if (has_parent && parent_type == DetailedVertexType::BHadronDecay)
              return DetailedVertexType::TauDecayFromBHadron;
            return DetailedVertexType::TauDecay;
          }
          if ( num_child_taus == 1 && num_child_photons == 1 )
            return DetailedVertexType::Bremsstrahlung;
          return std::nullopt;
        }

        std::optional<DetailedVertexType> FatVertex::classifyStrangeVertex(
            DetailedVertexType parent_type, bool has_parent
        ) const {
          if ( outparts.size() == 1 && outparts[0]->isStrangeHadron() )
            return DetailedVertexType::StrangeOscillation;
          if ( !has_parent ) return DetailedVertexType::StrangeDecay;
          switch (parent_type) {
            case DetailedVertexType::BHadronDecay:
              return DetailedVertexType::StrangeFromBHadron;
            case DetailedVertexType::CHadronDecay:
              return DetailedVertexType::StrangeFromCHadron;
            case DetailedVertexType::TauDecay:
              return DetailedVertexType::StrangeFromTau;
            case DetailedVertexType::CHadronDecayFromBHadron:
              return DetailedVertexType::StrangeFromCFromBHadron;
            case DetailedVertexType::TauDecayFromBHadron:
              return DetailedVertexType::StrangeFromTauFromBHadron;
            default:
              return DetailedVertexType::StrangeDecay;
          }
        }

        std::optional<DetailedVertexType> FatVertex::classifyPhotonVertex(
            size_t truth_out_parts, size_t truth_valid_out_parts,
            const std::vector<const xAOD::TruthParticle*>& truth_children
        ) const {
          // Gamma -> e+e- : Conversion
          if ( outparts.size() == 2 && outparts[0]->isChLepton() && outparts[1]->isChLepton() &&
               (outparts[0]->pdgId() + outparts[1]->pdgId()) == 0 ) {
            return DetailedVertexType::Conversion;
          }
          // Conversion from truth children (handles cases where fat vertex merging changed outparts)
          if ( truth_valid_out_parts == 2 &&
               (truth_children[0]->pdgId() + truth_children[1]->pdgId()) == 0 ) {
            return DetailedVertexType::Conversion;
          }
          // Photon -> single electron: photoelectric emission
          if ( std::abs(outparts[0]->pdgId()) == 11 ) {
            return DetailedVertexType::PhotoelectricEmission;
          }
          // Conversion where one lepton was absorbed by material
          if ( outparts.size() == 1 && internal.size() == 1 &&
               internal[0]->isChLepton() && outparts[0]->isChLepton() ) {
            return DetailedVertexType::Conversion;
          }
          // Conversion with missing truth record for one lepton
          if ( outparts.size() == 1 && internal.empty() && truth_out_parts == 2 &&
               truth_valid_out_parts == 1 && outparts[0]->isChLepton() ) {
            return DetailedVertexType::Conversion;
          }
          // gamma -> gamma + e : Compton scattering
          if ( outparts.size() == 2 &&
               ((outparts[0]->isPhoton() && outparts[1]->pdgId() == 11) ||
                (outparts[1]->isPhoton() && outparts[0]->pdgId() == 11)) ) {
            return DetailedVertexType::ComptonScattering;
          }
          // Catch-all for photon vertices with 3+ outparts containing electrons:
          // conversion + bremsstrahlung merged by fat vertex builder
          for (auto p : outparts) {
            if (p && p->isChLepton()) return DetailedVertexType::Conversion;
          }
          // Photon interaction with no electrons — generic material interaction
          return DetailedVertexType::MaterialInteraction;
        }

        std::optional<DetailedVertexType> FatVertex::classifyLeptonVertex(
            const SG::ConstAccessor<int>& uid_accessor,
            size_t truth_out_parts, size_t truth_valid_out_parts,
            const std::vector<const xAOD::TruthParticle*>& truth_children
        ) const {
          // Standard lep->lep+gamma : Bremsstrahlung
          if ( outparts.size() == 2 &&
               ((outparts[0]->isPhoton() && outparts[1]->isChLepton()) ||
                (outparts[1]->isPhoton() && outparts[0]->isChLepton())) ) {
            return DetailedVertexType::Bremsstrahlung;
          }
          // e->e+gamma but the electron doesn't stay in the truth record
          if ( (truth_out_parts == 2 || truth_out_parts == 1) && truth_valid_out_parts == 1 &&
               outparts[0]->isPhoton() ) {
            return DetailedVertexType::Bremsstrahlung;
          }
          // l -> gamma + l which is still brem
          if ( truth_valid_out_parts == 2 &&
               ((truth_children[0]->isPhoton() && truth_children[1]->pdgId() == inparts[0]->pdgId()) ||
                (truth_children[1]->isPhoton() && truth_children[0]->pdgId() == inparts[0]->pdgId())) ) {
            return DetailedVertexType::Bremsstrahlung;
          }
          // e+ e- annihilation: e+ -> gamma gamma
          if ( inparts[0]->pdgId() == -11 && outparts.size() == 2 &&
               outparts[0]->isPhoton() && outparts[1]->isPhoton() ) {
            return DetailedVertexType::EPAnnihilation;
          }
          // e+ -> gamma (boosted, partner low energy)
          if ( inparts[0]->pdgId() == -11 && outparts.size() == 1 &&
               outparts[0]->isPhoton() ) {
            return DetailedVertexType::EPAnnihilation;
          }
          // e+ -> child is single photon
          if ( inparts[0]->pdgId() == -11 && inparts[0]->nChildren() == 1 &&
               inparts[0]->child(0)->isPhoton() ) {
            return DetailedVertexType::EPAnnihilation;
          }
          // Lepton scattering via Geant (same base UID → material interaction)
          int in_base_bar = uid_accessor(*inparts[0]) % HepMC::SIM_REGENERATION_INCREMENT;
          if ( truth_valid_out_parts == 2 && truth_out_parts == 2 &&
               (in_base_bar == uid_accessor(*outparts[0]) % HepMC::SIM_REGENERATION_INCREMENT ||
                in_base_bar == uid_accessor(*outparts[1]) % HepMC::SIM_REGENERATION_INCREMENT) ) {
            return DetailedVertexType::MaterialInteraction;
          }
          return std::nullopt;
        }

        bool FatVertex::operator<(const FatVertex& other) const {
          if ( inparts.size() != other.inparts.size() )
            return inparts.size() < other.inparts.size();
          if ( internal.size() != other.internal.size() )
            return internal.size() < other.internal.size();
          if ( outparts.size() != other.outparts.size() )
            return outparts.size() < other.outparts.size();
          return getPT() < other.getPT();
        }

        bool FatVertex::operator==(const FatVertex& other) const {
          return this->inparts == other.inparts && this->internal == other.internal &&
                 this->outparts == other.outparts;
        }


        void FatVertex::doParentChildLinks(std::vector<FatVertex>& vertices) {
          // Iterates through all the vertices to set parent and children

          FatVertex* found_parent = nullptr;
          std::vector<const FatVertex*> found_children;

          for (auto& vertex2 : vertices) {
            if ( this == &vertex2) continue; // Compare addresses, not objects

            // Check if any of vertex2's outputs match vertex's inputs
            if (std::find(vertex2.outparts.begin(), vertex2.outparts.end(), inparts[0]) != vertex2.outparts.end()) {
              if (found_parent) {
                throw std::runtime_error("Multiple parents detected for a vertex");
              }
              found_parent = &vertex2;
            }

            // Check if vertex2 is a child
            if (std::find(outparts.begin(), outparts.end(), vertex2.inparts[0]) != outparts.end()) {
              found_children.push_back(&vertex2);
            }
          }

          parent = found_parent;
          children = std::move(found_children);
        }
      } // End of FatVertex namespace
} // End of ParticleJetTools namespace