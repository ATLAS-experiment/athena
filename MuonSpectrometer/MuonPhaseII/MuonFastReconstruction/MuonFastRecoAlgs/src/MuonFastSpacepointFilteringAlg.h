/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONFASTRECOALGS_MUONFASTSPACEPOINTFILTERINGALG__H
#define MUONR4_MUONFASTRECOALGS_MUONFASTSPACEPOINTFILTERINGALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"


#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonFastRecoEvent/GlobalPattern.h"

namespace MuonR4 {
    /// @brief Algortithm to filter out the space points not associated with
    ///        any global pattern.
    ///
    /// This algorithm consumes the global patterns found in previous steps 
    /// and filters out the space points not associated with any global pattern. 
    /// This is done to reduce the number of space points to be used in the 
    /// subsequent steps of the Phase-2 fast reconstruction. It splits the
    /// filtered space points into two containers, one for NSW space points
    /// and one for non-NSW space points.

    class MuonFastSpacepointFilteringAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~MuonFastSpacepointFilteringAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            /** @brief Write handle key for the output space points */ 
            SG::WriteHandleKey<SpacePointContainer> m_outSpacePoints{this, "OutSpacePoints", "MuonFastRecoSpacePoints", "Output space point container"};
            /** @brief Write handle key for the output NSW space points */ 
            SG::WriteHandleKey<SpacePointContainer> m_outNswSpacePoints{this, "OutNswSpacePoints", "MuonFastRecoNswSpacePoints", "Output NSW space point container"};
            /** @brief Input global patterns */ 
            SG::ReadHandleKey<GlobalPatternContainer> m_inPatterns{this, "InPatterns", "MuonR4GlobalPatterns", "Input global pattern container"};
    };
}

#endif