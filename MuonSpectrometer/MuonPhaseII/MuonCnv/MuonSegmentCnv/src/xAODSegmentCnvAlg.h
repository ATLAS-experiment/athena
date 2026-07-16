/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_XAODSEGMENTCNVALG_H
#define MUONSEGMENTCNV_XAODSEGMENTCNVALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODMuonViews/FillContainer.h"
#include "StoreGate/WriteDecorHandle.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPatternEvent/MuonPatternContainer.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripAuxContainer.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h" 
#include "ActsEvent/AuxiliaryMeasurementHandler.h"

#include <Acts/Propagator/StraightLineStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/Navigator.hpp>

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
            /** @brief Abrivation to call an uncalibrated measurement container */
            using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
            /** @brief Abrivation to call the link to an element inside an 
             *         uncalibrated measurement container */
            using PrdLink_t = ElementLink<PrdCont_t>;
            /** @brief Abrivation of a collection of Prd links */
            using PrdLinkVec_t = std::vector<PrdLink_t>;

            /** @brief Auxiliary struct to ship the WriteDecorHandle and writeHandles
             *         needed to link the uncalibrated meausrements to the segment. */
            struct PrepDataCollectorShip {
                /** @brief Constructor to initialize the ship
                 *  @param parent: The reference to the instantiatting algorithm. 
                 *                 Needed to fetch the neccessary keys
                 * @param ctx: Current EventContext to instantiate the handles
                 * @param tgContext: The geometry context to instantiate the auxiliary
                 *                    measurement creator */
                PrepDataCollectorShip(const xAODSegmentCnvAlg& parent,
                                      const EventContext& ctx,
                                      const Acts::GeometryContext& tgContext):
                    dec_prdLinks{parent.m_prdLinkKey, ctx},
                    dec_prdStates{parent.m_prdStateKey, ctx},
                    beamSpotMeasCreator{parent.m_auxMeasProv.makeHandle(ctx, tgContext)}{}
                /** @brief List of element links to the associated measurements */
                SG::WriteDecorHandle<xAOD::MuonSegmentContainer, PrdLinkVec_t> dec_prdLinks;
                /** @brief Flag indicating whether each measurement is valid or an outlier */
                SG::WriteDecorHandle<xAOD::MuonSegmentContainer, std::vector<char>> dec_prdStates;
                /** @brief Container handle to create the beamspot measurement, if needed */
                Acts::Result<ActsTrk::AuxiliaryMeasurementHandler::MeasurementProvider, 
                            ActsTrk::AuxiliaryMeasurementHandler::HandleStatus> beamSpotMeasCreator;
                /** @brief Container handle to create the combined measurements of Rpc/Tgc/sTgc */
                xAOD::FillContainer<xAOD::CombinedMuonStripContainer,
                            xAOD::CombinedMuonStripAuxContainer> prdCombContainer{};
                /** @brief Pointer to the commonly shared beamspot measurement */
                const xAOD::UncalibratedMeasurement* beamSpot{};
            };

            /** @brief Helper struct to keep track of the number of precision,
              *        trigger phi and trigger eta hits */
            struct Counter{
                std::uint8_t precision{0};
                std::uint8_t triggerPhi{0};
                std::uint8_t triggerEta{0};
            };
            /** @brief Decorate the prd links onto the output muon segment. Eta & phi measurements are absorbed converted
             *         into a CombinedMuonStrip which is a source link linke object carrying a link to both prds. In this way,
             *         only one track state is generated later in the track fit from the two measurements. 
             *         Two assumptions are made for the linking
             *                - There's exclusivley one eta & one phi measurement @maximum on the segment
             *                - The measurements are sorted along the segment trajectory.
             *  @param gctx: The geometry context to construct the local segment parameters  
             *  @param inSegment: The segment from which the space points are collected
             *  @param targetSegment: The xAOD segment onto which all the links are decorated
             *  @param ship: Reference to the auxiliary ship carrying all the write decor handles
             *               needed for the linking */
            StatusCode linkMeasurements(const ActsTrk::GeometryContext& gctx,                       
                                        const Segment& inSegment,
                                        xAOD::MuonSegment& targetSegment,
                                        PrepDataCollectorShip& ship) const;

            /** @brief Calculate the number of hits, outliers and holes on the segment
             *         and set the hit summary fields of the passed segment
             *  @param ctx: EventContext to access conditions data and to propagate
             *  @param segment: The segment for which the hit summary shall be evaluated */
            void evaluateSummary(const EventContext& ctx, 
                                 xAOD::MuonSegment& segment) const;

            /** @brief Find potential sensitive surfaces crossed by the segment but without
              *        contributing measurements. The segment line is propagated from the startPars
              *        to the target surface + some margin using the Acts::Propagator. The propaator
              *        records every surface crossing. The associated SurfacePlacements are then used
              *        to determine the ATLAS identifier and then to assign whether there's a hole or not.
              * @param tgContext: The ATLAS geometry context to align the surfaces during propagation
              * @param startPars: The start parameters from which the propagation starts. The surface is the 
              *                    bottom boundary surface of the volume where's the first segment measurement.
              * @param target: The top boundary surface of the volume with the last segment measurement.
              * @param geoIdsWithHits: The list of ATLAS identifiers with segmen measurements (outlier + hit)
              * @param holeCounter: Mutable reference to the segment hole counter.  */                   
            void findHoles(const Acts::GeometryContext& tgContext,
                           const Acts::BoundTrackParameters& startPars,
                           const Acts::Surface* target,
                           const std::unordered_set<Identifier>& geoIdsWithHits,
                           Counter& holeCounter) const;

            /** @brief IdHelperSvc for Identifier printing & manipulation */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc",  
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Input segment container key */
            SG::ReadHandleKeyArray<SegmentContainer> m_readKeys{this, "InSegmentKeys", {"R4MuonSegments"}};
            /** @brief Output segment container key */
            SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_writeKey{this, "OutSegmentKey", "MuonSegmentsFromR4"};
            /** @brief Alignment container key */            
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Tracking geometry tool to search for holes  */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
 
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
            /** @brief Flag to tell whether the hole summary shall be written */
            Gaudi::Property<bool> m_estimateHoles{this, "estimateHoles", true};
            /** @brief Abrivation for the straight line propagator to detect the holes */
            using Propagator_t = Acts::Propagator<Acts::StraightLineStepper, Acts::Navigator>;
            /** @brief Straight line propagtor needed for hole search  */
            std::unique_ptr<Propagator_t> m_propagator{};
    };
}
#endif