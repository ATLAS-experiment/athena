/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "IsoCloseByCorrectionAlg.h"

#include <IsolationSelection/IsolationCloseByCorrectionTool.h>

#include <algorithm>

#include "FourMomUtils/xAODP4Helpers.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "AsgTools/CurrentContext.h"

namespace CP {

    IsoCloseByCorrectionAlg::IsoCloseByCorrectionAlg(const std::string& name, ISvcLocator* svcLoc) :
        AthReentrantAlgorithm(name, svcLoc) {}
    StatusCode IsoCloseByCorrectionAlg::initialize() {

        ATH_MSG_INFO("Initialize IsoCloseByCorrectionAlg " );


        for (const SG::ReadHandleKey<xAOD::IParticleContainer>& contKey : m_contKeys) {
            ATH_MSG_INFO("Initialize " <<  contKey.key());
        }
        if (!m_muonSelKey.empty()) ATH_MSG_INFO("Initialize muon sel " <<  m_muonSelKey.key() << ", " << m_muonSelKey.isEventStore() );
        if (!m_elecSelKey.empty()) ATH_MSG_INFO("Initialize elec sel " <<  m_elecSelKey.key() << ", " << m_elecSelKey.isEventStore() );
        if (!m_photSelKey.empty()) ATH_MSG_INFO("Initialize phot sel " <<  m_photSelKey.key() << ", " << m_photSelKey.isEventStore() );
        ATH_MSG_INFO("Initialize MinMuonPt " <<  m_minMuonPt.value());
        ATH_MSG_INFO("Initialize MinElecPt " <<  m_minElecPt.value());
        ATH_MSG_INFO("Initialize MinPhotPt " <<  m_minPhotPt.value());
        if (m_muonSelTool.isEnabled()) ATH_MSG_INFO("Initialize muon sel tool " <<  m_muonSelTool.name());
        if (m_elecSelTool.isEnabled()) ATH_MSG_INFO("Initialize elec sel tool " <<  m_elecSelTool.name());
        if (m_photSelTool.isEnabled()) ATH_MSG_INFO("Initialize phot sel tool " <<  m_photSelTool.name());        

        ATH_CHECK(m_contKeys.initialize(!m_contKeys.empty()));
        ATH_CHECK(m_muonSelKey.initialize(!m_muonSelKey.empty()));
        ATH_CHECK(m_elecSelKey.initialize(!m_elecSelKey.empty()));
        ATH_CHECK(m_photSelKey.initialize(!m_photSelKey.empty()));
        return StatusCode::SUCCESS;
    }

    StatusCode IsoCloseByCorrectionAlg::execute(const EventContext& ctx) const {

        ATH_MSG_DEBUG("execute: entering " );

        // Loop over input IParticleContainer, and fill different ConstDataVectors for muons, electrons and photons.
        // There may be more than one container for each.
        // Then apply selections of objects, decorating with "isoSelIsOK" for the IsoCloseByTool, and then pass the ConstDataVectors to the tool.

        // Use isLRT decoration for LLP particles to avoid looking for tracks from the primary vertex in the closeBy tool
        SG::AuxElement::Decorator<char> isLRT("isLRT");


        ConstDataVector<xAOD::MuonContainer>     muons{SG::VIEW_ELEMENTS};
        ConstDataVector<xAOD::ElectronContainer> electrons{SG::VIEW_ELEMENTS};
        ConstDataVector<xAOD::PhotonContainer>   photons{SG::VIEW_ELEMENTS};
        
        for (const SG::ReadHandleKey<xAOD::IParticleContainer>& contKey : m_contKeys) {
            SG::ReadHandle<xAOD::IParticleContainer> parts (contKey, ctx);
            for ( const xAOD::IParticle* part : *parts ) {

                // flag LLP particles
                isLRT(*part) = (contKey.key().find("LRT")  != std::string::npos);

                // Check type of container and apply selection as appropriate
                if (part->type() == xAOD::Type::Muon) {
                    // cast to muon container
                    const xAOD::Muon* muon = static_cast<const xAOD::Muon*>(part);
                    muons.push_back(muon);
                }
                else if (part->type() == xAOD::Type::Electron) {
                     // cast to electron container
                    const xAOD::Electron* electron = static_cast<const xAOD::Electron*>(part);
                    electrons.push_back(electron);
                }
                else if (part->type() == xAOD::Type::Photon) {
                    // cast to photon container
                    const xAOD::Photon* photon = static_cast<const xAOD::Photon*>(part);
                    photons.push_back(photon);
                }
            }
        }

        ATH_MSG_DEBUG("execute: apply selections " );

        /// Apply selection to muons, electrons and photons - setting selection decorator
        ATH_CHECK(selectLeptonsAndPhotons(ctx, muons));
        ATH_CHECK(selectLeptonsAndPhotons(ctx, electrons));
        ATH_CHECK(selectLeptonsAndPhotons(ctx, photons));

        ATH_MSG_DEBUG("execute: apply closeBy correction " );

        /// Now apply correction to close by leptons and photons
        if (m_closeByCorrTool->getCloseByIsoCorrection(ctx, electrons.asDataVector(), muons.asDataVector(), photons.asDataVector()) == CorrectionCode::Error) {
            ATH_MSG_FATAL("Failed to do close by iso correction ");
            return StatusCode::FAILURE;
        }

        ATH_MSG_DEBUG("execute: after closeBy correction " );

        return StatusCode::SUCCESS;
    }

    template <class CONT_TYPE>
    StatusCode IsoCloseByCorrectionAlg::selectLeptonsAndPhotons(const EventContext& ctx, 
                                                                CONT_TYPE particles) const {

        ATH_MSG_DEBUG("selectLeptonsAndPhotons: entering" );

        for ( auto particle : particles ) {
            ATH_MSG_DEBUG("selectLeptonsAndPhotons: pt, eta, ph " << particle->pt()/1000. << ", " <<  particle->eta() << ", " <<  particle->phi() );
            ATH_CHECK(applySelection(ctx, particle));
        }
        return StatusCode::SUCCESS;
    } 

    StatusCode IsoCloseByCorrectionAlg::applySelection(const EventContext& ctx, const xAOD::Muon* muon) const {

        // outgoing selection decorator
        SG::AuxElement::Decorator<char> isOK("isoSelIsOK");

        // Check incoming selection decorator
        if (!m_muonSelKey.empty()) {
            SG::ReadDecorHandle<xAOD::MuonContainer, char>  decor{m_muonSelKey, ctx};
            if (!decor(*muon)) {
                ATH_MSG_VERBOSE("applySelection: muon fails " << m_muonSelKey.key());
                isOK(*muon) = false;
                return StatusCode::SUCCESS;
            }
        }

        // Check incoming selection tool
        if (!m_muonSelTool.empty()) {
            if (!m_muonSelTool->accept(*muon)) {
                ATH_MSG_VERBOSE("applySelection: muon fails Loose cut");
                isOK(*muon) = false;
                return StatusCode::SUCCESS;
            }
        } 
        // Check pt
        if (muon->pt() < m_minMuonPt) {
            ATH_MSG_VERBOSE("applySelection: muon fails pt cut: " << muon->pt() << ", " << m_minMuonPt);
            isOK(*muon) = false;
            return StatusCode::SUCCESS;
        }

        isOK(*muon) = true;
        ATH_MSG_VERBOSE("applySelection: " << muon->type() << ", " << muon->pt() << ", " << muon->eta() << ", " << muon->phi() << ", " << (int)isOK(*muon));

        return StatusCode::SUCCESS;
    }

    StatusCode IsoCloseByCorrectionAlg::applySelection(const EventContext& ctx, const xAOD::Electron* elec) const {

        // outgoing selection decorator
        SG::AuxElement::Decorator<char> isOK("isoSelIsOK");

        // Check incoming selection decorator
        if (!m_elecSelKey.empty()) {
            SG::ReadDecorHandle<xAOD::ElectronContainer, char> decor{m_elecSelKey, ctx};
            if (!decor(*elec)) {
                ATH_MSG_VERBOSE("applySelection: electron fails " << m_elecSelKey.key());
                isOK(*elec) = false;
                return StatusCode::SUCCESS;
            }
        }
        // Check incoming selection tool
        if (!m_elecSelTool.empty()) {
            if (!m_elecSelTool->accept(ctx, elec)) {
                ATH_MSG_VERBOSE("applySelection: electron fails VeryLooseLH cut");
                isOK(*elec) = false;
                return StatusCode::SUCCESS;
            }
        } 
        // Check pt
        if (elec->pt() < m_minElecPt) {
            ATH_MSG_VERBOSE("applySelection: electron fails pt cut: " << elec->pt() << ", " << m_minElecPt);
            isOK(*elec) = false;
            return StatusCode::SUCCESS;
        }
        isOK(*elec) = true;

        ATH_MSG_VERBOSE("applySelection: " << elec->type() << ", " << elec->pt() << ", " << elec->eta() << ", " << elec->phi() << ", " << (int)isOK(*elec));
        
        return StatusCode::SUCCESS;
    }

    StatusCode IsoCloseByCorrectionAlg::applySelection(const EventContext& ctx, const xAOD::Photon* phot) const {

        // outgoing selection decorator
        SG::AuxElement::Decorator<char> isOK("isoSelIsOK");

        // Check incoming selection decorator
        if (!m_photSelKey.empty()) {
            SG::ReadDecorHandle<xAOD::PhotonContainer, char> decor{m_photSelKey, ctx};
            if (!decor(*phot)) {
                ATH_MSG_VERBOSE("applySelection: photon fails " << m_photSelKey.key());
                isOK(*phot) = false;
                return StatusCode::SUCCESS;
            }
        }

        // Check incoming selection tool
        if (!m_photSelTool.empty()) {
            if (!m_photSelTool->accept(ctx, phot)) {
                ATH_MSG_VERBOSE("applySelection: photon fails IsEMLoose cut");
                isOK(*phot) = false;
                return StatusCode::SUCCESS;
            }
        }

        // Check pt
        if (phot->pt() < m_minPhotPt) {
            ATH_MSG_VERBOSE("applySelection: photon fails pt cut: " << phot->pt() << ", " << m_minPhotPt);
            isOK(*phot) = false;
            return StatusCode::SUCCESS;
        }

        isOK(*phot) = true;

        ATH_MSG_VERBOSE("applySelection: " << phot->type() << ", " << phot->pt() << ", " << phot->eta() << ", " << phot->phi() << ", " << (int)isOK(*phot));
        
        return StatusCode::SUCCESS;
    }

}  // namespace CP
