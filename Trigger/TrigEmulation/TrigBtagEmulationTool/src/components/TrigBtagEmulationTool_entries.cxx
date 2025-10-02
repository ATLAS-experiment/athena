/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrigBtagEmulationTool.h"
#include "src/JetManagerTool.h"

#ifndef XAOD_STANDALONE
#include "src/TrigBtagValidationTest.h"
#endif

#include "AsgTools/AsgComponentFactories.h"


DECLARE_COMPONENT(Trig::TrigBtagEmulationTool)
DECLARE_COMPONENT(Trig::JetManagerTool)

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT(Trig::TrigBtagValidationTest)
#endif
