/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "../GlobalSimulationAlg.h"
#include "../GlobalAlgs/Hypothesis/UCL/HypoTestBenchAlg.h"
#include "../GlobalAlgs/Hypothesis/UCL/InvMassDPhiInc2TestBenchAlg.h"

#include "../GlobalAlgs/FirstChain/Egamma1_LArStrip_Fex.h"
#include "../GlobalAlgs/FirstChain/Egamma1_LArStrip_Fex_RowAware.h"
#include "../GlobalAlgs/FirstChain/EMB1CellsFromCaloCells.h"
#include "../GlobalAlgs/FirstChain/eFexRoIAlgTool.h"
#include "../GlobalAlgs/ERatioAlgTool.h"
#include "../GlobalAlgs/FirstChain/Egamma1BDTAlgTool.h"
#include "../GlobalAlgs/FirstChain/Egamma1eRatioAlgTool.h"

#include "../GlobalAlgs/Hypothesis/UCL/eEmSortSelectCountContainerAlgTool.h"
#include "../GlobalAlgs/Hypothesis/UCL/eEmSortSelectCountContainerComparator.h"

#include "../GlobalAlgs/FirstChain/LArCellPreparationAlg.h"
#include "../GlobalAlgs/FirstChain/LArCellMuxAlg.h"
#include "../GlobalAlgs/FirstChain/GlobalCellTowerAlgTool.h"
#include "../GlobalAlgs/FirstChain/eFexCvtrAlgTool.h"
#include "../GlobalAlgs/FirstChain/eEmMultAlgTool.h"


DECLARE_COMPONENT(GlobalSim::GlobalSimulationAlg)
DECLARE_COMPONENT(GlobalSim::HypoTestBenchAlg)
DECLARE_COMPONENT(GlobalSim::InvMassDPhiInc2TestBenchAlg)


DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex)
DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex_RowAware)
DECLARE_COMPONENT(GlobalSim::EMB1CellsFromCaloCells)
DECLARE_COMPONENT(GlobalSim::eFexRoIAlgTool)
DECLARE_COMPONENT(GlobalSim::ERatioAlgTool)
DECLARE_COMPONENT(GlobalSim::Egamma1BDTAlgTool)
DECLARE_COMPONENT(GlobalSim::Egamma1eRatioAlgTool)

DECLARE_COMPONENT(GlobalSim::eEmSortSelectCountContainerAlgTool)
DECLARE_COMPONENT(GlobalSim::eEmSortSelectCountContainerComparator)

DECLARE_COMPONENT(GlobalSim::LArCellPreparationAlg)
DECLARE_COMPONENT(GlobalSim::LArCellMuxAlg)
DECLARE_COMPONENT(GlobalSim::GlobalCellTowerAlgTool)

DECLARE_COMPONENT(GlobalSim::eFexCvtrAlgTool)
DECLARE_COMPONENT(GlobalSim::eEmMultAlgTool)
