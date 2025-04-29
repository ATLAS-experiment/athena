/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONOBJECTMARKER_MEASUREMENTMARKERALG_H
#define MUONOBJECTMARKER_MEASUREMENTMARKERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

namespace MuonR4{
  /** @brief Algorithm that marks the uncalibrated measurements making up a segment, where the segment
   *          passes an arbitrary selection requirement. The requirement needs to be decorated by an algorithm
   *          upstream with a <bool> decorator. The same decoration is then copied to the measurements.
   *          If "SegmentLinkKey" is not empty, the algorithm additionally decorates a link to the segment. */  
  class MeasurementMarkerAlg: public AthReentrantAlgorithm{
       public:
         using AthReentrantAlgorithm::AthReentrantAlgorithm;
         ~MeasurementMarkerAlg() = default;
          
          virtual StatusCode initialize() override final;
          virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Key to the primary muon container to select the muon from  */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segKey{this, "SegmentKey", "MuonSegments" };
            /** @brief Key to the decoration to fetch the marked muons */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_readMarkKey{this, "SelectSegments", m_segKey, ""};
            /** @brief Key to the prd association decoration */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_prdLinkKey{this, "PrdAssocKey", m_segKey, "prdLinks"};
            /** @brief Key to the segment container to fetch the marked segments */
            SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_measKeys{this, "PrdContainer",{}};
            /** @brief Key to the decoration. Will be copied from m_readMarkKey */
            SG::WriteDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_writeMarkKeys{this, "OutMarkerKeys", {}};

            Gaudi::Property<std::string> m_segLink{this, "SegmentLinkKey", ""};
            /** @brief Key to the decoration. Will be copied from m_segLink */
            SG::WriteDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_writeSegLinkKeys{this, "SegLinkKeys", {}};

    };
}

#endif