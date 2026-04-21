/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTOOLINTERFACES_IONBOUNDSTATECALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IONBOUNDSTATECALIBRATORTOOL_H

#include <GaudiKernel/IAlgTool.h>

#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"

#include "xAODInDetMeasurement/PixelClusterAuxDataCacheCollection.h"
#include "xAODInDetMeasurement/StripClusterAuxDataCacheCollection.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

namespace ActsTrk {

class IOnBoundStateCalibratorTool : virtual public IAlgTool {
public:
   DeclareInterfaceID(IOnBoundStateCalibratorTool, 1, 0);
      using PixelCluster_t = traits::ElementProxies<const PixelClusterAuxDataCacheCollection >::ClusterProxy<Utils::AccessPolicy::Const>;
      using StripCluster_t = traits::ElementProxies<const StripClusterAuxDataCacheCollection >::ClusterProxy<Utils::AccessPolicy::Const>;
      using PixelPos = xAOD::MeasVector<2>;
      using PixelCov = xAOD::MeasMatrix<2>;
      // @TODO should pass through bound state
      using PixelCalibrator = Acts::Delegate<
         std::pair<PixelPos, PixelCov>(const Acts::GeometryContext&,
                                       const Acts::CalibrationContext&,
                                       const PixelCluster_t &,
                                       const Acts::BoundTrackParameters &)>;

      using StripPos = xAOD::MeasVector<1>;
      using StripCov = xAOD::MeasMatrix<1>;
      using StripCalibrator = Acts::Delegate<
         std::pair<StripPos, StripCov>(const Acts::GeometryContext&,
                                       const Acts::CalibrationContext&,
                                       const StripCluster_t &,
                                       const Acts::BoundTrackParameters &)>;

      using HgtdPos = xAOD::MeasVector<3>;
      using HgtdCov = xAOD::MeasMatrix<3>;
      using HGTDCalibrator = Acts::Delegate<
         std::pair<HgtdPos, HgtdCov>(const Acts::GeometryContext&,
                                       const Acts::CalibrationContext&,
                                       const xAOD::HGTDCluster &,
                                       const Acts::BoundTrackParameters &)>;


      // @TODO should pass through bound state

      PixelCalibrator pixelCalibrator;
      StripCalibrator stripCalibrator;
      HGTDCalibrator hgtdCalibrator;

   virtual void connectPixelCalibrator([[maybe_unused]] PixelCalibrator &calibrator) const {}
   virtual void connectStripCalibrator([[maybe_unused]] StripCalibrator &calibrator) const {}
   virtual void connectHGTDCalibrator([[maybe_unused]] HGTDCalibrator &calibrator) const {}

   virtual bool calibrateAfterMeasurementSelection() const =0;
};

} // namespace ActsTrk

#endif
