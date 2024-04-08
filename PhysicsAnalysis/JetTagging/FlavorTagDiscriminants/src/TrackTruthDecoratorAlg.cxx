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
    ATH_MSG_DEBUG( "    ** " << m_MuonContainerKey       );
    ATH_CHECK( m_TrackContainerKey.initialize() );
    ATH_CHECK( m_MuonContainerKey.initialize() );

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
    m_dec_muon_origin_label = m_TrackContainerKey.key() + "." + m_dec_muon_origin_label.key();
    CHECK( m_dec_origin_label.initialize() );
    CHECK( m_dec_type_label.initialize() );
    CHECK( m_dec_source_label.initialize() );
    CHECK( m_dec_vertex_index.initialize() );
    CHECK( m_dec_barcode.initialize() );
    CHECK( m_dec_parent_barcode.initialize() );
    CHECK( m_dec_muon_origin_label.initialize() );
    
    // Retrieve tools
    ATH_CHECK( m_trackTruthOriginTool.retrieve() );
    ATH_CHECK( m_truthLeptonTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode TrackTruthDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );
    using TPC = xAOD::TrackParticleContainer;

    // read collections
    SG::ReadHandle<TPC> tracks(m_TrackContainerKey, ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << tracks->size() << " tracks..." );
    SG::ReadHandle<xAOD::MuonContainer> muons(m_MuonContainerKey,ctx);
    CHECK( tracks.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << muons->size() << " muons..." );

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
    WDH dec_muon_origin_label(m_dec_muon_origin_label, ctx);

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
      dec_type_label(*track) = truth ? acc_type_label(*truth) : TruthDecoratorHelpers::TruthType::Label::NoTruth;
      dec_source_label(*track) = truth ? acc_source_label(*truth) : TruthDecoratorHelpers::TruthSource::Label::NoTruth;
      dec_vertex_index(*track) = truth ? acc_vertex_index(*truth) : -2;
      dec_muon_origin_label(*track) = -2;

    }

    // decorate muon tracks with truth origin
    // defining a map for proper muon origin labels 
    std::map<unsigned int,unsigned int> muTruthMap = {
      {0, 0}, //Fake(Uknown) muon
      {1, 1}, //KnownUknown muon
      {4, 2}, //Prompt muon
      {8, 3}, //From B muon
      {13, 4}, //From B/C (decay chain) muon (not included in the muon TruthClassificationTool)
      {9, 5}, //From C muon 
      {10, 6}, //From light muon
    };

    for ( const auto muon : *muons ) {
      
      // Classify muon truth origin
      unsigned int muTruthOrigin = 0;
      ATH_CHECK(m_truthLeptonTool->classify(*muon, muTruthOrigin));
      
      // Get the track associated to the muon
      auto track_link = muon->inDetTrackParticleLink();
      if ( !track_link.isValid() ) { continue; }
      auto track = *track_link;

      // Get the track truth origin
      int trackTruthOrigin = m_trackTruthOriginTool->getTrackOrigin(track);
      if ( muTruthOrigin == 9 && InDet::TrkOrigin::isFromDfromB(trackTruthOrigin) ) {
        // Check if a muon isFromC and the associated track isFromBC
        muTruthOrigin = 11;
      }
      else if ( muTruthOrigin == 9 && !InDet::TrkOrigin::isFromDfromB(trackTruthOrigin) ) {
        muTruthOrigin = 9;
      }

      if ( muTruthMap.find(muTruthOrigin) != muTruthMap.end() ) {
        muTruthOrigin = muTruthMap[muTruthOrigin];
        ATH_MSG_DEBUG("Muon truth Origin after mapping: " << muTruthOrigin);
        
        // decorate track
        dec_muon_origin_label(*track) = muTruthOrigin;
      }
    }
    return StatusCode::SUCCESS;
  }
}


