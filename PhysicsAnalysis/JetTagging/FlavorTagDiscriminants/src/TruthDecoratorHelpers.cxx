/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"


namespace FlavorTagDiscriminants {
    namespace TruthDecoratorHelpers {

        bool sort_particles(const xAOD::IParticle* particle_A, 
                            const xAOD::IParticle* particle_B) {
            return particle_A->pt() < particle_B->pt();
        }

        const xAOD::TruthVertex* get_truth_vertex(
            const xAOD::TruthParticle* truth) {
            // no truth
            if ( not truth ) { return nullptr; }

            // no vertex
            const xAOD::TruthVertex* truth_vertex = truth->prodVtx();
            if ( not truth_vertex || truth_vertex->perp() > 440.0 ) {
                return nullptr;
            }

            return truth_vertex;
        }

        float get_distance(const xAOD::TruthVertex* vertex_A, 
                           const xAOD::TruthVertex* vertex_B) {
            if ( !vertex_A || !vertex_B ) { return 999.0; }
            return (vertex_A->v4().Vect() - vertex_B->v4().Vect()).Mag();
        }

        bool is_bc_hadron(const xAOD::TruthParticle* truth_particle, int flavour) {
            if( flavour == 5 && truth_particle->isBottomHadron() ) { return true; }
            if( flavour == 4 && truth_particle->isCharmHadron()  ) { return true; }
            return false;
        }
        
        bool is_weakly_decaying_hadron(const xAOD::TruthParticle* truth_particle, int flavour) {
            if (is_bc_hadron(truth_particle, flavour)) {
                if ( not truth_particle->hasDecayVtx() ) { return false; }
                const auto vx = truth_particle->decayVtx();
                for ( size_t i = 0; i < vx->nOutgoingParticles(); i++ ) {
                    const auto out_part = vx->outgoingParticle(i);
                    if ( is_bc_hadron(out_part, flavour) ) { return  false; }
                } 
                return true;
            }
            return false;
        }
        
        bool is_weakly_decaying_hadron(const xAOD::TruthParticle* truth_particle) {
            return is_weakly_decaying_hadron(truth_particle, 5) || is_weakly_decaying_hadron(truth_particle, 4);
        }

        const xAOD::TruthParticle* get_parent_hadron(const xAOD::TruthParticle* truth_particle, bool user_called) {
            // get the weakly decaying parent hadron of truth_particle
            if ( truth_particle == nullptr ) { return nullptr; }
            if ( !user_called && is_weakly_decaying_hadron(truth_particle) )  {
                return truth_particle;
            }
            for (unsigned int p = 0; p < truth_particle->nParents(); p++) {
                const auto parent = truth_particle->parent(p);
		if(parent == truth_particle) continue;// avoid infinite recursion
                const auto parent_hadron = get_parent_hadron(parent, false);
                if ( parent_hadron != nullptr ) {
                    return parent_hadron;
                }
            }
            return nullptr;
        }

        int get_truth_type(const xAOD::TruthParticle* truth_particle) {
            if (!truth_particle) {
                return TruthType::Label::NoTruth;
            }
            // simple pdgid check for pion based on
            // PhysicsAnalysis/MCTruthClassifier/Root/MCTruthClassifierGen.cxx#L1159
            if (std::abs(truth_particle->pdgId()) == 211) {
                return TruthType::Label::Pion * truth_particle->charge();
            }
            else if (truth_particle->isStrangeMeson()) {
                return TruthType::Label::Kaon * truth_particle->charge();
            }
            if (std::abs(truth_particle->pdgId()) == 3122) {
                return TruthType::Label::Lambda;
            }
            else if (truth_particle->isElectron()) {
                return TruthType::Label::Electron * truth_particle->charge() * -1;
            }
            else if (truth_particle->isMuon()) {
                return TruthType::Label::Muon * truth_particle->charge() * -1;
            }
            else if (truth_particle->isPhoton()) {
                return TruthType::Label::Photon;
            }
            return TruthType::Label::Other;
        }

        int get_source_type(const int origin) {
            /* this label gives information about the origin of secondary
            particles (material interactions, gamme conversions, etc.)*/

            if (!(InDet::TrkOrigin::isSecondary(origin))){
	        return TruthSource::Label::NotSecondary;
	        }
            else if ( InDet::TrkOrigin::isHadronicInteraction(origin) ) {
                return TruthSource::Label::HadronicInteraction;
            }
            else if ( InDet::TrkOrigin::isKshortDecay(origin) ) {
                return TruthSource::Label::KshortDecay;
            }
            else if ( InDet::TrkOrigin::isLambdaDecay(origin) ) {
                return TruthSource::Label::LambdaDecay;
            }
            else if ( InDet::TrkOrigin::isGammaConversion(origin) ) {
                return TruthSource::Label::GammaConversion;
            }
	    // For simulation tracks not included in the above categories
            return TruthSource::Label::Other;
        }

        int get_vertex_index(const xAOD::TruthVertex* this_vertex,
                             const xAOD::TruthVertex* truth_PV,
                             std::vector<const xAOD::TruthVertex*>& seen_vertices,
                             const float truthVertexMergeDistance) {
            // no vertex
            if (!this_vertex) {
                return -2;
            }
            // primary vertex
            if (get_distance(this_vertex, truth_PV) < truthVertexMergeDistance) {
                return 0;
            }
            // have we already seen this vertex?
            for ( size_t i = 0; i != seen_vertices.size(); i++) {
                float dr = get_distance(seen_vertices.at(i), this_vertex);
                if ( dr < truthVertexMergeDistance ) {
                    // a vertex is nearby, reuse it
                    return i + 1;
                }
            }
            seen_vertices.push_back(this_vertex);
            return seen_vertices.size();
        }
    }
}
