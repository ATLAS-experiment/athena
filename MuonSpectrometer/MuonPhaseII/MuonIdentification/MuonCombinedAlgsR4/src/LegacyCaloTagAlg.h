/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_LEGACYCALOTAGALG_H
#define MUONCOMBINEDALGSR4_LEGACYCALOTAGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonTrackEvent/MuonTag.h"
#include "MuonCombinedEvent/InDetCandidateToTagMap.h"



namespace MuonCombinedR4{
    /** @brief The LegacyCaloTagAlg copies the output from the MuonCombinedCaloTagAl
     *         to a MuonTag container such that it can be swallowed by the new MuonCreatorAlg
     *         The Algorithm is meant to be placed TEMPORARY until the developments on the 
     *         new CaloTagging is mature enough */
    class LegacyCaloTagAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief The key of the selected ID track candidates extrapolated to the CaloExit  */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_idTrkKey{this, "IdTrackKey", "MuonInDetCandidates"};
            /** @brief Key to the legacy calorimeter tagging output */
            SG::ReadHandleKey<MuonCombined::InDetCandidateToTagMap> m_caloTagMap{this, "CaloTagMap", "caloTagMap"};
             /** @brief The key of the selected ID track candidates extrapolated to the CaloExit  */
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_writeKey{this, "WriteKey", "LegacyCaloTags"};
    };

}



#endif