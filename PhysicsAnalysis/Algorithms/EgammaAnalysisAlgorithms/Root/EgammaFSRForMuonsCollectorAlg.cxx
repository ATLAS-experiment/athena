/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   EgammaFSRForMuonsCollectorAlg
//
//   Algorithm to collect photons and electrons which close in dR 
//   to muons as FSR candidates
///////////////////////////////////////////////////////////////////

#include "EgammaAnalysisAlgorithms/EgammaFSRForMuonsCollectorAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <xAODEgamma/Electron.h>
#include <SelectionHelpers/SelectionHelpers.h>

namespace CP
{
    EgammaFSRForMuonsCollectorAlg::EgammaFSRForMuonsCollectorAlg(const std::string &name, ISvcLocator *svcLoc)
        : EL::AnaAlgorithm(name, svcLoc)
    {
    }

    StatusCode EgammaFSRForMuonsCollectorAlg::initialize()
    {
        // Greet the user:
        ATH_MSG_INFO("Initialising");

        // WP ID selection
        if (m_selectionName.empty()) {
            ATH_MSG_ERROR("Empty string passed as WP selection flag!");
            return StatusCode::FAILURE;
        }

        ATH_CHECK(m_egammaContH.initialize(m_systematicsList));
        ATH_CHECK(m_muonContH.initialize(m_systematicsList));
        ANA_CHECK (m_systematicsList.initialize());
        m_wpDec = std::make_unique<SG::AuxElement::Decorator<uint32_t> > (m_selectionName.value());


        ATH_MSG_INFO("Reading container " << m_egammaContH.getNamePattern() << " for FSR search for muons from " <<  m_muonContH.getNamePattern() << ". Those passing " << m_selectionName.value() << " are also accepted.");

        // Return gracefully:
        return StatusCode::SUCCESS;
    }


    StatusCode EgammaFSRForMuonsCollectorAlg::execute()
    {

        // const EventContext &ctx = Gaudi::Hive::currentContext();

        // Loop over systematics
        for (const auto& sys : m_systematicsList.systematicsVector()) {


            // Retrieve electrons or photons
            const xAOD::IParticleContainer* egammaCont = nullptr;
            ANA_CHECK (m_egammaContH.retrieve (egammaCont, sys));

            // Retrieve muons 
            const xAOD::MuonContainer* muonCont = nullptr;
            ANA_CHECK (m_muonContH.retrieve (muonCont, sys));

            // Loop over each electron or photon. If already passing WP selection, decorate passWPorFSR as true
            for ( auto eg : *egammaCont ) {
                // If passes std WP, accept
                if ((*m_wpDec)(*eg) == selectionAccept()) {
                    ATH_MSG_DEBUG("Eg passed WP - pt, eta: " << eg->type() << ", " << eg->pt()/1000. << ", " << eg->eta() << ", " << (*m_wpDec)(*eg)  );
                    continue; // ok, skip to next el/ph
                }
                else ATH_MSG_DEBUG("Eg failed WP: " <<  m_selectionName.value() << ", " << (*m_wpDec)(*eg) );

                const xAOD::Electron* el = (eg->type() == xAODType::Electron) ? dynamic_cast<const xAOD::Electron*>(eg) : 0;

                ATH_MSG_DEBUG( "Incoming eg: pt, eta, phi " << eg->pt()/1000. << ", " << eg->eta() << ", " << eg->phi() << ", is electron " << (el != 0));

                // Loop over muons and check dR
                for ( auto mu : *muonCont ) {

                    float dR = xAOD::P4Helpers::deltaR(*eg, *mu);

                    ATH_MSG_DEBUG( "dR with mu: pt, eta, phi " << dR << ", " << mu->pt()/1000. << ", " << mu->eta() << ", " << mu->phi() );

                    if (dR < m_dRMax) {

                        // if electron (not photon) check track matching 
                        bool elmutrackmatchOK = true; // default true for photons
                        if (el) {
                            const xAOD::TrackParticle* electron_track = el->trackParticle();
                            const xAOD::TrackParticle* muon_track     = mu->primaryTrackParticle();

                            elmutrackmatchOK = 
                            ( (fabs(electron_track->theta()- muon_track->theta()) < 0.01) &&
                            (xAOD::P4Helpers::deltaPhi(electron_track->phi(),  muon_track->phi())   < 0.01) );
                            if (elmutrackmatchOK) ATH_MSG_DEBUG( "track match OK");
                            else                  ATH_MSG_DEBUG( "track match NOT OK");
                        }
                        if (elmutrackmatchOK) {
                            (*m_wpDec)(*eg) = selectionAccept();
                            ATH_MSG_DEBUG( "dR OK - wp " << (*m_wpDec)(*eg) );
                            break;
                        }
                    }
                    ATH_MSG_DEBUG( "dR not OK");
                }
            }
        }

        ATH_MSG_DEBUG("Done !");

        return StatusCode::SUCCESS;
    }

}
