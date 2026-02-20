/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/SoftElectronDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"


namespace FlavorTagDiscriminants {

  SoftElectronDecoratorAlg::SoftElectronDecoratorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode SoftElectronDecoratorAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << "... " );

    // Initialize Container keys
    ATH_MSG_DEBUG( "Initializing containers:"            );
    ATH_MSG_DEBUG( "    ** " << m_ElectronContainerKey      );
    ATH_MSG_DEBUG( "    ** " << m_VertexContainerKey      );

    ATH_CHECK( m_ElectronContainerKey.initialize() );
    ATH_CHECK( m_VertexContainerKey.initialize() );

    // Initialize decorators
    ATH_MSG_DEBUG( "Initializing decorators:"  );
    ATH_MSG_DEBUG( "    ** " << m_dec_electron_et );
    ATH_MSG_DEBUG( "    ** " << m_dec_electron_deltaPOverP );
    ATH_MSG_DEBUG( "    ** " << m_dec_electron_isoPtOverPt );
    ATH_MSG_DEBUG( "    ** " << m_dec_electron_energyOverP );
    ATH_MSG_DEBUG( "    ** " << m_dec_electron_z0 );
    ATH_MSG_DEBUG( "    ** " << m_dec_electron_z0_significance );

    ATH_CHECK( m_dec_electron_et.initialize() );
    ATH_CHECK( m_dec_electron_deltaPOverP.initialize() );
    ATH_CHECK( m_dec_electron_isoPtOverPt.initialize() );
    ATH_CHECK( m_dec_electron_energyOverP.initialize() );
    ATH_CHECK( m_dec_electron_z0.initialize() );
    ATH_CHECK( m_dec_electron_z0_significance.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode SoftElectronDecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    using EC = xAOD::ElectronContainer;

    // read collections
    SG::ReadHandle<EC> electrons(m_ElectronContainerKey,ctx);
    ATH_CHECK( electrons.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << electrons->size() << " electrons..." );
    SG::ReadHandle<xAOD::VertexContainer> primary_vertices(m_VertexContainerKey,ctx);
    ATH_CHECK( primary_vertices.isValid() );
    ATH_MSG_DEBUG( "Retrieved " << primary_vertices->size() << " primary vertices..." );
    
    // instantiate decorators
    SG::WriteDecorHandle<EC, float> dec_electron_et(m_dec_electron_et, ctx);
    SG::WriteDecorHandle<EC, float> dec_electron_deltaPOverP(m_dec_electron_deltaPOverP, ctx);
    SG::WriteDecorHandle<EC, float> dec_electron_isoPtOverPt(m_dec_electron_isoPtOverPt, ctx);
    SG::WriteDecorHandle<EC, float> dec_electron_energyOverP(m_dec_electron_energyOverP, ctx);
    SG::WriteDecorHandle<EC, float> dec_electron_z0(m_dec_electron_z0, ctx);
    SG::WriteDecorHandle<EC, float> dec_electron_z0_significance(m_dec_electron_z0_significance, ctx);

    // get the primary vertex
    const xAOD::Vertex* pv = primary(*primary_vertices);

    for (const auto el : *electrons)
    {
        // kinematic vars
        float el_et = -1;
        float el_qoverp = -1;
        // tracking vars
        float el_z0 = -1;
        float el_z0_sig = -1;
        // additional var (x1)
        float el_iso_pt = -1;
        // tracking dnn
        float el_dpop = -1;
        // kinematic vars
        float el_pt = el->pt();

        float energy = el->caloCluster()->e();
        auto track = el->trackParticle();
        el_et = energy / std::cosh(track->eta());

        // tracking vars 
        el_z0 = track->z0() + (track->vz() - pv->z());
        el_z0_sig = el_z0 / std::sqrt(track->definingParametersCovMatrixDiagVec().at(1));

        // additional vars
        el_iso_pt = m_pt_varcone30(*el) / el_pt;
        // tracking dnn
        unsigned int index;
        el_qoverp = track->qOverP();
        if (track->indexOfParameterAtPosition(index, xAOD::LastMeasurement))
        {
            double refittedTrack_LMqoverp = track->charge() / std::sqrt(std::pow(track->parameterPX(index), 2) +
                                                                        std::pow(track->parameterPY(index), 2) +
                                                                        std::pow(track->parameterPZ(index), 2));
            el_dpop = 1 - el_qoverp / (refittedTrack_LMqoverp);
        }

        // kinematic var
        dec_electron_et(*el) = el_et;
        // track var
        dec_electron_z0(*el) = el_z0;
        dec_electron_z0_significance(*el) = el_z0_sig;
        // additional var (x1)
        dec_electron_isoPtOverPt(*el) = el_iso_pt;
        // dnn tracking
        dec_electron_deltaPOverP(*el) = el_dpop;
        // track cluster
        dec_electron_energyOverP(*el) = energy * std::abs(track->qOverP());
    }
    return StatusCode::SUCCESS;
  }
  
  const xAOD::Vertex* SoftElectronDecoratorAlg::primary(const xAOD::VertexContainer& vertices) const {
    if (vertices.size() == 0) {
      throw std::runtime_error("no primary vertices");
    }
    for ( const xAOD::Vertex *vertex : vertices ) {
      if ( vertex->vertexType() == xAOD::VxType::PriVtx ) {
        return vertex;
      }
    }
    // if we find nothing else this should be the beam spot
    return vertices.front();
  }
}




