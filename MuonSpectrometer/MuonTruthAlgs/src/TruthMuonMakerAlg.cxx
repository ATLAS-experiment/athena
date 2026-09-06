/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthMuonMakerAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODMuonViews/ContainerDecorator.h"

namespace {
    using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
}
namespace Muon {

    // Initialize method:
    StatusCode TruthMuonMakerAlg::initialize() {
        if (m_truthRecordKeys.empty()) {
            ATH_MSG_ERROR("No truth container configured");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_truthRecordKeys.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_truthOriginKey.initialize());
        ATH_CHECK(m_truthTypeKey.initialize());
        ATH_CHECK(m_truthClassificationKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_truthClassifier.retrieve());
        return StatusCode::SUCCESS;
    }

    // Execute method:
    StatusCode TruthMuonMakerAlg::execute(const EventContext& ctx) const {
        // create output container
        SG::WriteHandle muonTruthContainer(m_writeKey, ctx);
        ATH_CHECK(muonTruthContainer.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                            std::make_unique<xAOD::TruthParticleAuxContainer>()));
        ATH_MSG_DEBUG("Recorded TruthParticleContainer with key: " << m_writeKey);

        xAOD::ContainerDecorator<xAOD::TruthParticleContainer, int> truthOrigin{m_truthOriginKey, ctx};
        xAOD::ContainerDecorator<xAOD::TruthParticleContainer, int> truthType{m_truthTypeKey, ctx};
        xAOD::ContainerDecorator<xAOD::TruthParticleContainer, unsigned int> truthClassification{m_truthClassificationKey, ctx};
        xAOD::ContainerDecorator<xAOD::TruthParticleContainer, TruthLink_t> truthLink{m_truthLinkKey, ctx};

        // loop over truth coll
        const xAOD::TruthParticleContainer* truthContainer{nullptr};

        for (const SG::ReadHandleKey<xAOD::TruthParticleContainer>& key : m_truthRecordKeys) {
            ATH_CHECK(SG::get(truthContainer,key, ctx));
 
            for (const xAOD::TruthParticle* truth : *truthContainer) {
                if (!MC::isStable(truth) || !m_pdgIds.value().count(truth->absPdgId()) || truth->pt() < m_pt) continue;
                xAOD::TruthParticle* truthParticle = muonTruthContainer->push_back(std::make_unique<xAOD::TruthParticle>());
                truthParticle->setPdgId(truth->pdgId());
                truthParticle->setUid(HepMC::uniqueID(truth));
                truthParticle->setStatus(truth->status());
                truthParticle->setPx(truth->px());
                truthParticle->setPy(truth->py());
                truthParticle->setPz(truth->pz());
                truthParticle->setE(truth->e());
                truthParticle->setM(truth->m());
                if (truth->hasProdVtx()) truthParticle->setProdVtxLink(truth->prodVtxLink());

                TruthLink_t itruthLink(*truthContainer, truth->index());
                itruthLink.toPersistent();
                truthLink(*truthParticle) = itruthLink;
                ATH_MSG_DEBUG("Found stable muon: " << truth->pt() << " eta " << truth->eta() << " phi " << truth->phi() << " mass "
                              << truth->m() << " unique ID " << HepMC::uniqueID(truth) << " HepMC::uniqueID(truthParticle) "
                              << HepMC::uniqueID(truthParticle) << " HepMC::uniqueID(*itruthLink) " << HepMC::uniqueID(*itruthLink) << " "
                              << itruthLink);
                int iType{0}, iOrigin{0};
                unsigned int iClassification{0};

                // if configured look up truth classification
                if (!m_truthClassifier.empty()) {
                    // if configured also get truth classification
                    std::pair<MCTruthPartClassifier::ParticleType, 
                              MCTruthPartClassifier::ParticleOrigin> truthClass =
                        m_truthClassifier->particleTruthClassifier(truth);
                    iType = truthClass.first;
                    iOrigin = truthClass.second;
                    iClassification = std::get<0>(MCTruthPartClassifier::defOrigOfParticle(truth)); // See AGENE-2351
                    ATH_MSG_VERBOSE("Got truth type  " << iType << "  origin " << iOrigin << " classification " << iClassification);
                }
                truthOrigin(*truthParticle) = iOrigin;
                truthType(*truthParticle) = iType;
                truthClassification(*truthParticle) = iClassification;

                ATH_MSG_DEBUG("good muon with type " << iType << " origin" << iOrigin << " and classification " << iClassification);
            }
        }

        ATH_MSG_DEBUG("Registered " << muonTruthContainer->size() << " truth muons ");

        return StatusCode::SUCCESS;
    }

}  // namespace Muon
