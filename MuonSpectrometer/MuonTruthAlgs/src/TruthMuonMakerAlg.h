/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  * @brief This algorithm retrieves the container of all the truth particles coming from the MC generator and copies
  *        the truth muons in a new container. Muons'properties include PDG, MC BARCODE, status, momentum components, 
  *        charge and mass. The link to the production vertex, if present, is also attached. 
  *        Then, truth muons are decorated with type and origin.
    
*/

#ifndef MUONTRUTHALGS_TruthMuonMakerAlg_H
#define MUONTRUTHALGS_TruthMuonMakerAlg_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace Muon {

    class TruthMuonMakerAlg : public AthReentrantAlgorithm {
    public:

        // Constructor with parameters:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        // Basic algorithm methods:
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        // ReadHandleKey for the truth particles'container and WriteHandleKeys for the truth muons'container and muon origin decorators
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthRecordKey{this, "TruthParticleContainerName", "TruthParticles"};

        SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outTruthMuonKey{this, "MuonTruthParticleContainerName","MuonTruthParticles"};
        /// FIXME WriteDecorHandle should not be used for additional
        /// dynamic variables applied by the same algorithm which
        /// created the container, instead SG::AuxElement::Accessor
        /// should be used.
        SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_truthOriginKey{this, "truthOriginKey", m_outTruthMuonKey, "truthOrigin"};
        SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_truthTypeKey{this, "truthTypeKey", m_outTruthMuonKey, "truthType"};
        SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_truthClassificationKey{this, "truthClassificationKey", m_outTruthMuonKey, "truthClassification"};
        SG:: WriteDecorHandleKey<xAOD::TruthParticleContainer> m_truthLinkKey{this, "truthLinkKey", m_outTruthMuonKey, "truthParticleLink"};
        Gaudi::Property<float> m_pt{this, "ptCut", 1000.};

        Gaudi::Property<std::set<int>> m_pdgIds{this, "pdgIds", {13,}};

        //IdHelper service handle
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        //TruthClassifier tool handle
        ToolHandle<IMCTruthClassifier> m_truthClassifier{this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier"};

    };

}  // namespace Muon
#endif
