

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoModelHelpers/TransformSorter.h"
///
#include "ActsInterop/Logger.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Enumerate.hpp"

#include "MuonReadoutGeometryR4/BoundsExpander.h"
#include "MuonReadoutGeometryR4/TrapezoidBoundsExpander.h"
#include "MuonReadoutGeometryR4/CylinderBoundsExpander.h"

#include <stdlib.h>

std::ostream& operator<<(std::ostream& ostr, const std::vector<Amg::Vector3D>& vectors) {
    for (const auto& [idx, v] : Acts::enumerate(vectors)) {
        ostr<<" * "<<(idx+1)<<") - "<<Amg::toString(v)<<std::endl;
    }
    return ostr;
}

constexpr Acts::Logging::Level lvl = Acts::Logging::Level::DEBUG;
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
    return true;
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



int main () {
    int retCode = EXIT_SUCCESS;
    if (!testTrapezoidVertices(*Acts::getDefaultLogger("TrapezoidVertexTest" , lvl))){
        retCode = EXIT_FAILURE;
    }
    if (!testRectangleVertices(*Acts::getDefaultLogger("RecTangleVertexTest", lvl))) {
        retCode = EXIT_FAILURE;
    }

    return retCode;
}

