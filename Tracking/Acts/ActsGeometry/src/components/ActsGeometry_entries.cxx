/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EventPrimitives/EventPrimitives.h"
// needed here to get the ATLAS eigen plugins in before the ACTS eigen plugins
#include "ActsGeometry/ActsCaloTrackingVolumeBuilder.h"
#include "ActsGeometry/DetectorVolumeSvc.h"
#include "ActsGeometry/ActsExtrapolationAlg.h"
#include "ActsGeometry/ActsExtrapolationTool.h"
#include "ActsGeometry/ActsMaterialJsonWriterTool.h"
#include "ActsGeometry/ActsMaterialMapping.h"
#include "ActsGeometry/ActsMaterialStepConverterTool.h"
#include "ActsGeometry/ActsMaterialTrackWriterSvc.h"
#include "ActsGeometry/ActsObjWriterTool.h"
#include "ActsGeometry/ActsPropStepRootWriterSvc.h"
#include "ActsGeometry/ActsSurfaceMappingTool.h"
#include "ActsGeometry/ActsTrackingGeometrySvc.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometry/ActsVolumeMappingTool.h"
#include "ActsGeometry/ActsWriteTrackingGeometry.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"
#include "../ActsVolumeIdToDetectorElementCollectionMappingAlg.h"
#include "../SimpleCylinderDetBuilderTool.h"
#include "../ItkBlueprintNodeBuilder.h"


DECLARE_COMPONENT(ActsExtrapolationAlg)
DECLARE_COMPONENT(ActsWriteTrackingGeometry)
DECLARE_COMPONENT(ActsWriteTrackingGeometryTransforms)
DECLARE_COMPONENT(ActsTrackingGeometrySvc)
DECLARE_COMPONENT(ActsExtrapolationTool)

DECLARE_COMPONENT(ActsMaterialMapping)
DECLARE_COMPONENT(ActsSurfaceMappingTool)
DECLARE_COMPONENT(ActsVolumeMappingTool)
DECLARE_COMPONENT(ActsObjWriterTool)
// DECLARE_COMPONENT( ActsExCellWriterSvc )
DECLARE_COMPONENT(ActsMaterialTrackWriterSvc)
DECLARE_COMPONENT(ActsMaterialStepConverterTool)
DECLARE_COMPONENT(ActsMaterialJsonWriterTool)

DECLARE_COMPONENT(ActsTrackingGeometryTool)

DECLARE_COMPONENT(ActsPropStepRootWriterSvc)
DECLARE_COMPONENT(ActsCaloTrackingVolumeBuilder)
DECLARE_COMPONENT(ActsTrk::DetectorVolumeSvc)
DECLARE_COMPONENT(ActsTrk::SimpleCylinderDetBuilderTool)
DECLARE_COMPONENT(ActsTrk::ActsVolumeIdToDetectorElementCollectionMappingAlg)

DECLARE_COMPONENT(ActsTrk::ItkBlueprintNodeBuilder)

