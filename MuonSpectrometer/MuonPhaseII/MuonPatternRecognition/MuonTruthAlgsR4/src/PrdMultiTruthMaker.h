/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_PRDMULTITRUTHMAKER_H
#define MUONTRUTHALGSR4_PRDMULTITRUTHMAKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    /** @brief The PrdMultiTruthMaker translates the UncalibratedMeasurementContainers of
     *         a given MS technology to PRD_MultiTruthCollections, which are later needed for
     *         a TruthParticle <-> SDO matching. The HepMCParticleLinks of the associated SDO
     *         hits are directly translated into a PRD_MultiTruthCollection object. */
    class PrdMultiTruthMaker : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            
            using xAODPrdCont_t = xAOD::UncalibratedMeasurementContainer;
            using xAODPrdKey_t = SG::ReadHandleKey<xAODPrdCont_t>;
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            Gaudi::Property<std::string> m_simLinkDecor{this, "SimHitLinkDecoration", "simHitLink"};

            SG::ReadHandleKeyArray<xAODPrdCont_t> m_xAODPrdKeys{this, "PrdContainer", {}};
            SG::ReadDecorHandleKeyArray<xAODPrdCont_t> m_simHitDecorKeys{this, "TruthSimDecorKeys", {}};

            SG::WriteHandleKey<PRD_MultiTruthCollection> m_writeKey{this, "WriteKey", ""};
    };
}
#endif