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

class ActsGeometryContext;
namespace MuonR4{
    class CalibratedSpacePoint;
    class Segment;
    
    /** @brief Returns the hough tanTheta  [y] / [z]
     *  @param v: Arbitrary direction vector */
    double houghTanTheta(const Amg::Vector3D& v);
    /** @brief: Returns the hough tanPhi [x] / [z] 
      * @param v: Arbitrary direction vector */
    double houghTanPhi(const Amg::Vector3D& v);
    namespace SegmentFit {
        /**  @brief Returns the parsed parameters into an Eigen line parametrization.
         *          The first operand is the position. The other is the direction. */
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars);

        std::string makeLabel(const Parameters& pars);
        std::string toString(const Parameters& pars);
        std::string toString(const ParamDefs par);
        /** @brief Constructs a direction vector from tanPhi & tanTheta
         *  @param tanPhi: Tangent of the [x] to [z] axis
         *  @param tanTheta: Tangent of the [y] to [z] axis  */
        Amg::Vector3D dirFromTangents(const double tanPhi, const double tanTheta);
        /** @brief Returns the localSegPars decoration from a xAODMuon::Segment*/
        Parameters localSegmentPars(const xAOD::MuonSegment& seg);
        /** @brief Returns the local segment parameters from a segment object
         *  @param gctx: Geometry context storing the local -> global transformation
         *  @param segment: Reference to the segment */
        Parameters localSegmentPars(const ActsGeometryContext& gctx,
                                    const Segment& segment);
    }
}

#endif 
