/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORK_MuonTruthIsolationDecorAlg_H
#define DERIVATIONFRAMEWORK_MuonTruthIsolationDecorAlg_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"

namespace DerivationFramework {
    class MuonTruthIsolationDecorAlg : public AthReentrantAlgorithm {
    public:
        /** Constructor with parameters */
        MuonTruthIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator);
        /** Destructor */
        virtual ~MuonTruthIsolationDecorAlg() = default;

        StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        SG::ReadHandleKey<xAOD::IParticleContainer> m_partSGKey{this, "ContainerKey", "",
                                                                "Key of the container to run the truth isolation on "};
        SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthSGKey{this, "TruthContainerKey", "TruthEvents"};

        /// Decor handle keys. Never set them via the JO as they're overwritten
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_topoetcone20_Key{this, "Key_TopoEtCone20", m_partSGKey, ""};
        /// PtCone20
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ptcone20_pt500_Key{this, "Key_PtCone20_pt500",m_partSGKey, ""};
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ptcone20_Key{this, "Key_PtCone20", m_partSGKey, ""};
        /// PtVarCone20
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ptvarcone20_pt500_Key{this, "Key_PtVarCone20_pt500",m_partSGKey, ""};
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ptvarcone20_Key{this, "Key_PtVarCone20", m_partSGKey, ""};
        /// Pt varcone 30
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ptvarcone30_pt500_Key{this, "Key_PtVarCone30_pt500", m_partSGKey, ""};
        SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_ptvarcone30_Key{this, "Key_PtVarCone30", m_partSGKey,""};
    };
}  // namespace DerivationFramework
#endif  //
