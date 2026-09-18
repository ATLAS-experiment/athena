/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SPACEPOINT_CONVERSION_UTILITIES_H
#define SPACEPOINT_CONVERSION_UTILITIES_H

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SiSpacePoint/PixelSpacePoint.h"

#include "SiSpacePoint/SCT_SpacePoint.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"

namespace TrackingUtilities {

  StatusCode convertTrkToXaodPixelSpacePoint(const InDet::PixelSpacePoint& trkSpacePoint,
					     xAOD::SpacePoint& xaodSpacePoint);

  StatusCode convertTrkToXaodStripSpacePoint(const InDet::SCT_SpacePoint& trkSpacePoint,
                                             xAOD::SpacePoint& xaodSpacePoint);

}

#endif
