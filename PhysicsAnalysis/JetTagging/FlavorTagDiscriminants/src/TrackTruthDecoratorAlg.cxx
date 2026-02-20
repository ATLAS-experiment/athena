/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "TruthUtils/MagicNumbers.h"

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
    m_acc_parent_uniqueID = "TruthParticles." + m_acc_parent_uniqueID.key();
    ATH_CHECK( m_acc_type_label.initialize() );
    ATH_CHECK( m_acc_source_label.initialize() );
    ATH_CHECK( m_acc_vertex_index.initialize() );
    ATH_CHECK( m_acc_parent_uniqueID.initialize() );

    // Initialize decorators
    m_dec_origin_label = m_TrackContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_TrackContainerKey.key() + "." + m_dec_type_label.key();
    m_dec_source_label = m_TrackContainerKey.key() + "." + m_dec_source_label.key();
    m_dec_vertex_index = m_TrackContainerKey.key() + "." + m_dec_vertex_index.key();
    m_dec_uniqueID = m_TrackContainerKey.key() + "." + m_dec_uniqueID.key();
    m_dec_parent_uniqueID = m_TrackContainerKey.key() + "." + m_dec_parent_uniqueID.key();
    m_dec_muon_origin_label = m_TrackContainerKey.key() + "." + m_dec_muon_origin_label.key();
    CHECK( m_dec_origin_label.initialize() );
    CHECK( m_dec_type_label.initialize() );
    CHECK( m_dec_source_label.initialize() );
    CHECK( m_dec_vertex_index.initialize() );
    CHECK( m_dec_uniqueID.initialize() );
    CHECK( m_dec_parent_uniqueID.initialize() );
    CHECK( m_dec_muon_origin_label.initialize() );

    // Retrieve tools
    ATH_CHECK( m_trackTruthOriginTool.retrieve() );
    if (!m_truthLeptonTool.empty()) {
      ATH_CHECK( m_truthLeptonTool.retrieve() );
    }

    // ATLASRECTS-8290: this should be removed eventually
    if (m_use_barcode) m_uid = SG::ConstAccessor<int>("barcode");

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
    RDH acc_parent_uniqueID(m_acc_parent_uniqueID, ctx);

    // instantiate decorators
    using WDH = SG::WriteDecorHandle<TPC, int>;
    WDH dec_origin_label(m_dec_origin_label, ctx);
    WDH dec_type_label(m_dec_type_label, ctx);
    WDH dec_source_label(m_dec_source_label, ctx);
    WDH dec_vertex_index(m_dec_vertex_index, ctx);
    WDH dec_uniqueID(m_dec_uniqueID, ctx);
    WDH dec_parent_uniqueID(m_dec_parent_uniqueID, ctx);
    WDH dec_muon_origin_label(m_dec_muon_origin_label, ctx);

    // decorate loop
    std::vector<const xAOD::TrackParticle*> tracks_vector(tracks->begin(), tracks->end());
    for ( const auto& track : tracks_vector ) {
      
      // for the origin label we need to start from the track object (to label fake tracks)
      int trackTruthOrigin = m_trackTruthOriginTool->getTrackOrigin(track);
      dec_origin_label(*track) = InDet::ExclusiveOrigin::getExclusiveOrigin(trackTruthOrigin);

      // everything else is already decorated to the associated truth particle
      const auto truth = m_trackTruthOriginTool->getTruth(track);
      // ATLASRECTS-8290: replace m_uid with HepMC::uniqueID
      dec_uniqueID(*track) = truth ? m_uid(*truth) : HepMC::UNDEFINED_ID;
      dec_parent_uniqueID(*track) = truth ? acc_parent_uniqueID(*truth) : HepMC::UNDEFINED_ID;
      dec_type_label(*track) = truth ? acc_type_label(*truth) : TruthDecoratorHelpers::TruthType::Label::NoTruth;
      dec_source_label(*track) = truth ? acc_source_label(*truth) : TruthDecoratorHelpers::TruthSource::Label::NoTruth;
      dec_vertex_index(*track) = truth ? acc_vertex_index(*truth) : -2;
      dec_muon_origin_label(*track) = -2;

    }
    if ( !m_truthLeptonTool.empty() ) {

      // decorate muon tracks with truth origin
      for ( const auto muon : *muons ) {

        // Check if the muon is a combined muon
        if (muon->muonType() != xAOD::Muon::MuonType::Combined) { continue; }

        // Classify muon truth origin (https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/AnalysisCommon/TruthClassification)
        unsigned int muTruthOrigin = 0;
        ATH_CHECK(m_truthLeptonTool->classify(*muon, muTruthOrigin));
        Truth::Type muTruthOriginType = static_cast<Truth::Type>(muTruthOrigin);

        // Get the track associated to the muon
        auto track_link = muon->inDetTrackParticleLink();
        if ( !track_link.isValid() ) { continue; }
        auto track = *track_link;

        // Get the truth particle associated to the track
        const auto truth = m_trackTruthOriginTool->getTruth(track);
        if ( !truth ) { continue; }

        // Get the track truth origin
        int trackTruthOrigin = m_trackTruthOriginTool->getTrackOrigin(track);

        if ( abs(truth->pdgId()) != 13 ) {
          // Check if the truth particle associated to the track is not a muon
          muTruthOrigin = 9;
        }
        else if ( muTruthOriginType == Truth::Type::CHadronDecay && InDet::TrkOrigin::isFromDfromB(trackTruthOrigin) ) {
          // Check if a muon is FromC and the associated track is FromBC
          muTruthOrigin = 4;
        }
        else if ( muTruthOriginType == Truth::Type::CHadronDecay && !InDet::TrkOrigin::isFromDfromB(trackTruthOrigin) ) {
          // Check if a muon is FromC and the associated track is not FromBC
          muTruthOrigin = 5;
        }
        else {
          // any alternative truth origin label are taken from TruthClassificationTool and mapped
          auto it = m_muTruthMap.find(muTruthOriginType);
          if ( it != m_muTruthMap.end() ){
            muTruthOrigin = it->second;
          }
          else {
            // raise an error if the muon truth origin label is not found in the map
            ATH_MSG_ERROR("Muon truth origin not found in the map: " << static_cast<int>(muTruthOriginType));
            return StatusCode::FAILURE;
          }
        }

        // decorate the muon track
        dec_muon_origin_label(*track) = muTruthOrigin;
      }
    }
    return StatusCode::SUCCESS;
  }
}


