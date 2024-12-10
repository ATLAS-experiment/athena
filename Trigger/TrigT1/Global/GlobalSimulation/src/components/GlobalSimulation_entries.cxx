/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "../GlobalSimulationAlg.h"

#include "../L1TopoAlgs/cTauMultiplicityAlgTool.h"
#include "../L1TopoAlgs/EnergyThresholdAlgTool_jXE.h"
#include "../L1TopoAlgs/eEmSelectAlgTool.h"
#include "../L1TopoAlgs/jJetSelectAlgTool.h"
#include "../L1TopoAlgs/DeltaRSqrIncl2AlgTool.h"
#include "../L1TopoAlgs/SimpleConeAlgTool.h"

#include "../L1TopoAlgs/jJetInputAlgTool.h"
#include "../L1TopoAlgs/eEmInputAlgTool.h"
#include "../L1TopoAlgs/cTauInputAlgTool.h"
#include "../L1TopoAlgs/jXEInputAlgTool.h"

#include "../GlobalAlgs/Egamma1_LArStrip_Fex.h"
#include "../GlobalAlgs/EMB1CellsFromCaloCells.h"
#include "../GlobalAlgs/eFexRoIAlgTool.h"
#include "../GlobalAlgs/ERatioAlgTool.h"
#include "../GlobalAlgs/Egamma1BDTAlgTool.h"

DECLARE_COMPONENT(GlobalSim::GlobalSimulationAlg)

DECLARE_COMPONENT(GlobalSim::cTauMultiplicityAlgTool)
DECLARE_COMPONENT(GlobalSim::EnergyThresholdAlgTool_jXE)
DECLARE_COMPONENT(GlobalSim::eEmSelectAlgTool)
DECLARE_COMPONENT(GlobalSim::jJetSelectAlgTool)
DECLARE_COMPONENT(GlobalSim::DeltaRSqrIncl2AlgTool)
DECLARE_COMPONENT(GlobalSim::SimpleConeAlgTool)

DECLARE_COMPONENT(GlobalSim::jJetInputAlgTool)
DECLARE_COMPONENT(GlobalSim::eEmInputAlgTool)
DECLARE_COMPONENT(GlobalSim::cTauInputAlgTool)
DECLARE_COMPONENT(GlobalSim::jXEInputAlgTool)

DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex)
DECLARE_COMPONENT(GlobalSim::EMB1CellsFromCaloCells)
DECLARE_COMPONENT(GlobalSim::eFexRoIAlgTool)
DECLARE_COMPONENT(GlobalSim::ERatioAlgTool)
DECLARE_COMPONENT(GlobalSim::Egamma1BDTAlgTool)
