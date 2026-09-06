/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_XAODSEGMENTCNVALG_H
#define MUONSEGMENTCNV_XAODSEGMENTCNVALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "MuonRecToolInterfacesR4/IxAODSegmentCnvTool.h"

#include "MuonPatternEvent/MuonPatternContainer.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripContainer.h"

#include "ActsEvent/AuxiliaryMeasurementHandler.h"

namespace MuonR4{
    /** @brief The xAODSegmentCnvAlg takes MuonR4::Segments and  converts them 
      *        into a single xAOD::MuonSegmentContainer. The segments carry the global
      *        segment position and direction, the fit quality in terms of chi2 & nDoF,
      *        and the hit, outlier and hole hit count summary. For the latter, the algortihm
      *        optionally launches a hole search using the Acts::TrackingGeometry. Further, 
      *        the local parameters and covariance from the fit are decorated onto the segment.
      *        Finally, the ElementLinks to the contributing measurements are attached. Prds from
      *        the uncombined space points (mainly Mdt, Mm, BI-RPC, sTGC pad) are directly appended
      *        to the list. The eta & phi measurements of Rpc/Tgc/sTgc which are within the same 
      *        gasgap are combined in a CombindMuonStrip object which itself is then appended
      *        to the list. Optionally, the conversion alg can also convert the beamspot measurement
      *        into a xAOD::UncalibratedMeasurement. For each measurement, the flag whether it was an
      *        outlier or not is also stored. */
    class xAODSegmentCnvAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        
        private:

            /** @brief Input segment container key */
            SG::ReadHandleKeyArray<SegmentContainer> m_readKeys{this, "InSegmentKeys", {"R4MuonSegments"}};
            /** @brief Output xAOD::segment container key */
            SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_writeKey{this, "OutSegmentKey", "MuonSegmentsFromR4"};
            /** @brief Segment converter tool */
            ToolHandle<IxAODSegmentCnvTool> m_segmentCnvTool{this, "SegmentCnvTool", ""};
            /** @brief Alignment container key */            
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            /** @brief Abrivation of the extra declared auxVariables  */
            using DecorKey_t = SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer>;
            /** @brief Decoration to the links to the associated Uncalibrated measurements */
            DecorKey_t m_prdLinkKey{this, "PrdLinkKey",  m_writeKey, "prdLinks" };
            /** @brief Decoration to the PrdLink state (I.e. outlier or valid) */
            DecorKey_t m_prdStateKey{this, "PrdStateKey", m_writeKey, "prdState"};
            /** @brief Decoration of the local segment parameters */
            DecorKey_t m_localSegParKey{this, "LocalSegParKey", m_writeKey, "localSegPars"};
            /** @brief Decoration of the local fit covariance parameters */
            DecorKey_t m_localSegCovKey{this, "LocalCovParKey", m_writeKey, "localSegCov"};
            /** @brief Decoration of the original segment */
            DecorKey_t m_parentSegKey{this, "ParentSegmentKey", m_writeKey, "parentSegment"};
            /** @brief Auxiliary container to model two measurements in the same gas gap as a single track state */
            SG::WriteHandleKey<xAOD::CombinedMuonStripContainer> m_combMeasKey{this, "combinedPrdKey", "CombinedMuonPrds"};
            /** @brief Handler to parse the auxiliary beam spot constaint */
            ActsTrk::AuxiliaryMeasurementHandler m_auxMeasProv{this};

            /** @brief Flag to convert the beamspot constaint as well */
            Gaudi::Property<bool> m_convertBeamSpot{this, "convertBeamSpot", false};
    };
}
#endif