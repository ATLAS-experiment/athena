/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsGeometry/CylinderUtils.h"


#include "Acts/Utilities/MathHelpers.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Definitions/Units.hpp"

#include "Acts/Utilities/detail/RealQuadraticEquation.hpp"

using namespace Acts::UnitLiterals;
namespace {
    constexpr double halfLength = 7.5_m;
}


namespace ActsTrk {
   std::array<double, 2>
      cylinderIntersectPaths(const Amg::Vector3D& pos, 
                             const Amg::Vector3D& dir,
                             const double cylinderR) {
        /** Solve equation
          *  (Px + lambda Dx)^{2} + (Py + lambda Dy)^{2} - R^{2} = 0
          *   Px^{2} + Py^{2} - R^{2} + 2 lambda*(Px * Dx + Py * Dy)  + 
          *      lambda^{2}*(Dx^{2} + Dy^{2}) = 0
          *
          *  A = (Dx^{2} + Dy^{2})
          *  B = 2 * (Px+Dx + Py *Dy)
          *  C = (Px^{2} + Py^{2} - R^{2}) */
        assert (cylinderR > 0.);
        Acts::detail::RealQuadraticEquation solution{dir.block<2,1>(0,0).perp2(),
                                                     2.*pos.block<2,1>(0,0).dot(dir.block<2,1>(0,0)),
                                                     pos.block<2,1>(0,0).perp2() - Acts::square(cylinderR)};
        switch(solution.solutions) {
            case 2: {
              auto ret = std::array{solution.first, solution.second};
              if ((ret[1] > 0 && (ret[0] < 0  || ret[0] > ret[1])) ||
                  (ret[1] < 0 && (ret[0] < ret[1]))) {
                std::swap(ret[0], ret[1]);
              }
              return ret;
            } case 1:
              return std::array{solution.first, std::numeric_limits<double>::max()};
            default:
              break;
        }
        return std::array<double , 2>{};
    }
    std::optional<Acts::BoundTrackParameters> 
        expressOnCylinder(const Acts::GeometryContext& tgContext,
                           const Acts::BoundTrackParameters& boundPars,
                           const double cylinderR,
                           const Acts::Direction dir) {

        const Amg::Vector3D globDir = boundPars.direction();
        const std::array paths{cylinderIntersectPaths(boundPars.position(tgContext),
                                                      globDir, cylinderR)};
        /// The zero-th solution is always the closest positive one. Or the larger negative one
        /// if both are negative
        const double solution = (dir == Acts::Direction::Forward() ? paths[0] : 
                                 paths[0] < 0 ? paths[0] : paths[1]);
        ///
        if ( (solution < 0. && dir == Acts::Direction::Forward()) ||
             (solution > 0. && dir == Acts::Direction::Backward())) {
            return std::nullopt;
        }
        auto outSurface = Acts::Surface::makeShared<Acts::CylinderSurface>(Acts::Transform3::Identity(), 
                                                                           cylinderR, halfLength);
        const Amg::Vector3D globPos = boundPars.position(tgContext) + solution * globDir;
        const auto locPos = outSurface->globalToLocal(tgContext, globPos);
        if (!locPos.ok()) {
            return std::nullopt;
        }
        Acts::BoundVector outPars{boundPars.parameters()};
        outPars[Acts::eBoundLoc0] = (*locPos)[Acts::eX];
        outPars[Acts::eBoundLoc1] = (*locPos)[Acts::eY];
        // In Acts units the speed of light is 1 -> just add the path length 
        outPars[Acts::eBoundTime] += solution;

        std::optional<Acts::BoundMatrix> cov = boundPars.covariance();
        // The covariance 
        if (cov && outSurface->type() != boundPars.referenceSurface().type()) {
            const Acts::BoundMatrix trf = outSurface->freeToBoundJacobian(tgContext, globPos, globDir) *
                                          boundPars.referenceSurface().boundToFreeJacobian(tgContext, globPos, globDir);
            (*cov) = trf.transpose() * (*cov) * trf;
        }
        return Acts::BoundTrackParameters{outSurface, outPars, cov,
                                          boundPars.particleHypothesis()};
    }
    std::vector<Acts::BoundTrackParameters> 
        expressOnCylinders(const Acts::GeometryContext& tgContext,
                            const Acts::BoundTrackParameters& boundPars,
                            std::span<const double> cylinderRadii,
                            const Acts::Direction dir) {
        std::vector<Acts::BoundTrackParameters> result{};
        result.reserve(cylinderRadii.size());
        for (const auto radius : cylinderRadii) {
            auto pars = expressOnCylinder(tgContext, result.empty() ? boundPars : 
                                           result.back(),
                                           radius, dir);
            if (pars) {
              result.emplace_back(std::move(*pars));
            }
        }
        std::ranges::sort(result, [](const Acts::BoundTrackParameters& a, 
                                     const Acts::BoundTrackParameters& b){
            using enum Acts::CylinderBounds::BoundValues;
            return static_cast<const Acts::CylinderBounds&>(a.referenceSurface().bounds()).get(eR) <
                   static_cast<const Acts::CylinderBounds&>(b.referenceSurface().bounds()).get(eR) ;
        });
        return result;
    }
}
