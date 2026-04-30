/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Decorates a jet with summary information about contained truth vertices
// See TruthVertexDecoratorAlg.h for more information

#include "ParticleJetTools/FatVertex.h"
#include "JetTruthVertexSummaryDecoratorAlg.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadDecorHandle.h"

#include "xAODBase/IParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"


namespace ParticleJetTools{
  JetTruthVertexSummaryDecoratorAlg::JetTruthVertexSummaryDecoratorAlg(
    const std::string& name, ISvcLocator* loc) : AthReentrantAlgorithm(name, loc) {}

    StatusCode JetTruthVertexSummaryDecoratorAlg::initialize(){
      ATH_MSG_INFO("Initializing " << name() << "... ");

      ATH_CHECK(m_TruthContainerKey.initialize());
      ATH_CHECK(m_JetContainerKey.initialize());

      m_vertexValid = m_TruthContainerKey.key() + "." + m_vertexValid.key();
      m_vertexSimpleDecayType = m_TruthContainerKey.key() + "." + m_vertexSimpleDecayType.key();
      m_vertexID = m_TruthContainerKey.key() + "." + m_vertexID.key();

      ATH_CHECK(m_vertexSimpleDecayType.initialize());
      ATH_CHECK(m_vertexValid.initialize());
      ATH_CHECK(m_vertexID.initialize());

      m_decoNumBVerts = m_JetContainerKey.key() + "." + m_decoNumBVerts.key();
      m_decoNumCVerts = m_JetContainerKey.key() + "." + m_decoNumCVerts.key();
      m_decoNumTauVerts = m_JetContainerKey.key() + "." + m_decoNumTauVerts.key();
      m_decoNumStrangeVerts = m_JetContainerKey.key() + "." + m_decoNumStrangeVerts.key();
      m_decoNumPionVerts = m_JetContainerKey.key() + "." + m_decoNumPionVerts.key();
      m_decoNumMaterialIntVerts = m_JetContainerKey.key() + "." + m_decoNumMaterialIntVerts.key();
      m_decoNumOtherVerts = m_JetContainerKey.key() + "." + m_decoNumOtherVerts.key();
      m_decoNumVerts = m_JetContainerKey.key() + "." + m_decoNumVerts.key();

      ATH_CHECK(m_decoNumBVerts.initialize());
      ATH_CHECK(m_decoNumCVerts.initialize());
      ATH_CHECK(m_decoNumTauVerts.initialize());
      ATH_CHECK(m_decoNumStrangeVerts.initialize());
      ATH_CHECK(m_decoNumPionVerts.initialize());
      ATH_CHECK(m_decoNumMaterialIntVerts.initialize());
      ATH_CHECK(m_decoNumOtherVerts.initialize());
      ATH_CHECK(m_decoNumVerts.initialize());

      return StatusCode::SUCCESS;
    }

    StatusCode JetTruthVertexSummaryDecoratorAlg::execute(const EventContext& ctx) const {
      ATH_MSG_DEBUG("Executing " << name() << "... ");

      // read collections
      SG::ReadHandle<xAOD::TruthParticleContainer> truth_particles(m_TruthContainerKey, ctx);
      SG::ReadHandle<xAOD::JetContainer> jets(m_JetContainerKey, ctx);
      ATH_CHECK(truth_particles.isValid());
      ATH_CHECK(jets.isValid());
      ATH_MSG_DEBUG("Retrieved " << truth_particles->size() << " truth_particles...");

      // get jets sorted by descending pt
      std::vector<const xAOD::Jet*> sorted_jets;
      for (const auto jet : *jets) { sorted_jets.push_back(jet); }
      std::sort(
        sorted_jets.begin(), sorted_jets.end(),
        [](const xAOD::Jet* j1, const xAOD::Jet* j2) {
          return j1->pt() > j2->pt();
        }
      );


      SG::ReadDecorHandle<xAOD::TruthParticleContainer, int> acc_vertexId(m_vertexID, ctx);
      SG::ReadDecorHandle<xAOD::TruthParticleContainer, int> acc_vertexValid(m_vertexValid, ctx);

      // Select one representative truth particle per fat vertex: the inpart
      // (parent) particle. Inparts carry vertexValid=1 and vertexDecayID>=0,
      // while internal chain particles carry only vertexDecayID>=0 (not valid).
      // Since there is at most one inpart per fat vertex, this filter yields
      // exactly one representative per vertex without needing an ID dedupe.
      std::vector<const xAOD::TruthParticle*> tps_vec;
      for (const auto tp : *truth_particles) {
        if(acc_vertexId(*tp) < 0) continue;
        if(!acc_vertexValid(*tp)) continue;
        tps_vec.push_back(tp);
      }

      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumBVerts(m_decoNumBVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumCVerts(m_decoNumCVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumTauVerts(m_decoNumTauVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumStrangeVerts(m_decoNumStrangeVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumPionVerts(m_decoNumPionVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumMaterialIntVerts(m_decoNumMaterialIntVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumOtherVerts(m_decoNumOtherVerts, ctx);
      SG::WriteDecorHandle<xAOD::IParticleContainer, int> decoNumVerts(m_decoNumVerts, ctx);

      SG::ReadDecorHandle<xAOD::TruthParticleContainer, int> acc_vertexSimpleType(m_vertexSimpleDecayType, ctx);

      // Iterate jets in descending pT order. Each truth particle is assigned
      // exclusively to the highest-pT jet it matches (to avoid double-counting vertices).
      for(const auto jet : sorted_jets){
        std::vector<const xAOD::TruthParticle*> searched;
        int num_b = 0;
        int num_c = 0;
        int num_tau = 0;
        int num_strange = 0;
        int num_pion = 0;
        int num_material_int = 0;
        int num_other = 0;
        int num_verts = 0;

        // iterate TPs and associate
        for(const xAOD::TruthParticle* tp : tps_vec){
          // If we only want verts in the ID, check if we sit inside the ID bounds defined by the user
          if (m_onlyID) {
            const xAOD::TruthVertex* prod_vtx = tp->prodVtx();
            if (!tp->hasProdVtx() || !prod_vtx) {
              searched.push_back(tp);
              continue;
            }
            if (prod_vtx->perp() > m_lxyIDThreshold ||
                std::abs(prod_vtx->z()) > m_zIDThreshold) {
              searched.push_back(tp);
              continue;
            }
          }

          // Then check if we match to this jet
          if(jet->p4().DeltaR(tp->p4()) > m_drThreshold) continue;

          searched.push_back(tp);
          num_verts++;
          int simple_type_i = acc_vertexSimpleType(*tp);
          FatVertex::SimpleVertexType simple_type = static_cast<FatVertex::SimpleVertexType>(simple_type_i);
          switch(simple_type){
            case FatVertex::SimpleVertexType::BHadronDecay: num_b++; break;
            case FatVertex::SimpleVertexType::CHadronDecay: num_c++; break;
            case FatVertex::SimpleVertexType::TauDecay: num_tau++; break;
            case FatVertex::SimpleVertexType::StrangeDecay: num_strange++; break;
            case FatVertex::SimpleVertexType::PionDecay: num_pion++; break;
            case FatVertex::SimpleVertexType::MaterialInteraction: num_material_int++; break;
            case FatVertex::SimpleVertexType::OtherSecondaryVertex: num_other++; break;
            case FatVertex::SimpleVertexType::Other: num_other++; break;
            default: break;
          }
        }
        // Erase everything we've searched and found to no longer care about (e.g. we've decorated it,
        // or its not something we care about, for *speed*)
        std::unordered_set<const xAOD::TruthParticle*> searched_set(searched.begin(), searched.end());
        tps_vec.erase(std::remove_if(tps_vec.begin(), tps_vec.end(),
                       [&](const xAOD::TruthParticle* x) { return searched_set.count(x); }),
                       tps_vec.end());

        decoNumBVerts(*jet) = num_b;
        decoNumCVerts(*jet) = num_c;
        decoNumTauVerts(*jet) = num_tau;
        decoNumStrangeVerts(*jet) = num_strange;
        decoNumPionVerts(*jet) = num_pion;
        decoNumMaterialIntVerts(*jet) = num_material_int;
        decoNumOtherVerts(*jet) = num_other;
        decoNumVerts(*jet) = num_verts;

      }
      return StatusCode::SUCCESS;
    }
}
