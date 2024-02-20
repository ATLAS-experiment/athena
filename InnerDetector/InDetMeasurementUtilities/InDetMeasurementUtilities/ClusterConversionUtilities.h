/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CLUSTER_CONVERSION_UTILITIES_H
#define CLUSTER_CONVERSION_UTILITIES_H

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "HGTD_PrepRawData/HGTD_Cluster.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "HGTD_Identifier/HGTD_ID.h"

namespace TrackingUtilities {

  StatusCode convertInDetToXaodCluster(const InDet::PixelCluster& indetCluster, 
				       const InDetDD::SiDetectorElement& element, 
				       xAOD::PixelCluster& xaodCluster);
  
  StatusCode convertInDetToXaodCluster(const InDet::SCT_Cluster& indetCluster, 
				       const InDetDD::SiDetectorElement& element,
				       xAOD::StripCluster& xaodCluster);
  
  StatusCode convertXaodToInDetCluster(const xAOD::PixelCluster& xaodCluster,
				       const InDetDD::SiDetectorElement& element,
				       const PixelID& pixelID,
				       InDet::PixelCluster*& indetCluster);

  StatusCode convertXaodToInDetCluster(const xAOD::StripCluster& xaodCluster,
                                       const InDetDD::SiDetectorElement& element,
                                       const SCT_ID& stripID,
                                       InDet::SCT_Cluster*& indetCluster,
                                       double shift = 0.);  

  // HGTD
  StatusCode convertInDetToXaodCluster(const HGTD_Cluster& indetCluster,
                                       const InDetDD::HGTD_DetectorElement& element,
                                       xAOD::HGTDCluster& xaodCluster);

  StatusCode convertXaodToInDetCluster(const xAOD::HGTDCluster& xaodCluster,
                                       const InDetDD::HGTD_DetectorElement& element,
				       ::HGTD_Cluster*& indetCluster);  
} // Namespace

#endif
