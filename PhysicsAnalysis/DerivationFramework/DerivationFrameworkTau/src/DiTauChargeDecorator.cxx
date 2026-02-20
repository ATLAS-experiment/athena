/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/DiTauChargeDecorator.h"
#include "xAODTau/DiTauJetContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

  StatusCode DiTauChargeDecorator::initialize()
  {
    // initialize read/write handle keys
    ATH_CHECK( m_ditauContainerKey.initialize() );
    ATH_CHECK( m_chargeKey.initialize() );

    return StatusCode::SUCCESS;
  }


  StatusCode DiTauChargeDecorator::addBranches(const EventContext& ctx) const
  {


    // retrieve tau container
    SG::ReadHandle<xAOD::DiTauJetContainer> ditauJetsReadHandle(m_ditauContainerKey, ctx);
    if (!ditauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve DiTauJetContainer with key " << ditauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::DiTauJetContainer* ditauContainer = ditauJetsReadHandle.cptr();

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> dec_charge (m_chargeKey, ctx);

    for (const auto ditau : *ditauContainer) {
      float ditau_charge = 0;
      for (const auto& xTrack : ditau->trackLinks()) {
        if (!xTrack.isValid())
          continue;

        if(ditau->nSubjets() >= 2){
          for (int i = 0; i < 2; ++i) { // loop over two leading subjets
            TLorentzVector tlvSubjet = TLorentzVector();
            tlvSubjet.SetPtEtaPhiE(ditau->subjetPt(i), ditau->subjetEta(i),
                                   ditau->subjetPhi(i), ditau->subjetE(i));
            double dR = tlvSubjet.DeltaR((*xTrack)->p4());
            if (dR < 0.1) {
              ditau_charge += (*xTrack)->charge();
              break; //prevents double counting of tracks
            }
          }  // loop over subjets
        }
      } // loop over tracks
      dec_charge(*ditau) = ditau_charge;
    }

    return StatusCode::SUCCESS;
  }
}
