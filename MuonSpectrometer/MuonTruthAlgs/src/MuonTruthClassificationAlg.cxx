/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTruthClassificationAlg.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"

namespace Muon {

    // Initialize method:
    StatusCode MuonTruthClassificationAlg::initialize() {
        ATH_CHECK(m_truthRecordKey.initialize());
        ATH_CHECK(m_outTruthMuonKey.initialize());
        ATH_CHECK(m_truthOriginKey.initialize());
        ATH_CHECK(m_truthTypeKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_truthClassifier.retrieve());
        return StatusCode::SUCCESS;
    }

    // Execute method:
    StatusCode MuonTruthClassificationAlg::execute(const EventContext& ctx) const {
        // skip if no input data found
        SG::ReadHandle truthContainer(m_truthRecordKey, ctx);
        ATH_CHECK(truthContainer.isValid());

        // create output container
        SG::WriteHandle muonTruthContainer(m_outTruthMuonKey, ctx);
        ATH_CHECK(muonTruthContainer.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                            std::make_unique<xAOD::TruthParticleAuxContainer>()));
        ATH_MSG_DEBUG("Recorded TruthParticleContainer with key: " << m_outTruthMuonKey);

        // loop over truth coll
        for (const xAOD::TruthParticle* truth : *truthContainer) {
            if (!MC::isStable(truth)	 || !m_pdgIds.value().count(truth->absPdgId()) || truth->pt() < m_pt) continue;
            xAOD::TruthParticle* truthParticle = muonTruthContainer->push_back(std::make_unique<xAOD::TruthParticle>());
            truthParticle->setPdgId(truth->pdgId());
            truthParticle->setBarcode(HepMC::barcode(truth)); // FIXME barcode-based
            truthParticle->setStatus(truth->status());
            truthParticle->setPx(truth->px());
            truthParticle->setPy(truth->py());
            truthParticle->setPz(truth->pz());
            truthParticle->setE(truth->e());
            truthParticle->setM(truth->m());
            if (truth->hasProdVtx()) truthParticle->setProdVtxLink(truth->prodVtxLink());
            ElementLink<xAOD::TruthParticleContainer> truthLink(*muonTruthContainer, muonTruthContainer->size() - 1);
            truthLink.toPersistent();
            ATH_MSG_DEBUG("Found stable muon: " << truth->pt() << " eta " << truth->eta() << " phi " << truth->phi() << " mass "
                          << truth->m() << " barcode " << HepMC::barcode(truth) << " truthParticle->barcode "
                          << HepMC::barcode(truthParticle) << " (*truthLink)->barcode " << HepMC::barcode(*truthLink) << " "
                                                << truthLink); // FIXME barcode-based
            int iType = 0;
            int iOrigin = 0;

            // if configured look up truth classification
            if (!m_truthClassifier.empty()) {
                // if configured also get truth classification
                std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> truthClass =
                    m_truthClassifier->particleTruthClassifier(truth);
                iType = truthClass.first;
                iOrigin = truthClass.second;
                ATH_MSG_VERBOSE("Got truth type  " << iType << "  origin " << iOrigin);
                SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> truthOrigin(m_truthOriginKey, ctx);
                truthOrigin(*truthParticle) = iOrigin;
                SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> truthType(m_truthTypeKey, ctx);
                truthType(*truthParticle) = iType;
            }

            ATH_MSG_DEBUG("good muon with type " << iType << " and origin" << iOrigin);
        }

        ATH_MSG_DEBUG("Registered " << muonTruthContainer->size() << " truth muons ");

        return StatusCode::SUCCESS;
    }

}  // namespace Muon

