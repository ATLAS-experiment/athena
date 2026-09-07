/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackEvent/TrackMatchingUtils.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Surfaces/DiscBounds.hpp"
#include "Acts/Definitions/Units.hpp"
#include "FourMomUtils/P4Helpers.h"

using namespace P4Helpers;
using namespace Acts::UnitLiterals;

namespace MuonCombinedR4 {

    std::optional<Acts::BoundTrackParameters> 
        makeDiffParameters(const Acts::GeometryContext& tgContext,
                           const Acts::BoundTrackParameters& idParameters,
                           const Acts::BoundTrackParameters& msParameters,
                           const Acts::Logger& logger,
                           const double boundTolerance) {
        
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Calculate difference parameters between \n"
                    <<idParameters<<"\n\n and \n\n"<<msParameters);
        Acts::BoundVector diffPars = msParameters.parameters() -idParameters.parameters();
       
        const Acts::Surface& idSurf{idParameters.referenceSurface()};
        const Acts::Surface& msSurf{msParameters.referenceSurface()};
        
        std::optional<Acts::BoundMatrix> msCov{msParameters.covariance()};
        const std::optional<Acts::BoundMatrix>& idCov{idParameters.covariance()};

        if (idSurf.geometryId() != msSurf.geometryId() || 
            (idSurf.geometryId() == Acts::GeometryIdentifier{} && 
             idSurf.getSharedPtr() != msSurf.getSharedPtr())) {
            switch (msSurf.type()) {
                using enum Acts::Surface::SurfaceType;
                case Disc:{
                    if (static_cast<const Acts::DiscBounds&>(msSurf.bounds()).rMax() - msParameters.get<Acts::eBoundLoc0>() > boundTolerance) {
                        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - The parameters are too far apart from the bounds ");
                        return std::nullopt;
                    }
                    break;
                } case Cylinder : {
                    using enum Acts::CylinderBounds::BoundValues;
                    if (static_cast<const Acts::CylinderBounds&>(msSurf.bounds()).get(eHalfLengthZ) -
                        std::abs(msParameters.get<Acts::eBoundLoc1>()) >  boundTolerance) {
                        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - The parameters are too far apart from the bounds ");
                        return std::nullopt;
                    }
                    break;
                } default: {
                    ACTS_WARNING(__func__<<"() "<<__LINE__<<" - Surface type "<<msSurf.type()<<" is not implemented");
                    break;
                }
            }
            /// Propagate the MS parameters to the ID parameters
            const Acts::Intersection3D idIsect = idSurf.intersect(tgContext,
                                                                  msParameters.position(tgContext), 
                                                                  msParameters.direction(),
                                                                  Acts::BoundaryTolerance::Infinite()).closest();
            if (!idIsect.isValid()) {
                ACTS_WARNING(__func__<<"() "<<__LINE__<<" - The propgation of "<<msParameters<<" to "
                            <<idSurf.geometryId()<<", "<<idSurf.bounds()<<" failed.");
                return std::nullopt;
            }
            auto locPos = idSurf.globalToLocal(tgContext, idIsect.position(), msParameters.direction());
            if (!locPos.ok()) {
                ACTS_WARNING(__func__<<"() "<<__LINE__<<" - Local to global of  "<<Amg::toString(idIsect.position())
                    <<" is not on "<<idSurf.geometryId()<<", "<<idSurf.bounds()<<".");
                return std::nullopt;
            }
            /// Update the difference parameters
            diffPars[Acts::eBoundLoc0] = (*locPos)[Acts::eX] - idParameters.get<Acts::eBoundLoc0>();
            diffPars[Acts::eBoundLoc1] = (*locPos)[Acts::eY] - idParameters.get<Acts::eBoundLoc1>();
            /// Update the MS covariance
            if (msCov) {
                (*msCov) = idSurf.freeToBoundJacobian(tgContext, idIsect.position(), msParameters.direction()) *
                           msSurf.boundToFreeJacobian(tgContext, msParameters.position(tgContext), 
                                                       msParameters.direction())  * (*msCov);
            }
        }
        std::optional<Acts::BoundMatrix> diffCov{};
        if (idCov || msCov) {
            diffCov = idCov.value_or(Acts::BoundMatrix::Zero()) +
                      msCov.value_or(Acts::BoundMatrix::Zero());
        }
        /// Map the loc1 parameter to [-pi, pi]
        if (idSurf.type() == Acts::Surface::SurfaceType::Disc) {
            diffPars[Acts::eBoundLoc1] = deltaPhi(msParameters.get<Acts::eBoundLoc1>(), 
                                                  idParameters.get<Acts::eBoundLoc1>());
        } else if (idSurf.type() == Acts::Surface::SurfaceType::Cylinder) {
            using enum Acts::CylinderBounds::BoundValues;
            /// Map the loc0 parameter to [-pi, pi] x R
            const double R = static_cast<const Acts::CylinderBounds&>(idSurf.bounds()).get(eR);
            diffPars[Acts::eBoundLoc0] = deltaPhi(msParameters.get<Acts::eBoundLoc0>() / R, 
                                                  idParameters.get<Acts::eBoundLoc0>() / R) * R;
        }
        Acts::BoundTrackParameters retPars{idSurf.getSharedPtr(),
                                           /*Ensure that theta remains positive otherwise 
                                             the constructor flips the direction */
                                           Acts::copySign(diffPars, diffPars[Acts::eBoundTheta]),  
                                            diffCov, msParameters.particleHypothesis()};
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Successfully created \n"<<retPars);
        return retPars;
    }

    double longitudinalParam(const Acts::BoundTrackParameters& pars,
                             const Acts::Logger& logger) {
               switch (pars.referenceSurface().type()) {
            using enum Acts::Surface::SurfaceType;
            case Disc:
                return pars.get<Acts::eBoundLoc0>();
            case Cylinder:
                return pars.get<Acts::eBoundLoc1>();
            default:
                ACTS_WARNING(__func__<<"() "<<__LINE__<<" Surface type "<<pars.referenceSurface().type()
                                <<" is not implemented");
                break;
        }
        return -1._km;
    }
    /** @brief Returns the local angular polar angle of the track parameter
      * @param pars: Reference to the parameters of interest */
    double localPolarAngle(const Acts::BoundTrackParameters& pars,
                           const Acts::Logger& logger) {
       switch (pars.referenceSurface().type()) {
            using enum Acts::Surface::SurfaceType;
            case Disc:
                return pars.get<Acts::eBoundLoc1>();
            case Cylinder: {
                using enum Acts::CylinderBounds::BoundValues;
                const double R = static_cast<const Acts::CylinderBounds&>(pars.referenceSurface().bounds()).get(eR);
                return pars.get<Acts::eBoundLoc0>() / R;
            } default:
                ACTS_WARNING(__func__<<"() "<<__LINE__<<" Surface type "<<pars.referenceSurface().type()
                                <<" is not implemented");
                break;
        }
        return 360._degree;
    }


}