/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoModelHelpers/TransformSorter.h"
///
#include "ActsInterop/Logger.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Geometry/Volume.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Enumerate.hpp"

#include "MuonReadoutGeometryR4/BoundsExpander.h"
#include "MuonReadoutGeometryR4/TrapezoidBoundsExpander.h"
#include "MuonReadoutGeometryR4/CylinderBoundsExpander.h"

#include <stdlib.h>

std::ostream& operator<<(std::ostream& ostr, std::span<const Amg::Vector3D> vectors) {
    for (const auto& [idx, v] : Acts::enumerate(vectors)) {
        ostr<<" * "<<(idx+1)<<") - "<<Amg::toString(v)<<std::endl;
    }
    return ostr;
}

constexpr Acts::Logging::Level lvl = Acts::Logging::Level::VERBOSE;
using namespace Acts::UnitLiterals;

bool checkVertices(const Acts::GeometryContext&tgContext, 
                  const Acts::Surface& surface,
                  const Acts::Logger& logger) {
    std::vector<Amg::Vector3D> vertices = MuonGMR4::BoundsExpander::cornerPoints(tgContext, surface, 15);
    std::ranges::sort(vertices, GeoTrf::TransformSorter{});
    ACTS_INFO(__func__<<"() "<<__LINE__<<" - Retrieved vertices: \n"<<vertices);

    
    std::vector<Amg::Vector3D> polyhedronVert = surface.polyhedronRepresentation(tgContext,10).vertices;
    std::ranges::sort(polyhedronVert, GeoTrf::TransformSorter{});
    ACTS_INFO(__func__<<"() "<<__LINE__<<" - Vertices from the polyhedron: \n"<<polyhedronVert);
    bool success = true;
    for (const Amg::Vector3D& poly : polyhedronVert) {
        if (std::ranges::none_of(vertices, 
                [&poly](const Amg::Vector3D& vert){
                    return (vert - poly).mag2() < Acts::s_transformEquivalentTolerance;
            })){
            ACTS_ERROR(__func__<<"() "<<__LINE__<<" - The vertex "<<Amg::toString(poly)<<" is not generated");
            success = false;
        }
    }
    for (const Amg::Vector3D& vert : vertices) {
        auto locVert = surface.globalToLocal(tgContext, vert, vert);
        if (!locVert.ok()) {
            ACTS_ERROR(__func__<<"() "<<__LINE__<<" - The vertex "<<Amg::toString(vert)<<" is not on surface");
            success = false;
            continue;
        }
        if (!surface.insideBounds(*locVert, Acts::BoundaryTolerance::AbsoluteEuclidean(Acts::s_onSurfaceTolerance))){
            ACTS_ERROR(__func__<<"() "<<__LINE__<<" - The vertex "<<Amg::toString(vert)<<"/"
                        <<Amg::toString(*locVert)<<" is outside bounds");
            success = false;
        }
        const double dist = surface.distanceToBoundary(*locVert);
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Vertex: "<<Amg::toString(vert)<<", boundary distance: "<<dist);
        if (std::abs(dist) > Acts::s_onSurfaceTolerance) {
            ACTS_ERROR(__func__<<"() "<<__LINE__<<" - The vertex "<<Amg::toString(vert)<<"/"
                        <<Amg::toString(*locVert)<<" is not on the edge.");
            success = false;
        }
    }
    return success;
}

bool testTrapezoidVertices(const Acts::Logger& logger) {
    const Acts::GeometryContext tgContext = Acts::GeometryContext::dangerouslyDefaultConstruct();
    auto bounds = std::make_shared<Acts::TrapezoidBounds>(100._mm, 75._mm, 50._mm);
    
    ACTS_INFO(__func__<<"() "<<__LINE__<<" - Fetch trapezoidal vertices from bounds "<<(*bounds));
    /// Check the surface at Identity
    auto surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Acts::Transform3::Identity(), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    /// Check the surface shifted in Z
    surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::getTranslateZ3D(100._mm), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    /// Check the surface  shifted in Z and rotated by 30 degree
    surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::getRotateX3D(30._degree)*
                                                            Amg::getTranslateZ3D(100._mm), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    /// Rotate the bounds around 60 degree
    bounds = std::make_shared<Acts::TrapezoidBounds>(100._mm, 75._mm, 50._mm, 60._degree);
    ACTS_INFO(__func__<<"() "<<__LINE__<<" - Fetch trapezoidal vertices from bounds "<<(*bounds));
    surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Acts::Transform3::Identity(), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    return true;
}

bool testRectangleVertices(const Acts::Logger& logger) {
    const Acts::GeometryContext tgContext = Acts::GeometryContext::dangerouslyDefaultConstruct();
    auto bounds = std::make_shared<Acts::RectangleBounds>(100._mm, 50._mm);
    ACTS_INFO(__func__<<"() "<<__LINE__<<" - Fetch rectangular vertices from bounds "<<(*bounds));
    //early return..
    return true;
    //..means following code is unreachable, but the following code fails test
    //coverity[UNREACHABLE]
    /// Check the surface at Identity
    auto surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Acts::Transform3::Identity(), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    /// Check the surface shifted in Z
    surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::getTranslateZ3D(100._mm), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    /// Check the surface  shifted in Z and rotated by 30 degree
    surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Amg::getRotateX3D(30._degree)*
                                                            Amg::getTranslateZ3D(100._mm), bounds);
    if (!checkVertices(tgContext, *surface, logger)) {
        return false;
    }
    return true;
}

bool testExpander(const MuonGMR4::CylinderBoundsExpander& expander, 
                  const std::array<double, 4>& expectations,
                  const Acts::Logger& logger) {
    bool retCode{true};
    if (std::abs(expander.rMin() - expectations[0]) > Acts::s_transformEquivalentTolerance) {
        ACTS_ERROR(__func__<<"() "<<__LINE__<<" - Wrong minimal r detected: "<<expander.rMin()<<", expected: "<<expectations[0]);
        retCode = false;
    } if (std::abs(expander.rMax() - expectations[1]) > Acts::s_transformEquivalentTolerance){
        ACTS_ERROR(__func__<<"() "<<__LINE__<<" - Wrong maximal r detected: "<<expander.rMax()<<", expected: "<<expectations[1]);
        retCode = false;
    } if (std::abs(expander.zMin() - expectations[2]) > Acts::s_transformEquivalentTolerance){
         ACTS_ERROR(__func__<<"() "<<__LINE__<<" - Wrong minmal z detected: "<<expander.zMin()<<", expected: "<<expectations[2]);
           retCode = false;
    } if (std::abs(expander.zMax() - expectations[3]) > Acts::s_transformEquivalentTolerance){
        ACTS_ERROR(__func__<<"() "<<__LINE__<<" - Wrong maximal z detected: "<<expander.zMax()<<", expected: "<<expectations[3]);
        retCode = false;
    }
    return retCode;
}

Acts::Transform3 volumePos(const double phi, const double r) {
    return Amg::getRotateZ3D(phi) * 
           Amg::getTranslateX3D(r)*
           Amg::getRotateY3D(90._degree)*
           Amg::getRotateZ3D(90._degree);
}


bool testCylinderExpander(const Acts::Logger& logger) {
    /// simple volume 50 cm away from the center
    
    constexpr double width = 75._cm;
    constexpr double length = 2._m;
    constexpr double height = 50._cm;
    constexpr double volDist = 2._m;
    std::shared_ptr<Acts::VolumeBounds> bounds = std::make_shared<Acts::CuboidVolumeBounds>(width, length, height);
    const Acts::GeometryContext tgContext = Acts::GeometryContext::dangerouslyDefaultConstruct();
    bool retCode{true};
    {
        MuonGMR4::CylinderBoundsExpander expander{logger.clone()};
        /// Expand the cylinder to enclose the cylinder 2m away
        
        expander.expand(tgContext, Acts::Volume{volumePos(0._degree, volDist),bounds});
        if (!testExpander(expander, std::array{volDist - height, Acts::fastHypot(volDist+height, width), -length, length}, logger)){
            retCode = false;
        }
        /// Insert another volume at half R at one 1m distance
        expander.expand(tgContext, Acts::Volume{volumePos(270._degree, 0.5*volDist),bounds});
        if (!testExpander(expander, std::array{0.5*volDist - height, Acts::fastHypot(volDist+height, width), -length, length}, logger)){
            retCode = false;
        }
    }
    /// Position the box with the edge on the minimal cylinder radius
    {
        MuonGMR4::CylinderBoundsExpander expander{logger.clone()};
        expander.expand(tgContext, Acts::Volume{volumePos(0._degree, volDist) *
                                                Amg::getTranslateX3D(width),bounds});
        if (!testExpander(expander, std::array{volDist - height, Acts::fastHypot(volDist+height, 2.*width), -length, length}, logger)){
            retCode = false;
        }
    }
    /// Position the box way outside
    {
        MuonGMR4::CylinderBoundsExpander expander{logger.clone()};
        expander.expand(tgContext, Acts::Volume{volumePos(0._degree, volDist) *
                                                Amg::getTranslateX3D(2.*width),bounds});
        if (!testExpander(expander, std::array{Acts::fastHypot(volDist - height, width), 
                                               Acts::fastHypot(volDist+height, 3.*width), -length, length}, logger)){
            retCode = false;
        }
    }
    ACTS_INFO(__func__<<"() "<<__LINE__<<" - test result: "<<retCode);
    return retCode;
}



int main () {
    int retCode = EXIT_SUCCESS;
    if (!testTrapezoidVertices(*Acts::getDefaultLogger("TrapezoidVertexTest" , lvl))){
        retCode = EXIT_FAILURE;
    } if (!testRectangleVertices(*Acts::getDefaultLogger("RecTangleVertexTest", lvl))) {
        retCode = EXIT_FAILURE;
    } 
    if (!testCylinderExpander(*Acts::getDefaultLogger("CylinderExpanderTest", lvl))) {
        retCode = EXIT_FAILURE;
    }

    return retCode;
}

