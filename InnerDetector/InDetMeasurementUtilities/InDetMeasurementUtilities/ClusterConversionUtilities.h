/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CLUSTER_CONVERSION_UTILITIES_H
#define CLUSTER_CONVERSION_UTILITIES_H

#include "InDetPrepRawData/PixelCluster.h"
#include "InDetPrepRawData/SCT_Cluster.h"
#include "HGTD_PrepRawData/HGTD_Cluster.h"

#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/HGTDCluster.h"

#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"


#include <memory>

class PixelID;
class SCT_ID;
class HGTD_ID;
class SiDetectorElement;

namespace TrackingUtilities {

  StatusCode convertInDetToXaodCluster(const InDet::PixelCluster& indetCluster, 
				       const InDetDD::SiDetectorElement& element, 
				       xAOD::PixelCluster& xaodCluster);
  
  StatusCode convertInDetToXaodCluster(const InDet::SCT_Cluster& indetCluster, 
				       const InDetDD::SiDetectorElement& element,
				       xAOD::StripCluster& xaodCluster,
               bool isITk = true);
  
  /// Resolve the readout design of a pixel/strip detector element. The result only
  /// depends on the element, so callers converting many clusters of the same element
  /// should hoist this out of their loop: the cast is not free and the conversion is
  /// run once per cluster in the event.
  const InDetDD::PixelModuleDesign* pixelModuleDesign(const InDetDD::SiDetectorElement& element);
  const InDetDD::SCT_ModuleSideDesign* stripModuleSideDesign(const InDetDD::SiDetectorElement& element);

  std::unique_ptr<InDet::PixelCluster> 
  convertXaodToInDetCluster(const xAOD::PixelCluster& xaodCluster,
				       const InDetDD::SiDetectorElement& element,
				       const InDetDD::PixelModuleDesign& design,
				       const PixelID& pixelID);

  std::unique_ptr<InDet::SCT_Cluster> 
  convertXaodToInDetCluster(const xAOD::StripCluster& xaodCluster,
    const InDetDD::SiDetectorElement& element, const InDetDD::SCT_ModuleSideDesign& design,
    const SCT_ID& stripID, double shift = 0.);  

  // HGTD
  StatusCode 
  convertInDetToXaodCluster(const ::HGTD_Cluster& indetCluster, const InDetDD::HGTD_DetectorElement& element, xAOD::HGTDCluster& xaodCluster);

  std::unique_ptr<::HGTD_Cluster> 
  convertXaodToInDetCluster(const xAOD::HGTDCluster& xaodCluster, const InDetDD::HGTD_DetectorElement& element);  

  // Low level conversion for SCT cluster pulled out for use in calibrator
  std::pair<xAOD::MeasVector<1>, xAOD::MeasMatrix<1>> convertSCT_LocalPosCov(const InDet::SCT_Cluster &cluster, bool isITk = true);
  std::pair<xAOD::MeasVector<2>, xAOD::MeasMatrix<2>> convertPix_LocalPosCov(const InDet::PixelCluster &cluster);
  std::pair<xAOD::MeasVector<3>, xAOD::MeasMatrix<3>> convertHGTD_LocalPosCov(const HGTD_Cluster &cluster);
} // Namespace

#endif
