/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// This source file intentionally contains only header includes. 
// It exists so that ActsGPUEvent can be built as a SHARED library, 
// enabling atlas_generate_cliddb(ActsGPUEventLib) to generate CLID definitions.
// See: https://atlas-software.docs.cern.ch/athena/developers/cmake/#atlas_generate_cliddb

#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccSeedCollection.h"
#include "ActsGPUEvent/TracccTrkParamCollection.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "ActsGPUEvent/TracccMagField.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"