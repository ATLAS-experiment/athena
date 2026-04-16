/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PARTICLEJETTOOLS_FATVERTEX_H
#define PARTICLEJETTOOLS_FATVERTEX_H

#include <TLorentzVector.h>
#include <optional>

#include "AthContainers/AuxElement.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertexContainerFwd.h"

namespace ParticleJetTools{

    namespace FatVertex{

        TLorentzVector sum_4vec(const std::vector<const xAOD::TruthParticle*>& parts);
        float vertex_distance(const xAOD::TruthVertex* v1, const xAOD::TruthVertex* v2);
        float vertex_distance_xy(const xAOD::TruthVertex* v1, const xAOD::TruthVertex* v2);
        int num_valid_children(const xAOD::TruthParticle* part);
        bool has_valid_child(const xAOD::TruthParticle* part);
        std::vector<const xAOD::TruthParticle*> get_valid_children_by_pt(const xAOD::TruthParticle* part);

        // -1 represents invalid, -2 represents pileup, and -3 is fake
        enum class DetailedVertexType{
            PrimaryVertex,
            BHadronDecay,
            CHadronDecay,
            CHadronDecayFromBHadron,
            TauDecay,
            TauDecayFromBHadron,
            StrangeDecay,
            StrangeFromBHadron,
            StrangeFromCHadron,
            StrangeFromCFromBHadron,
            StrangeFromTau,
            StrangeFromTauFromBHadron,
            PionDecay,
            MaterialInteraction,
            MaybeMaterialInteraction,
            StrangeOscillation,
            OtherSecondaryVertex,
            PhotoelectricEmission,
            ComptonScattering,
            EPAnnihilation,
            Bremsstrahlung,
            Conversion,
            Other,
            OtherNInparts,
            OtherNoOutparts,
            OtherSingleOutpart,
        };
        enum class SimpleVertexType{
          PrimaryVertex,
          BHadronDecay,
          CHadronDecay,
          TauDecay,
          StrangeDecay,
          PionDecay,
          MaterialInteraction,
          OtherSecondaryVertex,
          Other,
      };

      struct VertexType{
        DetailedVertexType detailedType;
        VertexType(DetailedVertexType dt) : detailedType(dt) {}
        SimpleVertexType getSimpleType() const {
          switch(detailedType){

            case DetailedVertexType::PrimaryVertex:
              return SimpleVertexType::PrimaryVertex;

            case DetailedVertexType::BHadronDecay:
              return SimpleVertexType::BHadronDecay;

            case DetailedVertexType::CHadronDecay:
            case DetailedVertexType::CHadronDecayFromBHadron:
              return SimpleVertexType::CHadronDecay;

            case DetailedVertexType::TauDecay:
            case DetailedVertexType::TauDecayFromBHadron:
              return SimpleVertexType::TauDecay;

            case DetailedVertexType::StrangeDecay:
            case DetailedVertexType::StrangeOscillation:
            case DetailedVertexType::StrangeFromBHadron:
            case DetailedVertexType::StrangeFromCHadron:
            case DetailedVertexType::StrangeFromCFromBHadron:
            case DetailedVertexType::StrangeFromTau:
            case DetailedVertexType::StrangeFromTauFromBHadron:
              return SimpleVertexType::StrangeDecay;

            case DetailedVertexType::PionDecay:
              return SimpleVertexType::PionDecay;

            case DetailedVertexType::MaterialInteraction:
            case DetailedVertexType::MaybeMaterialInteraction:
            case DetailedVertexType::PhotoelectricEmission:
            case DetailedVertexType::ComptonScattering:
            case DetailedVertexType::EPAnnihilation:
            case DetailedVertexType::Bremsstrahlung:
            case DetailedVertexType::Conversion:
              return SimpleVertexType::MaterialInteraction;

            case DetailedVertexType::Other:
            case DetailedVertexType::OtherNInparts:
            case DetailedVertexType::OtherNoOutparts:
            case DetailedVertexType::OtherSingleOutpart:
              return SimpleVertexType::Other;
            default:
              return SimpleVertexType::Other;

          };
        }
      };

        struct FatVertex{
            std::vector<const xAOD::TruthParticle*> inparts;
            std::vector<const xAOD::TruthParticle*> internal;
            std::vector<const xAOD::TruthParticle*> outparts;
            bool is_pv;
            const FatVertex* parent=nullptr;
            std::vector<const FatVertex*> children;

            FatVertex(std::vector<const xAOD::TruthParticle*> in_parts, std::vector<const xAOD::TruthParticle*> intern, std::vector<const xAOD::TruthParticle*> out_parts, bool pv) :
              inparts(std::move(in_parts)), internal(std::move(intern)),
              outparts(std::move(out_parts)), is_pv(pv) {}

            float getPT() const;
            void doParentChildLinks(std::vector<FatVertex>& vertices);
            int getNumChargedDecays(float minPt=0.0) const;
            int getNumNeutralDecays(float minPt=0.0, bool include_neutrinos=false) const;
            VertexType getType(
              const SG::ConstAccessor<int>& uid_accessor
            ) const;

            bool operator<(const FatVertex& other) const;
            bool operator==(const FatVertex& other) const;

          private:
            std::optional<DetailedVertexType> classifyTauVertex(
                DetailedVertexType parent_type, bool has_parent) const;
            std::optional<DetailedVertexType> classifyStrangeVertex(
                DetailedVertexType parent_type, bool has_parent) const;
            std::optional<DetailedVertexType> classifyPhotonVertex(
                size_t truth_out_parts, size_t truth_valid_out_parts,
                const std::vector<const xAOD::TruthParticle*>& truth_children) const;
            std::optional<DetailedVertexType> classifyLeptonVertex(
                const SG::ConstAccessor<int>& uid_accessor,
                size_t truth_out_parts, size_t truth_valid_out_parts,
                const std::vector<const xAOD::TruthParticle*>& truth_children) const;
        };
        void generateFatVertex(
            const xAOD::TruthVertex* vertex,
            std::vector<const xAOD::TruthParticle*>& fat_inparts,
            std::vector<const xAOD::TruthParticle*>& fat_internal,
            std::vector<const xAOD::TruthParticle*>& fat_outparts,
            bool internal,
            const float truthVertexMergeDistance,
            const std::vector<const xAOD::TruthParticle*>& additional_in_parts = std::vector<const xAOD::TruthParticle*>()
          ) ;
        std::vector<FatVertex> generateFatVertices(
          const xAOD::TruthVertex* pv,
          const float truthVertexMergeDistance,
          const std::vector<const xAOD::TruthParticle*>& truth_particles
        );
    } // End of FatVertex namespace
} // End of ParticleJetTools namespace

#endif // PARTICLEJETTOOLS_FATVERTEX_H
