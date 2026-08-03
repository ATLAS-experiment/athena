/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../GlobalSimComponents/GlobalSimulationAlg.h"

#include "../Egamma1BDT/Egamma1BDTAlgTool.h"
#include "../Egamma1BDT/eRatioAlgTool_UCL.h"

#include "../FEX_Unpacker/eFexRoIAlgTool.h"
#include "../FEX_Unpacker/eFexCvtrAlgTool.h"
#include "../FEX_Unpacker/gFexRhoCvtrAlgTool.h"

#include "../Hypothesis/eEmMultAlgTool.h"
#include "../Hypothesis/eEmEg1BDTMultAlgTool.h"
#include "../Hypothesis/CommonMultAlgTool.h"
#include "../Hypothesis/eEmMultTestBench.h"
#include "../Hypothesis/eEmMultTestComparator.h"

#include "../Jet1/GlobalJet1AlgTool.h"

#include "../Lar_Preproc/Egamma1_LArStrip_Fex.h"
#include "../Lar_Preproc/Egamma1_LArStrip_Fex_RowAware.h"
#include "../Lar_Preproc/Egamma1_OnlineMapNbhood.h"
#include "../Lar_Preproc/EMBE1CellsFromCaloCells.h"
#include "../Lar_Preproc/LArCellPreparationAlg.h"
#include "../Lar_Preproc/LArCellMuxAlg.h"
#include "../Lar_Preproc/GlobalCellTowerAlgTool.h"

#include "../PU1/PU1SuppTestBench.h"
#include "../PU1/PU1SuppAlgTool.h"

#include "../GraphSvc.h"

DECLARE_COMPONENT(GlobalSim::GlobalSimulationAlg)

DECLARE_COMPONENT(GlobalSim::Egamma1BDTAlgTool)
DECLARE_COMPONENT(GlobalSim::eRatioAlgTool_UCL)

DECLARE_COMPONENT(GlobalSim::eFexRoIAlgTool)
DECLARE_COMPONENT(GlobalSim::eFexCvtrAlgTool)
DECLARE_COMPONENT(GlobalSim::gFexRhoCvtrAlgTool)

DECLARE_COMPONENT(GlobalSim::eEmMultAlgTool)
DECLARE_COMPONENT(GlobalSim::eEmEg1BDTMultAlgTool)
DECLARE_COMPONENT(GlobalSim::CommonMultAlgTool)
DECLARE_COMPONENT(GlobalSim::eEmMultTestBench)
DECLARE_COMPONENT(GlobalSim::eEmMultTestComparator)

DECLARE_COMPONENT(GlobalSim::GlobalJet1AlgTool)

DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex)
DECLARE_COMPONENT(GlobalSim::Egamma1_LArStrip_Fex_RowAware)
DECLARE_COMPONENT(GlobalSim::Egamma1_OnlineMapNbhood)
DECLARE_COMPONENT(GlobalSim::EMBE1CellsFromCaloCells)
DECLARE_COMPONENT(GlobalSim::LArCellPreparationAlg)
DECLARE_COMPONENT(GlobalSim::LArCellMuxAlg)
DECLARE_COMPONENT(GlobalSim::GlobalCellTowerAlgTool)

DECLARE_COMPONENT(GlobalSim::PU1SuppTestBenchAlg)
DECLARE_COMPONENT(GlobalSim::PU1SuppAlgTool)

DECLARE_COMPONENT(GlobalSim::GraphSvc)
