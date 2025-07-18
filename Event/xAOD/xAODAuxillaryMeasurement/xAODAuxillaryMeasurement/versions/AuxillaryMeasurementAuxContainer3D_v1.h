/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODAUXILLARYMEASUREMENT_XAODAUXILLARYMEASUREMENTAUXCONTAINER_3D_v1_H
#define XAODAUXILLARYMEASUREMENT_XAODAUXILLARYMEASUREMENTAUXCONTAINER_3D_v1_H
 
// System include(s):
#include <stdint.h>
#include <vector>

// Core include(s):
#include "AthLinks/ElementLink.h"
#include "xAODCore/AuxContainerBase.h"
 
// xAOD include(s):
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODTracking/TrackSurfaceContainer.h" 
 
namespace xAOD {
 
   ///
   class AuxillaryMeasurementAuxContainer3D_v1 : public AuxContainerBase {
 
   public:
      /// Default constructor
      AuxillaryMeasurementAuxContainer3D_v1();
 
   private:
     std::vector<char>                                calibProjector{};
     std::vector<PosAccessor<3>::element_type>        localPosition{};
     std::vector<CovAccessor<3>::element_type>        localCovariance{};
      /// @name Links 
      /// @{    
      std::vector<ElementLink<TrackSurfaceContainer>> surfaceLink{};
      /// @}
   };
} // namespace xAOD
 
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::AuxillaryMeasurementAuxContainer3D_v1, xAOD::AuxContainerBase ); 
 
#endif
