/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_UTILFUNCTIONS_H
#define XAODMUONPREPDATA_UTILFUNCTIONS_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "MuonStationIndex/MuonStationIndex.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MuonMeasurementFwd.h"
#include "xAODMuonPrepData/CombinedMuonStripFwd.h"


namespace ActsTrk {
    class GeometryContext;
}
class Identifier;
class IdentifierHash;
namespace MuonGMR4{
    class MuonReadoutElement;
}
namespace Acts {
    class Surface;
}
namespace xAOD{ 
    /** @brief Returns the associated Acts surface to the measurement */
    const Acts::Surface& muonSurface(const UncalibratedMeasurement* meas);
     /** @brief Returns the associated identifier from the muon measurement */
    const Identifier& identify(const UncalibratedMeasurement* meas);
    /** @brief Transforms the uncalibrated measurement type to a technology index
        @param aodType Uncalibrated measurement type */
    ::Muon::MuonStationIndex::TechnologyIndex toTechnologyIndex(const UncalibMeasType aodType);
    /** @brief Returns the position and covariance from a combined strip measurement
     *  @param combinedPrd: Combined strip measurement */
    std::pair<Amg::Vector2D, AmgSymMatrix(2)> positionAndCovariance(const CombinedMuonStrip* combinedPrd);
    /** @brief Returns the position and covariance from two single strip measurements
     *  @param etaStrip: Pointer to the eta strip measurement
     *  @param phiStrip: Pointer to the phi stirp measurement */
    std::pair<Amg::Vector2D, AmgSymMatrix(2)> positionAndCovariance(const MuonMeasurement* etaStrip,
                                                                    const MuonMeasurement* phiStrip);
    /** @brief Returns the 1D position of the uncalibrated measurement expressed in the coordinate system of the 
     *         measurement. Attention for strip-like measurements no distinction is made between eta or phi measurements
     *         The drift radius and the associated covariance is returned for the drift circle type measurements
     * @param oneDimMeas: Pointer to the muon measurement of interest */
    std::pair<Amg::Vector2D, AmgSymMatrix(2)> positionAndCovariance(const MuonMeasurement* oneDimMeas);
}

#endif