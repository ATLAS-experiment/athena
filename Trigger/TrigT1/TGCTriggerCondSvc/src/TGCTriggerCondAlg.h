/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCTRIGGERCONDALG_H
#define TGCTRIGGERCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "TGCTriggerCondSvc/TGCTriggerLUTs.h"
#include "TGCTriggerCondSvc/TGCTriggerBWCWReader.h"

  class TGCTriggerCondAlg: public AthCondAlgorithm {

  public:
    using AthCondAlgorithm::AthCondAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey_bw{this, "ReadKeyBw", "/TGC/TRIGGER/CW_BW_RUN3"};
    SG::WriteCondHandleKey<TGCTriggerLUTs> m_writeKey{this, "WriteKey", "TGCTriggerLUTs"};

    // LUT Readers 
    TGCTriggerBWCWReader m_bwCWReader{0};
  };

#endif


