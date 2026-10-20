/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_MeasTruthAssocMapProvider_H
#define MUONTRUTHALGSR4_MeasTruthAssocMapProvider_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODMuonPrepData/MuonMeasurementContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"


#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"


namespace MuonR4 {
    /** @brief The MeasurementTruthAssoMapProvider creates the Hit -> TruthAssociation maps which 
     *         are used by the Track -> truth matching algorithm to determine the truth matched
     *         particles from the ITk and the Muon hits. The Maps are generated per Uncalibrated
     *         measurement container */
    class MeasTruthAssocMapProvider : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Declare the container for which the Association map shall be built */
            SG::ReadHandleKey<xAOD::MuonMeasurementContainer> m_readKey{this, "PrdContainer", "SetMe"};
            /** @brief Declare the dependency on the associated sim hit */
            SG::ReadDecorHandleKey<xAOD::MuonMeasurementContainer> m_linkKey{this, "LinkKey", m_readKey, 
                                                                             "simHitLink"};
            /** @brief The dependency on the truth particle container */
            SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
            /** @brief Dependency on the TruthSegment -> TruthParticle association  */
            SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_segLinkKey{this, "SegLinkKey", m_truthKey,
                                                                              "truthSegmentLinks"};
            /** @brief Name of the written association map */
            SG::WriteHandleKey<ActsTrk::MeasurementToTruthParticleAssociation> m_writeKey{this, "AssociationMap", ""};
    };

}


#endif
