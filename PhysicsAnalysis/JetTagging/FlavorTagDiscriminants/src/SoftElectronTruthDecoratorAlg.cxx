/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/SoftElectronTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "TruthUtils/MagicNumbers.h"

namespace FlavorTagDiscriminants {

  SoftElectronTruthDecoratorAlg::SoftElectronTruthDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode SoftElectronTruthDecoratorAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Initializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_ElectronContainerKey   );

    ATH_CHECK( m_ElectronContainerKey.initialize() );

    // Initialise accessors
    m_acc_origin_label = "TruthParticles." + m_acc_origin_label.key();
    m_acc_type_label = "TruthParticles." + m_acc_type_label.key();
    m_acc_source_label = "TruthParticles." + m_acc_source_label.key();
    m_acc_vertex_index = "TruthParticles." + m_acc_vertex_index.key();
    m_acc_parent_barcode = "TruthParticles." + m_acc_parent_barcode.key(); // FIXME make variable names consistent
    ATH_CHECK( m_acc_origin_label.initialize() );
    ATH_CHECK( m_acc_type_label.initialize() );
    ATH_CHECK( m_acc_source_label.initialize() );
    ATH_CHECK( m_acc_vertex_index.initialize() );
    ATH_CHECK( m_acc_parent_barcode.initialize() ); // FIXME make variable names consistent

    // Initialise decorators
    m_dec_origin_label = m_ElectronContainerKey.key() + "." + m_dec_origin_label.key();
    m_dec_type_label = m_ElectronContainerKey.key() + "." + m_dec_type_label.key();
    m_dec_source_label = m_ElectronContainerKey.key() + "." + m_dec_source_label.key();
    m_dec_vertex_index = m_ElectronContainerKey.key() + "." + m_dec_vertex_index.key();
    m_dec_barcode = m_ElectronContainerKey.key() + "." + m_dec_barcode.key(); // FIXME make variable names consistent
    m_dec_parent_barcode = m_ElectronContainerKey.key() + "." + m_dec_parent_barcode.key(); // FIXME make variable names consistent
    ATH_CHECK( m_dec_origin_label.initialize() );
    ATH_CHECK( m_dec_type_label.initialize() );
    ATH_CHECK( m_dec_source_label.initialize() );
    ATH_CHECK( m_dec_vertex_index.initialize() );
    ATH_CHECK( m_dec_barcode.initialize() ); // FIXME make variable names consistent
    ATH_CHECK( m_dec_parent_barcode.initialize() ); // FIXME make variable names consistent

    return StatusCode::SUCCESS;
  }

  StatusCode SoftElectronTruthDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    using EC = xAOD::ElectronContainer;

    // read collections
    SG::ReadHandle<EC> electrons(m_ElectronContainerKey,ctx);
    ATH_CHECK( electrons.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << electrons->size() << " electrons..." );

    // instantiate accessors
    using RDH = SG::ReadDecorHandle<xAOD::TruthParticleContainer, int>;
    RDH acc_origin_label(m_acc_origin_label, ctx);
    RDH acc_type_label(m_acc_type_label, ctx);
    RDH acc_source_label(m_acc_source_label, ctx);
    RDH acc_vertex_index(m_acc_vertex_index, ctx);
    RDH acc_parent_barcode(m_acc_parent_barcode, ctx); // FIXME make variable names consistent
    
    // instantiate decorators
    using WDH = SG::WriteDecorHandle<EC, int>;
    WDH dec_origin_label(m_dec_origin_label, ctx);
    WDH dec_type_label(m_dec_type_label, ctx);
    WDH dec_source_label(m_dec_source_label, ctx);
    WDH dec_vertex_index(m_dec_vertex_index, ctx);
    WDH dec_barcode(m_dec_barcode, ctx); // FIXME make variable names consistent
    WDH dec_parent_barcode(m_dec_parent_barcode, ctx); // FIXME make variable names consistent

    std::vector<const xAOD::Electron*> el_vector(electrons->begin(), electrons->end());
    for ( const auto& electron : el_vector ) {

      // get the linked truth particle
      const auto truth_link = m_truthParticleLink(*electron);

      if (!truth_link || !truth_link.isValid()) {
        // if the truth link is broken, assume PU
        dec_origin_label(*electron) = InDet::ExclusiveOrigin::Pileup;
        dec_type_label(*electron) = TruthDecoratorHelpers::TruthType::Label::NoTruth;
        dec_source_label(*electron) = TruthDecoratorHelpers::TruthSource::Label::NoTruth;
        dec_vertex_index(*electron) = -2;
        dec_barcode(*electron) = HepMC::UNDEFINED_ID; // FIXME make variable names consistent
        dec_parent_barcode(*electron) = HepMC::UNDEFINED_ID; // FIXME make variable names consistent
      } else {
        const auto *truth = *truth_link;
        int electron_type = m_classifierParticleType(*truth);

        if (m_valid_types.count(electron_type)) {
          dec_origin_label(*electron) = acc_origin_label(*truth);
        }
        else {
          dec_origin_label(*electron) = InDet::ExclusiveOrigin::Fake;
        }

        dec_vertex_index(*electron) = acc_vertex_index(*truth);
        dec_type_label(*electron) = acc_type_label(*truth);
        dec_source_label(*electron) = acc_source_label(*truth);
        dec_barcode(*electron) = HepMC::uniqueID(truth); // FIXME make variable names consistent
        dec_parent_barcode(*electron) = acc_parent_barcode(*truth); // FIXME make variable names consistent
      }
    }
    return StatusCode::SUCCESS;
  }
}
