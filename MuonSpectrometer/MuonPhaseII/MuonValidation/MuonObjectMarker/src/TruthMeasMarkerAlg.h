/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONOBJECTMARKER_TRUTHMEASMARKER_H
#define MUONOBJECTMARKER_TRUTHMEASMARKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    /*** @brief TruthMeasMarkerAlg loops over the truth segments and searches the xAOD::UncalibrtedMeasurements 
     *          matched to the sim hits making up the segment */
    class TruthMeasMarkerAlg: public AthReentrantAlgorithm {
       public:
         using AthReentrantAlgorithm::AthReentrantAlgorithm;
         ~TruthMeasMarkerAlg() = default;
          
          virtual StatusCode initialize() override final;
          virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Key to the primary muon container to select the muon from  */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segKey{this, "SegmentKey", "MuonTruthSegments" };
            /** @brief Key that's decorated to mark the uncalibrated measurement */
            Gaudi::Property<std::string> m_writeMarker{this, "writeMarker", "matchedToTruthSeg"};
            /** @brief Key to indicate the associated MuonSegment link */
            Gaudi::Property<std::string> m_segLink{this, "SegmentLinkKey", "truthSegmentLinks"};
            /** @brief Key to the segment container to fetch the marked segments */
            SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_measKeys{this, "PrdContainer",{}};
            /** @brief Key to the marker decoration. Will be copied from writeMarker */
            SG::WriteDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_writeMarkKeys{this, "OutMarkerKeys", {}};
            /** @brief Key to the segment link decoration. */
            SG::WriteDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_writeSegLinkKeys{this, "SegLinkKeys", {}};
    };
}

#endif