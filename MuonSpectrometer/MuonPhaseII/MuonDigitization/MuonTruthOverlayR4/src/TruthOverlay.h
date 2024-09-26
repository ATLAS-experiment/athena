/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHOVERLAYR4_TRUTHOVERLAY_H
#define MUONTRUTHOVERLAYR4_TRUTHOVERLAY_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "GaudiKernel/SystemOfUnits.h"
namespace MuonR4{
    class TruthOverlay: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
        private:
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_sigKey{this, "SignalInputKey", ""};
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_bkgKey{this, "BkgInputKey", ""};
            SG::WriteHandleKey<xAOD::MuonSimHitContainer> m_writeKey{this, "WriteKey" , ""};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            /** @brief Define a detector dead time. Hits within this time window are deduplicated */
            Gaudi::Property<double> m_deadTime{this, "deadTime", 0. *Gaudi::Units::ns};
            /** @brief Define a detector merge time. Signals arriving within this time are merged.
             *         Their deposited energy is adde together. */
            Gaudi::Property<double> m_mergeTime{this, "mergeTime", 0. * Gaudi::Units::ns};

            Gaudi::Property<bool> m_includePileUpTruth{this, "IncludePileUpTruth", true, "Include pile-up truth info"};
            Gaudi::Property<int> m_vetoPileUpTruthLinks{this, "VetoPileUpTruthLinks", true,
                                                        "Ignore links to suppressed pile-up truth"};

    };
}



#endif
