/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EventPrimitives/EventPrimitives.h"
// needed here to get the ATLAS eigen plugins in before the ACTS eigen plugins
#include "ActsGeometry/ActsCaloTrackingVolumeBuilder.h"
#include "ActsGeometry/ActsExtrapolationAlg.h"
#include "ActsGeometry/ExtrapolationTool.h"
#include "ActsGeometry/ActsMaterialJsonWriterTool.h"
#include "ActsGeometry/ActsPropStepRootWriterSvc.h"
#include "ActsGeometry/ActsTrackingGeometrySvc.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometry/ActsWriteTrackingGeometry.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"
#include "../ActsVolumeIdToDetectorElementCollectionMappingAlg.h"
#include "../ItkBlueprintNodeBuilder.h"
#include "../CaloBlueprintNodeBuilder.h"


DECLARE_COMPONENT(ActsExtrapolationAlg)
DECLARE_COMPONENT(ActsWriteTrackingGeometry)
DECLARE_COMPONENT(ActsWriteTrackingGeometryTransforms)
DECLARE_COMPONENT(ActsTrackingGeometrySvc)

DECLARE_COMPONENT(ActsMaterialJsonWriterTool)

DECLARE_COMPONENT(ActsTrackingGeometryTool)

DECLARE_COMPONENT(ActsPropStepRootWriterSvc)
DECLARE_COMPONENT(ActsCaloTrackingVolumeBuilder)
DECLARE_COMPONENT(ActsTrk::ActsVolumeIdToDetectorElementCollectionMappingAlg)

DECLARE_COMPONENT(ActsTrk::ItkBlueprintNodeBuilder)
DECLARE_COMPONENT(ActsTrk::ExtrapolationTool)
DECLARE_COMPONENT(ActsTrk::CaloBlueprintNodeBuilder)

