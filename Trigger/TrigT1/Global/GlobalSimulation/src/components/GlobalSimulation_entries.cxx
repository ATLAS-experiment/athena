/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "../GlobalSimComponents/GlobalSimulationAlg.h"

#include "../FirstChain/Egamma1_LArStrip_Fex.h"
#include "../FirstChain/Egamma1_LArStrip_Fex_RowAware.h"
#include "../FirstChain/EMB1CellsFromCaloCells.h"
#include "../FirstChain/eFexRoIAlgTool.h"
#include "../FirstChain/Egamma1BDTAlgTool.h"
#include "../FirstChain/Egamma1eRatioAlgTool.h"

#include "../FirstChain/LArCellPreparationAlg.h"
#include "../FirstChain/LArCellMuxAlg.h"
#include "../FirstChain/GlobalCellTowerAlgTool.h"
#include "../FirstChain/eFexCvtrAlgTool.h"
#include "../FirstChain/eEmMultAlgTool.h"

#include "../FirstChain/eEmMultTestBench.h"
#include "../FirstChain/eEmMultTestComparator.h"


DECLARE_COMPONENT(GlobalSim::GlobalSimulationAlg)

DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex)
DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex_RowAware)
DECLARE_COMPONENT(GlobalSim::EMB1CellsFromCaloCells)
DECLARE_COMPONENT(GlobalSim::eFexRoIAlgTool)
DECLARE_COMPONENT(GlobalSim::Egamma1BDTAlgTool)
DECLARE_COMPONENT(GlobalSim::Egamma1eRatioAlgTool)

DECLARE_COMPONENT(GlobalSim::LArCellPreparationAlg)
DECLARE_COMPONENT(GlobalSim::LArCellMuxAlg)
DECLARE_COMPONENT(GlobalSim::GlobalCellTowerAlgTool)

DECLARE_COMPONENT(GlobalSim::eFexCvtrAlgTool)
DECLARE_COMPONENT(GlobalSim::eEmMultAlgTool)

DECLARE_COMPONENT(GlobalSim::eEmMultTestBench)
DECLARE_COMPONENT(GlobalSim::eEmMultTestComparator)
