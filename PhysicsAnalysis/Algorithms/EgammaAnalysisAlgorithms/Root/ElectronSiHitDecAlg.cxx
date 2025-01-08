/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   ElectronSiHitDecAlg
//
//   Add in decorations for a SiHit electron to include minimal information from
//   track and cluster required for a minimal background estimation.
///////////////////////////////////////////////////////////////////

#include "EgammaAnalysisAlgorithms/ElectronSiHitDecAlg.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include <SelectionHelpers/SelectionHelpers.h>

namespace CP
{
    ElectronSiHitDecAlg::ElectronSiHitDecAlg(const std::string &name, ISvcLocator *svcLoc)
        : EL::AnaAlgorithm(name, svcLoc)
    {
    }

    StatusCode ElectronSiHitDecAlg::initialize()
    {
        // Greet the user:
        ATH_MSG_DEBUG("Initialising");


        ATH_CHECK(m_eventInfoKey.initialize(m_systematicsList));
        ATH_CHECK(m_vertexKey.initialize(m_systematicsList));
        ATH_CHECK(m_electronContainerKey.initialize(m_systematicsList));
        ATH_CHECK(m_analMuonContKey.initialize(m_systematicsList));
        ATH_CHECK(m_analElectronContKey.initialize(m_systematicsList));

        ATH_CHECK(m_z0stheta.initialize(m_systematicsList, m_electronContainerKey));
        ATH_CHECK(m_d0Normalized.initialize(m_systematicsList, m_electronContainerKey));
        ATH_CHECK(m_nInnerExpPix.initialize(m_systematicsList, m_electronContainerKey));
        ATH_CHECK(m_clEta.initialize(m_systematicsList, m_electronContainerKey));
        ATH_CHECK(m_clPhi.initialize(m_systematicsList, m_electronContainerKey));
        ATH_CHECK(m_evtOKDec.initialize(m_systematicsList, m_electronContainerKey));

        ANA_CHECK (m_systematicsList.initialize());

        ATH_MSG_INFO("Reading " << m_eventInfoKey.getNamePattern() << ", vertex " <<  m_vertexKey.getNamePattern() << ", and electrons " << m_electronContainerKey.getNamePattern() << " for decorating SiHit electrons.");

        if (m_requireTwoLeptons) ATH_MSG_INFO("Requiring at least one pair of leptons from containers "
                                 << m_analMuonContKey.getNamePattern() << " and "
                                 <<  m_analElectronContKey.getNamePattern());
        else ATH_MSG_INFO("No requirement on pairs of leptons. ");


        // Return gracefully:
        return StatusCode::SUCCESS;
    }


    StatusCode ElectronSiHitDecAlg::execute()
    {

        ATH_MSG_DEBUG("Entering execute");

        // Loop over systematics
        for (const auto& sys : m_systematicsList.systematicsVector()) {

            // Check if event has at least one pair of muons or electrons
            bool eventHasLeptonPair = false;

            if (m_requireTwoLeptons) {
                const xAOD::ElectronContainer* analEls = nullptr;
                ANA_CHECK (m_analElectronContKey.retrieve (analEls, sys));
                ATH_MSG_DEBUG("Retrieved electrons: " << analEls->size());
                if (analEls->size() > 1) eventHasLeptonPair = true;
                else {
                    const xAOD::MuonContainer* analMus = nullptr;
                    ANA_CHECK (m_analMuonContKey.retrieve (analMus, sys));
                    ATH_MSG_DEBUG("Retrieved muons: " << analMus->size());
                    if (analMus->size() > 1) eventHasLeptonPair = true;
                }
            }
            else eventHasLeptonPair = true;
            ATH_MSG_DEBUG("Event has lepton pair?: " << (int)eventHasLeptonPair);


            // Retrieve EventInfo
            const xAOD::EventInfo* ei = nullptr;
            ANA_CHECK (m_eventInfoKey.retrieve (ei, sys));
            ATH_MSG_DEBUG("Retrieved EventInfo");

            // Retrieve vertices
            const xAOD::VertexContainer* vtxs = nullptr;
            ANA_CHECK (m_vertexKey.retrieve (vtxs, sys));
            ATH_MSG_DEBUG("Retrieved primary vertex");

            // Retrieve electrons
            const xAOD::ElectronContainer* els = nullptr;
            ANA_CHECK (m_electronContainerKey.retrieve (els, sys));
            ATH_MSG_DEBUG("Retrieved electrons: " << els->size());

            // get primary vertex
            const xAOD::Vertex*  primaryVtx = nullptr;
            for ( auto vtx : *vtxs) {
                if (vtx->vertexType() == xAOD::VxType::PriVtx) {
                    primaryVtx = vtx;
                }
            }

            if (primaryVtx) {
                ATH_MSG_DEBUG("Primary vtx z ntrk " << primaryVtx->z() << " "
                            << primaryVtx->nTrackParticles() << " index " << primaryVtx->index());
            }

            // Set the needed decorations for SiHit electrons
            uint8_t val8;
            for ( auto el : *els ) {

                // Select or not SiHits depending on whether this event has an electron pair
                char evtOK = (eventHasLeptonPair) ? 1 : 0;
                m_evtOKDec.set(*el, evtOK, sys);
                ATH_MSG_DEBUG( "SiHit el passes?: " << evtOK );

                // Number of pixel hits in innermost or next to innermost pixel layer
                int el_nInnerExpPix = -1;
                int expInPix             = el->trackParticleSummaryValue(val8, xAOD::expectInnermostPixelLayerHit)
                    ? val8 : -999;
                int expNextInPix         = el->trackParticleSummaryValue(val8, xAOD::expectNextToInnermostPixelLayerHit)
                    ? val8 : -999;
                if (1 == expInPix) {
                    el_nInnerExpPix = el->trackParticleSummaryValue(val8, xAOD::numberOfInnermostPixelLayerHits)
                        ? val8 : -999;
                }
                else if (1 == expNextInPix) {
                    el_nInnerExpPix = el->trackParticleSummaryValue(val8, xAOD::numberOfNextToInnermostPixelLayerHits)
                        ? val8 : -999;
                }
                m_nInnerExpPix.set(*el, el_nInnerExpPix, sys);

                // set z0stheta
                auto tp = el->trackParticle();
                float z0stheta = 0;
                if (primaryVtx) z0stheta = (tp->z0() - primaryVtx->z() + tp->vz()) * sin(tp->theta());
                m_z0stheta.set(*el, z0stheta, sys);

                // Set d0 normalized
                float d0Normalized = std::abs(xAOD::TrackingHelpers::d0significance(tp, ei->beamPosSigmaX(), ei->beamPosSigmaY(), ei->beamPosSigmaXY()));
                m_d0Normalized.set(*el, d0Normalized, sys);

                // cluster eta, phi
                float clEta = el->caloCluster()->eta();
                float clPhi = el->caloCluster()->phi();
                m_clEta.set(*el, clEta, sys);
                m_clPhi.set(*el, clPhi, sys);



                ATH_MSG_DEBUG("el pt,eta,ph " << el->pt()/1000. << ", " << el->eta() << ", " << el->phi());
                ATH_MSG_DEBUG("Set z0stheta to " << z0stheta);
                ATH_MSG_DEBUG("Set d0Norm to " << d0Normalized);
                ATH_MSG_DEBUG("Set nInnerExpPix to " << el_nInnerExpPix);
                ATH_MSG_DEBUG("Set cluster eta, phi to " << clEta << ", " << clPhi);
            }
        }

        ATH_MSG_DEBUG("Done !");

        return StatusCode::SUCCESS;
    }

}
