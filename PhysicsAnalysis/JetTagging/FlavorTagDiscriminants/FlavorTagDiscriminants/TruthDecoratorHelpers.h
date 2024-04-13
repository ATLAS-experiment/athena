/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_DECORATOR_HELPERS_HH
#define TRUTH_DECORATOR_HELPERS_HH

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

namespace FlavorTagDiscriminants {
    namespace TruthDecoratorHelpers {
        namespace TruthType {
            enum Label {
                NoTruth  = 0,
                Other    = 1,
                Pion     = 2,
                Kaon     = 3,
                Lambda   = 4,
                Electron = 5,
                Muon     = 6,
                Photon   = 7
            };
        }
        namespace TruthSource {
            enum Label {
                NoTruth             = 0,
                NotSecondary        = 1,
                HadronicInteraction = 2,
                KshortDecay         = 3,
                LambdaDecay         = 4,
                GammaConversion     = 5,
                Other               = 6
            };
        }
        bool sort_particles(const xAOD::IParticle* particle_A, const xAOD::IParticle* particle_B);
        int get_truth_type(const xAOD::TruthParticle* truth_particle);
        int get_source_type(const int origin);
        
        bool is_bc_hadron(const xAOD::TruthParticle* truth_particle, int flavour);
        bool is_weakly_decaying_hadron(const xAOD::TruthParticle* truth_particle);
        bool is_weakly_decaying_hadron(const xAOD::TruthParticle* truth_particle, int flavour);
        const xAOD::TruthParticle* get_parent_hadron(const xAOD::TruthParticle* truth_particle, bool user_called=true, int depth=0);

        const xAOD::TruthVertex* get_truth_vertex(const xAOD::TruthParticle* truth );
        float get_distance(const xAOD::TruthVertex* vertex_A, const xAOD::TruthVertex* vertex_B);
        int get_vertex_index(const xAOD::TruthVertex* vertex, 
                             const xAOD::TruthVertex* truth_PV, 
                             std::vector<const xAOD::TruthVertex*>& seen_vertices,
                             const float truthVertexMergeDistance);
    }
}

#endif
