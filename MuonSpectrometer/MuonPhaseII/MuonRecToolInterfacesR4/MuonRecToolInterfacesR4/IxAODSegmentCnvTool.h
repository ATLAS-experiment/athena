/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_IXAODSEGMENTCNVTOOL_H
#define MUONRECTOOLINTERFACESR4_IXAODSEGMENTCNVTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include "StoreGate/WriteDecorHandleKey.h"

#include "MuonPatternEvent/MuonHoughDefs.h"
#include "MuonPatternEvent/Segment.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripAuxContainer.h"
#include "xAODMuonViews/FillContainer.h"
#include "xAODMuonViews/ContainerDecorator.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h" 
#include "ActsEvent/AuxiliaryMeasurementHandler.h"

namespace MuonR4{
    /** @brief The xAODSegmentCnvTool takes a MuonR4::Segment and converts it into a
      *        xAOD::MuonSegment. The segments carry the global
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

    class IxAODSegmentCnvTool : virtual public IAlgTool {
        public:
            virtual ~IxAODSegmentCnvTool() = default;
            
            DeclareInterfaceID(IxAODSegmentCnvTool, 1, 0);

            /** @brief Structure to hold the data needed for the conversion and decoration */
            struct DataShip {
                using DecorKey_t = SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer>;
                /** @brief Initialize the linking of the local segment parameters and covariance
                 *  @param localParKey The key for the local segment parameter decoration
                 *  @param localCovKey The key for the local segment covariance decoration
                 *  @param ctx The event context
                 *  @return StatusCode indicating success or failure */
                StatusCode setupLocalParameters(const DecorKey_t& localParKey,
                                                const DecorKey_t& localCovKey,
                                                const EventContext& ctx) {
                    if (localParKey.empty() || localCovKey.empty()) {
                        return StatusCode::FAILURE;
                    }
                    dec_localSegPars.initialize(localParKey, ctx);
                    dec_localSegCov.initialize(localCovKey, ctx);
                    return StatusCode::SUCCESS;              
                }
   
                /** @brief Instantiate the linking of the measurements
                 *  @param combinedKey The key for the combined measurement decoration
                 *  @param prdLinkKey The key for the measurement link decoration
                 *  @param prdStateKey The key for the measurement state decoration
                 *  @param ctx The event context
                 *  @return StatusCode indicating success or failure */
                StatusCode setupMeasurementLink(const SG::WriteHandleKey<xAOD::CombinedMuonStripContainer>& combinedKey,
                                                const DecorKey_t& prdLinkKey,
                                                const DecorKey_t& prdStateKey,
                                                const EventContext& ctx) {
                    if (combinedKey.empty() || prdLinkKey.empty() || prdStateKey.empty()) {
                        return StatusCode::FAILURE;
                    }
                    dec_prdLinks.initialize(prdLinkKey, ctx);
                    dec_prdStates.initialize(prdStateKey, ctx);
                    return prdCombContainer.record(combinedKey, ctx);
                }

                /** @brief Initialize the creation & linking of the beamspot measurement
                 *  @param auxMeasProv The auxiliary measurement provider
                 *  @param ctx The event context
                 *  @param gctx The geometry context
                 *  @return StatusCode indicating success or failure */
                StatusCode setupBeamSpotMeasurement(const ActsTrk::AuxiliaryMeasurementHandler& auxMeasProv,
                                                    const EventContext& ctx,
                                                    const ActsTrk::GeometryContext& gctx) {
                    beamSpotMeasCreator = auxMeasProv.makeHandle(ctx, gctx.context());
                    if (!beamSpotMeasCreator.ok()) {
                        return StatusCode::FAILURE;
                    }
                    return StatusCode::SUCCESS;
                }

                /** @brief Output segment container */
                xAOD::FillContainer<xAOD::MuonSegmentContainer,
                                    xAOD::MuonSegmentAuxContainer> segmentContainer{};
                
                /** @brief Abrivation of the decorated local segment parameters */
                using SegPars_t = xAOD::PosAccessor<Acts::toUnderlying(SegmentFit::ParamDefs::nPars)>::element_type;
                /** @brief Abrivation of the decorated local segment covariance */
                using SegCov_t = xAOD::PosAccessor<Acts::sumUpToN(Acts::toUnderlying(SegmentFit::ParamDefs::nPars))>::element_type;
    
                /** @brief Decoration of the 5 local segment parameters (not persitifiable) */
                xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, SegPars_t> dec_localSegPars{};
                /** @brief Decoration of the local segment fit covariance (not persitifiable) */
                xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, SegCov_t> dec_localSegCov{};
                
                /** @brief Container handle to create the beamspot measurement, if needed */
                Acts::Result<ActsTrk::AuxiliaryMeasurementHandler::MeasurementProvider, 
                            ActsTrk::AuxiliaryMeasurementHandler::HandleStatus> beamSpotMeasCreator{ActsTrk::AuxiliaryMeasurementHandler::HandleStatus::emptyKey};
                /** @brief Container handle to create the combined measurements of Rpc/Tgc/sTgc */
                xAOD::FillContainer<xAOD::CombinedMuonStripContainer,
                                    xAOD::CombinedMuonStripAuxContainer> prdCombContainer{};
                /** @brief Pointer to the commonly shared beamspot measurement */
                const xAOD::UncalibratedMeasurement* beamSpot{};

                using PrdLink_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
                /** @brief Decoration of the list of measurements used to construct the segment */
                xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, 
                                        std::vector<PrdLink_t>> dec_prdLinks{};
                /** @brief Extra column containing the fitState of each measurement
                 *         (Valid, Outlier, wrongly calibrated) */
                xAOD::ContainerDecorator<xAOD::MuonSegmentContainer,
                                         std::vector<char>> dec_prdStates{}; 
            };
            /** @brief Convert a MuonR4::Segment to an xAOD::MuonSegment and decorate it 
             *         with the necessary information
             *  @param ctx: The event context
             *  @param inSegment: The input segment
             *  @param ship: The data ship
             *  @return: The converted segment */
            virtual xAOD::MuonSegment* convertSegment(const EventContext& ctx,
                                                      const MuonR4::Segment& inSegment,
                                                      DataShip& ship) const = 0;
    };
}

#endif
