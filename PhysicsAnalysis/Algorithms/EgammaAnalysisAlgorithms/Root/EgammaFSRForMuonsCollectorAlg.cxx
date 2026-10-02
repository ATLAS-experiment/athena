/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

#include "xAODEgamma/ElectronxAODHelpers.h"

#include <SelectionHelpers/SelectionHelpers.h>

namespace CP
{
    StatusCode EgammaFSRForMuonsCollectorAlg::initialize()
    {
        ANA_CHECK(m_egammaContKey.initialize(m_systematicsList));
        ANA_CHECK(m_muonContKey.initialize(m_systematicsList));
        ANA_CHECK(m_wpSelection.initialize(m_systematicsList, m_egammaContKey));
        ANA_CHECK(m_outputDec.initialize(m_systematicsList, m_egammaContKey));
        ANA_CHECK(m_systematicsList.initialize());

        if (!m_vetoFSR) {
            ATH_MSG_INFO("Reading container " << m_egammaContKey.getNamePattern() << " for FSR search for muons from " <<  m_muonContKey.getNamePattern() << ". Those passing " << m_wpSelection.getSelectionName() << " are also accepted.");
        }
        else {
            ATH_MSG_INFO("Reading container " << m_egammaContKey.getNamePattern() << " for FSR search for muons from " <<  m_muonContKey.getNamePattern() << ". The electrons or photons matching FSR requirements with muons will be vetoed with " << m_outputDec.getDecorName());
        }

        // Return gracefully:
        return StatusCode::SUCCESS;
    }


    StatusCode EgammaFSRForMuonsCollectorAlg::execute(const EventContext& ctx)
    {
        // Loop over systematics
        for (const auto& sys : m_systematicsList.systematicsVector()) {

            // Retrieve electrons or photons
            const xAOD::IParticleContainer* egammaCont = nullptr;
            ANA_CHECK(m_egammaContKey.retrieve(egammaCont, sys, ctx));

            // Retrieve muons
            const xAOD::MuonContainer* muonCont = nullptr;
            ANA_CHECK(m_muonContKey.retrieve(muonCont, sys, ctx));

            // Loop over each electron or photon and compute the combined selection
            for (const xAOD::IParticle* eg : *egammaCont) {

                // Read the WP selection (if configured)
                bool passesWP = m_wpSelection.getBool(*eg, sys);

                // Check if this particle passes FSR criteria (close to a muon)
                bool passesFSR = false;

                const xAOD::Electron* el = (eg->type() == xAOD::Type::Electron)
                    ? static_cast<const xAOD::Electron*>(eg) : nullptr;

                ATH_MSG_DEBUG("Incoming eg: pt, eta, phi " << eg->pt()/1000. << ", "
                    << eg->eta() << ", " << eg->phi() << ", is electron " << (el != nullptr)
                    << ", passesWP " << passesWP);

                // The FSR match only matters if the WP selection alone does not decide the outcome:
                // normal mode is WP || FSR, veto mode is WP && !FSR
                const bool needFSR = (m_vetoFSR == passesWP);

                // Loop over muons and check dR
                if (needFSR) {
                    for (const xAOD::Muon* mu : *muonCont) {
                        double dR = xAOD::P4Helpers::deltaR(*eg, *mu);

                        ATH_MSG_DEBUG("dR with mu: " << dR << ", pt, eta, phi "
                            << mu->pt()/1000. << ", " << mu->eta() << ", " << mu->phi());

                        if (dR < m_dRMax) {
                            // if electron (not photon) check track matching
                            bool elmutrackmatchOK = true; // default true for photons
                            if (el) {
                                const xAOD::TrackParticle* electron_track = el->trackParticle();
                                const xAOD::TrackParticle* elOrig_track = xAOD::EgammaHelpers::getOriginalTrackParticle(el);
                                const xAOD::TrackParticle* muon_track = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);

                                if (electron_track && muon_track) {
                                    elmutrackmatchOK =
                                        (std::abs(electron_track->theta() - muon_track->theta()) < 0.01) &&
                                        (std::abs(xAOD::P4Helpers::deltaPhi(electron_track->phi(), muon_track->phi())) < 0.01);
                                    ATH_MSG_DEBUG("dtheta trk " << std::abs(electron_track->theta() - muon_track->theta())
                                        << ", dphi trk " << std::abs(xAOD::P4Helpers::deltaPhi(electron_track->phi(), muon_track->phi())));
                                    if (elOrig_track) {
                                        ATH_MSG_DEBUG("origTrk: dtheta trk " << std::abs(elOrig_track->theta() - muon_track->theta())
                                            << ", dphi trk " << std::abs(xAOD::P4Helpers::deltaPhi(elOrig_track->phi(), muon_track->phi())));
                                    }
                                } else {
                                    elmutrackmatchOK = false;
                                }
                                ATH_MSG_DEBUG("track match " << (elmutrackmatchOK ? "OK" : "NOT OK"));
                            }

                            if (elmutrackmatchOK) {
                                passesFSR = true;
                                ATH_MSG_DEBUG("FSR match found");
                                break;
                            }
                        }
                    }
                }

                // Compute and write the output selection
                char outputValue;
                if (!m_vetoFSR) {
                    // Normal mode: output = WP || FSR
                    outputValue = (passesWP || passesFSR) ? 1 : 0;
                } else {
                    // Veto mode: output = WP && !FSR
                    outputValue = (passesWP && !passesFSR) ? 1 : 0;
                }

                m_outputDec.set(*eg, outputValue, sys);
                ATH_MSG_DEBUG("Output selection: " << static_cast<int>(outputValue)
                    << " (passesWP=" << passesWP << ", passesFSR=" << passesFSR << ")");
            }
        }

        ATH_MSG_DEBUG("Done !");

        return StatusCode::SUCCESS;
    }

}
