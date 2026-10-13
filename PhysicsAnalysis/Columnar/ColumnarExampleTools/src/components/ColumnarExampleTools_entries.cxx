/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

// Local include(s).
#include <ColumnarExampleTools/ConfigurableColumnExampleTool.h>
#include <ColumnarExampleTools/LinkColumnExampleTool.h>
#include <ColumnarExampleTools/ModularExampleTool.h>
#include <ColumnarExampleTools/MomentumAccessorExampleTool.h>
#include <ColumnarExampleTools/OptionalColumnExampleTool.h>
#include <ColumnarExampleTools/SimpleSelectorExampleTool.h>
#include <ColumnarExampleTools/StringExampleTool.h>
#include <ColumnarExampleTools/VariantExampleTool.h>
#include <ColumnarExampleTools/VectorExampleTool.h>

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT (ADD_CMODE(columnar)::ConfigurableColumnExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::LinkColumnExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::ModularExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::MomentumAccessorExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::OptionalColumnExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::SimpleSelectorExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::StringExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::VariantExampleTool)
DECLARE_COMPONENT (ADD_CMODE(columnar)::VectorExampleTool)
