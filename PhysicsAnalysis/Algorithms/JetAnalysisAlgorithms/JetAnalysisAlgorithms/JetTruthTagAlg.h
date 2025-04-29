/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETANALYSISALGORITHMS_JETTRUTHTAGALG_H
#define JETANALYSISALGORITHMS_JETTRUTHTAGALG_H

#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "SystematicsHandles/SysReadHandle.h"
#include "SystematicsHandles/SysListHandle.h"
#include "xAODJet/JetContainer.h"

#include <optional>

namespace CP {
    /// @brief An algorithm for tagging reco-jets if they are close enough to a truth jet
    class JetTruthTagAlg final : public EL::AnaReentrantAlgorithm {
    public:
        using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
        virtual ~JetTruthTagAlg() override = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext &ctx) const override;

    private:
        SysListHandle m_systematicsList {this};
        SysReadHandle<xAOD::JetContainer> m_jets{this, "jets", "", "The input reco jet collection"};
        SG::ReadHandleKey<xAOD::JetContainer> m_truthJets{this, "truthJets", "AntiKt4TruthDressedWZJets", "The input truth jet collection"};
        Gaudi::Property<std::string> m_isHS{this, "isHSLabel", "isJvtHS", "The label to apply to HS-tagged jets"};
        Gaudi::Property<std::string> m_isPU{this, "isPULabel", "isJvtPU", "The label to apply to PU-tagged jets"};
        Gaudi::Property<float> m_hsMaxDR{this, "isHSMaxDR", 0.3, "Tag a reco jet as HS if it is at most this distance from a truth jet"};
        Gaudi::Property<float> m_hsMinPt{this, "isHSMinPt", 10e3, "Only consider truth jets above this pT for HS tagging"};
        Gaudi::Property<float> m_puMinDR{this, "isPUMinDR", 0.6, "Tag a reco jet as PU if it is at least this distance from any truth jet"};
        Gaudi::Property<float> m_puMinPt{this, "isPUMinPt", 0, "Only consider truth jets above this pT to veto jets for being PU tagged"};

        std::optional<SG::AuxElement::Decorator<char>> m_decIsHS;
        std::optional<SG::AuxElement::Decorator<char>> m_decIsPU;
    };
}

#endif //> !JETANALYSISALGORITHMS_JETTRUTHTAGALG_H
