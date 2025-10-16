/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNEVENT_SEGMENTFITEVENTDATA__H
#define MUONR4_MUONPATTERNEVENT_SEGMENTFITEVENTDATA__H

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <MuonPatternEvent/MuonHoughDefs.h>
#include <xAODMuon/MuonSegment.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>

#include "Acts/Seeding/CompositeSpacePointLineFitter.hpp"
#include "Acts/EventData/TrackParameters.hpp"

namespace MuonGMR4 {
   class MuonDetectorManager;
}

class ActsGeometryContext;
namespace MuonR4{
    class CalibratedSpacePoint;
    class Segment;
    
    /** @brief Returns the hough tanBeta  [y] / [z]
     *  @param v: Arbitrary direction vector */
    double houghTanBeta(const Amg::Vector3D& v);
    /** @brief: Returns the hough tanAlpha [x] / [z] 
      * @param v: Arbitrary direction vector */
    double houghTanAlpha(const Amg::Vector3D& v);
    namespace SegmentFit {
        /**  @brief Returns the parsed parameters into an Eigen line parametrization.
         *          The first operand is the position. The other is the direction. */
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars);
        /** @brief Dumps the parameters into a string in the form of TLatex.
         *         Distances are expressed in [mm], angles in [deg] and time in [ns]
         *  @param pars: Reference to the parameters to dump */
        std::string makeLabel(const Parameters& pars);
        /** @brief Dumps the parameters into a string with labels in front of each 
         *         number. Distances are expressed in [mm], angles in [deg] 
         *         and time in [ns]
         *  @param pars: Reference to the parameters to dump */
        std::string toString(const Parameters& pars);
        /** @brief Returns the parameter label
         *  @param par: Parameter of interest */
        std::string toString(const ParamDefs par);
        /** @brief Returns the localSegPars decoration from a xAODMuon::Segment */
        Parameters localSegmentPars(const xAOD::MuonSegment& seg);
        /** @brief Returns the local segment parameters from a segment object
         *  @param gctx: Geometry context storing the local -> global transformation
         *  @param segment: Reference to the segment */
        Parameters localSegmentPars(const ActsGeometryContext& gctx,
                                    const Segment& segment);
        /** @brief Returns the segment parameters as boundTrackParameters. The
         *         position is expressed locally on the sector surface & the direction in
         *         the global frame
         *  @param detMgr: Detector manager to pick up the proper sector object
         *  @param segment: Reference to the segment of interest
         *  @param cov: Uncertainty on the parsed parameters
         *  @param hypot: The particle hypothesis to plugin (Muon by default) */
        Acts::BoundTrackParameters boundSegmentPars(const MuonGMR4::MuonDetectorManager& detMgr,
                                                    const xAOD::MuonSegment& segment,
                                                    std::optional<Acts::BoundMatrix> cov = std::nullopt,
                                                    Acts::ParticleHypothesis hypot = Acts::ParticleHypothesis::muon());
        /** @brief Returns the segment parameters as boundTrackParameters. The
         *         position is expressed locally on the sector surface & the direction in
         *         the global frame */
        Acts::BoundTrackParameters boundSegmentPars(const ActsGeometryContext& gctx,
                                                   const Segment& segment,
                                                   const Acts::ParticleHypothesis hypot = Acts::ParticleHypothesis::muon());
    }
}

#endif 
