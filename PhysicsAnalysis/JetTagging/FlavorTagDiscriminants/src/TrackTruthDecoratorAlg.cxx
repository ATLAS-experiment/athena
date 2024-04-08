/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"


namespace FlavorTagDiscriminants {

  TrackTruthDecoratorAlg::TrackTruthDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode TrackTruthDecoratorAlg::initialize() {
    ATH_MSG_INFO( "Inizializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Inizializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_TrackContainerKey      );
    ATH_CHECK( m_TrackContainerKey.initialize() );

    // Initialize accessors
    m_acc_type_label = "TruthParticles." + m_acc_type_label.key();
    m_acc_source_label = "TruthParticles." + m_acc_source_label.key();
    m_acc_vertex_index = "TruthParticles." + m_acc_vertex_index.key();
    m_acc_parent_barcode = "TruthParticles." + m_acc_parent_barcode.key();
    ATH_CHECK( m_acc_type_label.initialize() );
    ATH_CHECK( m_acc_source_label.initialize() );
    ATH_CHECK( m_acc_vertex_index.initialize() );
    ATH_CHECK( m_acc_parent_barcode.initialize() );

    // Initialize decorators
    m_dec_origin_label = m_TrackContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_TrackContainerKey.key() + "." + m_dec_type_label.key();
    m_dec_source_label = m_TrackContainerKey.key() + "." + m_dec_source_label.key();
    m_dec_vertex_index = m_TrackContainerKey.key() + "." + m_dec_vertex_index.key();
    m_dec_barcode = m_TrackContainerKey.key() + "." + m_dec_barcode.key();
    m_dec_parent_barcode = m_TrackContainerKey.key() + "." + m_dec_parent_barcode.key();
    CHECK( m_dec_origin_label.initialize() );
    CHECK( m_dec_type_label.initialize() );
    CHECK( m_dec_source_label.initialize() );
    CHECK( m_dec_vertex_index.initialize() );
    CHECK( m_dec_barcode.initialize() );
    CHECK( m_dec_parent_barcode.initialize() );
    
    // Retrieve tools
    ATH_CHECK( m_trackTruthOriginTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode TrackTruthDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );
    using TPC = xAOD::TrackParticleContainer;

    // read collections
    SG::ReadHandle<TPC> tracks(m_TrackContainerKey, ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << tracks->size() << " tracks..." );

    // instantiate accessors
    using RDH = SG::ReadDecorHandle<xAOD::TruthParticleContainer, int>;
    RDH acc_type_label(m_acc_type_label, ctx);
    RDH acc_source_label(m_acc_source_label, ctx);
    RDH acc_vertex_index(m_acc_vertex_index, ctx);
    RDH acc_parent_barcode(m_acc_parent_barcode, ctx);

    // instantiate decorators
    using WDH = SG::WriteDecorHandle<TPC, int>;
    WDH dec_origin_label(m_dec_origin_label, ctx);
    WDH dec_type_label(m_dec_type_label, ctx);
    WDH dec_source_label(m_dec_source_label, ctx);
    WDH dec_vertex_index(m_dec_vertex_index, ctx);
    WDH dec_barcode(m_dec_barcode, ctx);
    WDH dec_parent_barcode(m_dec_parent_barcode, ctx);

    // decorate loop
    std::vector<const xAOD::TrackParticle*> tracks_vector(tracks->begin(), tracks->end());
    for ( const auto& track : tracks_vector ) {
      
      // for the origin label we need to start from the track object (to label fake tracks)
      int trackTruthOrigin = m_trackTruthOriginTool->getTrackOrigin(track);
      dec_origin_label(*track) = InDet::ExclusiveOrigin::getExclusiveOrigin(trackTruthOrigin);

      // everything else is already decorated to the associated truth particle
      const auto truth = m_trackTruthOriginTool->getTruth(track);
      dec_barcode(*track) = truth ? truth->barcode() : -2;
      dec_parent_barcode(*track) = truth ? acc_parent_barcode(*truth) : -2;
      dec_type_label(*track) = truth ? acc_type_label(*truth) : -2;
      dec_source_label(*track) = truth ? acc_source_label(*truth) : -2;
      dec_vertex_index(*track) = truth ? acc_vertex_index(*truth) : -2;

    }
    return StatusCode::SUCCESS;
  }
}


