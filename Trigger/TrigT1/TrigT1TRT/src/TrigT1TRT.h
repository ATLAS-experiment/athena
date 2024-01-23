/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIG_T1_TRT_H
#define TRIG_T1_TRT_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Outputs to CTP
#include "StoreGate/WriteHandleKey.h"
#include "TrigT1Interfaces/TrtCTP.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"

// Input containers
#include "StoreGate/ReadHandleKey.h"
#include "InDetRawData/TRT_RDO_Container.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

// Handles
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"
#include "TRT_ConditionsServices/ITRT_StrawNeighbourSvc.h"

namespace LVL1 {
  /** @brief level 1 TRT trigger simulation */
  class TrigT1TRT : public AthReentrantAlgorithm {

  public:
    // This is a standard algorithm constructor
    TrigT1TRT(const std::string& name, ISvcLocator* pSvcLocator);

    // These are the functions inherited from Algorithm
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    
  private:
    /* Output handles */
    SG::WriteHandleKey<TrtCTP> m_trtCTPLocation{this, "TrtCTPLocation", LVL1::DEFAULT_TrtCTPLocation, "Write handle key for TrtCTP"};

    /* Input handles */
    SG::ReadHandleKey<TRT_RDO_Container> m_trtRDOKey{this, "TrtRDOLocation", "TRT_RDOs", "Read handle key for TRT_RDO_Container"};

    /* Tool handles */
    ToolHandle<ITRT_StrawStatusSummaryTool> m_ConditionsSummary;

    /* Service handles */
    ServiceHandle<ITRT_StrawNeighbourSvc> m_TRTStrawNeighbourSvc;

    /* RDO hit containers */
    const InDetDD::TRT_DetectorManager *m_mgr;
    const TRT_ID* m_pTRTHelper;

    /* Gaudi Properties */
    Gaudi::Property<int> m_TTCMultiplicity{this, "TTCMultiplicity", 5, "TTC board multiplicity required to fire the trigger"};

    /* Variables and functions used in trigger logic */
    int BarrelChipToBoard(int chip) const;
    int EndcapChipToBoard(int chip) const;
    int EndcapStrawNumber(int strawNumber, int strawLayerNumber, int LayerNumber, int phi_stack, int side) const;
    int BarrelStrawNumber(int strawNumber, int strawlayerNumber, int LayerNumber) const;
    int BarrelStrawLayerNumber(int strawLayerNumber, int LayerNumber) const;

    unsigned char m_mat_chip_barrel[64][1642];
    unsigned char m_mat_chip_endcap[64][3840];

    int m_numberOfStraws[75];
  };
}

#endif
