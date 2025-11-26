/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_UTILFUNCTIONS_H
#define XAODMUONPREPDATA_UTILFUNCTIONS_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "xAODMeasurementBase/UncalibratedMeasurement.h"


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
    /** @brief Returns the associated readout element to the measurement*/
    const MuonGMR4::MuonReadoutElement* muonReadoutElement(const UncalibratedMeasurement* meas);
    /** @brief Returns the associated Acts surface to the measurement */
    const Acts::Surface& muonSurface(const xAOD::UncalibratedMeasurement* meas);
    /** @brief Returns the associated identifier from the muon measurement */
    const Identifier& identify(const UncalibratedMeasurement* meas);
    /** @brief Returns the layer hash from an uncalibrated meaurement */
    IdentifierHash layerHash(const UncalibratedMeasurement* meas);
}

#endif