/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONVISUALIZATIONHELPERSR4_OBJVISUALIZATIONHELPERS_H
#define MUONVISUALIZATIONHELPERSR4_OBJVISUALIZATIONHELPERS_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "xAODMuon/MuonSegment.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "GaudiKernel/SystemOfUnits.h"


#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"


namespace MuonValR4{
    /** @brief Draws the recorded propagation steps as a polygon line
     *  @param step: List of steps to draw
     * @param vsualHelper: Obj helper to which the drawn trajectory is appended
     * @param viewConfig: Configuration style of the drawn polygon. */
    void drawPropagation(const std::vector<Acts::detail::Step>& steps,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig = Acts::s_viewLine);
    /** @brief Draw a line representing the bound track parameters
     * @param gctx: Geometry context to align the parameters globally
     * @param pars: Bound track parameters to draw
     * @param vsualHelper: Obj helper to which the drawn line is appended
     * @param viewConfig: Configuration style of the drawn line
     * @param standardLength: Length of the segment as a fallback solution */
    void drawBoundParameters(const ActsTrk::GeometryContext& gctx,
                             const Acts::BoundTrackParameters& pars,
                             Acts::ObjVisualization3D& visualHelper,
                             const Acts::ViewConfig& viewConfig = Acts::s_viewLine,
                             const double standardLength = 3.*Gaudi::Units::cm);
    /** @brief Draw a segment line inside the obj file. If the segment is a reconstructed segment and
     *         has associated measurements, then the first and last surface position is used to determine the
     *         length of the segment, otherwise the standard lenth is used.
     *  @param gctx: Geometry context needed to fetch the positions of the first & last measurement
     *  @param segment: The segment which is meant to draw
     *  @param vsualHelper: Obj helper to which the segment is appended.
     *  @param viewConfig: Configuration style of the drawn line
     *  @param standardLength: Length of the segment as a fallback solution */
    void drawSegmentLine(const ActsTrk::GeometryContext& gctx,
                         const xAOD::MuonSegment& segment,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig = Acts::s_viewLine,
                         const double standardLength = 1.*Gaudi::Units::m);
    /** @brief Draw all uncalibrated measurements associated to the segment.   
     *  @param gctx: Geometry context needed to fetch the positions of the first & last measurement
     *  @param segment: The segment which from which the measurements are taken
     *  @param vsualHelper: Obj helper to which the measurements are appended.
     *  @param viewConfig: Configuration style of the drawn measurements */
    void drawSegmentMeasurements(const ActsTrk::GeometryContext& gctx,
                                 const xAOD::MuonSegment& segment,
                                 Acts::ObjVisualization3D& visualHelper,
                                 const Acts::ViewConfig& viewConfig = Acts::s_viewSensitive);
    /** @brief Draw an uncalibrated measurement inside the obj file. The measurement is translated to a temporary surface
     *         with adapted position & boundaries (e.g. drift radius or measurement uncertainty)
     *  @param gctx: Geometry context needed to fetch the positions of the surface
     *  @param meas: Pointer to the muon measurement to visualize
     *  @param vsualHelper: Obj helper to which the measurement is appended.
     *  @param viewConfig: Configuration style of the drawn measurement */
    void drawMeasurement(const ActsTrk::GeometryContext& gctx,
                         const xAOD::UncalibratedMeasurement* meas,
                         Acts::ObjVisualization3D& visualHelper,
                         const Acts::ViewConfig& viewConfig = Acts::s_viewSensitive);
}

#endif
