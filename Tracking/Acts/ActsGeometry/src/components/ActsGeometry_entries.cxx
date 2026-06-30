/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EventPrimitives/EventPrimitives.h"
// needed here to get the ATLAS eigen plugins in before the ACTS eigen plugins
#include "ActsGeometry/ActsCaloTrackingVolumeBuilder.h"
#include "ActsGeometry/ActsExtrapolationAlg.h"
#include "ActsGeometry/ExtrapolationTool.h"
#include "ActsGeometry/ActsPropStepRootWriterSvc.h"


#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"

#include "../ActsVolumeIdToDetectorElementCollectionMappingAlg.h"
#include "../ItkBlueprintNodeBuilder.h"
#include "../CaloBlueprintNodeBuilder.h"
#include "../ITkMaterialDecoratorTool.h"
#include "../WriteTrackingGeometry.h"
#include "../TrackingGeometryTool.h"
#include "../TrackingGeometrySvc.h"


DECLARE_COMPONENT(ActsExtrapolationAlg)
DECLARE_COMPONENT(ActsWriteTrackingGeometryTransforms)


DECLARE_COMPONENT(ActsPropStepRootWriterSvc)
DECLARE_COMPONENT(ActsCaloTrackingVolumeBuilder)
DECLARE_COMPONENT(ActsTrk::ActsVolumeIdToDetectorElementCollectionMappingAlg)

DECLARE_COMPONENT(ActsTrk::ItkBlueprintNodeBuilder)
DECLARE_COMPONENT(ActsTrk::ExtrapolationTool)
DECLARE_COMPONENT(ActsTrk::CaloBlueprintNodeBuilder)
DECLARE_COMPONENT(ActsTrk::ITkMaterialDecoratorTool)
DECLARE_COMPONENT(ActsTrk::WriteTrackingGeometry)
DECLARE_COMPONENT(ActsTrk::TrackingGeometrySvc)
DECLARE_COMPONENT(ActsTrk::TrackingGeometryTool)
