/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TruthParticleDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "TruthUtils/HepMCHelpers.h"


namespace FlavorTagDiscriminants {

  TruthParticleDecoratorAlg::TruthParticleDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode TruthParticleDecoratorAlg::initialize() {
    ATH_MSG_INFO( "Inizializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Inizializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_TruthContainerKey      );
    ATH_MSG_DEBUG( "    ** " << m_TruthPVsKey      );

    ATH_CHECK( m_TruthContainerKey.initialize() );
    ATH_CHECK( m_TruthPVsKey.initialize() );

    // Prepare decorators
    m_dec_origin_label = m_TruthContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_TruthContainerKey.key() + "." + m_dec_type_label.key();
    m_dec_source_label = m_TruthContainerKey.key() + "." + m_dec_source_label.key();
    m_dec_vertex_index = m_TruthContainerKey.key() + "." + m_dec_vertex_index.key();
    m_dec_parent_barcode = m_TruthContainerKey.key() + "." + m_dec_parent_barcode.key();

    CHECK( m_dec_origin_label.initialize() );
    CHECK( m_dec_type_label.initialize() );
    CHECK( m_dec_source_label.initialize() );
    CHECK( m_dec_vertex_index.initialize() );
    CHECK( m_dec_parent_barcode.initialize() );
    
    // Retrieve tools
    ATH_CHECK( m_truthOriginTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    using TPC = xAOD::TruthParticleContainer;

    // read collections
    SG::ReadHandle<TPC> truth_particles(m_TruthContainerKey,ctx);
    CHECK( truth_particles.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << truth_particles->size() << " truth_particles..." );
    SG::ReadHandle<xAOD::TruthVertexContainer> truth_PVs(m_TruthPVsKey, ctx);
    CHECK( truth_PVs.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << truth_PVs->size() << " truth PVs..." );
    
    // get the truth primary vertex
    if (truth_PVs->size() != 1) {
      ATH_MSG_ERROR( "Truth PVs != 1" );
      return StatusCode::FAILURE;
    }
    const xAOD::TruthVertex* truth_PV = truth_PVs->at(0);


    // instantiate decorators
    SG::WriteDecorHandle<TPC, int> dec_origin_label(m_dec_origin_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_type_label(m_dec_type_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_source_label(m_dec_source_label, ctx);
    SG::WriteDecorHandle<TPC, int> dec_vertex_index(m_dec_vertex_index, ctx);
    SG::WriteDecorHandle<TPC, int> dec_parent_barcode(m_dec_parent_barcode, ctx);

    // sort the particles by pt to ensure the vertex clustering is deterministic
    std::vector<const xAOD::TruthParticle*> sorted_truth_particles;
    for (const auto tp : *truth_particles) { sorted_truth_particles.push_back(tp); }
    std::sort(sorted_truth_particles.begin(), sorted_truth_particles.end(), TruthDecoratorHelpers::sort_particles);

    // first loop - decorate origin label, just store truth vertex for now
    auto tp_truth_vertices = std::vector<const xAOD::TruthVertex*>();
    for ( const auto& truth_particle : sorted_truth_particles ) {
      
      // for efficiency, skip unstable and low pt particles (< 500 MeV)
      // c-hadrons are exempt and always labelled so we can trace the b->c decay chains
      if ( (!MC::isStable(truth_particle) or truth_particle->pt() < 500) and !truth_particle->isCharmHadron()) {
        dec_origin_label(*truth_particle) = InDet::ExclusiveOrigin::Pileup;
        dec_type_label(*truth_particle) = TruthDecoratorHelpers::TruthType::Label::NoTruth;
        dec_source_label(*truth_particle) = TruthDecoratorHelpers::TruthSource::Label::NoTruth;
        tp_truth_vertices.push_back(nullptr);
        dec_vertex_index(*truth_particle) = -1;
        dec_parent_barcode(*truth_particle) = HepMC::UNDEFINED_ID; // FIXME make variable names consistent
        continue;
      }

      // get parent hadron and decorate barcode
      auto truth_parent = TruthDecoratorHelpers::get_parent_hadron(truth_particle);
      dec_parent_barcode(*truth_particle) = truth_parent ? HepMC::uniqueID(truth_parent) : HepMC::UNDEFINED_ID; // FIXME make variable names consistent

      // get truth origin and use it for exclusive origin and secondary origin
      int truth_origin = m_truthOriginTool->getTruthOrigin(truth_particle);

      int truth_origin_label = InDet::ExclusiveOrigin::getExclusiveOrigin(truth_origin);
      dec_origin_label(*truth_particle) = truth_origin_label;

      int truth_source_label = TruthDecoratorHelpers::get_source_type(truth_origin);
      dec_source_label(*truth_particle) = truth_source_label;

      
      // get the truth vertex of the particle and store for now
      auto truth_vertex = TruthDecoratorHelpers::get_truth_vertex(truth_particle);
      tp_truth_vertices.push_back(truth_vertex);

      // decorate truth type
      dec_type_label(*truth_particle) = TruthDecoratorHelpers::get_truth_type(truth_particle);
    }

    // check sorted_truth_particles and tp_truth_vertices have the same length
    if ( sorted_truth_particles.size() != tp_truth_vertices.size() ) {
      ATH_MSG_ERROR( "sorted_truth_particles and tp_truth_vertices have different lengths" );
      return StatusCode::FAILURE;
    }

    // decorate particle with truth vertex info
    auto seen_vertices = std::vector<const xAOD::TruthVertex*>();
    for ( size_t i = 0; i != tp_truth_vertices.size(); i++) {
      auto this_vert = tp_truth_vertices.at(i);
      auto this_tp = sorted_truth_particles.at(i);
      dec_vertex_index(*this_tp) = TruthDecoratorHelpers::get_vertex_index(this_vert, truth_PV, seen_vertices, m_truthVertexMergeDistance); // This is a position in an array of xAOD::TruthVertex objects
    }
    return StatusCode::SUCCESS;
  }
}


