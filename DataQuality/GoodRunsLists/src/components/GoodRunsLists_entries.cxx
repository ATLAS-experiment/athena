#include "GoodRunsLists/GoodRunsListSelectionTool.h"
#include "GoodRunsLists/GRLSelectorAlg.h"

#ifndef XAOD_STANDALONE
#include "GoodRunsLists/GoodRunsListSelectorTool.h"
#include "GoodRunsLists/TriggerRegistryTool.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( GoodRunsListSelectionTool )
DECLARE_COMPONENT( GRLSelectorAlg )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( GoodRunsListSelectorTool )
DECLARE_COMPONENT( TriggerRegistryTool )
#endif