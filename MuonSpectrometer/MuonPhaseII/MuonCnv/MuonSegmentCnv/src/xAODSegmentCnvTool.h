/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_XAODSEGMENTCNVTOOL_H
#define MUONSEGMENTCNV_XAODSEGMENTCNVTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonRecToolInterfacesR4/IxAODSegmentCnvTool.h"

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"

#include "ActsEvent/AuxiliaryMeasurementHandler.h"
#include "ActsEvent/ContextUtility.h"

namespace MuonR4{
    /** @copydoc IxAODSegmentCnvTool */
    class xAODSegmentCnvTool: public extends<AthAlgTool, IxAODSegmentCnvTool> {
        public:
            using base_class::base_class;
            ~xAODSegmentCnvTool() = default;

            StatusCode initialize() override;

            /** @copydoc IxAODSegmentCnvTool::convertSegment */
            xAOD::MuonSegment* convertSegment(const EventContext& ctx,
                                              const MuonR4::Segment& inSegment,
                                              DataShip& ship) const override;

        private:
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
             *  @param tgContext: The geometry context to construct the local segment parameters  
             *  @param inSegment: The segment from which the space points are collected
             *  @param targetSegment: The xAOD segment onto which all the links are decorated
             *  @param ship: Reference to the auxiliary ship carrying all the write decor handles
             *               needed for the linking */
            StatusCode linkMeasurements(const Acts::GeometryContext& tgContext,                       
                                        const Segment& inSegment,
                                        xAOD::MuonSegment& targetSegment,
                                        DataShip& ship) const;
            
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
              * @param ctx: EventContext to be passed to the extrapolation tool
              * @param startPars: The start parameters from which the propagation starts. The surface is the 
              *                    bottom boundary surface of the volume where's the first segment measurement.
              * @param target: The top boundary surface of the volume with the last segment measurement.
              * @param geoIdsWithHits: The list of ATLAS identifiers with segmen measurements (outlier + hit)
              * @param holeCounter: Mutable reference to the segment hole counter.  */                   
            void findHoles(const EventContext& ctx,
                           const Acts::BoundTrackParameters& startPars,
                           const Acts::Surface* target,
                           const std::unordered_set<Identifier>& geoIdsWithHits,
                           Counter& holeCounter) const;

            /** @brief IdHelperSvc for Identifier printing & manipulation */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc",  
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Handler to parse the auxiliary beam spot constaint */
            ActsTrk::AuxiliaryMeasurementHandler m_auxMeasProv{this};
            /** @brief Tracking geometry tool to search for holes  */
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
             /** @brief Acts extrapolation tool to search for holes */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};

            /** @brief Flag to tell whether the hole summary shall be written */
            Gaudi::Property<bool> m_estimateHoles{this, "estimateHoles", true};
            /** @brief Extra path length for the propagation after the last surface was crossed. */
            Gaudi::Property<double> m_extraHolePath{this, "extraHolePath", 20.*Gaudi::Units::cm};
    };
}


#endif