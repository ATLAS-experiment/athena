/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_PREPDATATOSIMHITASSOCALG_H
#define MUONTRUTHALGSR4_PREPDATATOSIMHITASSOCALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteDecorHandleKey.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMeasurementBase/UncalibratedMeasurementContainer.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <span>
namespace MuonR4{
    /** @brief The PrepDataToSimHitAssocAlg matches the uncalibrated measurements to the MuonSimHits or 
     *         to the MuonSDO objects. For the 4 strip detector technologies, the association is based on the
     *         closest local distance of the produced sim hit to the uncalibrated measurement. For the Mdts,
     *         the Identifier of the SimHit needs to match the one of the drift circle. If the measurement can be
     *         matched, an ElementLink to the SimHit is decorated to the measurement */
    class PrepDataToSimHitAssocAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext & ctx) const override final;
        private:
           /** @brief Key to the associated simHit container */
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_simHitsKey{this, "SimHits" , ""};
            /** @brief Key to the uncalibrated measurements to decorate */
            SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_prdHitKey{this, "Measurements", "Measurements"};
            /** @brief Attached sim-hit link decoration to the measurement container */
            using LinkType = ElementLink<xAOD::MuonSimHitContainer>;
            SG::WriteDecorHandleKey<xAOD::UncalibratedMeasurementContainer> m_decorKey{this, "Decoration", m_prdHitKey, "simHitLink"};
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Key to the geometry context. Needed to align the hits inside ATLAS */
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief The number of standard deviations a measurement may be pulled apart in order to be associated */
            Gaudi::Property<double> m_PullCutOff{this, "AssocPull", 3.};
    };
}


#endif