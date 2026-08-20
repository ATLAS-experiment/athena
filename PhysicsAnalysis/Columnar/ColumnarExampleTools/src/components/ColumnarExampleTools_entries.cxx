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

#define DECLARE_COLUMNAR_COMPONENT(name) DECLARE_COMPONENT(name)

DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::ConfigurableColumnExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::LinkColumnExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::ModularExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::MomentumAccessorExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::OptionalColumnExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::SimpleSelectorExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::StringExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::VariantExampleTool)
DECLARE_COLUMNAR_COMPONENT (ADD_CMODE(columnar)::VectorExampleTool)
